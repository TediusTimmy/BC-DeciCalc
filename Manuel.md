# DeciCalc - BCMath Edition

Have you ever wanted a spreadsheet that did math in decimal and didn't need to hide that binary floating-point was used under the covers? Well, this author did. The other DeciCalc uses decimal floating-point with 16 digits of precision, this one uses arbitrary-precision decimal fixed-point. So, you can compute the GDP of the United States in Iranian Rial exactly, if you were so inclined.

To unpack the term "arbitrary-precision decimal fixed-point":
* "arbitrary-precision" means that the program handles numbers in a manner that is only limited by the memory of your computer (and the algorithms that it performs)
* "decimal" means that math is performed in a manner consistent with it being done with ten digits (basically, how _you_ would probably do it)
* "fixed-point" means that it handles fractional numbers by computing some 'fixed' number of digits past the decimal point (so, one-half is .500... and one-third is .333...)

As the README states, "arbitrary" is probably somewhere in the hundred million digit range for a modern 32-bit computer (and probably your limit of patience for a 64-bit computer).

The rules for the number of digits past the decimal point that operations will result in can be found in the POSIX standard for bc. This is notably different from how Java's BigDecimal handles results, which follows the rules of the language Rexx. In the bc environment, there is a global "scale" variable. This is kept (and is probably a bottleneck to the program ever running "fast"). The rules are, assuming two numbers `A` and `B`, which have scales `a` and `b`, respectively, and the global scale variable `s`:

| Operation | Scale |
| --------- | ----- |
| Addition/Subtraction | `max(a, b)` |
| Multiplication | `min(a + b, max(a, b, s))` |
| Division | `s` |

