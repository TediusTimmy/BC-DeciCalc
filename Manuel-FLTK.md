# DeciCalc - BCMath Edition - FLTK front end

This manual is woefully incomplete without the curses version manual.  
Note that you should close the program between opening different spreadsheet files, because the program state is not correctly cleared between loading files.

### The Spreadsheet Screen
The contents of the top line:
* First, the current top-left corner of the view. The view is 500 rows by 100 columns
* Whether that cell is a formula (V) or a label (L)
* The last computed value for that cell

The second line is either: the formula in the current cell, or the computed value of the cell (if one is modifying the cell).  
The third line is the modification line: the formula or string in the current cell.  
Below this is the sheet proper. Hopefully, navigating the sheet proper is intuitive.

### Commands
* Arrow keys : navigate.
* Page Up / Page Down : move to the next screen of rows.
* `<` : start entering a label in this cell. Finish by pressing enter. (There are no centered or right-justified labels.)
* `=` : start entering a formula in this cell. Finish by pressing enter.
* `!` : recalculate the sheet
* `,` : change between `.` and `,` as the decimal separator

### Background Processing Notes

The program now handles sheet updates in a background thread. There is no indicator as to whether background processing is occurring. During sheet processing, no commands that modify the sheet will be executed. In addition, the second line of information will not be displayed for cells that haven't processed yet. Saving ... ought to work.
