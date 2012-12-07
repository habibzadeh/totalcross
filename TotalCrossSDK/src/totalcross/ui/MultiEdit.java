/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2001 Jean Rissoto                                              *
 *  Copyright (C) 2001-2012 SuperWaba Ltda.                                      *
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

import totalcross.ui.dialog.*;
import totalcross.ui.event.*;
import totalcross.ui.gfx.*;
import totalcross.ui.image.*;
import totalcross.ui.media.*;
import totalcross.sys.*;
import totalcross.util.*;

/**
 * MultiEdit is an Edit with support for multiple lines. A static scrollbar is added, but disabled/enabled as needed.
 * <p>
 * Here is an example showing an Edit control being used:
 *
 * <pre>
 * import totalcross.ui.*;
 *
 * public class MyProgram extends MainWindow
 * {
 *    MultiEdit mEdit;
 *
 *    public void initUI()
 *    {
 *       // the constructor method is called with the mask, the number of lines
 *       // and the vertical interval in pixel between two lines
 *       mEdit = new MultiEdit("",3,1);
 *       add(mEdit,LEFT,TOP);
 *       // add/setRect must precede setText
 *       mEdit.setText("What text you want"); // eventually
 *    }
 * }
 * </pre>
 * If the MultiEdit is not editable, the user can scroll the edit a page at a time
 * just by clicking in the middle upper or middle lower.
 * @author Jean Rissoto (in memoriam)
 * @author Guilherme Campos Hazan (guich)
 */


public class MultiEdit extends Container implements Scrollable
{
   private static final char ENTER = '\n';
   private static final char LINEFEED = '\r';
   private int lastZ1y = -9999;
   private Graphics drawg; // only valid while the edit has focus --   original
   private TimerEvent blinkTimer; // only valid while the edit has focus --   original
   private boolean hasFocus;
   private boolean cursorShowing;
   boolean firstPenDown;
   protected boolean editable = true;
   /** Set to false if you don't want the cursor to blink when the edit is not editable */
   public static boolean hasCursorWhenNotEditable = true; // guich@340_23
   /** Set to true if you want the control to decide whether to gain/lose focus automatically, without having to press ACTION. */ 
   protected boolean improvedGeographicalFocus;

   protected IntVector first = new IntVector(5); //JR@0.4.  indices of first character of each line. the value of last is len(text)+1
   private boolean forceDrawAll;
   private int firstToDraw; //JR@0.4
   private int numberTextLines; // JR@0.4
   private int hLine; // JR@0.4. height of a line
   private static Coord z1 = new Coord(), z2 = new Coord();
   private Coord z3 = new Coord();
   private Rect boardRect;
   private Rect textRect;
   protected StringBuffer chars = new StringBuffer(100);
   private int insertPos;
   private int startSelectPos;
   private int newInsertPos;
   private int pushedInsertPos; //protected in Edit
   private int pushedStartSelectPos; //protected in Edit
   protected ScrollBar sb;
   private int spaceBetweenLines;//
   private int tempGap, tempRowCount, tempRowCount0; // used on popup keyboard
   private int fColor,back0,back1; //JR@0.8
   private int fourColors[] = new int[4]; //JR@0.8
   private byte kbdType=Edit.KBD_KEYBOARD;
   private byte lastKbdType=Edit.KBD_KEYBOARD; //vik@421_24
   private String validChars;
   private int maxLength; // guich@200b4
   private int lastCommand;
   private int dragDistance;
   private boolean isScrolling;
   private boolean popupVKbd;
   /** While using geographical focus, editMode is toggled using the Action Key to allow navigation
   * and editing of the text inside the MultiEdit **/
   private boolean editMode = true;
   private boolean editModeValue = true; // guich@tc115_6
   private String mapFrom,mapTo; // guich@tc110_56
   private int oldTabIndex=-1;
   private boolean ignoreNextFocusIn;
   private Image npback;
   private int rowCount0=-1;
   private boolean scScrolled;
   int lastPenDown=-1;
   private static KeyEvent backspaceEvent = new KeyEvent(KeyEvent.SPECIAL_KEY_PRESS,SpecialKeys.BACKSPACE,0);

   private boolean scrollBarsAlwaysVisible;
   /** The mask used to infer the preferred width. Unlike the Edit class, the MultiEdit does not support real masking. */
   public String mask;

   /** Used to set the number of rows of this MultiEdit; used as parameter to compute the preferred height.
    * You must call setRect after changing this to resize the control in height.
    */
   public int rowCount; // guich@320_28

   /** Sets the capitalise settings for this MultiEdit. Text entered will made as is, uppercase or
    * lowercase
    * @see totalcross.ui.Edit#ALL_NORMAL
    * @see totalcross.ui.Edit#ALL_UPPER
    * @see totalcross.ui.Edit#ALL_LOWER
    */
   public byte capitalise;

   /** If set to true, the text will be auto-selected when the focus enters.
    * True by default on penless devices. */
   public boolean autoSelect = Settings.keyboardFocusTraversable; // guic@tc130

   /** If true, a dotted line appears under each row of text (on by default) */
   public boolean drawDots = true;

   /** The gap between the rows. */
   public int gap=1; // guich@320_28: made public to remove 2 almost unused methods

   /** Set to true to justify the text when the MultiEdit is NOT editable.
    * Note that this makes the text drawing a bit slower. */
   public boolean justify;

   /** The Flick object listens and performs flick animations on PenUp events when appropriate. */
   protected Flick flick;

   /** Constructs a MultiEdit with 1 pixel as space between lines and with no lines.
    * You must set the bounds using FILL or FIT. */
   public MultiEdit()
   {
      this(0, 1);
   }

   /** Constructor for a text Edit with a vertical scroll Bar, gap is 1
    by default and control's bounds must be specified with a setRect. Space between lines may be 0. */
   public MultiEdit(int rowCount, int spaceBetweenLines)  // vertical space between 2 lines in pixel
   {
      ignoreOnAddAgain = ignoreOnRemove = true;
      this.started = true;
      this.rowCount = rowCount;
      this.hLine = fmH + spaceBetweenLines;
      this.spaceBetweenLines = spaceBetweenLines;
      this.clearPosState();
      add(this.sb = Settings.fingerTouch ? new ScrollPosition(ScrollBar.VERTICAL) : new ScrollBar(ScrollBar.VERTICAL));
      if (!Settings.fingerTouch)
      {
         sb.setLiveScrolling(true);
         // don't let the scrollbar steal focus from us
         sb.setEnabled(false);      // gao - leave this disabled for visual effect until we know we need it
         sb.setFocusLess(true);
         sb.focusTraversable = false;
         sb.setVisible(false);
      }
      this.focusTraversable = true; // kmeehl@tc100
      if (Settings.fingerTouch)
         flick = new Flick(this);
   }

