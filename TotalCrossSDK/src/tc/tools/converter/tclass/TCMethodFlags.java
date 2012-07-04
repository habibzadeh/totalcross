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



package tc.tools.converter.tclass;

import totalcross.io.DataStreamLE;

public final class TCMethodFlags
{
   // Java access flags
   public boolean /*uint16*/ isPublic       ;// 1;
   public boolean /*uint16*/ isPrivate      ;// 1;
   public boolean /*uint16*/ isProtected    ;// 1;
   public boolean /*uint16*/ isStatic       ;// 1;
   public boolean /*uint16*/ isFinal        ;// 1;
   public boolean /*uint16*/ isNative       ;// 1;
   public boolean /*uint16*/ isAbstract     ;// 1;

   public void write(DataStreamLE ds) throws totalcross.io.IOException
   {
      int v = ((isPublic       ?1:0)     ) |
              ((isPrivate      ?1:0) << 1) |
              ((isProtected    ?1:0) << 2) |
              ((isStatic       ?1:0) << 3) |
              ((isFinal        ?1:0) << 4) |
              ((isNative       ?1:0) << 5) |
              ((isAbstract     ?1:0) << 6);
      ds.writeShort(v);
   }
}