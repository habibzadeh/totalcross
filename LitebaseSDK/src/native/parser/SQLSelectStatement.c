/*********************************************************************************
 *  TotalCross Software Development Kit - Litebase                               *
 *  Copyright (C) 2000-2011 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *********************************************************************************/

/**
 * Defines the functions to initialize, set, and process a select statement.
 */

#include "SQLSelectStatement.h"

/**
 * Creates and initializes a SQL select statement.
 *
 * @param parser The structure returned from the parsing process.
 * @param isPrepared Indicates if the delete statement is from a prepared statement.
 * @return A pointer to a <code>SQLSelectStatement</code> structure. 
 */
SQLSelectStatement* initSQLSelectStatement(LitebaseParser* parser, bool isPrepared)
{
	TRACE("initSQLSelectStatement")
	Heap heap = parser->heap;
   SQLSelectStatement* selectStmt = (SQLSelectStatement*)TC_heapAlloc(heap, sizeof(SQLSelectStatement));
	SQLBooleanClause* whereClause = selectStmt->whereClause = parser->whereClause; // Sets the where clause.
	SQLBooleanClause* havingClause = selectStmt->havingClause = parser->havingClause; // Sets the having clause.
	SQLColumnListClause* listClause;
	SQLSelectClause* selectClause;
	int32 count;

	selectStmt->type = CMD_SELECT; // Sets the type of statement.
	parser->select.heap = heap;
   
	// Sets the select clause, its field list, and its hash table.
	parser->select.htName2index = TC_htNew(MAXIMUMS << 1, heap);
   selectClause = selectStmt->selectClause = (SQLSelectClause*)TC_heapAlloc(heap, sizeof(SQLSelectClause));
	xmemmove(selectClause, &parser->select, sizeof(SQLSelectClause));
	selectClause->fieldList = (SQLResultSetField**)TC_heapAlloc(heap, count = ((selectClause->fieldsCount? selectClause->fieldsCount : MAXIMUMS) << 2));
	xmemmove(selectClause->fieldList, parser->selectFieldList, count);
   selectClause->tableList = (SQLResultSetTable**)TC_heapAlloc(heap, count = (parser->tableListSize << 2));
	xmemmove(selectClause->tableList, parser->tableList, count);

   if (isPrepared) // It is only necessary to re-allocate the parser structures if the statement is from a prepared statement.
	{
      if (parser->group_by.fieldsCount) // Sets the group by clause.
		{
			listClause = selectStmt->groupByClause = (SQLColumnListClause*)TC_heapAlloc(heap, sizeof(SQLColumnListClause));
			xmemmove(listClause, &parser->group_by, sizeof(SQLColumnListClause));
			listClause->fieldList = (SQLResultSetField**)TC_heapAlloc(heap, count = (parser->group_by.fieldsCount << 2));
		   xmemmove(listClause->fieldList, parser->groupByfieldList, count);
		}

		if (parser->order_by.fieldsCount) // Sets the order by clause.
		{
			listClause = selectStmt->orderByClause = (SQLColumnListClause*)TC_heapAlloc(heap, sizeof(SQLColumnListClause));
			xmemmove(listClause, &parser->order_by, sizeof(SQLColumnListClause));
		   listClause->fieldList = (SQLResultSetField**)TC_heapAlloc(heap, count = (parser->order_by.fieldsCount << 2));
		   xmemmove(listClause->fieldList, parser->orderByfieldList, count);
		}

		if (whereClause)
		{
         whereClause->fieldList = (SQLResultSetField**)TC_heapAlloc(heap, count = (whereClause->fieldsCount << 2));
			xmemmove(whereClause->fieldList, parser->whereFieldList, count);
			whereClause->paramList = (SQLBooleanClauseTree**)TC_heapAlloc(heap, count = (whereClause->paramCount << 2));
			xmemmove(whereClause->paramList, parser->whereParamList, count);
		}

		if (havingClause)
		{
         havingClause->fieldList = (SQLResultSetField**)TC_heapAlloc(heap, count = (havingClause->fieldsCount << 2));
			xmemmove(havingClause->fieldList, parser->havingFieldList, count);
			havingClause->paramList = (SQLBooleanClauseTree**)TC_heapAlloc(heap, count = (havingClause->paramCount << 2));
			xmemmove(havingClause->paramList, parser->havingParamList, count);
		}
	}
	else
	{
		if (parser->group_by.fieldsCount) // Sets the group by clause.
		{
         selectStmt->groupByClause = &parser->group_by;
			selectStmt->groupByClause->fieldList = parser->groupByfieldList;
		}
		if (parser->order_by.fieldsCount) // Sets the order by clause.
		{
         selectStmt->orderByClause = &parser->order_by;
         selectStmt->orderByClause->fieldList = parser->orderByfieldList;
		}
   
		if (whereClause)
		{
			whereClause->fieldList = parser->whereFieldList;
			whereClause->paramList = parser->whereParamList;
		}
		if (havingClause)
		{
			havingClause->fieldList = parser->havingFieldList;
			havingClause->paramList = parser->havingParamList;
		}	
	}
	return selectStmt;
}

/* 
 * Sets the value of a numeric parameter at the given index.
 *
 * @param context The thread context where the function is being executed.
 * @param selectStmt A SQL select statement.
 * @param index The index of the parameter.
 * @param value The value of the parameter.
 * @param type The type of the parameter.
 * @thows DriverException If the parameter index is invalid.
 */
void setNumericParamValueSel(Context context, SQLSelectStatement* selectStmt, int32 index, VoidP value, int32 type)
{
	TRACE("setNumericParamValueSel")
	SQLBooleanClause* whereClause = selectStmt->whereClause;
	SQLBooleanClause* havingClause = selectStmt->havingClause;
		
	// Gets the parameter count and checks if the index is within the range.
	int32 whereParamCount = whereClause? whereClause->paramCount : 0;

	if (index < 0 || index >= (whereParamCount + (havingClause? havingClause->paramCount : 0)))
      TC_throwExceptionNamed(context, "litebase.DriverException", getMessage(ERR_INVALID_PARAMETER_INDEX), index);
   else
   if (index < whereParamCount) // Sets the parameter value in its proper place.
      setNumericParamValue(context, whereClause->paramList[index], value, type);
   else
      setNumericParamValue(context, havingClause->paramList[index - whereParamCount], value, type);
}

/* 
 * Sets the value of a string parameter at the given index.
 *
 * @param context The thread context where the function is being executed.
 * @param selectStmt A SQL select statement.
 * @param index The index of the parameter.
 * @param value The value of the parameter.
 * @param length The length of the string.
 * @throws SQLParserException If a <code>null</code> is used as a parameter of a where clause.
 * @thows DriverException If the parameter index is invalid.
 * @return <code>false</code> if an error occurs; <code>true</code>, otherwise.
 */
bool setParamValueStringSel(Context context, SQLSelectStatement* selectStmt, int32 index, JCharP value, int32 length)
{
	TRACE("setParamValueStringSel")
	SQLBooleanClause* whereClause = selectStmt->whereClause;
	SQLBooleanClause* havingClause = selectStmt->havingClause;
		
	// Gets the parameter count and checks if the index is within the range.
	int32 whereParamCount = whereClause? whereClause->paramCount : 0;

	if (index < 0 || index >= (whereParamCount + (havingClause? havingClause->paramCount : 0)))
   {
      TC_throwExceptionNamed(context, "litebase.DriverException", getMessage(ERR_INVALID_PARAMETER_INDEX), index);
      return false;
   }
   else if (!value)
   {
      TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_PARAM_NULL)); 
      return false;
   }
   else if (index < whereParamCount) // Sets the parameter value in its proper place.
      return setParamValueString(context, whereClause->paramList[index], value, length);
   else
      return setParamValueString(context, havingClause->paramList[index - whereParamCount], value, length);
}

/**
 * Clears all parameter values of a prepared statement select.
 *
 * @param selectStmt A SQL select statement.
 */
void clearParamValuesSel(SQLSelectStatement* selectStmt)
{
	TRACE("clearParamValuesSel")
   int32 i;
   SQLBooleanClause* clause;
	SQLBooleanClauseTree** paramList;
	if ((clause = selectStmt->whereClause)) // Clears all the parameters of the where clause.
	{
		i = clause->paramCount;
		paramList = clause->paramList;
      while (--i >= 0)
         paramList[i]->isParamValueDefined = false;
	}

   if ((clause = selectStmt->havingClause)) // Clears all the parameters of the having clause.
	{
		i = clause->paramCount;
		paramList = clause->paramList;
      while (--i >= 0)
         paramList[i]->isParamValueDefined = false;
	}
}

/**
 * Checks if all parameters values are defined.
 *
 * @param selectStmt A SQL select statement.
 * @return <code>true</code>, if all parameters values are defined; <code>false</code> otherwise.
 */
bool allParamValuesDefinedSel(SQLSelectStatement* selectStmt)
{
	TRACE("allParamValuesDefinedSel")
   int32 i;
   SQLBooleanClause* clause;
	SQLBooleanClauseTree** paramList;
   if ((clause = selectStmt->whereClause)) // Checks if all the where clause parameters are defined.
	{
		i = clause->paramCount;
		paramList = clause->paramList;
      while (--i >= 0)
         if (!paramList[i]->isParamValueDefined)
            return false;
	}

   if ((clause = selectStmt->havingClause)) // Checks if all the having clause parameters are defined.
	{
		i = clause->paramCount;
		paramList = clause->paramList;
      while (--i >= 0)
         if (!paramList[i]->isParamValueDefined)
            return false;
	}

   return true;
}

/**
 * Executes a select statement.
 *
 * @param context The thread context where the function is being executed.
 * @param driver The connection with Litebase.
 * @param selectStmt A SQL select statement.
 * @return A result set returned by the query execution.
 * @throws DriverException If the record can't be removed from the indices.
 * @throws OutOfMemoryError If a heap memory allocation fails.
 */
Object litebaseDoSelect(Context context, Object driver, SQLSelectStatement* selectStmt)
{
	TRACE("litebaseDoSelect")
   ResultSet* bag;
   SQLSelectClause* selectClause = selectStmt->selectClause;
   SQLResultSetTable** tableList = selectClause->tableList;
   Table* rsBaseTable;
	bool isSimpleSelect = false;
	Object resultSet = null;
   Heap heap;

	int32 i = selectClause->tableListSize;
	while (--i >= 0)
	{
		if (!tableList[i]->table)
		{
			TC_throwExceptionNamed(context, "litebase.DriverException", getMessage(ERR_CANT_READ), tableList[i]->tableName);
			return null;
		}
	}

   // juliana@230_14: removed temporary tables when there is no join, group by, order by, and aggregation.
	// juliana@210_1: select * from table_name does not create a temporary table anymore.
	if (!selectStmt->groupByClause && !selectStmt->havingClause && !selectStmt->orderByClause && !selectStmt->whereClause
	 && selectClause->tableListSize == 1 && selectClause->hasWildcard)
	{
		isSimpleSelect = true;
		rsBaseTable = (*tableList)->table;
      rsBaseTable->answerCount = -1;
	}
	else // juliana@212_4: if the select fields are in the table order beginning with rowid, do not build a temporary table. 
	if (!selectStmt->groupByClause && !selectStmt->havingClause && !selectStmt->orderByClause && !selectStmt->whereClause 
	  && selectClause->tableListSize == 1 
	  && isCorrectOrder(selectClause->fieldList, (*tableList)->table->columnNames, selectClause->fieldsCount, (*tableList)->table->columnCount))
   {
      rsBaseTable = (*tableList)->table;
      rsBaseTable->answerCount = -1;
   }
	else // Generates the result set and stores it in a temporary table.
	{
		SQLResultSetField** fieldList = selectClause->fieldList;
		int32 fieldListLen = selectClause->fieldsCount;
		if (!(rsBaseTable = generateResultSetTable(context, driver, selectStmt))) // Temporary table.
			return null;

      if (!*rsBaseTable->name)
      {
         // Remaps the table column names to use the aliases of the select statement instead of the original column names.
		   // Releases the unused memory (rowinc is 100 by default for result sets). This must be used only for temporary tables.
         if (!remapColumnsNames2Aliases(context, rsBaseTable, fieldList, fieldListLen)
          || !plainShrinkToSize(context, rsBaseTable->db)) // guich@201_9: always shrink the .db and .dbo memory files.
		   {
			   freeTable(context, rsBaseTable, false, true);
			   return null;
		   }
	   }
   }

   heap = heapCreate();
   IF_HEAP_ERROR(heap)
   {
      heapDestroy(heap);
      if (!*rsBaseTable->name) // juliana@223_14: solved possible memory problems.
         freeTable(context, rsBaseTable, false, true);
      TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
      return null;
   }

	// Creates the result set without passing any fields or WHERE clause, since all the records and all the fields in the temporary table are part 
   // of the result set.
	bag = createResultSetForSelect(context, rsBaseTable, null, heap);
   bag->intHashtable = selectClause->htName2index;
	bag->selectClause = selectClause;
	bag->isSimpleSelect = isSimpleSelect; // juliana@210_1: select * from table_name does not create a temporary table anymore.
   bag->driver = driver; // juliana@227_2

   // juliana@223_13: corrected a bug that could break the application when freeing a result set of a prepared statement.
   bag->isPrepared = selectClause->isPrepared;

   if (rsBaseTable->answerCount >= 0)
   {
      bag->answerCount = rsBaseTable->answerCount;
      bag->allRowsBitmap = rsBaseTable->allRowsBitmap;
   }

   if ((resultSet = TC_createObject(context, "litebase.ResultSet")))
	{
      OBJ_ResultSetBag(resultSet) = (int64)bag;
      return resultSet;
   }

	freeResultSet(bag);
	return null;
}

/**
 * Binds a select statement.
 *
 * @param context The thread context where the function is being executed.
 * @param driver The Litebase connection.
 * @param selectStmt A SQL select statement.
 * @return <code>true</code>, if the statement was bound successfully; <code>false</code> otherwise.
 */
bool litebaseBindSelectStatement(Context context, Object driver, SQLSelectStatement* selectStmt)
{
	TRACE("litebaseBindSelectStatement")
   SQLResultSetTable** tableList = selectStmt->selectClause->tableList;
   int32 i = selectStmt->selectClause->tableListSize;
	Table *table;
  
   while (--i >= 0) // Gets all tables from the select statement.
   {
      if (!(table = tableList[i]->table = getTable(context, driver, tableList[i]->tableName)))
         return false;
   }

   if (!bindSelectStatement(context, selectStmt)) // Validates and binds the select statement to the table list.
      return false;
   orderTablesToJoin(selectStmt); // Finds the best table order for the join operation.
   if (!validateSelectStatement(context, selectStmt))
      return false; // Validates the select statement.
   return true;
}