Exponentiation and remainder are not native operations in the spreadsheet. These rules allow repeated multiplication operations to converge on the global scale variable, while allowing parallel computations that have increased precision (so long as division isn't used). Also note that the number system is not closed under division, and that division by zero throws an error, which kills all current processing, rather than returning an error value.

The biggest difference between this implementation of BCMath and POSIX bc is that all operations are rounded. The default rounding mode is TIES TO EVEN. This can be changed (but it's not intuitive).


#### Why fixed-point?

If we look at the motivation behind [PEP 327](https://peps.python.org/pep-0327/#motivation), we see that one intent of implementing decimal was for monetary considerations. And that is why I went with fixed-point: the amount of wrapper code for doing money with arbitrary-precision fixed-point is less than that for doing money with arbitrary-precision floating-point. With fixed-point, you already know the scale that you care about, and you let the number grow in the direction you don't care about (you care about the number of digits to the right of the decimal point, and don't care how the number grows to the left). With floating-point, you have to check every operation for the inexact flag and raise the working precision in order to maintain your pennies.

The other consideration I had was logarithmic encoding. If you look at [this issue](https://github.com/gavinhoward/bc/issues/66), the individual wants `((169287^137)^920)^13256118217109` or `169287^1670801140084418360`. The only way to handle that with most modern computers is by computing the logarithm. Or, you have Matt Parker's `pi^pi^pi^pi` [video](https://www.youtube.com/watch?v=BdHFLfv-ThQ). Or [Austin trying to compute the number of states of the Minecraft world](https://www.youtube.com/watch?v=kgveHrqM9KI). It is easier to deal with these numbers as the logarithm of the number. In this encoding, the scale of the number (the number of digits in the mantissa) is the precision of the significand in a floating-point number with an arbitrary-precision exponent. (And now you know why the significand of a floating-point number is often called the mantissa: this relationship.)

The only real issue is that scale is not as intuitive for algorithms. Mainly significant figures: floating-point at the limit of precision better follows significant figures rules. Take the `Pow(x;y)` function. The naive approach is `Exp(y*Log(x))`. If you analyze the results of `Pow(1024;32.1)` (which is the EXACT value `2^321`) by increasing and decreasing the working scale, you find a direct link between the digits of scale and the correct digits of the result. If you are working in precision, as in arbitrary-precision floating-point, then this mathematical relationship just works. When working in scale, extra work needs to be done due to significant figures.

Pros:
* Better fit for money
* Better fit for logarithmic encoding

Cons:
* Not as intuitive for significant figures


## Starting the Program

* The first accepted argument is `-l`, which specifies a Backwards library file to load. There can be a chain of multiple libraries, however: `-l MyBetterLib.txt -l TheBaseLibrarySucks.txt`. These must be at the beginning.
* The next accepted argument is `-b`, which initiates batch mode. For each `-b` argument, the next argument is expected to be a formula to evaluate. The program will evaluate each batch command and then stop before entering interactive mode. This can be used to: use DeciCalc as a command-line calculator; query the contents of a spreadsheet from a shell script; or output the value of a cell whose contents are too large to see in interactive mode.
* The first argument after all explicit arguments is a file to load. If no file is loaded, then an empty spreadsheet is given.
* The second argument is the file name to use to save files. If no second argument is specified, then the file is saved with the name of the file read in. If NO file name is specified, then the name "untitled.html" is used.
* Any other arguments are ignored.


## Navigating the Spreadsheet

Most of the navigation and commands are taken from the Unix tool `sc`. The tool `sc` was originally written with `vi` users in mind, and many choices reflect that. The color scheme, and that F7 exits, is taken from WordPerfect.

### Commands
* Arrow keys : navigate. One can also use the vi keys `hjkl`.
* Page Up / Page Down : move to the next screen of rows. One can also use `JK`.
* `H` / `L` : move to the next screen of columns.
* Home : goto cell A1
* `g` : type in a cell name, then enter, and the current cell cursor will be moved to that cell. Note that you cannot see the cell name that you are typing.
* `<` : start entering a label in this cell. Finish by pressing enter. (There are no centered or right-justified labels.)
* `=` : start entering a formula in this cell. Finish by pressing enter.
* `q` or F7 : exit. You must next press either 'y' to save and exit, or 'n' to not save and exit, in order to actually exit.
* `!` : recalculate the sheet
* `W` : save the sheet
* `dd` : clear (delete) the current cell
* `dr` : clear all cells in the current row
* `dc` : clear all cells in the current column
* `yy` : copy the current cell
* `pp` : paste the current cell
* `e` : edit the current cell's contents
* Shift left/right (also F9/F12) : widen or narrow the current column. Columns can be between 1 and 40 cells wide.
* `#` : Switch between column-major and row-major recalculation.
* `$` : Switch between top-to-bottom and bottom-to-top recalculation.
* `%` : Switch between left-to-right and right-to-left recalculation.
* `,` : Toggle between using ',' and '.' as the decimal separator. This is not a saved setting.
* `+` : If the current cell is empty, start entering a formula in this cell, else enter edit mode and append to this cell. If the current cell is a formula, append a '+' to the formula.
* `:)` : Goto column A of the current row. (This is actually `0`, but I don't like lifting my finger from the shift key.)
* `:$` : Goto the last column of the current row with meaningful data in it.
* `:^` : Goto row 1 of the current column.
* `:#` : Goto the last row of the current column with meaningful data in it.
* `xx` : remove the current cell (shifting cells up)
* `xX` : remove the current cell (shifting cells right)
* `xr` : remove the current row
* `xc` : remove the current column
* `ii` : insert a cell at the current location (shifting cells down)
* `ir` : insert a row at the current row
* `ic` : insert a column at the current column
* `oo` : open a cell at the current location (shifting cells right)
* `or` : open a row after the current row
* `oc` : open a column after the current column
* `vv` : replace the current cell with its evaluated value

### Edit Mode
Edit mode is entered when you start entering a label or formula.
* Left / Right : change cursor location
* Up / Down : End edit mode and navigate the pressed direction
* Page Up / Page Down : End edit mode and navigate Left / Right, respectively
* Insert : Toggle between insert / overwrite mode
* Delete : Delete character at cursor
* BackSpace : Delete character before cursor
* Home : Move cursor to beginning of input
* End : Move cursor to end of input

The sheet automatically recalculates after you finish entering a label or formula, and when you paste a cell. If a cell references a cell that hasn't been computed yet, then that cell will be computed, unless we are already in the process of computing that cell (circular reference).


## Entering Data

The easiest mode is label mode: it is just free-form text. It should probably work for non-English text if the console is in UTF-8 mode, maybe. It hasn't been tested and may not be compiled right.

Then, there is formula mode. Formula mode accepts formulas. It uses A1 cell references, despite immediately converting them to R1C1 under the covers, and thus the $A$1 fixed-reference format. Anchoring to a row or column only matters when you are copy/pasting cells. Formulas can refer to cells, have constants in them (though scientific notation is not supported), perform addition `+`/subtraction `-`/multiplication `*`/division `/`, select ranges `:`, or call functions `@FUN`. Functions start with `@` like in early spreadsheet applications; their arguments are separated by semicolons `;`. You can do comparisons with `=`, `>=`, `<=`, `>`, `>`, or `<>`: the result is 1 for true and 0 for false. Use `&` to concatenate the string representations of two cells. The only half-attempt at internationalization that is supported is that `12,5` and `12.5` are treated the same. When you type a comma on numeric input, all of the numeric outputs will change to displaying a comma as the decimal separator.

Example:  
`A$1+@SUM(C2:D3)+4/7`



## Standard Library

The following functions are all that is implemented. It is a curated list from the first version of VisiCalc, plus two functions that seem important. You can see the implementation in `Forwards/Tests/StdLib.txt`. If you load a library that redefines a function, it will successfully redefine that function. This can be used to improve the standard library (even though it is compiled into the program). For instance, the `limit_scale.txt` script overwrites SETSCALE to limit setting the scale to 10000 digits (from within the spreadsheet).

* MIN (%) - for functions marked (%), input is a variable number of arguments that can also be cell ranges. Empty cells and cells with labels are ignored. NaN is treated as an error value, not a missing value.
* MAX (%) - also, MIN and MAX return 'Empty' when given an empty set
* SUM (%)
* COUNT (%)
* AVERAGE (%) - literally SUM / COUNT
* ABS - absolute value
* INT - truncate to integer (return value has scale 0)
* ROUND - round to integer (ties away from zero) (return value has scale 0)
* GETSCALE - gets the scale setting
* SETSCALE - sets the scale setting
* GETROUND - gets the rounding setting
* SETROUND - sets the rounding setting


## New Standard Library

This adds some transcendental functions that I didn't want to include at first. EXP, SQRT, and LOG draw heavy inspiration from GNU bc's implementation of the same. POW makes arguments ungodly precise to produce good results.

* EXP - The base of the natural logarithms raised to the argument power
* LOG - The natural logarithm
* SQRT - Square root
* RAISE - The first argument raised to an integer power second argument
* POW - The first argument raised to an arbitrary power second argument
* PMT - (interest rate per repayment period; repayment periods; present value)
* GETLIBROUND - Get the internal rounding mode of EXP, LOG, SQRT, and POW.
* SETLIBROUND - Set the internal rounding mode of those functions.


# Backwards

All of the scripting utilizes this language called Backwards. All functions that are exposed to the spreadsheet must have a name that is ALL CAPS and contain no numbers or underscores. This function must take one argument, and the argument is the array of arguments. Note, that it does lazy evaluation, so only the arguments to a function call that the function explicitly asks for will be evaluated. Also, also note: there is no way for a function to change the value of a cell except by returning the value that the current cell ought to have; there is no way for a function to look up the value of an arbitrary cell, all cells that it is to consider MUST be passed to it.

Example:  
```
set IF to function (x) is
   if EvalCell(x[0]) then
      return x[1]
   end
   return x[2]
end
```

Note that the call `@IF(3;5;1/0)` is completely valid. The 'else' clause will not be evaluated because it is never used, so the division-by-zero exception is never thrown.

## The Language
This is the language as implemented. Some of the GoogleTests have good examples, others, not so much.

### Data Types:
* Float - this is the arbitrary-precision decimal fixed-point number, and I was too lazy to change the name
* String
* Array (ordinally indexed Dictionary?)
* Dictionary
* Function Pointer

### Operations
* \+  float addition; string catenation; for collections, the operation is performed over the contents of the collection
* \-  float subtraction; for collections, the operation is performed over the contents of the collection
* \-  float unary negation; for collections, the operation is performed over the contents of the collection
* \*  float multiplication; for collections, the operation is performed over the contents of the collection
* /   float division; for collections, the operation is performed over the contents of the collection
* ^   float exponentiation; this operator is right-associative
* !   logical not
* \>  greater than, only defined for strings and floats
* \>= greater than or equal to, only defined for strings and floats
* <   less than, only defined for strings and floats
* <=  less than or equal to, only defined for strings and floats
* =   equality, defined for all types
* <>  inequality, defined for all types
* ?:  ternary operator
* &   logical and, short-circuit
* |   logical or, short-circuit
* []  collection access
* .   syntactic sugar for collection access; x.y is equivalent to x["y"]; in addition . and , are interchangeable, so x.y is the same as x,y
* {}  collection creation: `{}` is an empty array; `{ x; y; z }` creates an array; `{ x : y ; z : w ; a : b }` creates a dictionary

### Operator Precedence
* ()  -- function call
* {}
* . []
* ^
* ! -  -- unary negation
* \* /
* \+ \-
* = <> > >= < <=
* | &
* ?:

### Statements
I'm going to mostly use BNF. Hopefully, I'm not doing anything fishy. Note that [] is zero-or-one and {} is zero-to-many.
#### Expression
`"call" <expression>`  
To simplify the language, we have specific keywords to find the beginning of a statement. The "first set" of a statement is intentionally small, so that parsing and error recovery is easier. It makes the language a little verbose, though. Sometimes you want to just call a function (for instance, to output a message), and this is how. It will also allow you to add three to the function's result.
#### Assignment
`"set" <identifier> { "[" <expression> "]" } "to" <expression>`  
`"set" <identifier> { "." <identifier> } "to" <expression>`  
Assigning to an undefined variable creates it. The parser will catch if you try to create a variable and access it like an array/dictionary in the same statement. However, the parser probably won't catch `set x to x` until runtime.  
#### If
`"if" <expression> "then" <statements> { "elseif" <statements> } [ "else" <statements> ] "end"`  
#### While
`"while" <expression> [ "call" <identifier> ] "do" <statements> "end"`  
The "call" portion gives the loop a name. See break and continue.
#### For
`"for" <identifier> "from" <expression> ( "to" | "downto" ) <expression> [ "step" <expression> ] [ "call" <identifier> ] "do" <statements> "end"`  
`"for" <identifier> "in" <expression> [ "call" <identifier> ] "do" <statements> "end"`  
The second form iterates over an array or dictionary. When a dictionary is iterated over, the loop control variable is successively set to a two element array of { key, value }. If the variable does not exist, it will be created, and it will remain alive after the loop.
#### Return
`"return" <expression>`
#### Select
`"select" <expression> "from"`  
`    [ "also" ] "case" [ ( "above" | "below" ) ] <expression> "is"`  
`    [ "also" ] "case" "from" <expression> "to" <expression> "is"`  
`    [ "also" ] "case" "else" "is"`  
`"end"`  
Select has a lot of forms and does a lot of stuff. Cases are breaking, and "also" is used to have them fall-through. Case else must be the last case.
##### Break
`"break" [ <identifier> ]`  
Breaks out of the current while or for loop, or the named while or for loop. This isn't completely a statement, in that it requires a loop to be in context.
##### Continue
`"continue" [ <identifier> ]`  
##### Function Definition
`"function" [ <identifier> ] "(" [ <identifier> { ";" <identifier> } ] ")" "is" <statements> "end"`  
This is actually an expression, not a statement. It resolves to the function pointer of the defined function. As such, a function name is optional (but assists in debugging). Function declaration is static, all variables are captured by reference (no closures), and a function cannot access the variables of an enclosing function. All arguments to function calls are pass-by-value, semantically.  
New addition : the function name must now be unique with respect to: all functions that are being defined, global variables, and scoped variables. Functions can use their name to recursively call themselves.  
Previous way:  
`set fib to function (x) is if x > 1 then return fib(x - 1) * x else return 1 end end`  
New way:  
`set x to function fib (y) is if y > 1 then return fib(y - 1) * y else return 1 end end`  
Newer addition : closure / runtime parameterized functions  
Before the function name, add a list of expressions to capture in brackets. After the argument list, add a bracketed list of names for those values to have in the function. This list will be evaluated when the function pointer is used, and becomes part of the type of the function. These values parameterize the function.  
To create an Info function which decorates the original Info function:  
`set Info to function [Info] decorated_info (x) [y] is return y("Decorated: " + x) end`  
Parameters are also required when calling a function recursively:  
`set x to function [3] fib (y) [z] is if y > 1 then return fib[z](y - 1) * y else return 1 end end`

### Comments
`"(*" Comment "*)"`  
Old-style Pascal comments really round out the language as being valid even when newlines are replaced with spaces.

### Standard Library
* float Abs (float)  # absolute value
* float Ceil (float)  # ceiling
* float ContainsKey (dictionary, value)  # determine if value is a key in dictionary (the language lacks a means to ask for forgiveness)
* string DebugPrint (string)  # log a debugging string, returns its argument
* float EnterDebugger ()  # enters the integrated debugger (if present), returns zero
* string Error (string)  # log an error string, returns its argument
* value Eval (string)  # parse and evaluate the given string, return its evaluated value
* value EvalCell (CellRef)  # evaluate the CellRef, return its evaluated value
* array ExpandRange (CellRange)  # expand the CellRange: 2d cell ranges return a column-major array of cell ranges; 1d cell ranges return an array of CellRefs
* Fatal (string)  # log a fatal message, this function does not return, calling this function stops execution
* float Floor (float)  # floor
* float FromCharacter (string)  # return the ASCII code of the only character of the string (the string must have only one character)
* value GetIndex (array; float)  # retrieve index float from array
* array GetKeys (dictionary)  # return an array of keys into a dictionary
* float GetDefaultPrecision () # returns the current global scale variable
* float GetPrecision (float) # returns the scale of the passed in float
* float GetRoundMode () # returns a numeric representation of the current rounding mode
* value GetValue (dictionary; value)  # retrieve the value with key value from the dictionary, die if value is not present (no forgiveness)
* string Info (string)  # log an informational string, returns its argument
* dictionary Insert (dictionary; value; value)  # insert value 2 into dictionary with value 1 as its key and return the modified dictionary (remember, this DOES NOT modify the passed-in dictionary)
* float IsArray (value)  # run-time type identification
* float IsCellRange (value)  # run-time type identification : the result from evaluating a cell and it being a cell range
* float IsCellRef (value)  # run-time type identification : the raw arguments passed into a function from the runtime
* float IsDictionary (value)  # run-time type identification
* float IsFloat (value)  # run-time type identification
* float IsFunction (value)  # run-time type identification
* float IsNil (value)  # run-time type identification : the result from evaluating a cell and it having no contents
* float IsString (value)  # run-time type identification
* float Length (string)  # length
* float Max (float; float)  # if either is NaN, returns NaN; returns the first argument if comparing positive and negative zero
* float Min (float; float)  # if either is NaN, returns NaN; returns the first argument if comparing positive and negative zero
* array NewArray ()  # returns an empty array
* array NewArrayDefault (float, value)  # returns an array of size float with all indices initialized to value
* dictionary NewDictionary ()  # returns an empty dictionary
* array PopBack (array)  # return a copy of the passed-in array with the last element removed
* array PopFront (array)  # return a copy of the passed-in array with the first element removed
* array PushBack (array; value)  # return a copy of the passed-in array with a size one greater and the last element the passed-in value
* array PushFront (array; value)  # return a copy of the passed-in array with a size one greater and the first element the passed-in value
* dictionary RemoveKey (dictionary; value)  # remove the key value or die
* float Round (float)  # ties to even
* array SetIndex (array; float; value)  # return a copy of array where index float is now value
* float SetDefaultPrecision (float) # sets the current global scale variable; returns its argument
* float SetPrecision (float, float) # sets the scale of the first argument to the second argument; returns the modified argument
* float SetRoundMode (float) # sets the current rounding mode by number; returns its argument
* float Size (array)  # size of an array
* float Size (dictionary)  # number of key,value pairs
* float Sqr (float)  # square
* float SubString (string; float; float)  # from character float 1 to character float 2 (java style)
* string ToCharacter (float)  # return a one character string of the given ASCII code (or die if it isn't ASCII)
* string ToString (float)  # return a string representation of a float: scientific notation, 9 significant figures
* float ValueOf (string)  # parse the string into a float value
* string Warn (string)  # log a warning string, returns its argument

### Rounding Mode Decoder Ring
Directed rounding modes are a part of IEEE-754 for doing algorithm analysis. Basically, it's a simple idea: change the rounding mode and see how the result changes. Note that the rounding mode only applies to addition, subtraction, multiplication, and division. A note on terminology: rounding to nearest means that behave as though we did the math to get the next digit, and then if the digit is 6-9, we round away from zero, and if 1-4 we round toward zero. A 5 is a "tie", and the round-to-nearest modes all specify how ties are handled, with "to even" and "to odd" meaning to make the least-significant digit of the result even or odd, respectively. Also note that the algorithms do analysis such that the next digit is only considered a 5 if it was EXACTLY a 5. If the next few digits are 500000001, then it is treated as a 6.
* 0 - Round to nearest, ties to even
* 1 - Round to nearest, ties away from zero
* 2 - Round to positive infinity
* 3 - Round to negative infinity
* 4 - Round to zero
* 5 - Round to nearest, ties to odd
* 6 - Round to nearest, ties to zero
* 7 - Round away from zero
* 8 - Round 05 away from zero : double rounding mode

The final rounding mode probably needs some explanation: it is round to zero, unless the last digit of the result is a zero or five, then it is round away from zero. This mode allows computations made at a higher precision to be double rounded later to a lower precision (correctly). The least significant digit is treated as a sticky digit ought to be for eventual rounding.

NOTE: Some testing violently reminded me that the algorithms for EXP and LOG don't work in rounding modes 2 or 7 (and LOG doesn't work in 3 either). They expect a value to go to zero that can never go to zero: we divide two numbers expecting to eventually get zero but these rounding mode prohibits this. The termination conditions for LOG and EXP were reworked so that they can be used in all rounding modes. Also, the algorithm for SQRT (and thus LOG) did not work in rounding mode 8. For whatever reason, it is very likely to hit cases where it bounces between two solutions one unit in the last place apart. You would think that implies that the SQRT algorithm was broken for all rounding modes, but I haven't seen cases in the wild that are broken for other rounding modes. Anyway, this has been fixed (if the last two solutions differ by an ulp, then we increase working precision).  
In addition to this, the functions LOG, EXP, POW, and SQRT have been modified so that they internally use the new double rounding mode. This should make the output more consistent when you change the rounding mode. Is it the right thing to do? Probably not. However, it hides the numerical instability of those algorithms when one is analyzing some other algorithm. There's an irony that I changed the algorithms to work with any rounding mode, then changed them to always use a rounding mode that they always worked with (except SQRT).
