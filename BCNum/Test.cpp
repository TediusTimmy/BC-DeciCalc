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

#include "Fixed.hpp"

   // Basically, making this legacy code no longer legacy by creating tests.
   // Run IO through its paces, so that we can use it as a root of trust for further tests.
TEST(FixedTests, testNoBadBoom)
 {
   BigInt::Fixed test ("1.2");
   EXPECT_EQ("1.2", test.toString());

   test.fromString("1,2");
   EXPECT_EQ("1.2", test.toString());

   test.fromString("9000");
   EXPECT_EQ("9000", test.toString());

   test.fromString("-9000");
   EXPECT_EQ("-9000", test.toString());

   test.fromString(".002");
   EXPECT_EQ("0.002", test.toString());

   test.fromString(",003");
   EXPECT_EQ("0.003", test.toString());

   test.fromString("9000.002");
   EXPECT_EQ("9000.002", test.toString());

   test.fromString("8000,003");
   EXPECT_EQ("8000.003", test.toString());

      // Gross. What was decade-ago me thinking?
   test.fromString("9000A");
   EXPECT_EQ("0", test.toString());

   test.fromString("9000.A");
   EXPECT_EQ("9000", test.toString());

   test.fromString("9000.");
   EXPECT_EQ("9000", test.toString());

   test.fromString("9000.02A");
   EXPECT_EQ("9000.02", test.toString());

      // I thought I saw a bug here....
   test.fromString("-9000.002");
   EXPECT_EQ("-9000.002", test.toString());

   test.fromString("-8000,003");
   EXPECT_EQ("-8000.003", test.toString());

   test.fromString("-.002"); // I did! I did see a bug here!
   EXPECT_EQ("-0.002", test.toString());

   test.fromString("-,003"); // I did! I did see a bug here!
   EXPECT_EQ("-0.003", test.toString());


   test.fromString("0");
   EXPECT_EQ("0", test.toString());

   test.fromString("0.00");
   EXPECT_EQ("0.00", test.toString());

   test.fromString("0.");
   EXPECT_EQ("0", test.toString());


   test.fromString("12.123");
   EXPECT_EQ("12.123", test.toString());

   BigInt::Fixed test2 (test);
   EXPECT_EQ(test.toString(), test2.toString());

   BigInt::Fixed test3 (test.toString());
   EXPECT_EQ(test.toString(), test3.toString());

   test2 = -test;
   EXPECT_EQ("12.123", test.toString());
   EXPECT_EQ("-12.123", test2.toString());


   test.fromString("12.123e0");
   EXPECT_EQ("12.123", test.toString());

   test.fromString("12.123e3");
   EXPECT_EQ("12123.000", test.toString());

   test.fromString("12.123e+3");
   EXPECT_EQ("12123.000", test.toString());

   test.fromString("12.123e-3");
   EXPECT_EQ("0.012123", test.toString());

   test.fromString("12.123E0");
   EXPECT_EQ("12.123", test.toString());

   test.fromString("12.123E3");
   EXPECT_EQ("12123.000", test.toString());

   test.fromString("12.123E+3");
   EXPECT_EQ("12123.000", test.toString());

   test.fromString("12.123E-3");
   EXPECT_EQ("0.012123", test.toString());

   BigInt::Fixed i (true, false);
   BigInt::Fixed n (false, true);

   EXPECT_EQ("Infinity", i.toString());
   EXPECT_EQ("Not a Result", n.toString());


   test.fromString("12e0");
   EXPECT_EQ("12", test.toString());

   test.fromString("12e1");
   EXPECT_EQ("120", test.toString());

   test.fromString("12e+1");
   EXPECT_EQ("120", test.toString());

   test.fromString("12e-1");
   EXPECT_EQ("1.2", test.toString());

   test.fromString("12e"); // None of the lexers should ever give me this,
   EXPECT_EQ("12", test.toString()); // but make sure it doesn't crash.

   test.fromString("12E0");
   EXPECT_EQ("12", test.toString());

   test.fromString("12E1");
   EXPECT_EQ("120", test.toString());

   test.fromString("12E+1");
   EXPECT_EQ("120", test.toString());

   test.fromString("12E-1");
   EXPECT_EQ("1.2", test.toString());

   test.fromString("12E"); // None of the lexers should ever give me this,
   EXPECT_EQ("12", test.toString()); // but make sure it doesn't crash.
 }