   /** Constructor for a text Edit with a vertical scroll Bar, gap is 1
     * by default and control's bounds must be given with a setRect. Space between lines may be 0.
     * The mask is used to compute the PREFERRED width of the control. Note that the mask does not <i>masks</i>
     * the input. If mask is "", the FILL width is choosen.
     */
   public MultiEdit(String mask, int rowCount, int spaceBetweenLines)
   {
      this(rowCount, spaceBetweenLines);
      this.mask = mask;
   }

   public boolean flickStarted()
   {
      dragDistance = 0;
      return isScrolling;
   }
   
   public void flickEnded(boolean atPenDown)
   {
   }
   
   public boolean canScrollContent(int direction, Object target)
   {
      if (Settings.fingerTouch)
         switch (direction)
         {
            case DragEvent.UP: return firstToDraw > 0;
            case DragEvent.DOWN: return (firstToDraw + rowCount) < numberTextLines;
         }
      return false;
   }
   
   public boolean scrollContent(int xDelta, int yDelta)
   {
      if (Math.abs(xDelta) > Math.abs(yDelta)) // MultiEdit has only vertical scrolling
         return false;
      
      int lastFirstToDrawLine = numberTextLines - rowCount;
      if (lastFirstToDrawLine <= 0 || (yDelta < 0 && firstToDraw == 0) || (yDelta > 0 && firstToDraw >= lastFirstToDrawLine)) // already at the top/bottom of the view window
         return false;
      
      dragDistance += yDelta;
      if ((dragDistance < 0 && dragDistance > -hLine) || (dragDistance >= 0 && dragDistance < hLine)) // not enough to move one single line, store accumulated increment and return
         return true;
      
      int lineDelta = dragDistance / hLine;
      dragDistance %= hLine;
      
      firstToDraw += lineDelta;
      if (firstToDraw < 0)
         firstToDraw = 0;
      else if (firstToDraw > lastFirstToDrawLine)
         firstToDraw = lastFirstToDrawLine;
      
      sb.setValue(firstToDraw);
      forceDrawAll = true;
      newInsertPos = zToCharPos(z1);
      
      Window.needsPaint = true;
      return true;
   }

   public int getScrollPosition(int direction)
   {
      if (direction == DragEvent.LEFT || direction == DragEvent.RIGHT)
         return 0;
      return dragDistance;
   }

   /** Maps the keys in the from char array into the keys in the to char array. For example enable a 'numeric pad'
    * on devices that has the 1 in the u character, you can use this:
    * <pre>
    * ed.mapKeys("uiojklnm!.","1234567890");
    * </pre>
    * To make sure that lowercase characters are also handled, you should also change the capitalise mode:
    * <pre>
    * ed.capitalise = Edit.ALL_LOWER;
    * </pre>
    * If you want to disable a set of keys, use the setValidChars method. Note that mapKeys have precendence over setValidChars.
    * @param from The source keys. Must have the same length of <code>to</code>. Set to null to disable mapping.
    * @param to The destination keys. Must have the same length of <code>from</code>
    * @since TotalCross 1.01
    * @see #setValidChars(String)
    */
   public void mapKeys(String from, String to)
   {
      if (from == null || to == null)
         from = to = null;
      else
      if (from.length() != to.length())
         throw new IllegalArgumentException("from.length must match to.length");
      this.mapFrom = from;
      this.mapTo = to;
   }

   public int getPreferredHeight()
   {
      if (rowCount0 == -1)
         rowCount0 = rowCount;
      return (hLine*rowCount0+ ((uiPalm || uiFlat)?2:4) + 2*gap) + insets.top+insets.bottom; //+2= minimal space between 2 lines
   }

   public int getPreferredWidth()
   {
      return (mask==null?(totalcross.sys.Settings.screenWidth>>2):(mask.length()==0)?FILL:(fm.stringWidth(mask) + (uiPalm?4:10))) + insets.left+insets.right; // guich@200b4_202: from 2 -> 4 is PalmOS style - guic@300_52: empty mask means FILL
   }

   /** Sets the desired maximum length for text entered in the Edit.
    * @since SuperWaba 2.0 beta 4 */
   public void setMaxLength(int length)
   {
      maxLength = length;
      if (length != 0 && maxLength < chars.length())  // jescoto@421_15: resize text if maxLength < len
         chars.setLength(length);
   }

   /** Used to change the default keyboard to be used with this Edit control.
     * Use the constants Edit.KBD_NONE and Edit.KBD_KEYBOARD.
     */
   public void setKeyboard(byte kbd)
   {
      this.lastKbdType=this.kbdType = kbd == Edit.KBD_NONE ? Edit.KBD_NONE : Edit.KBD_KEYBOARD; //vik@421_24
   }

   /** sets the valid chars that can be entered in this edit. if null is passed, any char can be entered. (case insensitive). */
   public void setValidChars(String validCharsString)
   {
      if (validCharsString != null)
         validChars = validCharsString.toUpperCase();
      else
         validChars = null;
   }

   /**
    * Returns the text displayed in the edit control.
    */
   public String getText()
   {
      return chars.toString();
   }

   /** Returns the text's buffer. Do NOT change the buffer contents, since changing it
    * will not affect the char widths array, thus, leading to a wrong display.
    * @since TotalCross 1.0
    */
   public StringBuffer getTextBuffer()
   {
      return chars;
   }

   /**
    * Sets the text displayed in the edit control.
    */
   public void setText(String s)
   {
      setText(s,Settings.sendPressEventOnChange);
   }

   /**
    * Sets the text displayed in the edit control.
    */
   public void setText(String s, boolean postPressed)
   {
      chars = new StringBuffer(Convert.replace(s, Convert.CRLF,"\n"));
      newInsertPos = numberTextLines = 0;
      if (textRect != null)
         calculateFirst();
      forceDrawAll=true;
      clearPosState();
      if (postPressed)
         postPressedEvent();
   }

   /** Sets if the control accepts input from the user.
    Note: If set to uneditable, keyboard is disabled. */
   public void setEditable(boolean on)
   {
      editable = on;
      kbdType = on ? this.lastKbdType:Edit.KBD_NONE;  //vik@421_24
   }

   /** Gets if the control accepts input from the user */
   public boolean isEditable()
   {
      return editable;
   }

   /** Gets total number of lines in the text */
   public int getNumberOfTextLines()
   {
      return numberTextLines;
   }

   /** Set to true to hide the vertical scrollbar when it isn't needed (instead of disabling it).
    * This must be done right after the constructor.
    * @since TotalCross 1.0
    */
   public void setScrollbarsAlwaysVisible(boolean asNeeded)
   {
      scrollBarsAlwaysVisible = asNeeded;
      if (!Settings.fingerTouch) sb.setVisible(asNeeded);
   }