/**
 * Tries to put as inner table a table that has an index used more often in the where clause, when the where clause has a comparison between 
 * fields from different tables. e.g.: <code>select * from table1, table2 where table1.field1 = table2.field2 </code> If only 
 * <code>table1.field1</code> has index, changes the select to: <code>select * from table2, table1 where table1.field1 = table2.field2</code>. 
 * If both tables has the same level of index using, sorts them by the row count.
 *
 * @param selectStmt A SQL select statement.
 */
void orderTablesToJoin(SQLSelectStatement* selectStmt)
{
	TRACE("orderTablesToJoin")
   SQLSelectClause* selectClause = selectStmt->selectClause;
   SQLResultSetTable** tableList = selectClause->tableList;
   SQLBooleanClause* whereClause = selectStmt->whereClause;
   SQLResultSetTable* rsTableAux1;
   SQLResultSetTable* rsTableAux2;
   Table* tableAux1;
	Table* tableAux2;
	SQLResultSetField** fieldList = whereClause? whereClause->fieldList : null;
	int32 size = selectClause->tableListSize,
			i = size, 
		   j,
			k,
			highest;
	uint8* startedIndex = (uint8*)TC_heapAlloc(selectClause->heap, size);
   uint8* changedTo = (uint8*)TC_heapAlloc(selectClause->heap, size);
  
	if (size == 1) // size == 1 is not a join,
      return; 

   // Starts the weight of the where clause expression tree.
   while (--i >= 0)
   {
      (*tableList)->table->weight = 0;
      startedIndex[i] = changedTo[i] = i;
   }
	if (whereClause)
		weightTheTree(whereClause->expressionTree);

   i = size;
	while (--i >= 0) // Reorders the tables according to the weight.
   {
      highest = -1;
      rsTableAux1 = tableList[j = i];
      while (--j >= 0)
      {
         rsTableAux2 = tableList[j];
         tableAux1 = rsTableAux1->table;
         tableAux2 = rsTableAux2->table;
         if (tableAux1->weight < tableAux2->weight 
			 || (!tableAux1->weight && !tableAux2->weight && tableAux1->db->rowCount > tableAux2->db->rowCount))
         {
            rsTableAux1 = rsTableAux2;
            highest = j;
         }
      }
      if (highest != -1) // Changes table order.
      {
         tableList[highest] = tableList[i];
         tableList[i] = rsTableAux1;
         changedTo[startedIndex[highest]] = i;
         changedTo[k = startedIndex[i]] = highest;
         startedIndex[i] = startedIndex[highest];
         startedIndex[highest] = k;
      }
   }
   if (whereClause) // Rearranges the indexRs of the where clause field list.
	{
		i = whereClause->fieldsCount;
		while (--i >= 0)
			fieldList[i]->indexRs = changedTo[fieldList[i]->indexRs];
	}
}

/**
 * Checks if the table column names is in the same order of the select field list names.
 * 
 * @param fieldList The select field list.
 * @param columnNames The column names list of the first table of the select.
 * @param fieldListLength the length of the select field list.
 * @param columnCount The number of columns of the first table of the select.
 * @return <code>true</code> if the order is the same; <code>false</code>, otherwise.
 */
bool isCorrectOrder(SQLResultSetField** fieldList, CharP* columnNames, int32 fieldListLength, int32 columnCount) // juliana@212_4
{
   if (fieldListLength != columnCount)
      return false;
   while (--fieldListLength >= 0)
		if (xstrcmp(fieldList[fieldListLength]->tableColName, columnNames[fieldListLength]))
         return false;   
   return true;
}

/**
 * Binds the SQLSelectStatement to the select clause tables.
 *
 * @param context The thread context where the function is being executed.
 * @param selectStmt A SQL select statement.
 * @return <code>true</code> if the statement could be corrected bound; <code>false</code>, otherwise.
 */
bool bindSelectStatement(Context context, SQLSelectStatement* selectStmt)
{
	TRACE("bindSelectStatement")
   
	// First thing to do is to bind the columns in all clauses.
   int32* columnTypes;
   SQLSelectClause* selectClause = selectStmt->selectClause;
   Hashtable* names2Index = &selectClause->htName2index;
	SQLResultSetTable** tableList = selectClause->tableList;
	int32 count = 0, 
			size = selectClause->tableListSize,
			i = size;
	Table* table;

   while (--i >= 0)
      count += tableList[i]->table->columnCount;
   columnTypes = (int32*)TC_heapAlloc(selectClause->heap, count << 2);

   count = 0;
	i = size;
	while (--i >= 0) // Adds the column types properties of all tables to a big array.
	{
		table = tableList[i]->table;
      xmemmove(&columnTypes[count], table->columnTypes, table->columnCount << 2);
		count += table->columnCount;
	}

   // Binds the SQL Clauses. Note: The HAVING clause will have a late binding.
   // Binds the select clause.
	// Binds the where clause if it exists.
	// Binds the group by clause if it exists.
	// Binds the order by clause if it exists.
	if (!bindColumnsSQLSelectClause(context, selectClause)
	 || (selectStmt->whereClause 
	  && !bindColumnsSQLBooleanClause(context, selectStmt->whereClause, names2Index, columnTypes, tableList, size, selectStmt->selectClause->heap))
    || (selectStmt->groupByClause && !bindColumnsSQLColumnListClause(context, selectStmt->groupByClause, names2Index, columnTypes, tableList, size))
    || (selectStmt->orderByClause && !bindColumnsSQLColumnListClause(context, selectStmt->orderByClause, names2Index, columnTypes, tableList, size)))
         return false; 
   return true;
}

/**
 * Validates the SQLSelectStatement.
 *
 * @param context The thread context where the function is being executed.
 * @param selectStmt The select statement to be validated.
 * @return <code>false</code> if a <code>SQLParseException</code> occurs; <code>true</code>, otherwise.
 * @throws SQLParseException If the order by and group by clauses do not match, if a query with group by is not well-formed, if there is a 
 * having clause without an aggregation, a field in the having clause is not in the select clause, there is no order by and there are aggregated 
 * functions mixed with real columns, or there is an aggregation with an order by clause and no group by clause.
 */
bool validateSelectStatement(Context context, SQLSelectStatement* selectStmt)
{
	TRACE("validateSelectStatement")
   SQLSelectClause* selectClause = selectStmt->selectClause;
	int32 selectCount = selectClause->fieldsCount,	
			i = selectCount;
   SQLColumnListClause* groupByClause = selectStmt->groupByClause;
   SQLColumnListClause* orderByClause = selectStmt->orderByClause;
	SQLBooleanClause* havingClause = selectStmt->havingClause;
   SQLResultSetField** selectFields = selectClause->fieldList;
	SQLResultSetField* field1;
   
	// Checks if the order by and group by clauses match.
   if (groupByClause && orderByClause && !sqlcolumnlistclauseEquals(groupByClause, orderByClause)) 
   {
      TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_ORDER_GROUPBY_MUST_MATCH), 0);
      return false;
   }

   if (groupByClause) // Validates the group by clause if it exists.
   {
      
      while (--i >= 0) // Checks if all fields referenced in the HAVING clause are listed as aliased aggregated functions in the SELECT clause.
      {
         field1 = selectFields[i];

         // For now, there is no support for queries with GROUP BY and virtual columns in the SELECT clause that are not aggregated functions.
         if (!field1->isAggregatedFunction && field1->isVirtual)
         {
            TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_VIRTUAL_COLUMN_ON_GROUPBY), 0);
            return false;
         }

			// Checks if every non-aggregated function field that is listed in the SELECT clause is listed in the GROUP BY clause.
         if (!field1->isAggregatedFunction && !sqlcolumnlistclauseContains(groupByClause, field1->tableColIndex))
         {
            TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_AGGREG_FUNCTION_ISNOT_ON_SELECT), 0);
            return false;
         }
      }

      if (havingClause) // Checks if all fields referenced in the HAVING clause are listed as aliased aggregated functions in the SELECT clause.
      {
         int32 havingFieldsCount = havingClause->fieldsCount,
				   j;
         SQLResultSetField** havingFields = havingClause->fieldList;
			SQLResultSetField* field2;
			bool found = false;
         i = havingFieldsCount; 
         while (--i >= 0)
         {
            field1 = havingFields[i];
            j = selectCount;
            while (--j >= 0)
            {
               if (field1->aliasHashCode == (field2 = selectFields[j])->aliasHashCode)
               {
                  if (field2->isAggregatedFunction)
                  {
                     found = true;
                     break;
                  }

                  // It is not an aggregated function. Throws an exception.
                  TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_IS_NOT_AGGREG_FUNCTION), field1->alias);
                  return false;
               }
            }

            if (!found) // The having clause fields must be in the select clause fields.
            {
               TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_WAS_NOT_LISTED_ON_AGGREG_FUNCTION), field1->alias);
               return false;
            }
         }
      }
   }
   else 
   {
		// If there is no 'GROUP BY' clause, there can not be aggregated functions mixed with real columns in the SELECT clause.
      if (selectClause->hasRealColumns && selectClause->hasAggFunctions)
      {
         TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_CANNOT_MIX_AGGREG_FUNCTION), 0);
         return false;
      }

      // If there are aggregate functions with an ORDER BY clause with no GROUP BY clause, also raises an exception.
      if (selectClause->hasAggFunctions && orderByClause)
      {
         TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_CANNOT_HAVE_AGGREG_AND_NO_GROUPBY), 0);
         return false;
      }
   }
   return true;
}

/**
 * Generates a table to store the result set.
 *
 * @param context The thread context where the function is being executed.
 * @param driver The connection with Litebase.
 * @param selectStmt The select statement to be validated.
 * @return The temporary result set table or null if an error occurs.
 * @throws OutOfMemoryError If there is not enougth memory allocate memory. 
 */