TEST(FixedTests, testAdds)
 {
   BigInt::Fixed a ("1.35");
   BigInt::Fixed b ("1.125");
   BigInt::Fixed c;

   c = a + b;
   EXPECT_EQ("2.475", c.toString());

   c = b + a;
   EXPECT_EQ("2.475", c.toString());

   c = a + a;
   EXPECT_EQ("2.70", c.toString());

      // Those three covered the Fixed adder. Now, let's get the Integer portion: negative numbers, zeros, result zero.

   b.fromString("0.000");

   c = a + b;
   EXPECT_EQ("1.350", c.toString());

   c = b + a;
   EXPECT_EQ("1.350", c.toString());


   b.fromString("-1.35");

   c = a + b;
   EXPECT_EQ("0.00", c.toString());

   a.fromString("-1.35");
   b.fromString("-1.125");

   c = a + b;
   EXPECT_EQ("-2.475", c.toString());

   c = b + a;
   EXPECT_EQ("-2.475", c.toString());

   c = a + a;
   EXPECT_EQ("-2.70", c.toString());

   a.fromString("-1.35");
   b.fromString("1.125");

   c = a + b;
   EXPECT_EQ("-0.225", c.toString());

   c = b + a;
   EXPECT_EQ("-0.225", c.toString());

   a.fromString("1.35");
   b.fromString("-1.125");

   c = a + b;
   EXPECT_EQ("0.225", c.toString());

   c = b + a;
   EXPECT_EQ("0.225", c.toString());
 }

TEST(FixedTests, testSubs)
 {
   BigInt::Fixed a ("1.35");
   BigInt::Fixed b ("1.125");
   BigInt::Fixed c;

   c = a - b;
   EXPECT_EQ("0.225", c.toString());

   c = b - a;
   EXPECT_EQ("-0.225", c.toString());

   c = a - a;
   EXPECT_EQ("0.00", c.toString());


   b.fromString("0.000");

   c = a - b;
   EXPECT_EQ("1.350", c.toString());

   c = b - a;
   EXPECT_EQ("-1.350", c.toString());


   b.fromString("-1.35");

   c = a - b;
   EXPECT_EQ("2.70", c.toString());

   a.fromString("-1.35");
   b.fromString("-1.125");

   c = a - b;
   EXPECT_EQ("-0.225", c.toString());

   c = b - a;
   EXPECT_EQ("0.225", c.toString());

   c = a - a;
   EXPECT_EQ("0.00", c.toString());

   a.fromString("-1.35");
   b.fromString("1.125");

   c = a - b;
   EXPECT_EQ("-2.475", c.toString());

   c = b - a;
   EXPECT_EQ("2.475", c.toString());

   a.fromString("1.35");
   b.fromString("-1.125");

   c = a - b;
   EXPECT_EQ("2.475", c.toString());

   c = b - a;
   EXPECT_EQ("-2.475", c.toString());
 }