   /** user method to popup the keyboard/calendar/calculator for this edit. */
   public void popupKCC()
   {
      if (kbdType == Edit.KBD_NONE || !editable || !enabled) return;
      if (Settings.virtualKeyboard)
         _onEvent(new Event(ControlEvent.FOCUS_IN,this,0)); // simulate a focus in event.
      else
      {
         if (Edit.keyboard == null) Edit.keyboard = new KeyboardBox();
         showInputWindow(Edit.keyboard);
      }
   }

   private void showInputWindow(Window w)
   {
      oldTabIndex = parent.tabOrder.indexOf(this);
      requestFocus(); // guich@200b4: bring focus back
      pushPosState();
      if (removeTimer(blinkTimer)) // guich@200b4_167
         blinkTimer = null;
      // guich@320: modify and restore later our state, bc the Keyboard needs a shrinked control
      tempGap = gap;
      tempRowCount = rowCount;
      tempRowCount0 = rowCount0;
      gap = 0;
      rowCount0 = rowCount = 2;
      w.popupNonBlocking();
      popPosState();
      requestFocus();
   }

   private int zToCharPos(Coord z)
   {
      z.y = Math.max(0, Math.min(this.height-1, z.y));
      int line = firstToDraw + (z.y - textRect.y) / hLine; // what's the line?
      if (line >= numberTextLines)
         line  = Math.max(numberTextLines-1,0);
      z.x = Math.max(0, Math.min(fm.sbWidth(chars,first.items[line],first.items[line+1]-first.items[line]), z.x));
      return Convert.getBreakPos(fm, chars, first.items[line], z.x, false);
   }

   private void charPosToZ(int n, Coord z)
   {
      z.x = textRect.x;
      z.y = textRect.y;
      int len = chars.length();
      if (len == 0 || n == 0) // no string or pos 0?
         return;
      if (n > len)
         n = len;

      int i,mid,end = first.size();
      for (i = 0;end-i > 1 ;) // kmeehl@tc100: compute the new line using binary search
      {
         mid = i + (end-i)/2;
         if (n > first.items[mid])
            i = mid;
         else
            end = mid;
      }

      //if (n == first.items[i+1]) i++; else - if char is at last space, put it in the next line - note that this doesn't work bc when pressing down key it skips 2 lines
      z.x += fm.sbWidth(chars,first.items[i],n-first.items[i]);
      z.y += (i-firstToDraw) * hLine;
   }

   protected void onBoundsChanged(boolean screenChanged)
   {
      drawg = null;
      int zOffset = (uiPalm || uiFlat)?0:2; // size of borders
      boardRect = new Rect(zOffset,zOffset,this.width-2*zOffset-(Settings.fingerTouch?0:sb.getPreferredWidth()),this.height-2*zOffset);    //JR @0.5
      textRect = boardRect.modifiedBy(gap,gap,-2*gap,-2*gap);
      rowCount = textRect.height / this.hLine; // kambiz@350_5: update rowCount according to the new size of the text area
      sb.setRect(RIGHT-(Settings.fingerTouch ? 1 : 0),TOP,PREFERRED,FILL, null, screenChanged);
      sb.setValues(0, rowCount, 0, rowCount);
      numberTextLines = 0;
      firstToDraw = 0;
      forceDrawAll = true;
      if (chars.length() > 0)
         calculateFirst();
      npback = null;
   }

   /** Compute the index of the first character of each line */
   private void calculateFirst() // guich@320_28: completely redesigned - guich@581_6: highly optimized
   {
      StringBuffer chars = this.chars; // cache
      int i=0, originalLineCount = numberTextLines;
      first.removeAllElements();
      first.addElement(0); // in line 0, the first char is always 0
      int tw = textRect.width;
      int n = chars.length();
      for (int pos = 0; pos < n; pos++)
      {
         int pos0 = pos == 0 || chars.charAt(pos-1) < ' ' ? pos : pos-1; // guich@tc113_37: when parsing "Update of /pcvsroot/LitebaseSDK/src/native/parser", it was breaking in the first /, but in the next loop iteration, it was skipping the first /, and, thus, computing a character less
         first.addElement(pos = Convert.getBreakPos(fm, chars, pos0, tw, true)); // guich@tc166: we'll take care of the initial space/ENTER during drawing 
      }
      if (n == 0 || chars.charAt(n-1) == ENTER)
         first.addElement(n);
      numberTextLines = first.size()-1;
      //try {for (i =0; i <= numberTextLines; i++) Vm.debug("first["+i+"]: "+first.items[i]+" '"+chars.charAt(first.items[i])+"'");} catch (Exception e) {Vm.debug("first["+i+"]: "+first.items[i]);}

      // has the number of lines changed? enable/disable scrollbar
      if (numberTextLines != originalLineCount)
      {
         forceDrawAll = true;
         boolean needScroll = numberTextLines > rowCount;
         if (!Settings.fingerTouch)
         {
            sb.setEnabled(needScroll);    // gao always visually enable / disable based on needScroll
            if (scrollBarsAlwaysVisible)
               sb.setVisible(true);
            else                    // gao make sure its enabled and visible only when needed
               sb.setVisible(needScroll);
         }
         sb.setMaximum(needScroll ? numberTextLines : 0);
      }

      // compute the new line of the cursor - kmeehl@tc100 changed a bit
      int end = first.size();
      int mid;
      for(i = 0; end-i > 1 ;) // kmeehl - compute the new line using binary search
      {
         mid = i + (end-i)/2;
         if (newInsertPos > first.items[mid])
            i = mid;
         else
            end = mid;
      }
      // change position only if the cursor is out of viewable area
      if (i < firstToDraw)
         firstToDraw = i;
      else
      if (i >= firstToDraw + rowCount) //bruno@tc114_47: fixed scrolling - when typing, the last line was being omitted
      {
         firstToDraw = i - rowCount + 1;
         if (firstToDraw < 0)
            firstToDraw = 0;
      }

      // need to change scrollbar position?
      if (sb.getValue() != firstToDraw)
      {
         forceDrawAll=true;
         sb.setValue(firstToDraw+1);
      }
   }