Table* generateResultSetTable(Context context, Object driver, SQLSelectStatement* selectStmt)
{
	TRACE("generateResultSetTable")
	SQLSelectClause* selectClause = selectStmt->selectClause;
   int32 numTables = selectClause->tableListSize, 
		   numFields,
	      i, 
			j,
		   count = 0,
			totalRecords = 0,
			selectFieldsCount = selectClause->fieldsCount,
	      aggFunctionsColsCount = 0,
         groupCount,
		   numOfBytes;
	bool aggFunctionExist, 
		  writeDelayed, 
		  isTableTemporary,
	     countQueryWithWhere = false,
	     useIndex = true;
   SQLResultSetTable** tableList = selectClause->tableList;
   SQLBooleanClause* whereClause = selectStmt->whereClause;
   SQLColumnListClause* groupByClause = selectStmt->groupByClause;
	SQLColumnListClause* orderByClause = selectStmt->orderByClause;
	SQLColumnListClause* sortListClause;
   SQLBooleanClause* havingClause = selectStmt->havingClause;
   SQLResultSetField** fieldList = selectClause->fieldList;
	SQLResultSetField** groupList = groupByClause? groupByClause->fieldList : null;
   Table* tempTable1 = null;
	Table* tempTable2 = null;
	Table* tempTable3 = null; 
   CharP countAlias = null;
   IntVector columnTypes, 
		       columnHashes, 
				 columnSizes, 
				 columnIndexes, 
				 columnIndexesTables,
				 aggFunctionsParamCols = emptyIntVector;
   SQLResultSetField* field;
   SQLResultSetField* param;
   ResultSet* rsTemp = null;
   Hashtable colIndexesTable = emptyHashtable, 
		       aggFunctionsTable = emptyHashtable;
   int32* columnTypesItems1; 
   int32* columnSizesItems1;
   int32* columnSizesItems2;
	int32* columnHashesItems; 
	int32* origColumnTypesItems;
   int32* origColumnSizesItems;
	int32* groupCountCols; 
	int32* aggFunctionsRealParamCols = null;
	int32* aggFunctionsParamColsItems;
	int32* aggFunctionsCodes = null; 
	int32* paramCols = null;
   SQLValue* aggFunctionsRunTotals = null;
   SQLValue** prevRecord = null;
	SQLValue** curRecord;
	SQLValue** record1;
	SQLValue** record2;
   uint8* nullsPrevRecord; 
   uint8* nullsCurRecord;
	uint8* columnNulls0;
   ResultSet** listRsTemp; //rnovais@200_4
   Heap heap = null, 
		  heap_1 = null, 
		  heap_2 = null, 
		  heap_3 = null;

   if (numTables == 1) // The query is not a join.
      tempTable1 = (*tableList)->table;

   // juliana@226_13: corrected a bug where a query of the form "select year(field) as years from table" could be confunded with 
   // "select count(*) as years from table".
   // Optimization for queries that just wants to count the number of records of a table ("SELECT COUNT(*) FROM TABLE").
   if (selectFieldsCount == 1 && !groupByClause && (*fieldList)->sqlFunction == FUNCTION_AGG_COUNT && (*fieldList)->isAggregatedFunction)
   {
      countAlias = (*fieldList)->alias;

      if (!whereClause)
      {
         Table* table;

         totalRecords = 1; 
			i = numTables;
         while (--i >= 0)
         {
            table = tableList[i]->table;
            totalRecords *= (table->db->rowCount - table->deletedRowsCount);
         }
         return createIntValueTable(context, driver, totalRecords, countAlias);
      }
      else
         countQueryWithWhere = true;
      selectClause->type = COUNT_WITH_WHERE;
   }

   // Gather metadata for the temporary table that will store the result set.
   heap = heapCreate();
   IF_HEAP_ERROR(heap)
   {
      heapDestroy(heap);
		if (tempTable1 && !*tempTable1->name)
         freeTable(context, tempTable1, false, false);
		else
			heapDestroy(heap_1);
		if (tempTable2)
         freeTable(context, tempTable2, false, false);
		else
		heapDestroy(heap_2);
		if (tempTable3)
         freeTable(context, tempTable3, false, false);
		else
		   heapDestroy(heap_3);
      TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
      return null;
   }

   columnTypes = newIntVector(null, numFields = (orderByClause? selectFieldsCount + (int32)orderByClause->fieldsCount : selectFieldsCount), heap);
   columnHashes = newIntVector(null, numFields, heap);
   columnSizes = newIntVector(null, numFields, heap);
   columnIndexes = newIntVector(null, numFields, heap);
   columnIndexesTables = newIntVector(null, numFields, heap);
   colIndexesTable = TC_htNew(MAXIMUMS, heap);
   aggFunctionsTable = TC_htNew(MAXIMUMS, heap); // Maps the aggregated function parameter column indexes to the aggregate function code.

	i = -1;
   while (++i < selectFieldsCount)
   {
      IntVectorAdd(null, &columnSizes, (field = fieldList[i])->size);

      // Decides which hash code to use as the column name in the temp table and which data type to assign to the temporary table.
      if (field->isVirtual)
      {
         if (field->isAggregatedFunction)
         {   
            // Finds the fields that are parameter for a MAX() and MIN() function that can use an index.
            // juliana@230_21: MAX() and MIN() now use indices on simple queries.
            if (field->sqlFunction == FUNCTION_AGG_MAX || field->sqlFunction == FUNCTION_AGG_MIN)
               findMaxMinIndex(field);
               
            TC_htPut32(&aggFunctionsTable, i, field->sqlFunction);
         }

         if ((param = field->parameter))
         {
            IntVectorAdd(null, &columnTypes, param->dataType);
            IntVectorAdd(null, &columnHashes, param->tableColHashCode);
            IntVectorAdd(null, &columnIndexes, param->tableColIndex);
            IntVectorAdd(null, &columnIndexesTables, (int32)field->table);
            TC_htPut32(&colIndexesTable, param->tableColIndex, 0);
         }
         else // Uses the parameter hash and data type.
         {
            IntVectorAdd(null, &columnHashes, field->aliasHashCode); // Uses the alias hash code instead.
            IntVectorAdd(null, &columnIndexes, -1); // This is just a place holder, since this column does not map to any column in the database.
            IntVectorAdd(null, &columnIndexesTables, 0);
            IntVectorAdd(null, &columnTypes, field->dataType); // Uses the field data type.
         }
      }
      else // A real column was selected.
      {
         IntVectorAdd(null, &columnTypes, field->dataType);
         IntVectorAdd(null, &columnHashes, field->tableColHashCode);
         IntVectorAdd(null, &columnIndexes, field->tableColIndex);
         IntVectorAdd(null, &columnIndexesTables, (int32)field->table);
         TC_htPut32(&colIndexesTable, field->tableColIndex, 0);
      }
   }

	sortListClause = (orderByClause? orderByClause : groupByClause);

   if (sortListClause)
   {
      // Checks if all columns listed in the order by/group by clause were selected. If not, includes the ones that are missing.
      // It must be remembered that, if both are present, group by and order by must match. So, it does not matter which one is picked.
      count = sortListClause->fieldsCount;
      fieldList = sortListClause->fieldList;

		i = -1;
      while (++i < count)
      {
         if (!TC_htGet32Inv(&colIndexesTable, (field = fieldList[i])->tableColIndex))
            continue;

         // The sorting column is missing. Adds it to the temporary table.
         IntVectorAdd(null, &columnTypes, field->dataType);
         IntVectorAdd(null, &columnSizes, field->size);
         IntVectorAdd(null, &columnHashes, field->tableColHashCode);
         IntVectorAdd(null, &columnIndexesTables, (int32)field->table);
         IntVectorAdd(null, &columnIndexes, field->tableColIndex);
      }
   }

   // juliana@230_21: MAX() and MIN() now use indices on simple queries.
   // Creates the temporary table to store the records that satisfy the WHERE clause.
   // For optimization, the first temporary table will NOT be created, in case there is no WHERE clause and sort clause (either ORDER BY or GROUP BY)
   // and the SELECT clause contains aggregated functions. In this case, the calculation of the aggregated functions will be made on the existing 
	// table.
   if (!whereClause && !sortListClause && selectClause->hasAggFunctions && numTables == 1)
   {   
		// In this case, there is no need to create the temporary table. Just points the necessary structures of the original table.
      totalRecords = tempTable1->db->rowCount;
      
      // The index should not be used for MAX() and MIN() if not all the fields are MAX() and MIN() or one of the parametes cannot use an index.
      i = -1;
      while (++i < selectFieldsCount)
         if (!(field = fieldList[i])->isAggregatedFunction || field->index < 0 
          || (field->sqlFunction != FUNCTION_AGG_MAX && field->sqlFunction != FUNCTION_AGG_MIN)) 
         {
            useIndex = false;
            break;
         }
   }
	else
   {
      int32 type = -1;

      // Creates a result set from the table, using the current WHERE clause.
      if (!(listRsTemp = createListResultSetForSelect(context, tableList, numTables, whereClause, heap)))
      {
         heapDestroy(heap);
         return null;
      }

      rsTemp = *listRsTemp;
      
      // The index should not be used for MAX() and MIN() if there is a join, a sort or the indices do not resolve all the query.
      if (!sortListClause && (!whereClause || !whereClause->expressionTree) && numTables == 1)
      {
         // The index should not be used for MAX() and MIN() if not all the fields are MAX() and MIN() or one of the parametes cannot use an index.
         i = -1; 
         while (++i < selectFieldsCount)
            if (!(field = fieldList[i])->isAggregatedFunction || field->index < 0 
             || (field->sqlFunction != FUNCTION_AGG_MAX && field->sqlFunction != FUNCTION_AGG_MIN)) 
            {
               useIndex = false;
               break;
            }
      }
      else
         useIndex = false;

      // juliana@230_14: removed temporary tables when there is no join, group by, order by, and aggregation.
      if (sortListClause || (!useIndex && selectClause->hasAggFunctions)  || numTables != 1)
      {
         // Optimization for queries of type "SELECT COUNT(*) FROM TABLE WHERE..." Just counts the records of the result set and write it to a table.
         if (countQueryWithWhere && numTables == 1) 
         {
            if (!sqlBooleanClausePreVerify(context, whereClause))
            {
               heapDestroy(heap);
               return null;
            }
            rsTemp->pos = -1;
            while (getNextRecord(context, rsTemp, heap))
               totalRecords++;

            heapDestroy(heap);
			   return createIntValueTable(context, driver, totalRecords, countAlias);
         }

		   heap_1 = heapCreate();
         IF_HEAP_ERROR(heap_1)
         {
			   heapDestroy(heap);
			   if (tempTable1 && !*tempTable1->name)
				   freeTable(context, tempTable1, false, false);
			   else
				   heapDestroy(heap_1);
			   if (tempTable2)
				   freeTable(context, tempTable2, false, false);
			   else
				   heapDestroy(heap_2);
			   if (tempTable3)
				   freeTable(context, tempTable3, false, false);
			   else
				   heapDestroy(heap_3);
			   TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
			   return null;
         }

		   if (whereClause) 
			   type = whereClause->type;

         // Creates a temporary table to store the result set records and writes the result set records to the temporary table..
         if ((tempTable1 = driverCreateTable(context, driver, null, null, intVector2Array(&columnHashes, heap_1), intVector2Array(&columnTypes, heap_1), 
			                        intVector2Array(&columnSizes, heap_1), null, null, NO_PRIMARY_KEY, NO_PRIMARY_KEY, null, 0, columnSizes.size, heap_1)))
         {
            // guich@201_7: if a single table is being all loaded, sets rowInc to the table's size.
            if (!whereClause && !groupByClause && !havingClause && numTables == 1)
            {
               Table* table = (*tableList)->table;
               tempTable1->db->rowInc = table->db->rowCount - table->deletedRowsCount; 
            }

            IF_HEAP_ERROR(heap_1) // juliana@223_14: solved possible memory problems.
            {
			      heapDestroy(heap);
			      if (tempTable1 && !*tempTable1->name)
				      freeTable(context, tempTable1, false, false);
			      else
				      heapDestroy(heap_1);
			      if (tempTable2)
				      freeTable(context, tempTable2, false, false);
			      else
				      heapDestroy(heap_2);
			      if (tempTable3)
				      freeTable(context, tempTable3, false, false);
			      else
				      heapDestroy(heap_3);
			      TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
			      return null;
            }
            totalRecords = writeResultSetToTable(context, listRsTemp, numTables, tempTable1, &columnIndexes, selectClause, &columnIndexesTables, type, heap); 
         }
         else // guich@570_97
         {
            heapDestroy(heap); // juliana@223_14: solved possible memory problems.
            return null;
         }

         if (totalRecords <= 0) // No records retrieved. Exit.
         {
            if (totalRecords < 0)
            {
               heapDestroy(heap);
				   freeTable(context, tempTable1, false, false);
				   return null;
            }
            heapDestroy(heap);
            return tempTable1;
         }
         if (selectClause->type == COUNT_WITH_WHERE)
         {
			   heapDestroy(heap);
            freeTable(context, tempTable1, 0, false);
			   return createIntValueTable(context, driver, totalRecords, countAlias);
         }
      }
      else if (!useIndex) // A query that use index for MAX() and MIN() should not check now which rows are answered.
      {
         uint8* allRowsBitmap = tempTable1->allRowsBitmap;
         int32 newLength = (tempTable1->db->rowCount + 7) >> 3,
               oldLength = allRowsBitmap? tempTable1->allRowsBitmapLength : -1;
         
         if (newLength > oldLength)
         {
            if (!(tempTable1->allRowsBitmap = allRowsBitmap = xrealloc(allRowsBitmap, tempTable1->allRowsBitmapLength = newLength)))
            {
               heapDestroy(heap);
               TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
               return null;
            }
         }
         else
            xmemzero(allRowsBitmap, oldLength);
         computeAnswer(context, rsTemp, heap);
         
         heapDestroy(heap);
         return tempTable1;
      }
   }
   
   if (sortListClause && !sortTable(context, tempTable1, groupByClause, orderByClause)) // Sorts the temporary table, if required.
   {
      heapDestroy(heap);
      freeTable(context, tempTable1, 0, false); // juliana@223_14: solved possible memory problems.
      return null;
   }

   // There is still one new temporary table to be created, if the select clause has aggregate functions or here is a group by clause.
   if (!selectClause->hasAggFunctions && !groupByClause)
   {
      heapDestroy(heap);
      return tempTable1;
   }

   count = tempTable1->columnCount;

   // When creating the new temporary table, removes the extra fields that were created to perform the sorting.
   if (sortListClause && count != selectFieldsCount)
   {
      columnTypes.size -= (i = count - selectFieldsCount);
      columnSizes.size -= i;
      columnHashes.size -= i;
      columnIndexes.size -= i;
   }

   // Also updates the types and hashcodes to reflect the types and aliases of the
   // final temporary table, since they may still reflect the aggregated functions paramList types and hashcodes.
   columnTypesItems1 = columnTypes.items;
   columnSizesItems1 = columnSizes.items;
   columnHashesItems = columnHashes.items;

   // First preserves the original types, since they will be needed in the aggregated functions running totals calculation.
   origColumnTypesItems = tempTable1->columnTypes;
   origColumnSizesItems = tempTable1->columnSizes;
   fieldList = selectClause->fieldList;
   i = selectClause->fieldsCount;

   while (--i >= 0)
   {
      columnTypesItems1[i] = (field = fieldList[i])->dataType;
      columnSizesItems1[i] = field->size;
      columnHashesItems[i] = field->aliasHashCode;
   }

   heap_2 = heapCreate();
   IF_HEAP_ERROR(heap_2)
   {
   	heapDestroy(heap);
		if (tempTable1 && !*tempTable1->name)
			freeTable(context, tempTable1, false, false);
		else
			heapDestroy(heap_1);
		if (tempTable2)
			freeTable(context, tempTable2, false, false);
		else
			heapDestroy(heap_2);
		if (tempTable3)
			freeTable(context, tempTable3, false, false);
		else
			heapDestroy(heap_3);
		TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
		return null;
   }

   // Creates the second temporary table.
   if (!(tempTable2 = driverCreateTable(context, driver, null, null, intVector2Array(&columnHashes, heap_2), intVector2Array(&columnTypes, heap_2), 
		                        intVector2Array(&columnSizes, heap_2), null, null, NO_PRIMARY_KEY, NO_PRIMARY_KEY, null, 0, columnSizes.size, heap_2)))
   {
		heapDestroy(heap);
      if (!*tempTable1->name)
         freeTable(context, tempTable1, false, false);
		else // juliana@223_14: solved possible memory problems.
			heapDestroy(heap_1);
		return null;
   }

   count = totalRecords; // Starts writing the records from the first temporary table into the second temporary table.
	 
   if ((aggFunctionExist = selectClause->hasAggFunctions) && !useIndex) // Initializes the aggregated functions running totals.
   {
      aggFunctionsRunTotals = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue) * selectFieldsCount);
      aggFunctionsParamCols = htGetKeys(&aggFunctionsTable, heap);
      aggFunctionsColsCount = aggFunctionsParamCols.size;
      aggFunctionsCodes = (int32*)TC_heapAlloc(heap, aggFunctionsColsCount << 2);

		i = aggFunctionsColsCount;
      while (--i >= 0)
         aggFunctionsCodes[i] = TC_htGet32Inv(&aggFunctionsTable,aggFunctionsParamCols.items[i]);

      // If the temporary table points to a real table, stores also the column indexes of aggregate functions parameter list of the real table.
		if (*tempTable1->name)
      {
         aggFunctionsRealParamCols = (int32*)TC_heapAlloc(heap, aggFunctionsColsCount << 2);
         i = aggFunctionsColsCount;
			while (--i >= 0)
            aggFunctionsRealParamCols[i] = (param = fieldList[aggFunctionsParamCols.items[i]]->parameter) ? param->tableColIndex : -1;
      }
   }
   
   // juliana@227_12: corrected a possible bug with MAX() and MIN() with strings.
   curRecord = record1 = newSQLValues(i = (count = tempTable1->columnCount) > selectFieldsCount? count : selectFieldsCount, heap);
   
   // juliana@230_21: MAX() and MIN() now use indices on simple queries.
   if (useIndex)
   {
      Index* index;
      IntVector* rowsBitmap = (rsTemp? &rsTemp->rowsBitmap : null);
      
      // No rows in the answer.
      if (!(tempTable1->db->rowCount - tempTable1->deletedRowsCount) || (whereClause && !bitCount(rowsBitmap->items, rowsBitmap->length)))
      {
         heapDestroy(heap);
         return tempTable2;
      }
      
      // Computes the MAX() and MIN() for all the fields.
      i = -1;
      curRecord[0]->isNull = true; // No rows yet.
      while (++i < selectFieldsCount)
      {
         if ((field = fieldList[i])->isComposed)
            index = tempTable1->composedIndexes[field->index]->index;
         else
            index = tempTable1->columnIndexes[field->index];
         if (field->sqlFunction == FUNCTION_AGG_MAX)
         {
            if (!findMaxValue(context, index, curRecord[i], rowsBitmap, heap))
            {
               freeTable(context, tempTable2, false, false);
               heapDestroy(heap);
               return null;
            }
         }
         else if (!findMinValue(context, index, curRecord[i], rowsBitmap, heap))
         {
            freeTable(context, tempTable2, false, false);
            heapDestroy(heap);
            return null;
         }
      }
      
      if (curRecord[0]->isNull) // No rows found: returns an empty table.
      {
         heapDestroy(heap);
         return tempTable2;
      }
      xmemzero(*tempTable2->columnNulls, NUMBEROFBYTES(selectFieldsCount));
      writeRSRecord(context, tempTable2, curRecord);
      heapDestroy(heap);
      return tempTable2;
   }
   
   prevRecord = record2 = newSQLValues(i, heap);
   numOfBytes = NUMBEROFBYTES(count);
   nullsPrevRecord = tempTable1->columnNulls[0];
   nullsCurRecord = tempTable1->columnNulls[1];
   writeDelayed = aggFunctionExist || groupByClause;

   // Loops through the records of the temporary table, to calculate the agregated values and/or write the group records.
   paramCols = (isTableTemporary = !*tempTable1->name)? aggFunctionsParamCols.items: aggFunctionsRealParamCols;
   groupCountCols = (int32*)TC_heapAlloc(heap, aggFunctionsColsCount << 2);  // Each column has a groupCount because of the null values.

   // juliana@230_20: solved a possible crash when using aggregation functions with strings.
	// Allocates the total space for the strings at once so that they do not need to be reallocated.
   columnTypesItems1 = tempTable1->columnTypes;
	columnSizesItems1 = tempTable1->columnSizes;
	columnSizesItems2 = tempTable2->columnSizes;
	
	
   i = selectFieldsCount;
	while (--i >= 0)
	{
		if ((i < count && columnSizesItems2[i] > columnSizesItems1[i]) || columnSizesItems2[i])
		{
			record1[i]->asChars = (JCharP)TC_heapAlloc(heap, (columnSizesItems2[i] << 1) + 2);
			record2[i]->asChars = (JCharP)TC_heapAlloc(heap, (columnSizesItems2[i] << 1) + 2);
      }
	}
   
   i = count;
	while (--i >= 0)
	{
		if ((i < selectFieldsCount && columnSizesItems1[i] > columnSizesItems2[i]) || (columnSizesItems1[i] && !record1[i]->asChars))
		{
			record1[i]->asChars = (JCharP)TC_heapAlloc(heap, (columnSizesItems1[i] << 1) + 2);
			record2[i]->asChars = (JCharP)TC_heapAlloc(heap, (columnSizesItems1[i] << 1) + 2);
      }
	}
	
   // juliana@226_5
   i = aggFunctionsColsCount;
   while (--i >= 0)
	{
	   // juliana@227_12: corrected a possible bug with MAX() and MIN() with strings.
      if (columnSizesItems1[paramCols[i]])
         aggFunctionsRunTotals[i].asChars = (JCharP)TC_heapAlloc(heap, (columnSizesItems1[paramCols[i]] << 1) + 2); 
   }

   if (groupByClause)
		count = groupByClause->fieldsCount;

	columnNulls0 = *tempTable2->columnNulls;
	aggFunctionsParamColsItems = aggFunctionsParamCols.items;
   for (i = -1, groupCount = 0; ++i < totalRecords; groupCount++)
   {
      if (!readRecord(context, tempTable1, curRecord, i, 1, null, 0, true, null, null)) // juliana@220_3 juliana@227_20
		{
			heapDestroy(heap);
			if (!*tempTable1->name)
				freeTable(context, tempTable1, false, false);
			else
				heapDestroy(heap_1);
			freeTable(context, tempTable2, false, false);
			return null;
		}

      // Because it is possible to be pointing to a real table, skips deleted records.
      if (!isTableTemporary && !recordNotDeleted(tempTable1->db->basbuf))
      {
         groupCount--;
         continue;
      }

      // In case there is a group by, checks if there was a change in the group record composition.
      if (groupByClause && groupCount && compareRecords(prevRecord, curRecord, nullsPrevRecord, nullsCurRecord, count, groupList))
      {
         if (aggFunctionExist) // Checks if there are aggregate functions and concludes any aggregated function calculation.
            endAggFunctionsCalc(prevRecord, groupCount, aggFunctionsRunTotals, aggFunctionsCodes, aggFunctionsParamCols.items, paramCols, 
				                                                                   aggFunctionsColsCount, origColumnTypesItems, groupCountCols);

         // Flushes the previous record and starts a new group counting, taking the null values for the non-aggregate fields into consideration.
         xmemmove(columnNulls0, nullsPrevRecord, numOfBytes); 
         j = aggFunctionsColsCount;
			while (--j >= 0)
         {
            setBit(columnNulls0, aggFunctionsParamColsItems[j], !groupCountCols[j]);
            groupCountCols[j] = 0;
         }

         if (!writeRSRecord(context, tempTable2, prevRecord))
         {
				heapDestroy(heap);
            if (!*tempTable1->name)
               freeTable(context, tempTable1, false, false);
            freeTable(context, tempTable2, false, false);
            return null;
         }
         groupCount = 0;
      }

      // Checks if there are aggregate functions and performs the calculation of the aggregate functions..
      if (aggFunctionExist)
         performAggFunctionsCalc(context, curRecord, nullsCurRecord, aggFunctionsRunTotals, aggFunctionsCodes, paramCols, aggFunctionsColsCount, 
                                                                                                                origColumnTypesItems, groupCountCols);

      if (!writeDelayed)
      {
         columnNulls0 = nullsCurRecord;
         if (!writeRSRecord(context, tempTable2, curRecord))
			{
            if (!*tempTable1->name)
               freeTable(context, tempTable1, false, false);
            freeTable(context, tempTable2, false, false);
            heapDestroy(heap);
            return null;
			}
      }

      prevRecord = curRecord;
      xmemmove(nullsPrevRecord, nullsCurRecord, numOfBytes);
      curRecord = (curRecord == record1)? record2 : record1;
   }

   // juliana@227_12: corrected a possible bug with MAX() and MIN() with strings.
   if (writeDelayed && groupCount > 0) // If there was adelayed writing, flushes the last record.
   {
      if (aggFunctionExist)
         // Concludes any aggregated function calculation.
         endAggFunctionsCalc(prevRecord, groupCount, aggFunctionsRunTotals, aggFunctionsCodes, aggFunctionsParamColsItems, paramCols, 
				                                                                aggFunctionsColsCount, origColumnTypesItems,groupCountCols);

      // Writes the last record.
      xmemmove(columnNulls0, nullsCurRecord, numOfBytes); // Takes the null values for the non-aggregate fields into consideration.
      j = aggFunctionsColsCount;
		while (--j >= 0)
      {
         setBit(columnNulls0, aggFunctionsParamColsItems[j], !groupCountCols[j]);
         groupCountCols[j] = 0;
      }

      if (!writeRSRecord(context, tempTable2, prevRecord))
		{
			heapDestroy(heap);
			if (!*tempTable1->name)
               freeTable(context, tempTable1, false, false);
         freeTable(context, tempTable2, false, false);
         return null;
		}
   }

   if (!*tempTable1->name) // Drops the first temporary table, if it is really a temporary table.
	{
		freeTable(context, tempTable1, false, false);
		tempTable1 = null;
      heap_1 = null; // juliana@223_14: solved possible memory problems.
	}

   if (!havingClause) // If there is no having clause, returns the temp table 2.
   {
      heapDestroy(heap);
		return tempTable2;
   }

   heap_3 = heapCreate();
   IF_HEAP_ERROR(heap_3)
   {
      heapDestroy(heap);
		if (tempTable2)
			freeTable(context, tempTable2, false, false);
		else
			heapDestroy(heap_2);
		if (tempTable3)
			freeTable(context, tempTable3, false, false);
		else
			heapDestroy(heap_3);
		TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
		return null;
   }
   
   // juliana@223_14: solved possible memory problems.
   // Creates the last and third temporary table to hold the records that comply to the "HAVING" clause.
   if (!(tempTable3 = driverCreateTable(context, driver, null, null, intVector2Array(&columnHashes, heap_3), intVector2Array(&columnTypes, heap_3), 
		                       intVector2Array(&columnSizes, heap_3), null, null, NO_PRIMARY_KEY, NO_PRIMARY_KEY, null, 0, columnSizes.size, heap_3)))
   {
		heapDestroy(heap);
      freeTable(context, tempTable2, false, false);
      return null;
   }

   // The HAVING clause only works with aliases. So, remaps the table column names, to use the aliases
   // of the select statement instead of the original column names.
   if (!remapColumnsNames2Aliases(context, tempTable3, fieldList, selectClause->fieldsCount) ||
       !bindColumnsSQLBooleanClause(context, havingClause, &tempTable3->htName2index, tempTable3->columnTypes, null, 0, heap_3))
   {
      heapDestroy(heap);
		freeTable(context, tempTable2, false, false);
      freeTable(context, tempTable3, false, false);
      return null;
   }

   // Creates a result set from the table, using the HAVING clause.
   if (!(rsTemp = createResultSetForSelect(context, tempTable2, havingClause, heap)))
	{
      heapDestroy(heap);
      freeTable(context, tempTable3, false, false);
      return null;
	}

   // juliana@223_14: solved possible memory problems.
   // Writes the result set to the temporary table 3.
   if (writeResultSetToTable(context, &rsTemp, 1, tempTable3, null, selectClause, null, -1, heap) <= 0) // Already frees rsTemp.
   {
      freeTable(context, tempTable3, false, false);
      return null;
   }
   else
      return tempTable3;
}