TEST(FixedTests, testComparisons)
 {
   BigInt::Fixed a ("1.25");
   BigInt::Fixed b ("2.25");

   EXPECT_FALSE(a >  b);
   EXPECT_FALSE(a >= b);
   EXPECT_TRUE (a <  b);
   EXPECT_TRUE (a <= b);
   EXPECT_FALSE(a == b);
   EXPECT_TRUE (a != b);

   EXPECT_TRUE (b >  a);
   EXPECT_TRUE (b >= a);
   EXPECT_FALSE(b <  a);
   EXPECT_FALSE(b <= a);
   EXPECT_FALSE(b == a);
   EXPECT_TRUE (b != a);

   BigInt::Fixed c ("1.25");
   BigInt::Fixed d ("1.250");

   EXPECT_FALSE(c >  d);
   EXPECT_TRUE (c >= d);
   EXPECT_FALSE(c <  d);
   EXPECT_TRUE (c <= d);
   EXPECT_TRUE (c == d);
   EXPECT_FALSE(c != d);

   EXPECT_FALSE(d >  c);
   EXPECT_TRUE (d >= c);
   EXPECT_FALSE(d <  c);
   EXPECT_TRUE (d <= c);
   EXPECT_TRUE (d == c);
   EXPECT_FALSE(d != c);

   BigInt::Fixed e ("-1.25");
   BigInt::Fixed f ("1.25");

   EXPECT_FALSE(e >  f);
   EXPECT_FALSE(e >= f);
   EXPECT_TRUE (e <  f);
   EXPECT_TRUE (e <= f);
   EXPECT_FALSE(e == f);
   EXPECT_TRUE (e != f);

   EXPECT_TRUE (f >  e);
   EXPECT_TRUE (f >= e);
   EXPECT_FALSE(f <  e);
   EXPECT_FALSE(f <= e);
   EXPECT_FALSE(f == e);
   EXPECT_TRUE (f != e);

   BigInt::Fixed g ("0");
   BigInt::Fixed h ("1");

   EXPECT_FALSE(g >  h);
   EXPECT_FALSE(g >= h);
   EXPECT_TRUE (g <  h);
   EXPECT_TRUE (g <= h);
   EXPECT_FALSE(g == h);
   EXPECT_TRUE (g != h);

   EXPECT_TRUE (h >  g);
   EXPECT_TRUE (h >= g);
   EXPECT_FALSE(h <  g);
   EXPECT_FALSE(h <= g);
   EXPECT_FALSE(h == g);
   EXPECT_TRUE (h != g);

   BigInt::Fixed i ("0");
   EXPECT_TRUE(g == i);

   BigInt::Fixed j ("-1.25");
   BigInt::Fixed k ("-2.25");

   EXPECT_TRUE (j >  k);
   EXPECT_TRUE (j >= k);
   EXPECT_FALSE(j <  k);
   EXPECT_FALSE(j <= k);
   EXPECT_FALSE(j == k);
   EXPECT_TRUE (j != k);

   EXPECT_FALSE(k >  j);
   EXPECT_FALSE(k >= j);
   EXPECT_TRUE (k <  j);
   EXPECT_TRUE (k <= j);
   EXPECT_FALSE(k == j);
   EXPECT_TRUE (k != j);
 }

   // Huh? Wonder where this table cam from?....
TEST(FixedTests, testRounding) // Is this OVERKILL? Maybe.
 {
   EXPECT_EQ(BigInt::ROUND_TIES_EVEN, BigInt::Fixed::getRoundMode());

   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_EVEN);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_AWAY);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_POSITIVE_INFINITY);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_NEGATIVE_INFINITY);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_ZERO);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_ODD);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_ZERO);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_AWAY);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_DOUBLE);

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, false, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, false));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, false, false));

   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, 0, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, 0, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(true, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(true, false, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, true, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, true, -1, false, true));
   EXPECT_FALSE(BigInt::Fixed::decideRound(false, false, -1, true, true));
   EXPECT_TRUE(BigInt::Fixed::decideRound(false, false, -1, false, true));


   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_EVEN);
 }

   // This function is the foundation of using Fixeds in code.
   // It is the only time that client code calls an Integer function directly.
TEST(FixedTests, testRoundInt)
 {
   BigInt::Fixed test ("2.5");
   EXPECT_EQ(2, test.roundToInteger().toInt()); // Assume round to even

   test.fromString("3.5");
   EXPECT_EQ(4, test.roundToInteger().toInt());

   test.fromString("-2.5");
   EXPECT_EQ(-2, test.roundToInteger().toInt());

   test.fromString("-3.5");
   EXPECT_EQ(-4, test.roundToInteger().toInt());

   test.fromString("3.2");
   EXPECT_EQ(3, test.roundToInteger().toInt());

   test.fromString("-3.2");
   EXPECT_EQ(-3, test.roundToInteger().toInt());

   test.fromString("5");
   EXPECT_EQ(5, test.roundToInteger().toInt());
 }