   private void focusOut()
   {
      if (Settings.virtualKeyboard && Settings.isWindowsDevice() && editable && kbdType != Edit.KBD_NONE) // if running on a PocketPC device, set the bounds of Sip in a way to not cover the edit
      {
         Window.isSipShown = false;
         Window.setSIP(Window.SIP_HIDE,null,false);
      }
      hasFocus = false;
      // see what to do when popup
      if (removeTimer(blinkTimer))
         blinkTimer = null;
      if (cursorShowing) // kambisDarabi@310_7 : remove the cursor, if it is currently shown
         draw(drawg, true);
      hasFocus = false;
      Window w = getParentWindow();
      if ((Settings.keyboardFocusTraversable || Settings.geographicalFocus) && w != null && w == Window.getTopMost()) // guich@tc110_81: remove highlight from us. - guich@tc120_39: only if we're in the topmost window
      {
         //parent.requestFocus(); - guich@tc115_91
         if (w.getFocus() == this) // parent didn't get focus
            w.removeFocus();
         w.setHighlighted(this);
      }
   }

   /** Called by the system to pass events to the edit control. */
   public void onEvent(Event event)
   {
      if (event.type == PenEvent.PEN_DOWN)
         scScrolled = false;
      
      if (event.target == this && textRect != null)
      {
         boolean redraw = false;
         boolean extendSelect = false;
         boolean clearSelect = false;
         newInsertPos = insertPos;
         switch (event.type)
         {
            case TimerEvent.TRIGGERED:
               Window w;
               if (blinkTimer != null && ((w=getParentWindow()) == null || w != Window.topMost)) // must check here and not in the onPaint method, otherwise it results in a problem: show an edit field, then popup a window and move it: the edit field of the other window is no longer being drawn
               {
                  focusOut();
                  event.consumed = true;
                  return;
               }
               if (parent != null && (editMode || Settings.fingerTouch)) 
                  draw(drawg, true);
               // guich@tc130: show the copy/paste menu
               if (editable && enabled && lastPenDown != -1 && Edit.clipboardDelay != -1 && (Vm.getTimeStamp() - lastPenDown) >= Edit.clipboardDelay)
                  if (showClipboardMenu())
                  {
                     event.consumed = true; // astein@230_5: prevent blinking cursor event from propagating
                     break;
                  }
               event.consumed = true; // astein@230_5: prevent blinking cursor event from propagating
               return;
            case ControlEvent.FOCUS_IN:
               firstPenDown = true;
               if (Settings.geographicalFocus) editMode = editModeValue || improvedGeographicalFocus; // kmeehl@tc100
               // guich@300_43: this is needed bc when popupKCC is called, the focus comes back to here; also, when the
               // popped up window is closed, the focus comes back again, so we could enter in an infinite loop
               if (ignoreNextFocusIn) // guich@tc126_21
                  ignoreNextFocusIn = false;
               else
               if (!Settings.fingerTouch)
                  showSip(); // guich@tc126_21
               if (drawg == null) drawg = getGraphics();
               hasFocus = true;
               if (blinkTimer == null) 
                  blinkTimer = addTimer(350);
               break;
            case ControlEvent.FOCUS_OUT:
               focusOut();
               break;
            case KeyEvent.KEY_PRESS:
            case KeyEvent.SPECIAL_KEY_PRESS:
               if (editable && enabled)
               {
                  KeyEvent ke = (KeyEvent) event;
                  if (ke.key == SpecialKeys.ACTION && (Settings.isWindowsDevice() || Settings.platform.equals(Settings.WIN32))) // guich@tc122_22: in WM, the ACTION key is mapped to the ENTER. so we revert it here
                     ke.key = SpecialKeys.ENTER;
                  if ((ke.key == SpecialKeys.ACTION || ke.key == SpecialKeys.ESCAPE) && !improvedGeographicalFocus)
                  {
                     //isHighlighting = true; // kmeehl@tc100: set isHighlighting first, so that Window.removeFocus() wont trample Window.highlighted - guich@tc110_81: commented out. this will be done in focusOut().
                     focusOut(); // remove the cursor
                     return;
                  }
                  if (!editMode) 
                     break; // kmeehl@tc100
                  int len = chars.length();
                  if (editable)
                  {
                     forceDrawAll = false;
                     if (ke.key == 0) return; // guich@402_41: sometimes, the left key causes a zero key being entered, crashing everything
                     if (ke.key == LINEFEED) // guich@tc100: ignore \r\n, so we don't have to keep checking for both.
                        break;
                     if ((ke.key == SpecialKeys.KEYBOARD_ABC || ke.key == SpecialKeys.KEYBOARD_123) && (Edit.keyboard == null || !Edit.keyboard.isVisible()))
                     {
                        popupKCC();
                        return;
                     }
                     boolean moveFocus = !Settings.geographicalFocus && ke.key == SpecialKeys.TAB;
                     if (event.target == this && moveFocus) // guich@tc125_26
                     {
                        if (parent != null && parent.moveFocusToNextEditable(this, ke.modifiers == 0) != null)
                           return;
                     }
                     // if ((Settings.keyboardFocusTraversable || Settings.geographicalFocus) && (ke.key == SpecialKeys.ESCAPE || ke.key == SpecialKeys.MENU))
   
                     boolean isControl = (ke.modifiers & SpecialKeys.CONTROL) != 0; // guich@320_46 - guich@tc100b4_25: also check for the type of event, otherwise the arrow keys won't work
                     if (Settings.platform.equals(Settings.PALMOS)) // guich@tc100b4_26: if the user pressed the command and then a key, assume is a control
                     {
                        if (lastCommand > 0 && (Vm.getTimeStamp() - lastCommand) < 2500)
                        {
                           ke.modifiers |= SpecialKeys.CONTROL;
                           isControl = true;
                           lastCommand = 0;
                        }
                        else if (ke.key == SpecialKeys.COMMAND) // just a single COMMAND? break
                        {
                           lastCommand = Vm.getTimeStamp();
                           showTip(this, Edit.commandStr, 2500, -1);
                           break;
                        }
                     }
                     boolean isPrintable = ke.key > 0 && (ke.modifiers & SpecialKeys.ALT) == 0 && (ke.modifiers & SpecialKeys.CONTROL) == 0
                           && event.type == KeyEvent.KEY_PRESS;
                     boolean isDelete = (ke.key == SpecialKeys.DELETE);
                     boolean isBackspace = (ke.key == SpecialKeys.BACKSPACE);
                     boolean isEnter = (ke.key == SpecialKeys.ENTER);
                     int del1 = -1;
                     int del2 = -1;
                     int sel1 = startSelectPos;
                     int sel2 = insertPos;
                     if (sel1 > sel2)
                     {
                        int temp = sel1;
                        sel1 = sel2;
                        sel2 = temp;
                     }
                     // clipboard -- original
                     if (isControl)
                     {
                        if (isControl)
                        {
                           if (0 < ke.key && ke.key < 32) ke.key += 64;
                           ke.modifiers &= ~SpecialKeys.CONTROL; // remove control
                        }
                        char key = Convert.toUpperCase((char) ke.key);
                        switch (key)
                        {
                           case 'X':
                              clipboardCut();
                              return;
                           case 'C':
                              clipboardCopy();
                              return;
                           case ' ':
                              setText("");
                              return;
                           case 'P':
                           case 'V':
                              clipboardPaste();
                              break;
                        }
                        clearSelect = true;
                        // break;
                     }
                     if (mapFrom != null) // guich@tc110_56
                     {
                        int idx = mapFrom.indexOf(Convert.toLowerCase((char)ke.key));
                        if (idx != -1)
                           ke.key = mapTo.charAt(idx);
                     }
                     if (isPrintable)
                     {
                        if (capitalise == Edit.ALL_NORMAL)
                           ;
                        else if (capitalise == Edit.ALL_UPPER)
                           ke.key = Convert.toUpperCase((char) ke.key);
                        else if (capitalise == Edit.ALL_LOWER) ke.key = Convert.toLowerCase((char) ke.key);
   
                        if (!isCharValid((char) ke.key)) // guich@101: tests if the key is in the valid char set - moved to here because a valid clipboard char can be an invalid edit char
                        {
                           Sound.beep();
                           break;
                        }
                     }
                     if (sel1 != -1 && (isPrintable || isDelete || isBackspace))
                     {
                        del1 = sel1;
                        del2 = sel2 - 1;
                     }
                     else if (isDelete)
                     {
                        del1 = insertPos;
                        del2 = insertPos;
                     }
                     else if (isBackspace)
                     {
                        del1 = insertPos - 1;
                        del2 = insertPos - 1;
                     }
                     if (isEnter)
                     {
                        ke.key = ENTER;
                        isPrintable = true;
                     }
                     if (del1 >= 0 && del2 < len)
                     {
                        if (len > del2 - 1) chars.delete(del1, del2 + 1); // Vm.arrayCopy(chars, del2 + 1, chars, del1, numOnRight);
                        newInsertPos = del1;
                        forceDrawAll = true;
                        clearSelect = true;
                     }
                     if (isPrintable && (maxLength == 0 || len < maxLength))
                     {
                        // grow the array if required (grows by charsStep) -- original
                        Convert.insertAt(chars, newInsertPos, (char) ke.key);
                        newInsertPos++;
                        redraw = true;
                        clearSelect = true;
                     }
                  }
                  boolean isMove = true;
                  try
                  {
                     switch (ke.key)
                     {
                        case SpecialKeys.HOME:
                           newInsertPos = 0;
                           if (firstToDraw != 0)
                           {
                              firstToDraw = 0;
                              forceDrawAll = true;
                              sb.setValue(0);
                           }
                           break;
                        case SpecialKeys.END:
                           newInsertPos = len;
                           if (numberTextLines > rowCount)
                           {
                              firstToDraw = numberTextLines - rowCount;
                              forceDrawAll = true;
                              sb.setValue(firstToDraw);
                           }
                           break;
                        case SpecialKeys.UP:
                        case SpecialKeys.PAGE_UP:
                           if (!editable && !hasCursorWhenNotEditable) // guich@tc114_62
                              sb.onEvent(event);
                           else
                           if (newInsertPos >= first.items[1])
                           {
                              charPosToZ(newInsertPos, z1);
                              z1.x = z3.x; // kmeehl@tc100: remember the previous horizontal position
                              if (z1.y <= textRect.y && firstToDraw > 0) // guich@550_22: check firstToDraw, otherwise it will insert blanks at the top
                              {
                                 int ii = sb.getValue();
                                 if (ii > sb.getMinimum()) firstToDraw--;
                                 sb.setValue(--ii);
                              }
                              else
                                 z1.y -= hLine;
   
                              int line = firstToDraw + (z1.y - textRect.y) / hLine;
                              if (line >= 0)
                              {
                                 if (chars.charAt(first.items[line]) == ENTER && first.items[line + 1] - first.items[line] == 1)
                                    newInsertPos = first.items[line + 1];
                                 else
                                    newInsertPos = zToCharPos(z1);
                                 charPosToZ(newInsertPos, z1);
                                 forceDrawAll = true;
                              }
                           }
                           break;
                        case SpecialKeys.LEFT:
                           newInsertPos--;
                           if (newInsertPos < 0) newInsertPos = 0;
                           while (newInsertPos < first.items[firstToDraw])
                           {
                              firstToDraw--;
                              forceDrawAll = true;
                              sb.setValue(sb.getValue() - 1);
                           }
                           charPosToZ(newInsertPos, z3); // kmeehl@tc100: remember the previous horizontal position
                           break;
                        case SpecialKeys.RIGHT:
                           newInsertPos++;
                           if (newInsertPos > len) newInsertPos = len;
                           if (numberTextLines > rowCount)
                           {
                              if ((firstToDraw + rowCount) < (first.size() - 1)) // guich@550_5: avoid AAOBE
                                 while (newInsertPos >= first.items[firstToDraw + rowCount])
                                 {
                                    firstToDraw++;
                                    forceDrawAll = true;
                                    sb.setValue(sb.getValue() + 1);
                                 }
                           }
                           charPosToZ(newInsertPos, z3); // kmeehl@tc100: remember the previous horizontal position
                           break;
                        case SpecialKeys.DOWN:
                        case SpecialKeys.PAGE_DOWN:
                           if (!editable && !hasCursorWhenNotEditable) // guich@tc114_62
                              sb.onEvent(event);
                           else
                           if (numberTextLines > 0 && newInsertPos <= first.items[numberTextLines - 1]) // -1 guich@573_44: check if > 0
                           {
                              charPosToZ(newInsertPos, z1);
                              z1.x = z3.x; // kmeehl@tc100: remember the previous horizontal position
                              if (z1.y >= textRect.height - hLine)
                              {
                                 int ii = sb.getValue();
                                 if (ii < sb.getMaximum()) firstToDraw++;
                                 sb.setValue(++ii);
                              }
                              else
                                 z1.y += hLine;
                              forceDrawAll = true;
                              int line = firstToDraw + (z1.y - textRect.y) / hLine;
                              if (chars.charAt(newInsertPos) == ENTER && first.items[line + 1] - first.items[line] == 1)
                                 newInsertPos++;
                              else
                                 newInsertPos = zToCharPos(z1);
   
                              if (line > firstToDraw + (z1.y - textRect.y) / hLine) // zToCharPos failed...
                                 newInsertPos++;
   
                              charPosToZ(newInsertPos, z1);
                           }
                           break;
                        default:
                           isMove = false;
                     }
                  }
                  catch (Exception e)
                  {
                     if (Settings.onJavaSE)
                        e.printStackTrace();
                  }
                  if (isMove && newInsertPos != insertPos)
                  {
                     if ((ke.modifiers & SpecialKeys.SHIFT) > 0)
                        extendSelect = true;
                     else
                        clearSelect = true;
                  }
                  if (!isMove) calculateFirst();
               }
               break;
            case PenEvent.PEN_UP: // kmeehl@tc100
               lastPenDown = -1;
               firstPenDown = false;
               if (!editable && !Settings.fingerTouch) // guich@tc100: allow the user to scroll by just clicking in the ME
               {
                  event.target = sb;
                  ((PenEvent) event).y = ((PenEvent) event).y < height / 2 ? 0 : height;
                  sb.onEvent(event);
                  break;
               }
               else
               if (popupVKbd)
               {
                  showSip();
                  popupVKbd = false;
               }
               charPosToZ(newInsertPos, z3); // kmeehl@tc100: remember the previous horizontal position
               isScrolling = false;
               break;
            case PenEvent.PEN_DOWN:
            {
               lastPenDown = event.timeStamp;
               if (!editable && !Settings.fingerTouch) // guich@tc100: allow the user to scroll by just clicking in the ME
               {
                  event.target = sb;
                  ((PenEvent) event).y = ((PenEvent) event).y < height / 2 ? 0 : height;
                  sb.onEvent(event);
                  break;
               }
               if (Settings.geographicalFocus) editMode = true; // kmeehl@tc100
               popupVKbd = true; // kmeehl@tc100

               PenEvent pe = (PenEvent) event;
               z1.x = pe.x;
               z1.y = pe.y;
               newInsertPos = firstPenDown && Settings.moveCursorToEndOnFocus ? chars.length() : zToCharPos(z1);
               if ((pe.modifiers & SpecialKeys.SHIFT) > 0)
                  extendSelect = true; // shift
               else
               if (firstPenDown && autoSelect)
               {
                  startSelectPos = 0;
                  newInsertPos = chars.length();
               }
               else
                  clearSelect = true;
               break;
            }
            case PenEvent.PEN_DRAG:
            {
               lastPenDown = -1;
               DragEvent de = (DragEvent) event;
               
               if (Settings.fingerTouch)
               {
                  if (isScrolling)
                  {
                     scrollContent(-de.xDelta, -de.yDelta);
                     event.consumed = true;
                  }
                  else
                  {
                     int direction = DragEvent.getInverseDirection(de.direction);
                     event.consumed = true;
                     if (canScrollContent(direction, de.target) && scrollContent(-de.xDelta, -de.yDelta))
                     {
                        isScrolling = scScrolled = true;
                        dragDistance = 0;
/* with this, dragging in a MultiEdit with keyboard open, closes the keyboard but the screen is kept shifted
                        if (Settings.fingerTouch && editable && Window.isSipShown) // guich@tc122_39: only when fingerTouch is enabled
                        {
                           Window.isSipShown = false;
                           Window.setSIP(Window.SIP_HIDE, null, false);
                        }
*/                        popupVKbd = false;
                     }
                  }
               }
               else
               if (editable)
               {
                  PenEvent pe = (PenEvent) event;
                  z1.x = pe.x;
                  z1.y = pe.y;
                  newInsertPos = zToCharPos(z1);
                  if (newInsertPos != insertPos && enabled)
                     extendSelect = true;
                  else
                     return; // guich@320_28: avoid unnecessary repaints
               }
               break;
            }
            case KeyEvent.ACTION_KEY_PRESS:
               try
               {
                  KeyEvent ke = (KeyEvent) event;
                  // allow ENTER to be handled as a normal key event
                  if (Settings.geographicalFocus && ke.key == SpecialKeys.ENTER && editMode)
                  {
                     event.type = KeyEvent.KEY_PRESS;
                     _onEvent(event);
                     break;
                  }
               }
               catch (ClassCastException cce)
               {
               }
               if (Settings.geographicalFocus && !improvedGeographicalFocus) editMode = !editMode;
               if (editMode)
               {
                  showSip();
                  if (blinkTimer == null) blinkTimer = addTimer(350);
               }
               else if (editable)
               {
                  if (Window.isSipShown)
                  {
                     Window.isSipShown = false;
                     Window.setSIP(Window.SIP_HIDE, null, false);
                  }
                  if (removeTimer(blinkTimer)) blinkTimer = null;
               }
               break;
            case KeyboardBox.KEYBOARD_ON_UNPOP:
               gap = tempGap;
               rowCount = tempRowCount;
               rowCount0 = tempRowCount0;
               return;
            case KeyboardBox.KEYBOARD_POST_UNPOP:
               if (oldTabIndex != -1) // reinsert this control in the previous position
               {
                  parent.tabOrder.removeElement(this);
                  parent.tabOrder.insertElementAt(this, oldTabIndex);
                  oldTabIndex = -1;
               }
               requestFocus();
               return;
            default:
               return;
         }
         if (extendSelect)
         {
            if (startSelectPos == -1)
               startSelectPos = insertPos;
            else if (newInsertPos == startSelectPos) startSelectPos = -1;
            redraw = true;
         }
         if (clearSelect && (startSelectPos != -1))
         {
            startSelectPos = -1;
            redraw = true;
         }
         newInsertPos = Math.min(chars.length(), newInsertPos);
         if (newInsertPos < 0) newInsertPos = 0;
         boolean insertChanged = (newInsertPos != startSelectPos);
         if (insertChanged && cursorShowing) draw(drawg, true); // erase cursor at old insert position
         insertPos = newInsertPos;
         if (redraw || insertChanged)
            Window.needsPaint = true;
         if (event.type == ControlEvent.FOCUS_OUT) drawg = null;
      }
      else if (event.target == sb && event.type == ControlEvent.PRESSED)
      {
         firstToDraw = sb.getValue();
         Window.needsPaint = true; // alexgross@340_17
      }
   }
   
