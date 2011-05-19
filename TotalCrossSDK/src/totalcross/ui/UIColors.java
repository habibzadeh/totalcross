/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2011 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *  This file is covered by the GNU LESSER GENERAL PUBLIC LICENSE VERSION 3.0    *
 *  A copy of this license is located in file license.txt at the root of this    *
 *  SDK or can be downloaded here:                                               *
 *  http://www.gnu.org/licenses/lgpl-3.0.txt                                     *
 *                                                                               *
 *********************************************************************************/



package totalcross.ui;

import totalcross.sys.*;
import totalcross.ui.gfx.*;

/** This class holds the colors used in some user interface dialogs, such as MessageBox,
 * InputDialog, Calculator, Keyboard, Calendar, ToolTip, KeyPad and others.
 * You can customize it as needed.
 * <p>
 * To correctly change the colors to your own, you must do it in the constructor of your
 * application.
 *
 * @since SuperWaba 5.64
 */
public final class UIColors // guich@564_6
{
   private UIColors() {}

   /** KeyboardBox background color. */
   public static int keyboardBack = 0xE6E68C;
   /** KeyboardBox foreground color. */
   public static int keyboardFore = 0xBEE68C;
   /** KeyboardBox action color. */
   public static int keyboardAction = 0xFAB428;

   /** CalculatorBox background color. */
   public static int calculatorBack = keyboardBack;
   /** CalculatorBox foreground color. */
   public static int calculatorFore = keyboardFore;
   /** CalculatorBox action color. */
   public static int calculatorAction = keyboardAction;

   /** CalendarBox background color. */
   public static int calendarBack = keyboardBack;
   /** CalendarBox foreground color. */
   public static int calendarFore = Color.BLACK;
   /** CalendarBox action color. */
   public static int calendarAction = keyboardAction;
   /** CalendarBox arrow colors. */
   public static int calendarArrows = Color.BLACK;

   /** MessageBox background color. */
   public static int messageboxBack = Color.RED;
   /** MessageBox foreground color. */
   public static int messageboxFore = 0xE6E6E6;
   /** MessageBox action color. */
   public static int messageboxAction = Color.WHITE;

   /** InputBox background color. */
   public static int inputboxBack = keyboardBack;
   /** InputBox foreground color. */
   public static int inputboxFore = Color.BLACK;
   /** InputBox action color. */
   public static int inputboxAction = keyboardAction;

   /** ColorChooserBox background color. */
   public static int colorchooserboxBack = keyboardBack;
   /** ColorChooserBox foreground color. */
   public static int colorchooserboxFore = Color.BLACK;
   /** ColorChooserBox action color. */
   public static int colorchooserboxAction = keyboardAction;

   /** ToolTip background color. */
   public static int tooltipBack = Color.YELLOW;
   /** ToolTip foreground color. */
   public static int tooltipFore = Color.BLACK;

   /** Keypad background color. */
   public static int keypadBack = Color.YELLOW;
   /** Keypad foreground color. */
   public static int keypadFore = Color.BLACK;

   /** Default control foreground color. */
   public static int controlsFore = Color.BLACK;
   /** Default control background color. */
   public static int controlsBack = Color.BRIGHT;

   /** Make the edit area have the same color of the background setting this to true */
   public static boolean sameColors; // guich@572_15

   /** These are the colors used to draw the highlight rectangle. */
   public static int[] highlightColors = (Settings.screenWidth > 200) // guich@573_23  - guich@580_
       ? new int[]{Color.GREEN,Color.GREEN,Color.CYAN,Color.CYAN,Color.WHITE,Color.WHITE}
       : new int[]{Color.GREEN,Color.CYAN,Color.WHITE};
   /** The default step used on Vista buttons to make the fade. Decrease the step to make the button lighter. */
   public static int vistaFadeStep = Settings.screenBPP == 16 ? 8 : 5;

   /** FileChooser foreground color. */
   public static int fileChooserFore = Color.BLACK;
   /** FileChooser background color. */
   public static int fileChooserBack = 0xEEEEAA;

   /** HtmlContainer background color for the Form controls. */
   public static int htmlContainerControlsFore = Color.BLACK;
   /** HtmlContainer foreground color for the Form controls. */
   public static int htmlContainerControlsBack = Color.WHITE;
   /** HtmlContainer link foreground color. */
   public static int htmlContainerLink = Color.BLUE;
   
   /** TimeBox visor's background color. */
   public static int timeboxVisorBack = Color.WHITE;
   /** TimeBox visor's cursor color. */
   public static int timeboxVisorCursor = Color.YELLOW;
   /** TimeBox OK button color. */
   public static int timeboxOk = Color.GREEN;
   /** TimeBox Clear button color. */
   public static int timeboxClear = Color.RED;   
   /** TimeBox background color. */
   public static int timeboxBack = keyboardBack;
   
   /** Default value to be used in all textShadowColor(s) set in the constructor of a control. Defaults to -1.
    * Note that it does not affect the shadow when you call setBackColor or setForeColor.
    * @see Control#BRIGHTER_BACKGROUND
    * @see Control#DARKER_BACKGROUND
    * @since TotalCross 1.27
    */
   public static int textShadowColor = -1;
   
   /** The color of the PositionBar for all places that use it. */
   public static int positionbarColor = Color.DARK;
   
   /** The color to fill the background of the PositionBar. Defaults to -1 (don't fill). */
   public static int positionbarBackgroundColor = -1;
   
   /** Spinner foreground color. */
   public static int spinnerFore = controlsFore;
   /** Spinner background color. */
   public static int spinnerBack = -1;
}
