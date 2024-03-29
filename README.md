BC-DeciCalc
===========

I am working on a manual, [here](Manuel.md).

I decided to create a version of DeciCalc that uses arbitrary-precision fixed-point numbers. The results of operations use the rules of POSIX bc. I stole some of my code from the Calculators repo to make the number system. While I took ideas from bc, I didn't take any code from any bc that I am familiar with (which is most of the versions of bc currently in production; in fact, looking at my conversion of GNU bc to GMP, I am not happy with how I performed division to get the correct scale (even if it is consistent with how Morris and Cherry did it)). In addition, some of the code I have doesn't make complete sense because of the pedigree of the code: before it used GMP as the back-end, it was using different code that I wrote. Oh yeah, this does require the GMP library, as it uses GMP to store and operate on numbers (plus that glue code I wrote to keep track of the operating scale). At this point, the number system is better, but removed, from bc.

POSIX rules: The scale of addition and subtraction is the max of the scale of the operands. The scale of division is the scale variable. The scale of multiplication is more complicated: let `a` and `b` be the scales of the two operands, and let `scale` be the value of the scale variable, then the resulting scale is `min(a+b, max(scale, a, b))`. Also, the default scale variable is initialized to zero.

Differences from POSIX bc: The default rounding mode is ties to even, and all operations are rounded. There is now an infinity and not-a-number representation, and it uses the projective closure.

New changes July 2023: I simplified the code to recalculate a cell. It no longer recurses nearly so much. Hopefully, the program will be more performant without too much of a loss in usability. Look at `AppKiller.html` for an example spreadsheet that really killed the program: it's a simple sheet to compute multiple iterations of Newton's Method.

New changes August 2023: The save file format has been modified in a backwards-compatible manner. However, new save files will load with missing columns with the older software.
Also August 2023: Save file format 3: the save file format will save loaded libraries into the saved spreadsheet so that they are automatically loaded when the spreadsheet is next loaded.
* Libraries loaded with -l will override functions saved in the spreadsheet.
* Libraries saved in the spreadsheet have all comments and formatting stripped from them.
* The save file format maintains a backwards-compatible HTML format that can be viewed with a web browser (albeit, the table is transposed). If no libraries are loaded and all columns are default width, then the spreadsheet is forwards-compatible.

November 2023: I have implemented features that are well beyond anything that I wanted out of this. It has taken on a life of its own, and is probably more capable than I even intended it to be. I hope you find it useful, but I intend no more major changes (must ... say ... NO ... to ... LAMBDA).

March 2024: Back in January, I got drunk and emailed Thomas Dickey, the maintainer of ncurses, and in a long, ranting email, I expressed my stance that my spreadsheet application was better than his tape calculator. But, also in the email, I asked for a project to make the spreadsheet better. He had a simple response: use non-blocking IO. It was very gracious of him to respond, and with a useful response. I've sat on that for two months because of implications. Really, if I made IO non-blocking, it would HAVE to be accompanied with making the sheet update non-blocking. At that point, though, why not add automation or automatic updates? Why not incorporate Backway (somehow)? Why not just make a spreadsheet-based curses game engine? Yeah: I went overboard in overthinking things. Finally, I decided to just make IO and sheet update non-blocking.


Limitations
-----------

GMP uses a 64 bit limb type and a 32 bit signed limb count. Theoretically, that will allow a 41 billion digit number. However, GMP doesn't appear to even attempt an allocation near four gigabytes (on Windows), so we're now down to ten billion digits. The program will most certainly unceremoniously crash before then. On Windows: I have had trouble generating numbers larger than `2^2^31` (six hundred million digits). I've also pushed scale up to `2^29` (around five hundred million) before the program crashes immediately. At `2^30` it doesn't even try. At that point, the program is almost unusable anyway. On Linux, I have lost patience before I have constructed a number that causes the program to crash.

You can probably "use" the program with a hundred million digits on each side of the decimal point. Your mileage will vary. If it is a concern: the `limit_scale.txt` file can be loaded as a library that limits the maximum scale that the scale variable can be set to. It is 10000 digits, but you can change that.


How To Build
------------

Unlike DeciCalc, this doesn't require checking out a second repo. Just type `make release` and you should be good.


Backwards
---------

I didn't rename the type from `Float` to `Fixed`, like I should have, but it was too much drudge work. The only changes to the language are: added GetPrecision, SetPrecision, GetDefaultPrecision, and SetDefaultPrecision. I recently closed the number system under division, and gave it an error form.


Standard Library
----------------

The following functions have been modified/added:

* MIN - now returns 'Empty' rather than positive infinity for an empty input
* MAX - now returns 'Empty' rather than negative infinity for an empty input
* GETSCALE - gets the scale setting
* SETSCALE - sets the scale setting
* GETROUND - gets the rounding setting
* SETROUND - sets the rounding setting
* EVAL - evaluate a label
* LET - define a name

New Standard Library
--------------------

The following functions are provided. EXP, SQRT, and LOG draw heavy inspiration from GNU bc's implementation of the same. I have no idea how ungodly precise POW makes arguments to produce good results (but I did make it a smaller number than my first attempt).

* EXP - The base of the natural logarithms raised to the argument power
* LOG - The natural logarithm
* SQRT - Square root
* RAISE - The first argument raised to an integer power second argument
* POW - The first argument raised to an arbitrary power second argument
* PMT - (interest rate per repayment period; repayment periods; present value)
* GETLIBROUND - Get the internal rounding mode of EXP, LOG, SQRT, and POW.
* SETLIBROUND - Set the internal rounding mode of those functions.