   private boolean showClipboardMenu()
   {
      lastPenDown = -1;
      firstPenDown = false;
      int idx = Edit.showClipboardMenu(this);
      if (0 <= idx && idx <= 3)
      {
         if (idx != 3 && startSelectPos == -1) // if nothing was selected, select everything
         {
            startSelectPos = 0;
            insertPos = chars.length();
         }
         if (idx == 0)
            clipboardCut();
         else
         if (idx == 1)
            clipboardCopy();
         else
         {
            if (idx == 2)
               chars.setLength(0);
            clipboardPaste();
            startSelectPos = -1;
            return true; // break instead of return on the caller
         }
      }             
      return false;
   }

   private void clipboardCut()
   {
      int sel1 = startSelectPos;
      int sel2 = insertPos;
      if (sel1 > sel2)
      {
         int temp = sel1;
         sel1 = sel2;
         sel2 = temp;
      }
      if (sel1 != -1)
      {
         Vm.clipboardCopy(chars.toString().substring(sel1, sel2)); // brunosoares@tc100: BlackBerry does not support StringBuffer.substring()
         showTip(this, Edit.cutStr, 500, -1);
         backspaceEvent.target = this;
         _onEvent(backspaceEvent);
      }
   }

   private void clipboardCopy()
   {
      int sel1 = startSelectPos;
      int sel2 = insertPos;
      if (sel1 > sel2)
      {
         int temp = sel1;
         sel1 = sel2;
         sel2 = temp;
      }
      if (sel1 != -1)
      {
         Vm.clipboardCopy(chars.toString().substring(sel1, sel2)); // brunosoares@tc100: BlackBerry does not support StringBuffer.substring()
         showTip(this, Edit.copyStr, 500, -1);
      }
   }
   
