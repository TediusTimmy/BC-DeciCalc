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
Below this is the sheet proper. Hopefully, navigating the sheet proper is intuitive (except for that it is a view). Remember that the input box at the top-left corner can change where in the sheet you are viewing.

### Commands
* Arrow keys : navigate.
* Page Up / Page Down : move to the next screen of rows.
* `<` : start entering a label in this cell. Finish by pressing enter. (There are no centered or right-justified labels.)
* `=` : start entering a formula in this cell. Finish by pressing enter.
* `!` : recalculate the sheet
* `,` : change between `.` and `,` as the decimal separator
* `m` : set a copy marker at this cell location
* `dd` : clear (delete) the current cell
* `dr` : clear all cells in the current row
* `dc` : clear all cells in the current column
* `dm` : clear all cells in the rectangle between the current cell and the marker
* `xx` : remove the current cell (shifting cells up)
* `xz` : remove the current cell (shifting cells right)
* `xr` : remove the current row
* `xc` : remove the current column
* `ii` : insert a cell at the current location (shifting cells down)
* `ir` : insert a row at the current row
* `ic` : insert a column at the current column
* `oo` : open a cell at the current location (shifting cells right)
* `or` : open a row after the current row
* `oc` : open a column after the current column
* `yy` : copy the current cell
* `yc` : copy the current column
* `yr` : copy the current row
* `ym` : copy the rectangle between the current cell and the marker
* `yd` : clear the copy buffer
* `pp` : paste to the current cell
* `pc` : paste to the current column
* `pr` : paste to the current row
* `pm` : paste the copied data as marked with this the top-right corner
* `pn` : paste the copied data transposed with this the top-right corner
* `pf` : paste the copied data into the rectangle between the current cell and the marker in column-major order
* `pt` : paste the copied data into the rectangle between the current cell and the marker in row-major order
* `vv` : replace the current cell with its evaluated value
* `vm` : replace the cell with its evaluated value for all cells in the rectangle between the current cell and the marker
* `v=` : flip the current cell between a label and a formula
* `v-` : flip the cell between a label and a formula for all cells in the rectangle between the current cell and the marker

### Background Processing Notes

The program now handles sheet updates in a background thread. There is no indicator as to whether background processing is occurring. During sheet processing, no commands that modify the sheet will be executed. In addition, the second line of information will not be displayed for cells that haven't processed yet. Saving ... ought to work.