/**
 * Generates a table to store the result set.
 *
 * @param context The thread context where the function is being executed.
 * @param tableList The table list of the select.
 * @param size The number of tables of the select.
 * @param whereClause the where clause of the select.
 * @param heap A heap to allocate the list.
 * @return The temporary result set table.
 */
ResultSet** createListResultSetForSelect(Context context, SQLResultSetTable** tableList, int32 size, SQLBooleanClause* whereClause, Heap heap)
{
	TRACE("createListResultSetForSelect")
   Table *table;
   ResultSet** rsList;
   ResultSet* resultSet;
   int32 i = -1;
   bool hasComposedIndex = false;

   rsList = (ResultSet**)TC_heapAlloc(heap, size * PTRSIZE);
   while (++i < size)
   {
      resultSet = createResultSet((table = tableList[i]->table), whereClause, heap);
      resultSet->decimalPlaces = (int8*)TC_heapAlloc(heap, resultSet->columnCount = table->columnCount);
      resultSet->indexRs = i; // Sets the table index.
      rsList[i] = resultSet; 
      xmemset(resultSet->decimalPlaces, -1, resultSet->columnCount);
      
		// It is only necessary to have one table with composed indices.
      hasComposedIndex = hasComposedIndex | (table->numberComposedIndexes > 0);
   }
   if (whereClause) // Tries to apply the table indices to generate a bitmap of the rows to be returned.
   {
      if (size > 1) // Join. 
         setIndexRsOnTree((*rsList)->whereClause->expressionTree);
      if (!generateIndexedRowsMap(context, rsList, size, hasComposedIndex, heap))
         return null;
   }
   return rsList;
}

/**
 * Generates an index bit map for a list of result sets.
 *
 * @param context The thread context where the function is being executed.
 * @param rsList The list of result sets.
 * @param size The number of tables of the select.
 * @param hasComposedIndex Indicates if the table has a composed index.
 * @param heap A heap to allocate temporary structures.
 * @return <code>true</code> if the function executed correctly; <code>false</code>, otherwise.
 */
bool generateIndexedRowsMap(Context context, ResultSet **rsList, int32 size, bool hasComposedIndex, Heap heap)
{
   TRACE("generateIndexedRowsMap")
	ResultSet* rsBag  = *rsList;
   Table* table = rsBag->table;
   SQLBooleanClause* whereClause = rsBag->whereClause;

   if (whereClause)
   {
      if (size > 1)
		{
			if (!applyTableIndexesJoin(whereClause))
				return true;
		} 
		else
         if (!applyTableIndexes(whereClause, table->columnIndexes, table->columnCount, hasComposedIndex, heap))
				return true;

      if (!computeIndex(context, rsList, size, size > 1, -1, null, -1, -1, heap))
         return false;
      if (!whereClause->expressionTree)
         while (--size >= 0) // There is no where clause left, since all rows can be returned using the indexes.
            rsList[size]->whereClause = null;
   }
   return true;
}

/**
 * Finds the rows that satisfy the query clause using the indices.
 *
 * @param context The thread context where the function is being executed.
 * @param rsList The result set list, one for each table.
 * @param size The number of tables of the select.
 * @param isJoin Indicates that the query has a join.
 * @param indexRsOnTheFly The index of the result set or -1 if the query is being indexed on the fly.
 * @param value The value to be indexed on the fly.
 * @param operator The operand type. Used only to index on the fly.
 * @param colIndex The index column. Used only to index on the fly.
 * @param heap A heap to allocate temporary structures.
 * @return <code>true</code> if the function executed correctly; <code>false</code>, otherwise.
 */
