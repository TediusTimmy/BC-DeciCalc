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

#include <iostream>

#include "Backwards/Engine/Logger.h"
#include "Backwards/Input/StringInput.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Expression.h"
#include "Forwards/Input/Lexer.h"
#include "Forwards/Parser/Parser.h"

class StringLogger final : public Backwards::Engine::Logger
 {
public:
   std::vector<std::string> logs;
   void log (const std::string& message) { logs.emplace_back(message); }
   std::string get () { return ""; }
 };

TEST(ParserTests, testICanParseParse)
 {
   std::string line = "12+13";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeMoreParse)
 {
   std::string line = "12*7+13/15-(12--5)";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeReferences)
 {
   std::string line = "A1+B2+$A1+A$1+$A$1";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
      EXPECT_EQ("B2+C3+$A2+B$1+$A$1", parse->toString(2U, 2U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeMoreReferences)
 {
   std::string line = "AA1+BBB2+$AA1+AA$1+$AAA$1";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
      EXPECT_EQ("AB2+BBC3+$AA2+AB$1+$AAA$1", parse->toString(2U, 2U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeFailure)
 {
   std::string line = "A12%3";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testRange)
 {
   std::string line = "A1:B3";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
      EXPECT_EQ("B2:C4", parse->toString(2U, 2U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testCompares)
 {
   std::string line = "(A1=B3)+(A2>B4)-(A1<>B1)+(C3<C4)-(A2>=B2)-(A2<=B2)";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
    }
   else
    {
      for (std::string msg : logger.logs)
       {
         std::cout << msg << std::endl;
       }
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testCat)
 {
   std::string line = "A1&B3";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeMoreFailure)
 {
   std::string line = "A12+";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeFunctions)
 {
   std::string line = "@FUN+@FUN(12;13;14)";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;
   map.insert(std::make_pair("FUN", std::shared_ptr<Backwards::Engine::Getter>()));

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ(line, parse->toString(1U, 1U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeFunctions2)
 {
   std::string line = "@FUN()";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;
   map.insert(std::make_pair("FUN", std::shared_ptr<Backwards::Engine::Getter>()));

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      EXPECT_EQ("@FUN", parse->toString(1U, 1U));
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }
 }

TEST(ParserTests, testSomeFunctionsBad)
 {
   std::string line = "@FUNNY()";
   Backwards::Input::StringInput string (line);
   Forwards::Input::Lexer lexer (string);

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Forwards::Engine::GetterMap map;
   map.insert(std::make_pair("FUN", std::shared_ptr<Backwards::Engine::Getter>()));

   std::shared_ptr<Forwards::Engine::Expression> parse = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 1U, 1U);

   if (nullptr != parse.get())
    {
      FAIL() << "Parse returned NULL.";
    }
 }