   private void clipboardPaste()
   {
      String pasted = Convert.replace(Vm.clipboardPaste(), Convert.CRLF, "\n");
      if (pasted == null || pasted.length() == 0)
         Sound.beep();
      else
      {
         showTip(this, Edit.pasteStr, 500, -1);
         int n = pasted.length();
         if (chars.length() == 0)
         {
            chars.append(pasted);
            newInsertPos = n;
         }
         else
            for (int i = 0; i < n; i++)
               Convert.insertAt(chars, newInsertPos++, pasted.charAt(i));
         calculateFirst();
         forceDrawAll = true;
      }
   }

   private void showSip() // guich@tc126_21
   {
      if (kbdType != Edit.KBD_NONE && Settings.virtualKeyboard && editMode && editable && !hadParentScrolled() && !Window.isScreenShifted()) // if running on a PocketPC device, set the bounds of Sip in a way to not cover the edit - kmeehl@tc100: added check for editMode and !dragScroll
      {
         int sbl = Settings.SIPBottomLimit;
         if (sbl == -1) sbl = Settings.screenHeight / 2;
         boolean onBottom = getAbsoluteRect().y < sbl || Settings.unmovableSIP;
         if (!Window.isSipShown || Settings.isWindowsDevice())
         {
            Window.isSipShown = true;
            Window.setSIP(onBottom ? Window.SIP_BOTTOM : Window.SIP_TOP, this, false);
         }
         if (Settings.unmovableSIP) // guich@tc126_21
            getParentWindow().shiftScreen(this,0);
         lastZ1y = -9999;
      }
   }