TEST(FixedTests, testMulterply)
 {
   BigInt::Fixed a ("2");
   BigInt::Fixed b ("3");
   BigInt::Fixed c;

   c = a * b;
   EXPECT_EQ("6", c.toString());

   c = b * a;
   EXPECT_EQ("6", c.toString());

   BigInt::Fixed d ("0");

   c = a * d;
   EXPECT_EQ("0", c.toString());

   c = d * a;
   EXPECT_EQ("0", c.toString());


   BigInt::Fixed e (".25");
   BigInt::Fixed f (".5");
   EXPECT_EQ(0U, BigInt::Fixed::getDefaultPrecision());

   c = e * f; // min(3, max(2, 1, 0)) == 2
   EXPECT_EQ("0.12", c.toString());

   c = f * e; // min(3, max(1, 2, 0)) == 2
   EXPECT_EQ("0.12", c.toString());

   BigInt::Fixed::setDefaultPrecision(3U);

   c = e * f; // min(3, max(2, 1, 3)) == 3
   EXPECT_EQ("0.125", c.toString());

   BigInt::Fixed::setDefaultPrecision(4U);

   c = e * f; // min(3, max(2, 1, 4)) == 3
   EXPECT_EQ("0.125", c.toString());

   BigInt::Fixed::setDefaultPrecision(0U);
 }

TEST(FixedTests, testDerverd)
 {
   BigInt::Fixed a ("0");
   BigInt::Fixed b ("3");
   BigInt::Fixed c;

   c = a / b;
   EXPECT_EQ("0", c.toString());

   c = b / a;
   EXPECT_EQ("Infinity", c.toString());

   a.fromString("1");
   c = a / b;
   EXPECT_EQ("0", c.toString());

   a.fromString("6");
   c = a / b;
   EXPECT_EQ("2", c.toString());

   a.fromString("2");
   b.fromString("-3");
   c = a / b;
   EXPECT_EQ("-1", c.toString());

   a.fromString("100.000");
   b.fromString("50.00");
   c = a / b;
   EXPECT_EQ("2", c.toString());

   a.fromString("125.000");
   b.fromString("50.00");
   c = a / b;
   EXPECT_EQ("2", c.toString());

   a.fromString("175.000");
   b.fromString("50.00");
   c = a / b;
   EXPECT_EQ("4", c.toString());

   a.fromString("170.000");
   b.fromString("50.00");
   c = a / b;
   EXPECT_EQ("3", c.toString());

   BigInt::Fixed::setDefaultPrecision(7U);
   a.fromString("2.000");
   b.fromString("3.00");
   c = a / b;
   EXPECT_EQ("0.6666667", c.toString());

   BigInt::Fixed::setRoundMode(BigInt::ROUND_ZERO);
   c = a / b;
   EXPECT_EQ("0.6666666", c.toString());

   BigInt::Fixed::setDefaultPrecision(0U);
   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_EVEN);
 }

