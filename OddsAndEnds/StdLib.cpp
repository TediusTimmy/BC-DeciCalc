/*
BSD 3-Clause License

Copyright (c) 2023, Thomas DiModica
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

extern const char* const STDLIB =

"set MAX to function (x) is "
   "set result to 'Empty' "
   "set found to 0 "
   "for item in x do "
      "set temp to item "
      "if IsCellRef(item) then "
         "set temp to EvalCell(item) "
      "end "
      "if IsFloat(temp) then "
         "if found then "
            "set result to Max(result; temp) "
         "else "
            "set result to temp "
            "set found to 1 "
         "end "
      "elseif IsCellRange(temp) then "
         "set temp to MAX(temp) "
         "if !IsString(temp) then "
            "if found then "
               "set result to Max(result; temp) "
            "else "
               "set result to temp "
               "set found to 1 "
            "end "
         "end "
      "end "
   "end "
   "return result "
"end "

"set MIN to function (x) is "
   "set result to 'Empty' "
   "set found to 0 "
   "for item in x do "
      "set temp to item "
      "if IsCellRef(item) then "
         "set temp to EvalCell(item) "
      "end "
      "if IsFloat(temp) then "
         "if found then "
            "set result to Min(result; temp) "
         "else "
            "set result to temp "
            "set found to 1 "
         "end "
      "elseif IsCellRange(temp) then "
         "set temp to MIN(temp) "
         "if !IsString(temp) then "
            "if found then "
               "set result to Min(result; temp) "
            "else "
               "set result to temp "
               "set found to 1 "
            "end "
         "end "
      "end "
   "end "
   "return result "
"end "

"set SUM to function (x) is "
   "set result to 0 "
   "for item in x do "
      "set temp to item "
      "if IsCellRef(item) then "
         "set temp to EvalCell(item) "
      "end "
      "if IsFloat(temp) then "
         "set result to result + temp "
      "elseif IsCellRange(temp) then "
         "set result to result + SUM(temp) "
      "end "
   "end "
   "return result "
"end "

"set COUNT to function (x) is "
   "set result to 0 "
   "for item in x do "
      "set temp to item "
      "if IsCellRef(item) then "
         "set temp to EvalCell(item) "
      "end "
      "if IsFloat(temp) then "
         "set result to result + 1 "
      "elseif IsCellRange(temp) then "
         "set result to result + COUNT(temp) "
      "end "
   "end "
   "return result "
"end "

"set AVERAGE to function (x) is "
   "return SUM(x) / COUNT(x) "
"end "

"set NAN to function (x) is "
   "return NaN() "
"end "

"set ABS to function (x) is "
   "set temp to EvalCell(x[0]) "
   "if !IsNil(temp) then "
      "set temp to Abs(temp) "
   "end "
   "return temp "
"end "

"set INT to function (x) is "
   "set temp to EvalCell(x[0]) "
   "if !IsNil(temp) then "
      "if temp < 0 then "
         "set temp to SetPrecision(Ceil(temp); 0) "
      "else "
         "set temp to SetPrecision(Floor(temp); 0) "
      "end "
   "end "
   "return temp "
"end "

"set ROUND to function (x) is "
   "set temp to EvalCell(x[0]) "
   "if !IsNil(temp) then "
      "set temp to SetPrecision(Round(temp); 0) "
   "end "
   "return temp "
"end "

"set SETSCALE to function (x) is "
   "return SetDefaultPrecision(EvalCell(x[0])) "
"end "

"set GETSCALE to function (x) is "
   "return GetDefaultPrecision() "
"end "

"set SETROUND to function (x) is "
   "return SetRoundMode(EvalCell(x[0])) "
"end "

"set GETROUND to function (x) is "
   "return GetRoundMode() "
"end "

"set EVAL to function (x) is "
   "set temp to EvalCell(x[0]) "
   "if !IsNil(temp) then "
      "set temp to CellEval(temp) "
   "end "
   "return temp "
"end "

"set LET to function (x) is "
   "return Let(EvalCell(x[0]); x[1]) "
"end "
;