   protected void draw(Graphics g, boolean cursorOnly)
   {
      if (g == null || !isDisplayed() || boardRect == null) return; // guich@tc114_65: check if its displayed
      if (forceDrawAll && !transparentBackground)
      {
         g.backColor = uiAndroid ? parent.backColor : back0;
         g.clearClip();
         int x2 = this.width - (Settings.fingerTouch ? 0 : sb.getPreferredWidth());
         g.fillRect(0, 0, x2, this.height);
         if (uiAndroid)
         {
            if (npback == null)
               try
               {
                  npback = NinePatch.getInstance().getNormalInstance(NinePatch.MULTIEDIT, width, height, enabled ? back0 : Color.interpolate(back0 == parent.backColor ? Color.BRIGHT : back0,parent.backColor), false,true);
               }
               catch (ImageException e) {}
            g.drawImage(npback, 0,0);
         }
         else
         if (!uiPalm) g.draw3dRect(0, 0, x2, this.height, Graphics.R3D_CHECK, false, false, fourColors);
      }
      g.setClip(boardRect);
      // draw the text and/or the selection --original
      if (!cursorOnly || forceDrawAll)
      {
         if (startSelectPos != -1 && editable) // guich@tc113_38: only if editable
         {
            // character regions are: -- original
            // 0 to (sel1-1) .. sel1 to (sel2-1) .. sel2 to last_char -- original
            int sel1 = Math.min(startSelectPos, insertPos);
            int sel2 = Math.max(startSelectPos, insertPos);
            charPosToZ(sel1, z1);
            charPosToZ(sel2, z2);
            g.backColor = back1;
            if (z1.y == z2.y)
               g.fillRect(z1.x, z1.y, z2.x - z1.x, fmH);
            else
            {
               g.fillRect(z1.x, z1.y, textRect.x2() - z1.x + 1, hLine);
               if (z2.y > z1.y) g.fillRect(textRect.x, z1.y + hLine, textRect.width, z2.y - z1.y - hLine);
               g.fillRect(textRect.x, z2.y, z2.x - textRect.x, fmH);
            }
         }
         int i;
         int h = textRect.y;
         int dh = textRect.y + fm.ascent;
         int maxh = h + textRect.height;
         g.foreColor = fColor;
         g.backColor = back0;
         int last = numberTextLines - 1;
         int len = chars.length();
         for (i = firstToDraw; i <= last && h < maxh; i++, h += hLine, dh += hLine)
         {
            if (!forceDrawAll) g.fillRect(boardRect.x + 1, h, boardRect.width - 2, hLine); // erase drawing area
            int k = first.items[i];
            int k2 = first.items[i + 1];
            if (chars.charAt(k) <= ' ') // guich@tc166: ignore space/ENTER at line start
               k++;
            g.drawText(chars, k, k2 - k, textRect.x, h, (!editable && justify && i < last && k2 < len && chars.charAt(k2) >= ' ') ? textRect.width : 0, textShadowColor != -1, textShadowColor); // don't justify if the line ends with <enter>
            if (drawDots)
            {
               g.drawDots(textRect.x, dh, textRect.x2(), dh); // guich@320_28: draw the dotted line
               g.backColor = back0;
            }
         }

         // guich@320_28: draw the dotted lines
         if (!forceDrawAll || drawDots) for (; i < firstToDraw + rowCount; i++, h += hLine, dh += hLine)
         {
            if (!forceDrawAll) g.fillRect(boardRect.x + 1, h, boardRect.width - 2, hLine); // erase drawing area
            if (drawDots) g.drawDots(textRect.x, dh, textRect.x2(), dh);
            g.backColor = back0;
         }
      }
      forceDrawAll = false;
      if (hasFocus && (editable || hasCursorWhenNotEditable))
      {
         // draw cursor
         charPosToZ(insertPos, z1);
         g.drawCursor(z1.x, z1.y - (spaceBetweenLines >> 1), 1, hLine);
         cursorShowing = cursorOnly ? !cursorShowing : true;
         if (Window.isScreenShifted() && lastZ1y != z1.y)
            getParentWindow().shiftScreen(this, lastZ1y = z1.y);
      }
      else
         cursorShowing = false;
   }
   
   protected void onWindowPaintFinished()
   {
       if (editable && !hasFocus) _onEvent(new Event(ControlEvent.FOCUS_IN,this,0)); // this event is called on the focused control of the parent window. so, if we are not in FOCUS state, set it now. --original - guich@350_7: added the editable check
   }