TEST(FixedTests, testNewRoundFunction)
 {
   BigInt::Fixed test ("2");
   EXPECT_EQ("2", test.roundToInteger(BigInt::ROUND_TIES_EVEN).toString());

   test.fromString("2.5");
   EXPECT_EQ("2.0", test.roundToInteger(BigInt::ROUND_TIES_EVEN).toString());
   EXPECT_EQ("2.0", test.roundToInteger(BigInt::ROUND_ZERO).toString());
   EXPECT_EQ("3.0", test.roundToInteger(BigInt::ROUND_POSITIVE_INFINITY).toString());
   EXPECT_EQ("2.0", test.roundToInteger(BigInt::ROUND_NEGATIVE_INFINITY).toString());

   test.fromString("3.5");
   EXPECT_EQ("4.0", test.roundToInteger(BigInt::ROUND_TIES_EVEN).toString());
   EXPECT_EQ("3.0", test.roundToInteger(BigInt::ROUND_ZERO).toString());
   EXPECT_EQ("4.0", test.roundToInteger(BigInt::ROUND_POSITIVE_INFINITY).toString());
   EXPECT_EQ("3.0", test.roundToInteger(BigInt::ROUND_NEGATIVE_INFINITY).toString());

   test.fromString("-3.5");
   EXPECT_EQ("-4.0", test.roundToInteger(BigInt::ROUND_TIES_EVEN).toString());
   EXPECT_EQ("-3.0", test.roundToInteger(BigInt::ROUND_ZERO).toString());
   EXPECT_EQ("-3.0", test.roundToInteger(BigInt::ROUND_POSITIVE_INFINITY).toString());
   EXPECT_EQ("-4.0", test.roundToInteger(BigInt::ROUND_NEGATIVE_INFINITY).toString());

   test.fromString("-4.5");
   EXPECT_EQ("-4.0", test.roundToInteger(BigInt::ROUND_TIES_EVEN).toString());
   EXPECT_EQ("-4.0", test.roundToInteger(BigInt::ROUND_ZERO).toString());
   EXPECT_EQ("-4.0", test.roundToInteger(BigInt::ROUND_POSITIVE_INFINITY).toString());
   EXPECT_EQ("-5.0", test.roundToInteger(BigInt::ROUND_NEGATIVE_INFINITY).toString());

   test.fromString("200.8675");
   EXPECT_EQ("201.0000", test.roundToInteger(BigInt::ROUND_TIES_EVEN).toString());
 }

TEST(FixedTests, testZeros)
 {
   BigInt::Fixed test (0LL, 2U);
   EXPECT_EQ("0.00", test.toString());
   test = -test;
   EXPECT_EQ("0.00", test.toString());
   EXPECT_EQ(0, test.roundToInteger().toInt());

   test = BigInt::Fixed("10000000000000000000000000");
   EXPECT_EQ(0, test.roundToInteger().toInt());
 }

TEST(FixedTests, testNewRoundMode)
 {
   BigInt::Fixed a ("1");
   BigInt::Fixed b ("3");
   BigInt::Fixed c;

   BigInt::Fixed::setRoundMode(BigInt::ROUND_DOUBLE);

   c = a / b; // 0 rem 1 : round away
   EXPECT_EQ("1", c.toString());

   a.fromString("16");
   c = a / b; // 5 rem 1 : round away
   EXPECT_EQ("6", c.toString());

   a.fromString("4");
   c = a / b; // 1 rem 1 : no round
   EXPECT_EQ("1", c.toString());

   a.fromString("19");
   c = a / b; // 6 rem 1 : no round
   EXPECT_EQ("6", c.toString());

   a.fromString("15");
   c = a / b; // 5 rem 0 : no round
   EXPECT_EQ("5", c.toString());

   a.fromString("30");
   c = a / b; // 10 rem 0 : no round
   EXPECT_EQ("10", c.toString());

   a.fromString("31");
   c = a / b; // 10 rem 1 : round away
   EXPECT_EQ("11", c.toString());

   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_EVEN);

 }

TEST(FixedTests, testNowImpossible)
 {
   BigInt::Integer three (3UL);
   BigInt::Integer zero (0UL);
   BigInt::Integer quot;
   BigInt::Integer rem;

      // This division case is no longer possible, because infinity detection happens first.
   BigInt::quotrem(three, zero, quot, rem);

   EXPECT_EQ(0, zero.compare(quot));
   EXPECT_EQ(0, three.compare(rem));
 }