bool computeIndex(Context context, ResultSet **rsList, int32 size, bool isJoin, int32 indexRsOnTheFly, SQLValue *value, int32 operator, 
																																								int32 colIndex, Heap heap)
{
   // Gets the list of indexes that were applied to the where clause together with the indexes values to search for and the bool operation to apply.
   TRACE("computeIndex")
   ResultSet* rsBag; 
	ResultSet** rsListPointer = null; // The resulting indexed row bitmap.
   bool onTheFly = (indexRsOnTheFly != -1),
		  ok,
		  isCI, // has composed index?
	     isMatch;
   Table** appliedIndexTables = null;
	Table* table;
	Index* index;
	SQLValue** leftVal = (SQLValue**)TC_heapAlloc(heap, 4);
   SQLValue** rightVal = (SQLValue**)TC_heapAlloc(heap, 4);
   MarkBits markBits;
   Monkey monkey;
   SQLBooleanClause* whereClause = (*rsList)->whereClause;
   int32 i,
		   j,
         count = 1, 
		   booleanOp,
			recordCount,
	      col, 
			op, 
			maxSize = 1;
   uint8* indexedCols = whereClause->appliedIndexesCols;
	uint8* relationalOps = whereClause->appliedIndexesRelOps;
   uint8* cols = TC_heapAlloc(heap, 1);
	uint8* ops = TC_heapAlloc(heap, 1);
	SQLBooleanClauseTree** indexedValues = whereClause->appliedIndexesValueTree;
   ComposedIndex** appliedComposedIndexes = whereClause->appliedComposedIndexes;
   IntVector auxBitmap;
	xmemzero(&markBits, sizeof(MarkBits));
   rightVal = (SQLValue**)TC_heapAlloc(heap, 4);
   
	// Gets the list of indexes that were applied to the where clause, together
   // with the indexes values to search for and the boolean operation to apply.
	if (onTheFly)
      booleanOp = rsList[indexRsOnTheFly]->rowsBitmapBoolOp;
	else
	{
      count = whereClause->appliedIndexesCount;
      booleanOp = whereClause->appliedIndexesBooleanOp;
		leftVal = (SQLValue**)TC_heapAlloc(heap, 4);
   }
   
   rsBag = onTheFly? rsList[indexRsOnTheFly] : *rsList;
   recordCount = rsBag->table->db->rowCount; 
   if (isJoin) // Puts the result set bag in order with the tables.
   {
      rsListPointer = (ResultSet**)TC_heapAlloc(heap, count * PTRSIZE);
      appliedIndexTables = whereClause->appliedIndexesTables;
   
		i = count;
		while (--i >= 0)
      {
         j = size;
			while (--j >= 0)
            if (rsList[j]->table == appliedIndexTables[i])
            {
               rsBag = rsListPointer[i] = rsList[j];
               rsListPointer[i]->rowsBitmap = newIntBits(recordCount = rsBag->table->db->rowCount-1, heap);
               break;
            }
      }
   }
   else
   {
      if (onTheFly)
         rsList[indexRsOnTheFly]->auxRowsBitmap = newIntBits(recordCount, heap);
      else
         (*rsList)->rowsBitmap = newIntBits(recordCount, heap);
   }

   auxBitmap = (count > 1)? newIntBits(recordCount, heap) : emptyIntVector; 
   monkey.onKey = markBitsOnKey;
   monkey.onValue = markBitsOnValue;
   monkey.markBits = &markBits;

   rsBag->indexCount = 0;
   table = rsBag->table;
   
	// Loops through all applied indexes, and records the indexed rows in the bitmap.
   i = -1;
	while (++i < count)
   {
      isCI = false; 
	   ok = true;

      // Prepares the index row bitmap.
      col = op = -1, 
      size = 1;
      isMatch = false;
      cols = ops = null;

      if (isJoin)
      {
         table = appliedIndexTables[i];
         rsBag = rsListPointer[i];
      }
      rsBag->indexCount++;

      if (onTheFly)
      {
         col = colIndex;
         op = operator;
         isMatch = op == OP_PAT_MATCH_NOT_LIKE || op == OP_PAT_MATCH_LIKE;
         *leftVal = value;
      }
      else
      {
         if ((isCI = (appliedComposedIndexes[i] != null)))
         {
				if ((size = appliedComposedIndexes[i]->numberColumns) > maxSize)
				{
					leftVal = (SQLValue**)TC_heapAlloc(heap, size * PTRSIZE);
					rightVal = (SQLValue**)TC_heapAlloc(heap, size * PTRSIZE);
					cols = TC_heapAlloc(heap, size);
					ops = TC_heapAlloc(heap, size);
					maxSize = size;
				}

            for (j = 0; j < size; j++)
            {
               if (!leftVal[j])
						leftVal[j] = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue));
               cols[j] = indexedCols[i + j];
               ops[j] = relationalOps[i + j];
               if (!(isMatch = ops[j] == OP_PAT_MATCH_NOT_LIKE || ops[j] == OP_PAT_MATCH_LIKE) 
					 && !getOperandValue(context, indexedValues[i + j], leftVal[j]))
                  return false;
            }
         }
         else
         {
            if (!*leftVal) 
					*leftVal = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue));
            if (!*rightVal)
					*rightVal = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue));
            col = indexedCols[i];
            op = relationalOps[i];
            if (!(isMatch = op == OP_PAT_MATCH_NOT_LIKE || op == OP_PAT_MATCH_LIKE) 
				 && !getOperandValue(context, indexedValues[i], *leftVal))
               return false;
         }
      }

      index = (isCI)? appliedComposedIndexes[i]->index : table->columnIndexes[col];

      markBits.leftKey.keys = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue) * size); 
      markBits.rightKey.keys = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue) * size);
      markBits.leftOp = TC_heapAlloc(heap, size);
      markBits.rightOp = TC_heapAlloc(heap, size);
      markBitsReset(&markBits, (rsBag->indexCount > 1)? &auxBitmap : (onTheFly? &rsBag->auxRowsBitmap 
                                                                              : &rsBag->rowsBitmap)); // Prepared the index row bitmap.
      
      j = size;
		while (--j >= 0)
      {
         if (isCI)
         {
            op = ops[j];
            isMatch = op == OP_PAT_MATCH_NOT_LIKE || op == OP_PAT_MATCH_LIKE;
         }

         // if the operation is 'like x%', then replaces the operand by the value without the % mask.
         if (isMatch)
         {
            // juliana@230_10: solved a bug that could crash the application when more than one index is applied.
            leftVal[j]->asChars = indexedValues[i + j]->strToMatch;
            leftVal[j]->length = indexedValues[i + j]->lenToMatch;
         }

			// Checks if this is a "between" operation.
         else if (booleanOp == OP_BOOLEAN_AND && i < (count - 1) && indexedCols[i + 1] == col && (op == OP_REL_GREATER || op == OP_REL_GREATER_EQUAL)
               && (relationalOps[i + j + 1] == OP_REL_LESS || relationalOps[i + j + 1] == OP_REL_LESS_EQUAL))
         {
            if (!getOperandValue(context, indexedValues[j + i + 1], rightVal[j]))
               return false;
            keySet(&markBits.rightKey, &rightVal[j], index, 1);  
            markBits.rightOp[j] = relationalOps[j + i++ + 1]; // The next operation is already processed.
         }
         else
         {
            switch (col = op) // When searching for !=, <, <=, we'll use the opposite operation instead.
            {
               case OP_REL_DIFF : 
						col = OP_REL_EQUAL;        
						break;
               case OP_REL_LESS : 
						col = OP_REL_GREATER_EQUAL; 
						break;
               case OP_REL_LESS_EQUAL : 
						col = OP_REL_GREATER;       
						break;
               case OP_PAT_MATCH_NOT_LIKE : 
						col = OP_PAT_MATCH_LIKE;    
            }
            if (op != col) // All the rows will be marked and only resets the rows that satisfy the opposite operation.
            {
               setAllBits(markBits.indexBitmap->items, markBits.indexBitmap->size);
               markBits.bitValue = false;
               op = col;
            }
         }
         markBits.leftOp[j] = op;
      }
      keySet(&markBits.leftKey, leftVal, index, size);

      switch (op) // Finally, marks all rows that match this value / range of values.
      {
         case OP_REL_EQUAL:
            ok = indexGetValue(context, &markBits.leftKey, &monkey);
            break;
         case OP_REL_GREATER:
         case OP_REL_GREATER_EQUAL:
         case OP_PAT_MATCH_LIKE:
            ok &= indexGetGreaterOrEqual(context, &markBits.leftKey, &monkey);
      }

      if (!ok) // guich_570_97
         return false;

      // If it is the first index, assigns the index bitmap to the resulting bitmap. Otherwise, merges the index
      // bitmap with the existing bitmap, using the boolean operator.
      if (rsBag->indexCount > 1)
         mergeBitmaps((onTheFly? &rsBag->auxRowsBitmap : &rsBag->rowsBitmap), markBits.indexBitmap, booleanOp);

      rsBag->rowsBitmapBoolOp = booleanOp; // juliana@230_15: corrected a bug when using joins with indices which could cause an OutOfMemoryExcepion.
      if (isCI)
         i += (size -1);
   }
   
   return true;
}

/**
 * Merges two bitmaps into the first bitmap using the given boolean operator.
 *
 * @param bitmap1 The first bitmap.
 * @param bitmap2 The second bitmap.
 * @param booleanOp The boolean operator to be applied.
 */
void mergeBitmaps(IntVector* bitmap1, IntVector* bitmap2, int32 booleanOp)
{
	TRACE("mergeBitmaps")
   int32* items1 = bitmap1->items;
   int32* items2 = bitmap2->items;
   int32 size = bitmap1->size;
   if (booleanOp == OP_BOOLEAN_AND)
      while (--size >= 0)
         *items1++ &= *items2++;
   else
      while (--size >= 0)
         *items1++ |= *items2++;
}

/**
 * Concludes the calculation of the given aggregated function running totals based on the given record and the group count.
 * 
 * @param record The record that is the parameter for the aggregated function.
 * @param groupCount The result of a COUNT(*).
 * @param aggFunctionsRunTotals The results of the aggregated functions. 
 * @param aggFunctionsCodes The aggregated function codes.
 * @param aggFunctionsParamCols The columns that are parameters to the aggregated functions.
 * @param aggFunctionsRealParamCols The real columns that are parameters to the aggregated functions.
 * @param aggFunctionsColsCount The number of columns that are parameters to the aggregated functions.
 * @param columnTypes The types of the columns.
 * @param groupCountCols The count for the groups.
 */
void endAggFunctionsCalc(SQLValue **record, int32 groupCount, SQLValue* aggFunctionsRunTotals, int32* aggFunctionsCodes, 
								 int32* aggFunctionsParamCols, int32* aggFunctionsRealParamCols, int32 aggFunctionsColsCount, int32* columnTypes, 
								                                                                                              int32* groupCountCols)
{
   TRACE("endAggFunctionsCalc")
   int32 j = aggFunctionsColsCount,
	      colIndex,
         realColIndex,
         sqlAggFunction,
			colType;
   SQLValue* aggValue;
   SQLValue* value;

   while (--j >= 0) // Concludes the calculation of the aggregate functions running totals.
   {
      colIndex = aggFunctionsParamCols[j];
      realColIndex = aggFunctionsRealParamCols[j];
      aggValue = &aggFunctionsRunTotals[j];
      value = record[colIndex];

      switch (sqlAggFunction = aggFunctionsCodes[j])
      {
         case FUNCTION_AGG_AVG:
         {
            if (groupCountCols[j] != 0)
               value->asDouble = aggValue->asDouble/groupCountCols[j];
            aggValue->asDouble = 0;
            break;
         }

         // juliana@226_5: the aggregation functions MAX() and MIN() now work for CHAR, VARCHAR, CHAR NOCASE, and VARCHAR NOCASE column types. 
         case FUNCTION_AGG_MAX:
         case FUNCTION_AGG_MIN:
         {
            
            switch (colType = columnTypes[realColIndex]) // Checks the type of the column. 
            {
               case SHORT_TYPE:
                  value->asShort = aggValue->asShort;
                  break;

               case FLOAT_TYPE:
                  value->asFloat = aggValue->asFloat;
                  break;

               case DATE_TYPE: //rnovais@567_2
               case INT_TYPE:
                  value->asInt = aggValue->asInt;
                  break;

               case LONG_TYPE:
                  value->asLong = aggValue->asLong;
                  break;

               case DOUBLE_TYPE:
                  value->asDouble = aggValue->asDouble;
                  break;

               case DATETIME_TYPE: //rnovais@567_2
                  value->asDate = aggValue->asDate;
                  value->asTime = aggValue->asTime;
                  break;

               // juliana@226_9: strings are not loaded anymore in the temporary table when building result sets. 
               // juliana@227_12: corrected a possible bug with MAX() and MIN() with strings.
               case CHARS_TYPE:
               case CHARS_NOCASE_TYPE:
                  xmemmove(value->asChars, aggValue->asChars, (value->length = aggValue->length) << 1); 
                  value->asInt = aggValue->asInt;
                  value->asBlob = aggValue->asBlob;
            }
            break;
         }

         case FUNCTION_AGG_COUNT:
         {
            value->asInt = groupCount;
            break;
         }

         case FUNCTION_AGG_SUM:
         {
            value->asDouble = aggValue->asDouble;
            aggValue->asDouble = 0;
         }
      }
   } 
}

/**
 * Creates a temporary table that stores only an integer value.
 *
 * @param context The thread context where the function is being executed.
 * @param driver The connection with Litebase.
 * @param intValue The value to be put in the table.
 * @param colName The column name of the single table column.
 * @return The table if the method executes correctlty; <code>null</code>, otherwise.
 * @throws OutOfMemoryError If there is not enougth memory alloc memory. 
 */
Table* createIntValueTable(Context context, Object driver, int32 intValue, CharP colName)
{
	TRACE("createIntValueTable")
   Table* table = null;
   SQLValue val;
	SQLValue* record;
	int32* colHash;
   int32* colType;
   int32* colSize;

	Heap heap = heapCreate();
   IF_HEAP_ERROR(heap)
   {
      heapDestroy(heap);
      TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
      return null;
   }

	// Creates unitary arrays with a hash code, type, and size.
   colHash = (int32*)TC_heapAlloc(heap, 4);
   colType = (int32*)TC_heapAlloc(heap, 4);
   colSize = (int32*)TC_heapAlloc(heap, 4);

   *colHash = TC_hashCode(colName);
   *colType = INT_TYPE;

   val.asInt = intValue;
   record = &val;

    // juliana@223_14: solved possible memory problems.
	if ((table = driverCreateTable(context, driver, null, null, colHash, colType, colSize, null, null, NO_PRIMARY_KEY, NO_PRIMARY_KEY, null, 0, 1, 
		                                                                                                heap)) && writeRSRecord(context, table, &record))
      return table;
   if (table) // juliana@223_14: solved possible memory problems.
      freeTable(context, table, true, false);
   return null;
}

/** 
 * Binds the column information of the underlying tables to the select clause. 
 *
 * @param context The thread context where the function is being executed.
 * @param clause The select clause.
 * @return <code>false</code> if an error occurs; <code>true</code>, otherwise.
 * @throws SQLParseException In case of an unknown or ambigous column name, or the parameter and the function data types are incompatible.
 */
