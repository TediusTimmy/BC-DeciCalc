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
#include "gtest/gtest.h"

#include "Forwards/Types/ValueType.h"

#include "Forwards/Types/FloatValue.h"
#include "Forwards/Types/StringValue.h"
#include "Forwards/Types/NilValue.h"
#include "Forwards/Types/CellRefValue.h"
#include "Forwards/Types/CellRangeValue.h"

TEST(TypesTests, testFloats)
 {
   Forwards::Types::FloatValue defaulted;
   Forwards::Types::FloatValue low (BigInt::Fixed("1"));
   Forwards::Types::FloatValue med (BigInt::Fixed("5"));
   Forwards::Types::FloatValue high (BigInt::Fixed("10"));

   EXPECT_EQ("Float", defaulted.getTypeName());

   EXPECT_EQ("0", defaulted.toString(0U, 0U));
   EXPECT_EQ("1", low.toString(0U, 0U));
   EXPECT_EQ("5", med.toString(0U, 0U));
   EXPECT_EQ("10", high.toString(0U, 0U));

   EXPECT_EQ(Forwards::Types::FLOAT, defaulted.getType());
   EXPECT_EQ(Forwards::Types::FLOAT, low.getType());
   EXPECT_EQ(Forwards::Types::FLOAT, med.getType());
 }

TEST(TypesTests, testStrings)
 {
   Forwards::Types::StringValue defaulted;
   Forwards::Types::StringValue low ("A");
   Forwards::Types::StringValue med ("M");
   Forwards::Types::StringValue high ("Z");

   EXPECT_EQ("String", defaulted.getTypeName());

   EXPECT_EQ("", defaulted.toString(0U, 0U));
   EXPECT_EQ("A", low.toString(0U, 0U));
   EXPECT_EQ("M", med.toString(0U, 0U));
   EXPECT_EQ("Z", high.toString(0U, 0U));

   EXPECT_EQ(Forwards::Types::STRING, defaulted.getType());
   EXPECT_EQ(Forwards::Types::STRING, low.getType());
   EXPECT_EQ(Forwards::Types::STRING, med.getType());
 }

TEST(TypesTests, testNil)
 {
   Forwards::Types::NilValue defaulted;
   Forwards::Types::NilValue low;
   Forwards::Types::NilValue med;
   Forwards::Types::NilValue high;

   EXPECT_EQ("Nil", defaulted.getTypeName());

   EXPECT_EQ("Nil", defaulted.toString(0U, 0U));
   EXPECT_EQ("Nil", low.toString(0U, 0U));
   EXPECT_EQ("Nil", med.toString(0U, 0U));
   EXPECT_EQ("Nil", high.toString(0U, 0U));

   EXPECT_EQ(Forwards::Types::NIL, defaulted.getType());
   EXPECT_EQ(Forwards::Types::NIL, low.getType());
   EXPECT_EQ(Forwards::Types::NIL, med.getType());
 }

TEST(TypesTests, testCellRef)
 {
   Forwards::Types::CellRefValue defaulted;
   Forwards::Types::CellRefValue low (true, 5, false, 5);
   Forwards::Types::CellRefValue med (false, 6, true, 6);
   Forwards::Types::CellRefValue high (true, 3, true, 4);
   Forwards::Types::CellRefValue _11 (false, -5, false, -4);

   EXPECT_EQ("CellRef", defaulted.getTypeName());

   EXPECT_EQ("B2", defaulted.toString(1U, 1U));
   EXPECT_EQ("F6", defaulted.toString(5U, 5U));
   EXPECT_EQ("$F7", low.toString(1U, 1U));
   EXPECT_EQ("$F11", low.toString(5U, 5U));
   EXPECT_EQ("H$7", med.toString(1U, 1U));
   EXPECT_EQ("M$7", med.toString(6U, 6U));
   EXPECT_EQ("$D$5", high.toString(1U, 1U));
   EXPECT_EQ("$D$5", high.toString(8U, 8U));
   EXPECT_EQ("B2", _11.toString(6U, 5U));
   EXPECT_EQ("G6", _11.toString(11U, 9U));

   EXPECT_EQ("Z1", defaulted.toString(25U, 0U));
   EXPECT_EQ("AA1", defaulted.toString(26U, 0U));
   EXPECT_EQ("ZZ1", defaulted.toString(701U, 0U));
   EXPECT_EQ("AAA1", defaulted.toString(702U, 0U));
   EXPECT_EQ("ZZZ1", defaulted.toString(18277U, 0U));

   EXPECT_EQ(Forwards::Types::CELL_REF, defaulted.getType());
   EXPECT_EQ(Forwards::Types::CELL_REF, low.getType());
   EXPECT_EQ(Forwards::Types::CELL_REF, med.getType());

   EXPECT_EQ("ZZW999999997", _11.toString(1U, 1U));
   EXPECT_EQ("ZZW1", _11.toString(1U, 4U));
   EXPECT_EQ("ZZZ999999999", _11.toString(4U, 3U));

   Forwards::Types::CellRefValue _12 (false, 5, false, 4);
   EXPECT_EQ("ZZZ999999999", _12.toString(18278U - 6U, 999999999U - 5U));
   EXPECT_EQ("A1", _12.toString(18278U - 5U, 999999999U - 4U));
 }

TEST(TypesTests, testCellRange)
 {
   Forwards::Types::CellRangeValue defaulted;
   Forwards::Types::CellRangeValue low (1, 1, 2, 2);
   Forwards::Types::CellRangeValue med (3, 5, 7, 9);
   Forwards::Types::CellRangeValue high (9, 7, 5, 3); // Not a real case, but no error checking.

   EXPECT_EQ("CellRange", defaulted.getTypeName());

   EXPECT_EQ("B2:B2", defaulted.toString(0U, 0U));
   EXPECT_EQ("B2:B2", defaulted.toString(5U, 5U));
   EXPECT_EQ("B2:C3", low.toString(0U, 0U));
   EXPECT_EQ("B2:C3", low.toString(5U, 5U));
   EXPECT_EQ("D6:H10", med.toString(0U, 0U));
   EXPECT_EQ("D6:H10", med.toString(6U, 6U));
   EXPECT_EQ("J8:F4", high.toString(0U, 0U));
   EXPECT_EQ("J8:F4", high.toString(8U, 8U));

   EXPECT_EQ(Forwards::Types::CELL_RANGE, defaulted.getType());
   EXPECT_EQ(Forwards::Types::CELL_RANGE, low.getType());
   EXPECT_EQ(Forwards::Types::CELL_RANGE, med.getType());
 }