   public void onPaint(Graphics g)
   {
      forceDrawAll=true;
      draw(g, false);
   }

   private void clearPosState()
   {
      insertPos = 0;
      startSelectPos = -1;
   }

   protected void pushPosState()
   {
      pushedInsertPos = insertPos;
      pushedStartSelectPos = startSelectPos;
   }

   protected void popPosState()
   {
      if (cursorShowing)
         draw(drawg,true);
      insertPos = pushedInsertPos;
      startSelectPos = pushedStartSelectPos;
   }

   /** Return true if the given char exists in the set of valid characters for this Edit */
   private boolean isCharValid(char c)
   {
      if (validChars == null) return true;
      c = Convert.toUpperCase(c);
      return validChars.indexOf(c) != -1;
   }

   protected void onColorsChanged(boolean colorsChanged)
   {
      fColor = getForeColor();
      back0  = Color.brighter(getBackColor());
      back1  = back0 != Color.WHITE?backColor:Color.getCursorColor(back0);//guich@300_20: use backColor instead of: back0.getCursorColor();
      if (!uiAndroid && !uiPalm) Graphics.compute3dColors(enabled,backColor,foreColor,fourColors);
      sb.setBackForeColors(backColor, foreColor);
      npback = null;
   }

   /** Sets the rect for this MultiEdit. Note that height is recomputed based
     * in the value for rowCount given in the constructor if the given height is PREFERRED
     */
   public void setRect(int x, int y, int width, int height, Control relative, boolean screenChanged)
   {
      if ((PREFERRED-RANGE) <= height && height <= (PREFERRED+RANGE)) // kambiz@330_24: use preferred height only if user wants
         height += getPreferredHeight() - PREFERRED;
      super.setRect(x,y,width,height,relative,screenChanged);
   }

   protected void onFontChanged() // guich@320_28
   {
      hLine = fmH + spaceBetweenLines;
   }

   /** Clears the text of this control. */
   public void clear() // guich@572_19
   {
      setText(clearValueStr);
   }

   public void getFocusableControls(Vector v)
   {
      if (visible && enabled) v.addElement(this);
   }

   /** Scrolls the text to the given line. */
   public void scrollToLine(int line)
   {
      if (line < 0) line = 0;
      else
      if (line >= numberTextLines)
         line  = Math.max(numberTextLines-1,0);

      if (line < sb.minimum)
         sb.setValue(sb.minimum);
      else
      if (line > sb.maximum)
         sb.setValue(sb.maximum);
      else
         sb.setValue(line);
      line = sb.getValue();
      insertPos = first.items[line];
      firstToDraw = line;
      Window.needsPaint = true;
   }

   public Control handleGeographicalFocusChangeKeys(KeyEvent ke)
   {
      boolean processEvent = editable && (!improvedGeographicalFocus ||
         (!ke.isPrevKey() && !ke.isNextKey()) ||
         (ke.key == SpecialKeys.LEFT && insertPos > 0) ||
         (ke.key == SpecialKeys.RIGHT && insertPos < chars.length()) ||
         ((ke.key == SpecialKeys.UP || ke.key == SpecialKeys.PAGE_UP) && numberTextLines > 1 && insertPos >= first.items[1]) ||
         ((ke.key == SpecialKeys.DOWN || ke.key == SpecialKeys.PAGE_DOWN) && numberTextLines > 1 && insertPos < first.items[numberTextLines - 1]));
      
      if (editMode && processEvent)
      {
         Object o = ke.target; // guich@tc115_6: otherwise, we will not process arrow events
         ke.consumed = false; // guich@tc115_20
         ke.target = this;
         _onEvent(ke);
         ke.target = o;
         return this;
      }
      else if (ke.isUpKey() || ke.isDownKey())
      {
         int val = sb.getValue();
         int inc = ke.isUpKey()?-1:1;
         if (inc == -1 && firstToDraw == 0)
            return null;
         int line = numberTextLines - textRect.height/hLine;
         if (line < 0 || (inc == 1 && firstToDraw == line))
            return null;

         sb.setValue(val + inc);
         firstToDraw += inc;
         forceDrawAll=true;
         Window.needsPaint = true;
         return this;
      }
      else
         return null;
   }

   /** Scrolls the text to bottom. */
   public void scrollToBottom()
   {
      int len = chars.length();
      newInsertPos = len;
      if (numberTextLines>rowCount)
      {
         firstToDraw=numberTextLines-rowCount;
         forceDrawAll=true;
         sb.setValue(firstToDraw);
      }
   }

   /** Scrolls the tex to the top. */
   public void scrollToTop()
   {
      newInsertPos = 0;
      if (firstToDraw!=0)
      {
         firstToDraw=0;
         forceDrawAll=true;
         sb.setValue(0);
      }
   }

   /** Returns the length of the text.
    * @since TotalCross 1.01
    */
   public int getLength()
   {
      return chars.length();
   }
   
   public void requestFocus() // guich@tc115_6: user requested focus directly, so enable editMode by default
   {
      editModeValue = true;
      super.requestFocus();
      editModeValue = false;
   }

   /** Returns the keyboard type of this Edit control.
    * @see Edit#KBD_NONE
    * @see Edit#KBD_DEFAULT
    * @see Edit#KBD_KEYBOARD
    * @see Edit#KBD_CALCULATOR
    * @see Edit#KBD_CALCULATOR
    * @since SuperWaba 5.67
    */
   public byte getKeyboardType() // guich@567_6
   {
      return kbdType;
   }

   /** Returns a copy of this Edit with almost all features. Used by Keyboard and SIPBox classes.
    * @since TotalCross 1.27
    */
   public MultiEdit getCopy()
   {
      MultiEdit ed = mask == null ? new MultiEdit(rowCount, spaceBetweenLines) : new MultiEdit(mask, rowCount, spaceBetweenLines);
      ed.startSelectPos = startSelectPos;
      ed.insertPos = insertPos;
      ed.setBackForeColors(backColor,foreColor);
      if (validChars != null)
         ed.setValidChars(validChars);
      ed.capitalise = capitalise;
      ed.maxLength = maxLength;
      ed.kbdType = kbdType;
      ed.editable = editable;
      ed.scrollBarsAlwaysVisible = scrollBarsAlwaysVisible;
      ed.rowCount = rowCount;
      ed.drawDots = drawDots;
      ed.justify = justify;
      ed.autoSelect = autoSelect;
      
      return ed;
   }
   
   public Flick getFlick()
   {
      return flick;
   }

   public boolean wasScrolled()
   {
      return scScrolled;
   }

   protected boolean willOpenKeyboard()
   {
      return editable && (kbdType == Edit.KBD_DEFAULT || kbdType == Edit.KBD_KEYBOARD);
   }
}