bool bindColumnsSQLSelectClause(Context context, SQLSelectClause* clause) // guich@512_2: added columnnames
{
	TRACE("bindColumnsSQLSelectClause")
   SQLResultSetField** fieldList = clause->fieldList;
   SQLResultSetTable** tableList = clause->tableList;
   int32 tableListSize = clause->tableListSize,
         fieldListSize = clause->fieldsCount,
         i, 
         j,
         index,
         columnCount;
   Hashtable htName2index = clause->htName2index;
   SQLResultSetTable* rsTable;
   SQLResultSetField* field;
   CharP tableName;
   Table* currentTable;
   CharP* columnNames;
   int32* columnHashes;
   int32* columnTypes;
   int32* columnSizes;
   
   if (!fieldListSize) // If the select clause has a wild card (is null), then expands the list using the column information from the given tables.
   {
      int32 count = 0,  
            position = 0;
      Table* table;

      j = tableListSize;
      while (--j >= 0)
         count += tableList[j]->table->columnCount - 1; // Excludes the rowid.
      clause->fieldsCount = count;

      count = 0;
      j = -1;
      while (++j < tableListSize)
      {
         table = (rsTable = tableList[j])->table;
         tableName = rsTable->tableName;
         i = 0;
         columnCount = table->columnCount;
         columnNames = table->columnNames; 
         columnHashes = table->columnHashes;
         columnTypes = table->columnTypes;
         columnSizes = table->columnSizes;

         while (++i < columnCount) // guich@503_10: changed loop to exclude the rowid.
         {
            field = fieldList[count++] = initSQLResultSetField(clause->heap); // SQLResultSetField - guich@503_10: added -1 to exclude the rowid.
            field->alias = columnNames[i]; // guich@_512_2: store the column name too
            field->aliasHashCode = field->tableColHashCode = columnHashes[i];
            field->dataType = columnTypes[i];
            field->size = columnSizes[i];
            field->tableColIndex = i;
            field->table = table;
            field->tableName = rsTable->tableName;
            TC_htPut32(&htName2index, TC_hashCodeFmt("sss", tableName, ".", columnNames[i]), position);
            TC_htPut32IfNew(&htName2index, columnHashes[i], position++);
         }
      }
   }
   else
   {
      int32 hashAliasTableName,
            hash1, 
            hash2,
            auxIndex,
            aggFunctionType, 
            dtFunctionType, 
            sqlFunction;
      bool foundFirst;
      CharP fieldName;
      Table* auxTable;
      SQLResultSetTable* rsTableAux;
      SQLResultSetField* param;;
            
               
      i = -1;
      while (++i < fieldListSize) // Binds the listed colums to the table.
      {
         index = -1;
         currentTable = null;
         rsTable = null;
         
         // Aggregation functions (count() doesn't have a column name yet).
         if (!(field = fieldList[i])->isAggregatedFunction || field->sqlFunction != FUNCTION_AGG_COUNT)
         {
            if (field->tableName) // Checks the names.
            {
               hashAliasTableName = TC_hashCode(field->tableName);
               
               // Verifies if it is a valid table name.
               currentTable = null;
               j = tableListSize;
               while (--j >= 0)
                  if (tableList[j]->aliasTableNameHashCode == hashAliasTableName)
                  {
                     currentTable = (rsTable = tableList[j])->table;
                     break;
                  }

               if (!currentTable)
               {
                  TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_UNKNOWN_COLUMN), field->alias);
                  return false;
               }
               if ((index = TC_htGet32Inv(&currentTable->htName2index, hash1 = TC_hashCode(fieldName = field->tableColName))) < 0)
               {
                  TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_UNKNOWN_COLUMN), field->alias);
                  return false;
               }

               TC_htPut32(&htName2index, field->aliasHashCode, i);
               hash2 = TC_hashCodeFmt("sss", tableName = field->tableName, ".", fieldName);
               if (field->aliasHashCode != hash2) // Used an explicit alias.
                  TC_htPut32(&htName2index, TC_hashCodeFmt("sss", tableName, ".", field->alias), i);
               else if (TC_htGet32Inv(&htName2index, hash1) < 0) // Stores the name of the field once; only the first.
                  TC_htPut32(&htName2index, hash1, i);  

            }
            else // Verifies if the column name in the field list is ambiguous.
            {
               rsTableAux = rsTable = null;
               foundFirst = false;
               currentTable = auxTable = null;
               
               j = tableListSize;
               while (--j >= 0)
               {
                  if ((auxIndex = TC_htGet32Inv(&(auxTable = (rsTableAux = tableList[j])->table)->htName2index, field->tableColHashCode)) >= 0)
                  {
                     if (foundFirst)
                     {
                        TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_AMBIGUOUS_COLUMN_NAME), field->alias);
                        return false;
                     }
                     else
                     {
                        foundFirst = true;
                        index = auxIndex;
                        TC_htPut32(&htName2index, TC_hashCodeFmt("sss", tableList[j]->tableName, ".", field->alias), i);
                        TC_htPut32(&htName2index, field->aliasHashCode, i);
                        TC_htPut32(&htName2index, field->tableColHashCode, i);
                        currentTable = auxTable;
                        rsTable = rsTableAux;
                     }
                  }
               }
               if (!foundFirst)
               {
                  TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_UNKNOWN_COLUMN), field->alias);
                  return false;
               }
            }
         }

         if (!field->isVirtual) // If the field is not virtual, it needs to be mapped directly to the underlying table.
         {
            field->dataType = currentTable->columnTypes[field->tableColIndex = index];
            field->size = currentTable->columnSizes[index];
            field->table = currentTable;
            field->tableName = rsTable->tableName;
            TC_htPut32(&htName2index, field->aliasHashCode, i);
         }

         // rnovais@568_10
         else if (field->isAggregatedFunction || field->isDataTypeFunction) // If it is an aggregated or data type function, maps its parameter. 
         {
            param = field->parameter;
            aggFunctionType = dtFunctionType = UNDEFINED_TYPE; // initialized with an UNDEFINED number.
            sqlFunction = field->sqlFunction;
            field->table = currentTable;
            TC_htPut32(&htName2index, field->aliasHashCode, i);

            if (field->isAggregatedFunction) // Stores the correct function code.
               aggFunctionType = aggregateFunctionsTypes[sqlFunction];
            else
               dtFunctionType = dataTypeFunctionsTypes[sqlFunction];

            if (param)
            {
               param->dataType = currentTable->columnTypes[param->tableColIndex = index];
               param->size = currentTable->columnSizes[index];

               if (field->isAggregatedFunction) // rnovais@568_10
               {

                  // Check if the parameter and aggregated function
                  // data types are compatible.
                  if (param->dataType == CHARS_TYPE 
                   && (aggFunctionType == INT_TYPE || aggFunctionType == DOUBLE_TYPE || aggFunctionType == NUMBER_TYPE))
                  {
                     TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_DATA_TYPE_FUNCTION), 
                 (sqlFunction == FUNCTION_AGG_COUNT)? "count": (sqlFunction == FUNCTION_AGG_MAX)? "max" 
               : (sqlFunction == FUNCTION_AGG_MIN)? "min" : (sqlFunction == FUNCTION_AGG_AVG)? "avg" : (sqlFunction == FUNCTION_AGG_SUM)? "sum": "");
                     return false;      
                  }

                  // For aggregated functions, if the function does not have a defined data type, it inherits the parameter size and type.
                  if (aggFunctionType == NUMBER_TYPE || aggFunctionType == UNDEFINED_TYPE)
                  {
                     field->dataType = param->dataType;
                     field->size = param->size;
                  }
               }
               else
               {
                  field->size = param->size; // rnovais@570_1

                  // rnovais@570_5: if UNDEFINED the datatype will be the same of the field thus, it is possible to have functions that can be 
                  // applyed to diferents fieds type. e.g. ABS(int) returns int, ABS(double) returns double, etc.
                  if (dtFunctionType == UNDEFINED_TYPE)
                     field->dataType = param->dataType;

                  // rnovais@568_10: checks if the parameter and the data type function data types are compatible.
                  if (!bindFunctionDataType(param->dataType, sqlFunction))
                  {
                     TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_DATA_TYPE_FUNCTION), 
                                                                                   dataTypeFunctionsName(sqlFunction));
                     return false;
                  }
               }
            }
         }
      }
      
   }
   clause->htName2index = htName2index;
   return true;
}

/**
 * Remaps a table column names, so it uses the alias names of the given field list, instead of the original names.
 * 
 * @param context The thread context where the function is being executed.
 * @param table The result set table.
 * @param fieldsList The field list of the select clause.
 * @param fieldsCount The number of fields of the select clause.
 * @return <code>false</code> if an error occurs; <code>true</code>, otherwise.
 * @throw OutOfMemoryError If a memory allocation fails.
 */
bool remapColumnsNames2Aliases(Context context, Table* table, SQLResultSetField** fieldsList, int32 fieldsCount)
{
   TRACE("remapColumnsNames2Aliases")
   Hashtable* tableName2Index = &table->htName2index;
   int32* columnHashes = table->columnHashes;
   SQLResultSetField* field;
   Heap heap = table->heap;

   if (!table->columnNames) // previously created? don't overrite it
   {
      IF_HEAP_ERROR(heap) // juliana@223_14: solved possible memory problems.
      {
         TC_throwExceptionNamed(context, "java.lang.OutOfMemoryError", null);
         return false;
      }

      table->columnNames = (CharP*)TC_heapAlloc(heap, fieldsCount * PTRSIZE);
   
      while (--fieldsCount >= 0)
      {
         table->columnNames[fieldsCount] = (field = fieldsList[fieldsCount])->alias;
     
         if (columnHashes[fieldsCount] == field->aliasHashCode) // Replaces the original mapping, if necessary.
            continue;
         if (!TC_htPut32(tableName2Index, field->aliasHashCode, fieldsCount)) // Already replaces old values.
            return false;
      }
   }
   return true;
}

/**
 * Writes the records of a result set to a table.
 *
 * @param context The thread context where the function is being executed.
 * @param list The result set list, one for each table in the from field.
 * @param numTables The number of tables of the select.
 * @param rs2TableColIndexes The mapping between result set and table columns.
 * @param selectClause The select clause of the query.
 * @param columnIndexesTables Has the indices of the tables for each resulting column.
 * @param whereClauseType Indicates the where clause is an <code>AND</code> or an <code>OR</code>.
 * @param heap A heap to allocate temporary structures.
 * @return The total number of records added to the table or -1 if an error occurs.
 */
int32 writeResultSetToTable(Context context, ResultSet** list, int32 numTables, Table* table, IntVector* rs2TableColIndexes, 
                                             SQLSelectClause* selectClause, IntVector* columnIndexesTables, int32 whereClauseType, Heap heap)
{
   TRACE("writeResultSetToTable")
   int32 count = table->columnCount;
   SQLValue** values = (SQLValue**)TC_heapAlloc(heap, count * PTRSIZE);
   SQLResultSetField** fieldList = selectClause->fieldList;
   ResultSet* resultSet = list[0];
   MemoryUsageEntry* memUsageEntry;
   PlainDB* tempDB = table->db;
   PlainDB* selectDB = selectClause->tableList[0]->table->db;
   XFile* memoryDB = &tempDB->db;
   IntVector rowsBitmap = resultSet->rowsBitmap;
   Table* rsTable = resultSet->table;
   int32* rsTypes = rsTable->columnTypes;
   int32* rsSizes = rsTable->columnSizes;
   int32* items = rs2TableColIndexes? rs2TableColIndexes->items : null;
	int32 countSelectedField = selectClause->fieldsCount, // rnovais@568_10: when it has an order by table.columnCount = selectClause.fieldsCount + 1.
         i = countSelectedField, 
         j,
         totalRecords = 0,
         rowSize = tempDB->rowSize,
         rsCount = rsTable->columnCount,
         bytes = NUMBEROFBYTES(rsCount);
   bool hasDataType = false;
   uint8* nulls0 = table->columnNulls[0];
   uint8* rsNulls0 = rsTable->columnNulls[0];
   uint8* buffer = rsTable->db->basbuf + rsTable->columnOffsets[rsCount];
   
   while (--i >= 0) // rnovais@568_10: checks if there is a data type function in the select.
   {
      if (fieldList[i]->isDataTypeFunction) // Changes the datatype and columns offset for the temporary table.
      {
         // Changes the table definition.
         table->columnTypes[i] = fieldList[i]->dataType;
         hasDataType = true;
      }
   }

   // guich@570_97: checks often.
   if (hasDataType && !computeColumnOffsets(context, table)) // The original types have changed, so the offsets must be recomputed. 
   {
      freeResultSet(resultSet);
      return -1;
   }
   
   // juliana@223_14: solved possible memory problems.
   // juliana@223_9: improved Litebase temporary table allocation on Windows 32, Windows CE, Palm, iPhone, and Android.
   // No indices and no where clause: allocs all the records space in the temporary .db.
   if (!resultSet->whereClause && !resultSet->rowsBitmap.size) 
   {
      if (!mfGrowTo(context, memoryDB, (tempDB->rowAvail = selectDB->rowCount) * rowSize))
      {
         freeResultSet(resultSet);
         return -1;
      }
   }
   else // Uses colected statistics.
   {
      LOCKVAR(parser);
      if ((memUsageEntry = TC_htGetPtr(&memoryUsage, selectClause->sqlHashCode)))
      {
         if (!mfGrowTo(context, memoryDB, memUsageEntry->dbSize))
         {
            freeResultSet(resultSet);
            return -1;
         }
         tempDB->rowAvail = memUsageEntry->dbSize / rowSize;
         if (!mfGrowTo(context, &tempDB->dbo, memUsageEntry->dboSize))
         {
            freeResultSet(resultSet);
            return -1;
         }
      }
      UNLOCKVAR(parser);
   }
   if (!tempDB->rowAvail && rowsBitmap.size) // Uses the indices if the other approachs can't be useful.
   {   
      int32 numberOfBits = bitCount(rowsBitmap.items, rowsBitmap.length) + 1;
      
      if (numberOfBits > 0)
      {
         if (!mfGrowTo(context, memoryDB, (tempDB->rowAvail = numberOfBits) * rowSize))
         {
            freeResultSet(resultSet);
            return -1;
         }
      }
   }
   
   if (numTables == 1)
   {
      int32 colIndex;

      resultSet->pos = -1;
      i = count;

      while (--i >= 0) // Allocates the record before using them.
      {
         if ((colIndex = (items)? items[i] : i) != -1)
         {
            values[i] = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue));
            if ((rsTypes[colIndex] == CHARS_TYPE || rsTypes[colIndex] == CHARS_NOCASE_TYPE))
               values[i]->asChars = (JCharP)TC_heapAlloc(heap, (rsSizes[colIndex] << 1) + 2);
         }
      }

      while (getNextRecord(context, resultSet, heap)) // No preverify needed.
      {
         j = 0;
         i = -1;
         
         xmemmove(rsNulls0, buffer, bytes); // Reads the bytes of the nulls.

         while (++i < count) // Gets the values of the result set columns.
         {
            colIndex = items? items[i] : i;

            // For columns that do no map directly to the underlying table of the result set, just skips the reading.
            if (colIndex != -1 && isBitUnSet(rsNulls0, colIndex) && !getTableColValue(context, resultSet, colIndex, values[i])) // juliana@220_3
            {
               freeResultSet(resultSet);
               return -1;
            }
            
            if (colIndex != -1)
               setBit(nulls0, i, isBitSet(rsNulls0, colIndex)); // Sets the null values of tempTable.
            else
               setBitOff(nulls0, i);

            if (j < countSelectedField) // rnovais@568_10
               j++;
         }
         
         if (writeRSRecord(context, table, values)) // Writes the record.
            totalRecords++;
         else
         {
            freeResultSet(resultSet);
            return -1;
		   }
      }
   }
   else // Join.
   {
      int32* columnIndexesTablesItems = columnIndexesTables->items;

      i = numTables;
      while (--i >= 0)
         list[i]->indexes = newIntVector(null, count, heap);
      
      i = count;
      while (--i >= 0)
         if (items[i] != -1) // count(*)
         {
            j = numTables;
            while (--j >= 0)
               if ((Table*)columnIndexesTablesItems[i] == list[j]->table)
               {
                  IntVectorAdd(null, &list[j]->indexes, i);
                  break;
               }
         }
      totalRecords = performJoin(context, list, numTables, table, rs2TableColIndexes, fieldList, values, whereClauseType, heap);
   }

   freeResultSet(resultSet);
   return totalRecords;
}