TEST(FixedTests, testNewComparisonCases)
 {
   BigInt::Fixed a ("1");
   BigInt::Fixed i (true, false);
   BigInt::Fixed n (false, true);

   EXPECT_FALSE(i < a);
   EXPECT_FALSE(a < i);
   EXPECT_FALSE(n < a);
   EXPECT_FALSE(a < n);
   EXPECT_FALSE(i < i);
   EXPECT_FALSE(n < n);

   EXPECT_FALSE(i <= a);
   EXPECT_FALSE(a <= i);
   EXPECT_FALSE(n <= a);
   EXPECT_FALSE(a <= n);
   EXPECT_FALSE(i <= i);
   EXPECT_FALSE(n <= n);

   EXPECT_FALSE(i > a);
   EXPECT_FALSE(a > i);
   EXPECT_FALSE(n > a);
   EXPECT_FALSE(a > n);
   EXPECT_FALSE(i > i);
   EXPECT_FALSE(n > n);

   EXPECT_FALSE(i >= a);
   EXPECT_FALSE(a >= i);
   EXPECT_FALSE(n >= a);
   EXPECT_FALSE(a >= n);
   EXPECT_FALSE(i >= i);
   EXPECT_FALSE(n >= n);

   EXPECT_FALSE(i == a);
   EXPECT_FALSE(a == i);
   EXPECT_FALSE(n == a);
   EXPECT_FALSE(a == n);
   EXPECT_TRUE(i == i);
   EXPECT_FALSE(n == n);

   EXPECT_TRUE(i != a);
   EXPECT_TRUE(a != i);
   EXPECT_TRUE(n != a);
   EXPECT_TRUE(a != n);
   EXPECT_FALSE(i != i);
   EXPECT_TRUE(n != n);
 }

TEST(FixedTests, testNewMathCases)
 {
   BigInt::Fixed a ("1");
   BigInt::Fixed i (true, false);
   BigInt::Fixed n (false, true);
   BigInt::Fixed z;
   BigInt::Fixed r;

   r = a + i;
   EXPECT_TRUE(r.isInf());

   r = i + a;
   EXPECT_TRUE(r.isInf());

   r = a + n;
   EXPECT_TRUE(r.isNaN());

   r = n + a;
   EXPECT_TRUE(r.isNaN());

   r = i + n;
   EXPECT_TRUE(r.isNaN());

   r = n + i;
   EXPECT_TRUE(r.isNaN());

   r = n + n;
   EXPECT_TRUE(r.isNaN());

   r = i + i;
   EXPECT_TRUE(r.isNaN());


   r = a - i;
   EXPECT_TRUE(r.isInf());

   r = i - a;
   EXPECT_TRUE(r.isInf());

   r = a - n;
   EXPECT_TRUE(r.isNaN());

   r = n - a;
   EXPECT_TRUE(r.isNaN());

   r = i - n;
   EXPECT_TRUE(r.isNaN());

   r = n - i;
   EXPECT_TRUE(r.isNaN());

   r = n - n;
   EXPECT_TRUE(r.isNaN());

   r = i - i;
   EXPECT_TRUE(r.isNaN());


   r = a * i;
   EXPECT_TRUE(r.isInf());

   r = i * a;
   EXPECT_TRUE(r.isInf());

   r = a * n;
   EXPECT_TRUE(r.isNaN());

   r = n * a;
   EXPECT_TRUE(r.isNaN());

   r = i * n;
   EXPECT_TRUE(r.isNaN());

   r = n * i;
   EXPECT_TRUE(r.isNaN());

   r = i * i;
   EXPECT_TRUE(r.isInf());

   r = i * z;
   EXPECT_TRUE(r.isNaN());

   r = z * i;
   EXPECT_TRUE(r.isNaN());


   r = a / i;
   EXPECT_TRUE(r.isZero());

   r = i / a;
   EXPECT_TRUE(r.isInf());

   r = a / n;
   EXPECT_TRUE(r.isNaN());

   r = n / a;
   EXPECT_TRUE(r.isNaN());

   r = i / n;
   EXPECT_TRUE(r.isNaN());

   r = n / i;
   EXPECT_TRUE(r.isNaN());

   r = i / i;
   EXPECT_TRUE(r.isNaN());

   r = a / z;
   EXPECT_TRUE(r.isInf());

   r = z / a;
   EXPECT_TRUE(r.isZero());

   r = z / z;
   EXPECT_TRUE(r.isNaN());
 }
