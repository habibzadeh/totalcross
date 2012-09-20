/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *********************************************************************************/



package tc.tools.converter.oper;

public class OperandExternal extends Operand
{
   public OperandReg regO;
   public OperandSym sym;

   public OperandExternal(OperandReg reg, OperandSym sym)
   {
      super(sym.kind);
      regO = reg;
      this.sym = sym;
      nWords = sym.nWords;
   }
}