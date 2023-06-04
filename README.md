BC-DeciCalc
===========

I decided to create a version of DeciCalc that uses arbitrary-precision fixed-point numbers. The results of operations use the rules of POSIX bc. I stole some of my code from the Calculators repo to make the number system. While I took ideas from bc, I didn't take any code from any bc that I am familiar with (which is most of the versions of bc currently in production; in fact, looking at my conversion of GNU bc to GMP, I am not happy with how I performed division to get the correct scale). In addition, some of the code I have doesn't make complete sense because of the pedigree of the code: before it used GMP as the back-end, it was using different code that I wrote. Oh yeah, this does require the GMP library, as it uses GMP to store and operate on numbers (plus that glue code I wrote to keep track of the operating scale).

POSIX rules: The scale of addition and subtraction is the max of the scale of the operands. The scale of division is the scale variable. The scale of multiplication is more complicated: let `a` and `b` be the scales of the two operands, and let `scale` be the value of the scale variable, then the resulting scale is `min(a+b, max(scale, a, b))`. Also, the default scale variable is initialized to zero.

Differences from POSIX bc: The default rounding mode is ties to even, and all operations are rounded.


How To Build
------------

Unlike DeciCalc, this doesn't require checking out a second repo. Just type `make release` and you should be good.


Backwards
---------

I didn't rename the type from `Float` to `Fixed`, like I should have, but it was too much drudge work. The only changes to the language are: removed NaN, IsNaN, and IsInfinity; added GetPrecision, SetPrecision, GetDefaultPrecision, and SetDefaultPrecision; and made division by zero throw an exception. So, the number system is no longer closed under division, and doesn't have an error form.


Standard Library
----------------

The following functions have been modified/removed/added:

* MIN - now returns 'Empty' rather than positive infinity for an empty input
* MAX - now returns 'Empty' rather than negative infinity for an empty input
* NAN - removed
* GETSCALE - gets the scale setting
* SETSCALE - sets the scale setting