/**
 * Counts the number of ON bits.
 *
 * @param elements The array where the bits will be counted.
 * @param length The array length.
 * @return The number of on bits.
 */
int32 bitCount(int32* elements, int32 length)
{
	TRACE("bitCount")
   int32 count = 0,
         value = 0;
   uint8* bitElems = (uint8*)elements;
   length *= 4;
   
   while (--length >= 0)
   {
      value = *bitElems++;
      count += bitsInNibble[value & 0xF] + bitsInNibble[(value >> 4) & 0xF]; 
   }
   return count;
}

/**
 * Executes a join operation.
 * 
 * @param context The thread context where the function is being executed.
 * @param list The list of the result sets.
 * @param numTables The number of tables of the select.
 * @param table The result set table.
 * @param rs2TableColIndexes The mapping between result set and table columns.
 * @param fieldList The select clause field list.
 * @param values The record to be joined with.
 * @param whereClauseType The type of operation used: <code>AND</code> or <code>OR</code>.
 * @param heap A heap to allocate temporary structures.
 * @return The number of records written to the temporary table or -1 if an error occurs.
 */
int32 performJoin(Context context, ResultSet** list, int32 numTables, Table* table, IntVector* rs2TableColIndexes, SQLResultSetField** fieldList,
                                                                                    SQLValue** values, int32 whereClauseType, Heap heap)
{
   TRACE("performJoin")
   int32 currentIndexTable = 0, 
         totalRecords = 0,
	      ret = NO_RECORD, 
         length = table->columnCount, 
         colIndex,
         position,
         rsCount;
   bool bitSet;
   uint8 verifyWhereCondition[MAXIMUMS];
   IntVector indexes;
   ResultSet* currentRs;
   Table* rsTable;
   int32* types = table->columnTypes;
   int32* sizes = table->columnSizes;
   uint8* nulls0 = table->columnNulls[0];
   uint8* rsNulls0;

   xmemset(verifyWhereCondition, true, numTables);

   while (--length >= 0) // Allocates the necessary records before using them.
   {
      values[length] = (SQLValue*)TC_heapAlloc(heap, sizeof(SQLValue));
      if (types[length] == CHARS_TYPE || types[length] == CHARS_NOCASE_TYPE)
         values[length]->asChars = (JCharP)TC_heapAlloc(heap, (sizes[length] << 1) + 2);
   }

   while (currentIndexTable >= 0)
   {
      currentRs = list[currentIndexTable];

      switch (ret = getNextRecordJoin(context, currentIndexTable, verifyWhereCondition[currentIndexTable], numTables, whereClauseType, list, heap))
      {
         case VALIDATION_RECORD_OK:
         case VALIDATION_RECORD_INCOMPLETE:
         {
            length = (indexes = currentRs->indexes).size;
            rsCount = (rsTable = currentRs->table)->columnCount;
            xmemmove(rsNulls0 = rsTable->columnNulls[0], rsTable->db->basbuf + rsTable->columnOffsets[rsCount], NUMBEROFBYTES(rsCount));
            
            while (--length >= 0) // Fills the data of the current ResultSet.
            {
               position = indexes.items[length];

               // If rs2TableColIndexes == null, it indicates that the result set and the table have the same sequence of columns.
               colIndex = rs2TableColIndexes? rs2TableColIndexes->items[position] : length;

               // For columns that do no map directly to the underlying table of the result set, just skips the reading.
               bitSet = isBitSet(rsNulls0, colIndex);

               // If it is null, just skips.
               if ((colIndex != -1) && !bitSet && !getTableColValue(context, currentRs, colIndex, values[position])) // juliana@220_3
                  return -1;

               if (colIndex != -1)
                  setBit(nulls0, position, bitSet); // Sets the null values from the temporary table.
               else
                  setBitOff(nulls0, position);
            }
            if (ret == VALIDATION_RECORD_OK)
            {
               if (currentIndexTable < numTables - 1) // Goes to the next table.
               {
                  currentIndexTable++;
                  verifyWhereCondition[currentIndexTable] = false;
               }
               else // It is the last resultSet, so stores the data.
               {
                  if (writeRSRecord(context, table, values)) // Writes the record.  
                     totalRecords++;
                  else
                     return -1;
               }
            }
            else // VALIDATION_RECORD_INCOMPLETE
            {
               currentIndexTable++;
               verifyWhereCondition[currentIndexTable] = true;
            }
            break;
         }
         case NO_RECORD:
         {
            currentIndexTable--;
            currentRs->pos = -1; // Restarts the current resultset to the next iteration.
         }
      }
   }
   return totalRecords;
}

/**
 * Gets the next record to perform the join operation.
 * 
 * @param context The thread context where the function is being executed.
 * @param rsIndex The index of the result set of the list used to get the next record.
 * @param verifyWhereCondition Indicates if the where clause needs to be verified.
 * @param totalRs The number of result sets (tables used in the join) in the result set list.
 * @param whereClauseType The type of expression in the where clause (OR or AND).
 * @param rsList The list of the result sets.
 * @param heap A heap to allocate temporary structures.
 * @return <code>VALIDATION_RECORD_OK</code>, <code>NO_RECORD</code>, <code>VALIDATION_RECORD_NOT_OK</code>,
 * <code>VALIDATION_RECORD_INCOMPLETE</code>, or -1 if an error occurs.
 */
int32 getNextRecordJoin(Context context, int32 rsIndex, bool verifyWhereCondition, int32 totalRs, int32 whereClauseType, ResultSet** rsList, 
                                                                                                                         Heap heap)
{
   TRACE("getNextRecordJoin")
   ResultSet* resultSet = rsList[rsIndex];
   PlainDB* plainDB = resultSet->table->db;
   uint8* basbuf = plainDB->basbuf;
   IntVector rowsBitmap = (resultSet->auxRowsBitmap.size > 0)? resultSet->auxRowsBitmap : resultSet->rowsBitmap;
   SQLBooleanClause* whereClause = resultSet->whereClause;
   int32 rowCountLess1 = plainDB->rowCount - 1,
         ret;

   // Desired rows partially computed using the indexes?
   if (rowsBitmap.size && verifyWhereCondition)
   {
      int32 position;
            
      if (resultSet->pos < rowCountLess1)
      {
         if (!resultSet->whereClause || resultSet->auxRowsBitmap.size)
         {
            // No WHERE clause. Just returns the rows marked in the bitmap.
            if ((position = findNextBitSet(&rowsBitmap, resultSet->pos + 1)) != -1 && position <= rowCountLess1)
            {
               if (plainRead(context, plainDB, resultSet->pos = position))
               {
                  if (resultSet->auxRowsBitmap.size && verifyWhereCondition)
                  {
                     whereClause->resultSet = resultSet;
                     return booleanTreeEvaluateJoin(context, whereClause->expressionTree, rsList, totalRs, heap);
                  }
                  if (whereClauseType == WC_TYPE_AND_DIFF_RS)
                     return (totalRs == resultSet->indexRs + 1)? VALIDATION_RECORD_OK : VALIDATION_RECORD_INCOMPLETE;
                  return VALIDATION_RECORD_OK;
               }
               else
                  return -1;
            }
         }
         else
         {
            // With a remaining WHERE clause there are 2 situations.
            // 1) The relationship between the bitmap and the WHERE clause is an AND relationship.
            // 2) The relationship between the bitmap and the WHERE clause is an OR relationship.
            if (resultSet->rowsBitmapBoolOp == OP_BOOLEAN_AND)
            {
               // AND case - Walks through the bits that are set in the bitmap and checks if the rows satisfy the where clause.
               while ((position = findNextBitSet(&rowsBitmap, resultSet->pos + 1)) != -1 && position <= rowCountLess1)
                  if (plainRead(context, plainDB, resultSet->pos = position))
                  {
                     whereClause->resultSet = resultSet;
                     return booleanTreeEvaluateJoin(context, whereClause->expressionTree, rsList, totalRs, heap);
                  }
                  else
                     return -1;
            }
            else
            {
               // OR case - Walks through all records. If the corresponding bit is set in the bitmap, does not need to evaluate WHERE clause.
               // Otherwise, checks if row satisfies WHERE clause.
               while (resultSet->pos < rowCountLess1 && plainRead(context, plainDB, ++resultSet->pos))
               {
                  if (IntVectorisBitSet(&rowsBitmap, resultSet->pos))
                     return VALIDATION_RECORD_OK;
                  whereClause->resultSet = resultSet;
                  if (recordNotDeleted(basbuf) 
                   && (ret = booleanTreeEvaluateJoin(context, whereClause->expressionTree, rsList, totalRs, heap)) != VALIDATION_RECORD_NOT_OK) 
                     return ret;
               }
            }
         }
      }
      return NO_RECORD;
   }
   else
   {
      while (resultSet->pos < rowCountLess1 && plainRead(context, plainDB, ++resultSet->pos))
         if (recordNotDeleted(basbuf))
         {
            if (!(whereClause && verifyWhereCondition))
               return VALIDATION_RECORD_OK;
            
				// juliana@213_2: corrected a bug that could make joins not work with ORs using indices.
				whereClause->resultSet = resultSet;
				if ((ret = booleanTreeEvaluateJoin(context, whereClause->expressionTree, rsList, totalRs, heap)) == VALIDATION_RECORD_NOT_OK 
             && resultSet->whereClause->appliedIndexesBooleanOp == OP_BOOLEAN_OR)
					while (++rsIndex < totalRs)
						if (rsList[rsIndex]->auxRowsBitmap.size || rsList[rsIndex]->rowsBitmap.size)
							return VALIDATION_RECORD_INCOMPLETE; 
            return ret;
         }
      return NO_RECORD;
   }
}

/**
 * Evaluates an expression tree for a join.
 * 
 * @param context The thread context where the function is being executed.
 * @param tree The expression tree to be evaluated.
 * @param rsList The list of the result sets.
 * @param totalRs The number of result sets (tables used in the join) in the result set list.
 * @param heap A heap to allocate temporary structures.
 * @return <code>VALIDATION_RECORD_OK</code>, <code>NO_RECORD</code>, <code>VALIDATION_RECORD_NOT_OK</code>,
 * <code>VALIDATION_RECORD_INCOMPLETE</code>, or -1 if an error occurs.
 */
