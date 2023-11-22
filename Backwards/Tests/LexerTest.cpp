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

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"
#include "Backwards/Input/LineBufferedStreamInput.h"
#include "Backwards/Input/BufferedGenericInput.h"

TEST(LexerTests, testEverythingAndTheKitchenSink)
 {
   std::vector<std::pair<std::string, Backwards::Input::Lexeme> > tests;

   tests.push_back(std::make_pair("12",                   Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.",                  Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12",                  Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2",                  Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,",                  Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12",                  Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2",                  Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12e3",                 Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.e3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12e3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2e3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,e3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12e3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2e3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12e+3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.e+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12e+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2e+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,e+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12e+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2e+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12e-3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.e-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12e-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2e-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,e-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12e-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2e-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12E3",                 Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.E3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12E3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2E3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,E3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12E3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2E3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12E+3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.E+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12E+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2E+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,E+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12E+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2E+3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12E-3",                Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12.E-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(".12E-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1.2E-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("12,E-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair(",12E-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("1,2E-3",               Backwards::Input::NUMBER));
   tests.push_back(std::make_pair("\"Hello\"",            Backwards::Input::STRING));
   tests.push_back(std::make_pair("\"Hello There\"",      Backwards::Input::STRING));
   tests.push_back(std::make_pair("\"Hello\nThere\"",     Backwards::Input::STRING));
   tests.push_back(std::make_pair("\"Hello There",        Backwards::Input::INVALID));
   tests.push_back(std::make_pair("'Hello'",              Backwards::Input::STRING));
   tests.push_back(std::make_pair("'Hello There'",        Backwards::Input::STRING));
   tests.push_back(std::make_pair("'Hello\nThere'",       Backwards::Input::STRING));
   tests.push_back(std::make_pair("'Hello There",         Backwards::Input::INVALID));
   tests.push_back(std::make_pair("program",              Backwards::Input::IDENTIFIER));
   tests.push_back(std::make_pair("function",             Backwards::Input::FUNCTION));
   tests.push_back(std::make_pair("end",                  Backwards::Input::END));
   tests.push_back(std::make_pair("set",                  Backwards::Input::SET));
   tests.push_back(std::make_pair("to",                   Backwards::Input::TO));
   tests.push_back(std::make_pair("call",                 Backwards::Input::CALL));
   tests.push_back(std::make_pair("if",                   Backwards::Input::IF));
   tests.push_back(std::make_pair("then",                 Backwards::Input::THEN));
   tests.push_back(std::make_pair("else",                 Backwards::Input::ELSE));
   tests.push_back(std::make_pair("elseif",               Backwards::Input::ELSEIF));
   tests.push_back(std::make_pair("while",                Backwards::Input::WHILE));
   tests.push_back(std::make_pair("do",                   Backwards::Input::DO));
   tests.push_back(std::make_pair("select",               Backwards::Input::SELECT));
   tests.push_back(std::make_pair("from",                 Backwards::Input::FROM));
   tests.push_back(std::make_pair("case",                 Backwards::Input::CASE));
   tests.push_back(std::make_pair("is",                   Backwards::Input::IS));
   tests.push_back(std::make_pair("also",                 Backwards::Input::ALSO));
   tests.push_back(std::make_pair("above",                Backwards::Input::ABOVE));
   tests.push_back(std::make_pair("below",                Backwards::Input::BELOW));
   tests.push_back(std::make_pair("break",                Backwards::Input::BREAK));
   tests.push_back(std::make_pair("continue",             Backwards::Input::CONTINUE));
   tests.push_back(std::make_pair("return",               Backwards::Input::RETURN));
   tests.push_back(std::make_pair("for",                  Backwards::Input::FOR));
   tests.push_back(std::make_pair("downto",               Backwards::Input::DOWNTO));
   tests.push_back(std::make_pair("step",                 Backwards::Input::STEP));
   tests.push_back(std::make_pair("in",                   Backwards::Input::IN));
   tests.push_back(std::make_pair(",",                    Backwards::Input::PERIOD));
   tests.push_back(std::make_pair(".",                    Backwards::Input::PERIOD));
   tests.push_back(std::make_pair("=",                    Backwards::Input::EQUALITY));
   tests.push_back(std::make_pair("<>",                   Backwards::Input::INEQUALITY));
   tests.push_back(std::make_pair("?",                    Backwards::Input::CONDITIONAL));
   tests.push_back(std::make_pair(":",                    Backwards::Input::ALTERNATIVE));
   tests.push_back(std::make_pair("&",                    Backwards::Input::SHORT_AND));
   tests.push_back(std::make_pair("|",                    Backwards::Input::SHORT_OR));
   tests.push_back(std::make_pair(">",                    Backwards::Input::GREATER_THAN));
   tests.push_back(std::make_pair("<",                    Backwards::Input::LESS_THAN));
   tests.push_back(std::make_pair(">=",                   Backwards::Input::GREATER_THAN_OR_EQUAL_TO));
   tests.push_back(std::make_pair("<=",                   Backwards::Input::LESS_THAN_OR_EQUAL_TO));
   tests.push_back(std::make_pair("+",                    Backwards::Input::PLUS));
   tests.push_back(std::make_pair("-",                    Backwards::Input::MINUS));
   tests.push_back(std::make_pair("*",                    Backwards::Input::MULTIPLY));
   tests.push_back(std::make_pair("/",                    Backwards::Input::DIVIDE));
   tests.push_back(std::make_pair("!",                    Backwards::Input::NOT));
   tests.push_back(std::make_pair(";",                    Backwards::Input::SEMICOLON));
   tests.push_back(std::make_pair("(",                    Backwards::Input::OPEN_PARENS));
   tests.push_back(std::make_pair(")",                    Backwards::Input::CLOSE_PARENS));
   tests.push_back(std::make_pair("[",                    Backwards::Input::OPEN_BRACKET));
   tests.push_back(std::make_pair("]",                    Backwards::Input::CLOSE_BRACKET));
   tests.push_back(std::make_pair("{",                    Backwards::Input::OPEN_BRACE));
   tests.push_back(std::make_pair("}",                    Backwards::Input::CLOSE_BRACE));
   tests.push_back(std::make_pair("#",                    Backwards::Input::INVALID));
   tests.push_back(std::make_pair("(* Hi there *) hello", Backwards::Input::IDENTIFIER));

   for (std::vector<std::pair<std::string, Backwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input (iter->first);
      Backwards::Input::Lexer lexi(input, iter->first);
      Backwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);
    }

   for (std::vector<std::pair<std::string, Backwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input ("   " + iter->first + "   ");
      Backwards::Input::Lexer lexi(input, iter->first);
      Backwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);
    }

   for (std::vector<std::pair<std::string, Backwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input ("\t\t\t" + iter->first + "\t\t\t");
      Backwards::Input::Lexer lexi(input, iter->first);
      Backwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);
    }

   for (std::vector<std::pair<std::string, Backwards::Input::Lexeme> >::const_iterator iter = tests.begin();
      iter != tests.end(); ++iter)
    {
      Backwards::Input::StringInput input ("\n\n\n" + iter->first + "\n\n\n");
      Backwards::Input::Lexer lexi(input, iter->first);
      Backwards::Input::Token test = lexi.getNextToken();
      EXPECT_EQ(iter->second, test.lexeme);
    }

    {
      Backwards::Input::FileInput input ("../Tests/TestFileLT.txt");
      Backwards::Input::Lexer lexer (input, "TestFile");
      EXPECT_EQ(Backwards::Input::FUNCTION, lexer.getNextToken().lexeme);
      EXPECT_EQ(Backwards::Input::END_OF_FILE, lexer.getNextToken().lexeme);
    }

    {
      Backwards::Input::StringInput input ("hello");
      Backwards::Input::BufferedGenericInput bgi (input);
      EXPECT_EQ('h', bgi.consume());
    }

    {
      Backwards::Input::FileInput input ("../Tests/NoSuchFile.txt");
      Backwards::Input::Lexer lexer (input, "TestFile");
      EXPECT_EQ(Backwards::Input::END_OF_FILE, lexer.getNextToken().lexeme);
    }
 }
