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

#include <vector>

#include "Forwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

TEST(LexerTests, testEverythingAndTheKitchenSink)
 {
   std::vector<std::pair<std::string, Forwards::Input::Lexeme> > tests;

   tests.push_back(std::make_pair("12",                   Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.",                  Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12",                  Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2",                  Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,",                  Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12",                  Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2",                  Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12e3",                 Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.e3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12e3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2e3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,e3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12e3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2e3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12e+3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.e+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12e+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2e+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,e+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12e+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2e+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12e-3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.e-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12e-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2e-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,e-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12e-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2e-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12E3",                 Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.E3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12E3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2E3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,E3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12E3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2E3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12E+3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.E+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12E+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2E+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,E+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12E+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2E+3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12E-3",                Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.E-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12E-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2E-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,E-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12E-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2E-3",               Forwards::Input::NUMBER));
   tests.push_back(std::make_pair("@program",             Forwards::Input::IDENTIFIER));
   tests.push_back(std::make_pair("@",                    Forwards::Input::INVALID));
   tests.push_back(std::make_pair("=",                    Forwards::Input::EQUALITY));
   tests.push_back(std::make_pair("<>",                   Forwards::Input::INEQUALITY));
   tests.push_back(std::make_pair(":",                    Forwards::Input::RANGE));
   tests.push_back(std::make_pair(">",                    Forwards::Input::GREATER_THAN));
   tests.push_back(std::make_pair("<",                    Forwards::Input::LESS_THAN));
   tests.push_back(std::make_pair(">=",                   Forwards::Input::GREATER_THAN_OR_EQUAL_TO));
   tests.push_back(std::make_pair("<=",                   Forwards::Input::LESS_THAN_OR_EQUAL_TO));
   tests.push_back(std::make_pair("+",                    Forwards::Input::PLUS));
   tests.push_back(std::make_pair("-",                    Forwards::Input::MINUS));
   tests.push_back(std::make_pair("*",                    Forwards::Input::MULTIPLY));
   tests.push_back(std::make_pair("/",                    Forwards::Input::DIVIDE));
   tests.push_back(std::make_pair("&",                    Forwards::Input::CAT));
   tests.push_back(std::make_pair(";",                    Forwards::Input::SEMICOLON));
   tests.push_back(std::make_pair("(",                    Forwards::Input::OPEN_PARENS));
   tests.push_back(std::make_pair(")",                    Forwards::Input::CLOSE_PARENS));
   tests.push_back(std::make_pair(",",                    Forwards::Input::INVALID));
   tests.push_back(std::make_pair("#",                    Forwards::Input::INVALID));
   tests.push_back(std::make_pair("A1",                   Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("AA1",                  Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("aa1",                  Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("AAA1",                 Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("AAAA1",                Forwards::Input::INVALID));
   tests.push_back(std::make_pair("A2",                   Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A22",                  Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A999999999",           Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A1000000000",          Forwards::Input::INVALID));
   tests.push_back(std::make_pair("A",                    Forwards::Input::INVALID));
   tests.push_back(std::make_pair("$",                    Forwards::Input::INVALID));
   tests.push_back(std::make_pair("$$",                   Forwards::Input::INVALID));
   tests.push_back(std::make_pair("$A1",                  Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("$AA1",                 Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("$AAA1",                Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("$AAAA1",               Forwards::Input::INVALID));
   tests.push_back(std::make_pair("A$2",                  Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A$22",                 Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A$999999999",          Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A$1000000000",         Forwards::Input::INVALID));
   tests.push_back(std::make_pair("$A$2",                 Forwards::Input::CELL_REFERENCE));
   tests.push_back(std::make_pair("A0",                   Forwards::Input::INVALID));

   for (std::vector<std::pair<std::string, Forwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input (iter->first);
      Forwards::Input::Lexer lexi(input);
      Forwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);

      if (("program" == test.text) || ("aa1" == test.text))
       {
         FAIL();
       }
    }

   for (std::vector<std::pair<std::string, Forwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input ("   " + iter->first + "   ");
      Forwards::Input::Lexer lexi(input);
      Forwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);
    }

   for (std::vector<std::pair<std::string, Forwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input ("\t\t\t" + iter->first + "\t\t\t");
      Forwards::Input::Lexer lexi(input);
      Forwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);
    }
 }