int32 booleanTreeEvaluateJoin(Context context, SQLBooleanClauseTree* tree, ResultSet** rsList, int32 totalRs, Heap heap)
{
	TRACE("booleanTreeEvaluateJoin")
   ResultSet* resultSet = tree->booleanClause->resultSet;
   SQLBooleanClauseTree* leftTree = tree->leftTree;
   SQLBooleanClauseTree* rightTree = tree->rightTree;
   int32 indexRs = resultSet->indexRs,
         indexTree = tree->indexRs;

   if (indexTree >= 0) // AND, OR and BOOLEAN_NOT have index = -1.
   {
      if (indexTree < indexRs) // It was avaliated before and can return true.
         return VALIDATION_RECORD_INCOMPLETE_OK;
      if (indexTree > indexRs) // juliana@211_5: solved a bug with joins which would return more answers than desired.
      {
         if (tree->bothAreIdentifier && leftTree->indexRs == indexRs) // Fills leftTree.value.
         {
            SQLValue* valueJoin = &leftTree->valueJoin;
            if (!valueJoin->asChars)
               valueJoin->asChars = (JCharP)TC_heapAlloc(heap, 2 * resultSet->table->columnSizes[leftTree->colIndex] + 2);
            if (!getOperandValue(context, leftTree, valueJoin)) 
               return -1;
				if (rightTree->hasIndex && !rsList[indexRs]->whereClause->appliedIndexesBooleanOp)
            {
               // juliana@225_13: join now behaves well with functions in columns with an index.
               SQLBooleanClause* booleanClause = tree->booleanClause;
               SQLResultSetField** fieldList = booleanClause->fieldList;
               int32 i = booleanClause->fieldsCount;
               
               while (--i >= 0)
                  if (fieldList[i]->tableColIndex == rightTree->colIndex && fieldList[i]->isDataTypeFunction)
                     return VALIDATION_RECORD_INCOMPLETE;

               // Despite this is a join the parameter 'false' is sent because this is a simple index calculation.
               if (!computeIndex(context, rsList, totalRs, false, rightTree->indexRs, valueJoin, tree->operandType, rightTree->colIndex, heap))
                  return -1;
            }
         }
         return VALIDATION_RECORD_INCOMPLETE;
      }
   }

   // The indexes match, so compare the records.

   switch (tree->operandType) // Checks what is the operand type of the tree.
   {
      // Relational operand.
      case OP_REL_EQUAL:
      case OP_REL_DIFF:
      case OP_REL_GREATER:
      case OP_REL_LESS:
      case OP_REL_GREATER_EQUAL:
      case OP_REL_LESS_EQUAL:
         switch (tree->valueType) // Calls the right operation accordingly to the values type.
         {
            case SHORT_TYPE:
            case INT_TYPE:
            case LONG_TYPE:
            case FLOAT_TYPE:
            case DOUBLE_TYPE:
            case DATE_TYPE:
            case DATETIME_TYPE:
               return compareNumericOperands(context, tree)? VALIDATION_RECORD_OK: VALIDATION_RECORD_NOT_OK;
            case BLOB_TYPE: // A blob can't be in a where clause.
               TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_BLOB_WHERE));
               return -1;
            case CHARS_TYPE:
               return compareStringOperands(context, tree, false, heap)? VALIDATION_RECORD_OK: VALIDATION_RECORD_NOT_OK;
            case CHARS_NOCASE_TYPE:
               return compareStringOperands(context, tree, true, heap)? VALIDATION_RECORD_OK: VALIDATION_RECORD_NOT_OK;
         }

      // juliana@201_4: joins with like were returning the opposite result.
      // Relational operand.
      case OP_PAT_MATCH_LIKE:
      case OP_PAT_MATCH_NOT_LIKE:
         return matchStringOperands(context, tree, tree->valueType == CHARS_NOCASE_TYPE, heap)? VALIDATION_RECORD_OK: VALIDATION_RECORD_NOT_OK;
      
      case OP_BOOLEAN_AND: // AND connector.
         if (leftTree && rightTree) // Expects both trees not be null.
         {
            switch (booleanTreeEvaluateJoin(context, leftTree, rsList, totalRs, heap))
            {
               case VALIDATION_RECORD_NOT_OK: 
                  return VALIDATION_RECORD_NOT_OK;
               case VALIDATION_RECORD_INCOMPLETE:
                  if (booleanTreeEvaluateJoin(context, rightTree, rsList, totalRs, heap) == VALIDATION_RECORD_NOT_OK) // Verifies the right branch. 
                     return VALIDATION_RECORD_NOT_OK;

                  // All other results return incomplete because the left side has returned incomplete.
                  return VALIDATION_RECORD_INCOMPLETE;
               case VALIDATION_RECORD_INCOMPLETE_OK:
               case VALIDATION_RECORD_OK:
                  switch (booleanTreeEvaluateJoin(context, rightTree, rsList, totalRs, heap)) // The left side returned true, so verifies the right branch.
                  {
                     case VALIDATION_RECORD_NOT_OK: 
                        return VALIDATION_RECORD_NOT_OK;
                     case VALIDATION_RECORD_INCOMPLETE_OK:
                     case VALIDATION_RECORD_OK: 
                        return VALIDATION_RECORD_OK; // Both sides returns true.
                     case VALIDATION_RECORD_INCOMPLETE:

                        // If the right side returns incomplete, incomplete must be returned, despite the left side returned OK.
                        return VALIDATION_RECORD_INCOMPLETE;
                  }
            }
         }

      case OP_BOOLEAN_OR: // OR connector.
         if (leftTree && rightTree) // Expects both trees to be not null.
         {
            switch (booleanTreeEvaluateJoin(context, leftTree, rsList, totalRs, heap))
            {
               case VALIDATION_RECORD_OK: 
                  return VALIDATION_RECORD_OK; // Short circuit.
               case VALIDATION_RECORD_INCOMPLETE_OK:
                  switch (booleanTreeEvaluateJoin(context, rightTree, rsList, totalRs, heap)) // Verifies the right branch.
                  {
                     case VALIDATION_RECORD_NOT_OK: 
                        return VALIDATION_RECORD_NOT_OK;
                     case VALIDATION_RECORD_INCOMPLETE: 
                        return VALIDATION_RECORD_INCOMPLETE;
                     case VALIDATION_RECORD_OK:
                     case VALIDATION_RECORD_INCOMPLETE_OK: 
                        return VALIDATION_RECORD_OK; // The right side returned true.
                  }
               case VALIDATION_RECORD_INCOMPLETE:
                  switch (booleanTreeEvaluateJoin(context, rightTree, rsList, totalRs, heap)) // Verifies the right branch.
                  {
                     case VALIDATION_RECORD_NOT_OK:
                     case VALIDATION_RECORD_INCOMPLETE: 
                        return VALIDATION_RECORD_INCOMPLETE;
                     case VALIDATION_RECORD_OK:
                     case VALIDATION_RECORD_INCOMPLETE_OK: 
                        return VALIDATION_RECORD_OK; // The right side returned true.
                  }
               case VALIDATION_RECORD_NOT_OK:
                  // The left side returned false, so continues verifing the right branch.
                  switch (booleanTreeEvaluateJoin(context, rightTree, rsList, totalRs, heap))
                  {
                     case VALIDATION_RECORD_NOT_OK:
                     case VALIDATION_RECORD_INCOMPLETE_OK: 
                        return VALIDATION_RECORD_NOT_OK;
                     case VALIDATION_RECORD_OK: 
                        return VALIDATION_RECORD_OK; // The right side returned true.
                     case VALIDATION_RECORD_INCOMPLETE: 
                        return VALIDATION_RECORD_INCOMPLETE;
                  }
            }
         }

      // juliana@214_4: nots were removed.

      // IS and IS NOT.
      case OP_PAT_IS:
      case OP_PAT_IS_NOT:
         return compareNullOperands(tree)? VALIDATION_RECORD_OK : VALIDATION_RECORD_NOT_OK;
   }

   return VALIDATION_RECORD_INCOMPLETE;
}

/**
 * Calculates aggregation functions. 
 *
 * @param context The thread context where the function is being executed.
 * @param record The record of the values to be used in the calculation.
 * @param nullsRecord The values of the record that are null.
 * @param aggFunctionsRunTotals The current totals for the aggregation functions.
 * @param aggFunctionsCodes The codes of the used aggregation functions.
 * @param aggFunctionsParamCols The columns that use aggregation functions.
 * @param aggFunctionsColsCount The number of columns that use aggregation functions.
 * @param columnTypes The types of the columns.
 * @param groupCountCols The columns that use count. 
 */
void performAggFunctionsCalc(Context context, SQLValue** record, uint8* nullsRecord, SQLValue* aggFunctionsRunTotals, int32* aggFunctionsCodes, 
                                              int32* aggFunctionsParamCols, int32 aggFunctionsColsCount, int32* columnTypes, int32* groupCountCols)
{
   TRACE("performAggFunctionsCalc")
	int32 i = aggFunctionsColsCount,
         colIndex,
         sqlAggFunction, 
         colType;
   double doubleVal;
   SQLValue* aggValue;
   SQLValue* value;

   while (--i >= 0) // Performs the calculation of the aggregation functions.
   {
      doubleVal = 0;
      colIndex = aggFunctionsParamCols[i];

      if (!aggFunctionsCodes[i])
      {
         groupCountCols[i]++;
         continue;
      }
      if (colIndex < 0)
         continue;

      if (isBitSet(nullsRecord, colIndex)) 
         continue;

      groupCountCols[i]++;

      sqlAggFunction = aggFunctionsCodes[i];
      colType = columnTypes[colIndex];
      aggValue = &aggFunctionsRunTotals[i];
      value = record[colIndex];

      switch (sqlAggFunction)
      {
         case FUNCTION_AGG_AVG:
         case FUNCTION_AGG_SUM:
         {
            switch (colType) // Checks the type of the column.
            {
               case SHORT_TYPE:
                  aggValue->asDouble += value->asShort;
                  break;

               case FLOAT_TYPE:
                  aggValue->asDouble += value->asFloat;
                  break;

               case INT_TYPE:
                  aggValue->asDouble += value->asInt;
                  break;

               case LONG_TYPE:
                  aggValue->asDouble += value->asLong;
                  break;

               case DOUBLE_TYPE:
                  aggValue->asDouble += value->asDouble;
                  break;

               case DATE_TYPE: // rnovais@567_2
               case DATETIME_TYPE:
                  TC_throwExceptionNamed(context, "litebase.SQLParseException", getMessage(ERR_SUM_AVG_WITH_DATE_DATETIME));
                  return;
            }
            break;
         }

         // juliana@226_5: the aggregation functions MAX() and MIN() now work for CHAR, VARCHAR, CHAR NOCASE, and VARCHAR NOCASE column types.
         case FUNCTION_AGG_MAX: 
         {
            switch (colType) // Checks the type of the column.
            {
               // juliana@226_9: strings are not loaded anymore in the temporary table when building result sets. 
               case CHARS_TYPE:
               case CHARS_NOCASE_TYPE:
                  if (groupCountCols[i] == 1 
                   || str16CompareTo(aggValue->asChars, value->asChars, aggValue->length, value->length, colType) < 0)
                  {
                     xmemmove(aggValue->asChars, value->asChars, (aggValue->length = value->length) << 1);
                     aggValue->asInt = value->asInt;
                     aggValue->asBlob = value->asBlob;
                  }
                  break;

               case SHORT_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asShort < value->asShort)
                     aggValue->asShort = value->asShort;
                  break;

               case FLOAT_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asFloat < value->asFloat)
                     aggValue->asFloat = value->asFloat;
                  break;

               case DATE_TYPE: // rnovais@567_2
               case INT_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asInt < value->asInt)
                     aggValue->asInt = value->asInt;
                  break;

               case LONG_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asLong < value->asLong)
                     aggValue->asLong = value->asLong;
                  break;

               case DOUBLE_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asDouble < value->asDouble)
                     aggValue->asDouble = value->asDouble;
                  break;

               case DATETIME_TYPE: // rnovais@567_2
                  if (groupCountCols[i] == 1)
                  {
                     aggValue->asDate = value->asDate;
                     aggValue->asTime = value->asTime;
                  }
                  else if (aggValue->asDate <= value->asDate)
                  {
                     if (aggValue->asDate < value->asDate || aggValue->asTime < value->asTime) // The date or time is smaller.
                     {
                        aggValue->asDate = value->asDate;
                        aggValue->asTime = value->asTime;
                     }
                  }
                  break;
            }
            break;
         }

         // juliana@226_5: the aggregation functions MAX() and MIN() now work for CHAR, VARCHAR, CHAR NOCASE, and VARCHAR NOCASE column types.
         case FUNCTION_AGG_MIN:
         {
            switch (colType) // Checks the type of the column.
            {
               // juliana@226_9: strings are not loaded anymore in the temporary table when building result sets. 
               case CHARS_TYPE:
               case CHARS_NOCASE_TYPE:
                  if (groupCountCols[i] == 1 
                   || str16CompareTo(aggValue->asChars, value->asChars, aggValue->length, value->length, colType) > 0)
                  {
                     xmemmove(aggValue->asChars, value->asChars, (aggValue->length = value->length) << 1);
                     aggValue->asInt = value->asInt;
                     aggValue->asBlob = value->asBlob;
                  }
                  break; 

               case SHORT_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asShort > value->asShort)
                     aggValue->asShort = value->asShort;
                  break;

               case FLOAT_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asFloat > value->asFloat)
                     aggValue->asFloat = value->asFloat;
                  break;

               case DATE_TYPE:
               case INT_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asInt > value->asInt)
                     aggValue->asInt = value->asInt;
                  break;

               case LONG_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asLong > value->asLong)
                     aggValue->asLong = value->asLong;
                  break;

               case DOUBLE_TYPE:
                  if (groupCountCols[i] == 1 || aggValue->asDouble > value->asDouble) 
                     aggValue->asDouble = value->asDouble;
                  break;

               case DATETIME_TYPE:
                  if (groupCountCols[i] == 1)
                  {
                     aggValue->asDate = value->asDate;
                     aggValue->asTime = value->asTime;
                  }
                  else
                  if (aggValue->asDate >= value->asDate)
                  {
                     if (aggValue->asDate > value->asDate || aggValue->asTime > value->asTime) // The date is greater.
                     {
                        aggValue->asDate = value->asDate;
                        aggValue->asTime = value->asTime;
                     }
                  }
                  break;
            }
            break;
         }
      }
   }
}

// juliana@230_14: removed temporary tables when there is no join, group by, order by, and aggregation.
/**
 * Calculates the answer of a select without aggregation, join, order by, or group by without using a temporary table.
 * 
 * @param context The thread context where the function is being executed.
 * @param resultSet The result set of the table.
 * @param heap A heap to allocate temporary structures.
 */
void computeAnswer(Context context, ResultSet* resultSet, Heap heap)
{
   int32 i;
   Table* table = resultSet->table;
   uint8* allRowsBitmap = table->allRowsBitmap;

   if (!resultSet->whereClause && !resultSet->rowsBitmap.size && !table->deletedRowsCount)
   {
      i = table->answerCount = table->db->rowCount;
      while (--i >= 0)
         setBitOn(allRowsBitmap, i);
   }
   else
   {
      i = 0;
      while (getNextRecord(context, resultSet, heap)) // No preverify needed.
      {
         setBitOn(allRowsBitmap, resultSet->pos);
         i++;
      }
      table->answerCount = i;
   }
}

// juliana@230_21: MAX() and MIN() now use indices on simple queries.
/**
 * Finds the best index to use in a min() or max() operation.
 *
 * @param field The field which may have a min() or max() operation.
 */
void findMaxMinIndex(SQLResultSetField* field)
{
   Table* table = field->table;
   int32 column = field->parameter->tableColIndex,
         i = table->numberComposedIndexes;
   ComposedIndex** composedIndices = table->composedIndexes;
      
   if (table->columnIndexes[column]) // If the field has a simple index, uses it.
   {
       field->index = column;
       field->isComposed = false;
   }
   else
   {
      while (--i >= 0)
      {
         if (*composedIndices[i]->columns == column) // Else, if the field is the first field of a composed index, uses it.
         {
            field->index = i;
            field->isComposed = true;
            break;
         }
      }
   }
   if (i == -1)
      field->index = -1;
}
