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

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"
#include "Backwards/Parser/ContextBuilder.h"

#include "Backwards/Engine/Statement.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/StackFrame.h"
#include "Backwards/Engine/Scope.h"

#include "Backwards/Types/FloatValue.h"

#include "Backwards/Engine/ProgrammingException.h"

class StringLogger final : public Backwards::Engine::Logger
 {
public:
   std::vector<std::string> logs;
   void log (const std::string& message) { logs.emplace_back(message); }
   std::string get () { return ""; }
 };

TEST(ParserTests, testICanParseParse)
 {
   Backwards::Input::StringInput string
      (
      "if 7 < 5 then "
      "   call Info('True') "
      "else "
      "   call Info('False') "
      "end"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("INFO: False", logger.logs[0]);
 }

static double parseAndEvaluateDouble (Backwards::Input::Lexer& lexer, Backwards::Parser::SymbolTable& table, StringLogger& logger, Backwards::Engine::CallingContext& context)
 {
   std::shared_ptr<Backwards::Engine::Expression> parse;
   std::shared_ptr<Backwards::Types::ValueType> res;

   logger.logs.clear();

   parse = Backwards::Parser::Parser::ParseFullExpression(lexer, table, logger);

   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr == parse.get())
    {
      throw "Parse returned NULL.";
    }

   res = parse->evaluate(context);
   EXPECT_EQ(0U, logger.logs.size());

   for (auto log : logger.logs) std::cerr << log << std::endl;

   if (nullptr == res.get())
    {
      throw "Evaluate returned NULL.";
    }
   if (typeid(Backwards::Types::FloatValue) != typeid(*res.get()))
    {
      throw "Result wasn't a Float.";
    }

   return std::stod(std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value.toString());
 }

TEST(ParserTests, testLetsDoMath)
 {
   Backwards::Engine::Scope global;
   Backwards::Engine::Scope local;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;
   context.pushScope(&local);

   try
    {
      Backwards::Input::StringInput string ( " 9 * 6 " ); // 42, of course
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(54.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      BigInt::Fixed::setDefaultPrecision(1U);
      Backwards::Input::StringInput string ( " 9 / 6 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.5, parseAndEvaluateDouble(lexer, table, logger, context));
      BigInt::Fixed::setDefaultPrecision(0U);
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 3 + 9 * 6 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(57.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 + 8 * 5 + 1 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(42.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " (1 + 8) * (5 + 1) " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(54.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 3 - 9 * 6 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(-51.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 ? 12 : 15 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(12.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 0 ? 12 : 15 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(15.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " --2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(2.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " -1 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(-1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " ---3 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(-3.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " !0 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " !!42.0 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " !!!42.0 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(0.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 > 2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(0.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 < 2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 >= 2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(0.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 <= 2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 = 2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(0.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 <> 2 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 1 & 0 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(0.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " 0 | 1 " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(1.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " { 1 ; 2 ; 3 } [1] " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(2.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " { 'hello' : 12 ; 'world' : 13 }.hello " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(12.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " { 'hello' : 12 ; 'world' : 13 } , world " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(13.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " Size({}) " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(0.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " function dumb (x ; y ; z) is return y end (1 ; 2 ; 3) " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(2.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " Eval('2 + 3') " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(5.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " Eval('2 + 3 end ') " );
      Backwards::Input::Lexer lexer (string, "InputString");
      parseAndEvaluateDouble(lexer, table, logger, context);
      FAIL() << "Didn't throw.";
    }
   catch (const Backwards::Types::TypedOperationException&)
    {
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " Eval(2) " );
      Backwards::Input::Lexer lexer (string, "InputString");
      parseAndEvaluateDouble(lexer, table, logger, context);
      FAIL() << "Didn't throw.";
    }
   catch (const Backwards::Types::TypedOperationException&)
    {
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " function [2; 3] dumb () [x; y] is return x end () " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(2.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " function [2; 3] dumb () [x; y] is return y end () " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(3.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }

   try
    {
      Backwards::Input::StringInput string ( " function [3] fib (x) [z] is if x > 1 then return fib[z](x - 1) * x else return 1 end end (5) " );
      Backwards::Input::Lexer lexer (string, "InputString");
      EXPECT_EQ(120.0, parseAndEvaluateDouble(lexer, table, logger, context));
    }
   catch (const char * failure)
    {
      FAIL() << failure;
    }
 }

TEST(ParserTests, testThrowSomeExceptionsExpressions)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Expression> parse;
   std::shared_ptr<Backwards::Types::ValueType> res;

   Backwards::Input::StringInput string ( " 2 * 3 end " );
   Backwards::Input::Lexer lexer (string, "InputString");

   parse = Backwards::Parser::Parser::ParseExpression(lexer, table, logger);
   ASSERT_NE(nullptr, parse.get());
   res = parse->evaluate(context);
   EXPECT_EQ(0U, logger.logs.size());
   ASSERT_NE(nullptr, res.get());
   ASSERT_EQ(typeid(Backwards::Types::FloatValue), typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("6"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);


   Backwards::Input::StringInput string2 ( " 2 * end " );
   Backwards::Input::Lexer lexer2 (string2, "InputString");

   parse = Backwards::Parser::Parser::ParseExpression(lexer2, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   parse = Backwards::Parser::Parser::ParseFullExpression(lexer2, table, logger);
   ASSERT_EQ(nullptr, parse.get());


   Backwards::Input::StringInput string3 ( " 2 * bob " );
   Backwards::Input::Lexer lexer3 (string3, "InputString");

   parse = Backwards::Parser::Parser::ParseExpression(lexer3, table, logger);
   ASSERT_EQ(nullptr, parse.get());


   Backwards::Input::StringInput string4 ( " function (zeeb] is return 2 end () " );
   Backwards::Input::Lexer lexer4 (string4, "InputString");

   parse = Backwards::Parser::Parser::ParseExpression(lexer4, table, logger);
   ASSERT_EQ(nullptr, parse.get());


   Backwards::Input::StringInput string5 ( " function (bob ; bob) is return bob end () " );
   Backwards::Input::Lexer lexer5 (string5, "InputString");

   parse = Backwards::Parser::Parser::ParseExpression(lexer5, table, logger);
   ASSERT_EQ(nullptr, parse.get());
 }

TEST(ParserTests, testParseFunctions)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   Backwards::Engine::Scope bob;
   bob.var.emplace(std::make_pair("NotAVariable", 0U));
   EXPECT_EQ(0U, gs.scopeGetters.size());
   table.pushScope(&bob);
   EXPECT_EQ(1U, gs.scopeGetters.size());
   context.pushScope(&bob);

   std::shared_ptr<Backwards::Engine::Statement> parse;

   Backwards::Input::StringInput string1
      (
      "set rage to 3 "
      "set Info to Warn "
      );
   Backwards::Input::Lexer lexer1 (string1, "InputString");

   parse = Backwards::Parser::Parser::ParseFunctions(lexer1, table, logger);
   ASSERT_NE(nullptr, parse.get());

   for (auto log : logger.logs) std::cerr << log << std::endl;

   Backwards::Input::StringInput string2
      (
      "set gnosis to 10 "
      "while 1 do end "
      );
   Backwards::Input::Lexer lexer2 (string2, "InputString");

   parse = Backwards::Parser::Parser::ParseFunctions(lexer2, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string3
      (
      "set willpower to end "
      );
   Backwards::Input::Lexer lexer3 (string3, "InputString");

   parse = Backwards::Parser::Parser::ParseFunctions(lexer3, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string4
      (
      "set willpower to function () is if 3 * bob then end end "
      );
   Backwards::Input::Lexer lexer4 (string4, "InputString");

   parse = Backwards::Parser::Parser::ParseFunctions(lexer4, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   context.popScope();
   table.popScope();
 }

TEST(ParserTests, testVariousSymbolTable)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse;

   Backwards::Input::StringInput string1
      (
      "set rage to 3 "
      "set Info to Warn "
      "set bob to function (arg) is set arg to 3 set larry to 4 return bob * larry end "
      );
   Backwards::Input::Lexer lexer1 (string1, "InputString");

   parse = Backwards::Parser::Parser::Parse(lexer1, table, logger);
   ASSERT_NE(nullptr, parse.get());

   for (auto log : logger.logs) std::cerr << log << std::endl;


   EXPECT_EQ(1U, gs.localsGetters.size());
   EXPECT_EQ(1U, gs.argsGetters.size());

   std::shared_ptr<Backwards::Engine::FunctionContext> fun = std::make_shared<Backwards::Engine::FunctionContext>();
   fun->args.emplace(std::make_pair("Heathens", 1U));
   fun->args.emplace(std::make_pair("Slow", 2U));
   fun->locals.emplace(std::make_pair("Ask", 1U));
   fun->locals.emplace(std::make_pair("Who", 2U));
   fun->locals.emplace(std::make_pair("Know", 3U));
   fun->captures.emplace(std::make_pair("Cow", 1U));

   table.injectContext(fun);

   EXPECT_EQ(3U, gs.localsGetters.size());
   EXPECT_EQ(2U, gs.argsGetters.size());
   EXPECT_EQ(1U, gs.capturesGetters.size());
 }

TEST(ParserTests, testStatementExceptions)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse;

   Backwards::Input::StringInput string1 ("call Info('Hi')");
   Backwards::Input::Lexer lexer1 (string1, "InputString");
   parse = Backwards::Parser::Parser::ParseStatement(lexer1, table, logger);
   ASSERT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string2 ("call Info(larry)");
   Backwards::Input::Lexer lexer2 (string2, "InputString");
   parse = Backwards::Parser::Parser::ParseStatement(lexer2, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   parse = Backwards::Parser::Parser::Parse(lexer2, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string3 ("set bob[12] to 13");
   Backwards::Input::Lexer lexer3 (string3, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer3, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string4 ("set larry.bob to 13");
   Backwards::Input::Lexer lexer4 (string4, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer4, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string5 ("if 3 > mouse then else end");
   Backwards::Input::Lexer lexer5 (string5, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer5, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string6 ("if 3 then set mouse to mickey elseif 5 then end");
   Backwards::Input::Lexer lexer6 (string6, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer6, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string7 ("while 5 call bob do while 6 call bob do set mickey to ultra end end");
   Backwards::Input::Lexer lexer7 (string7, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer7, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string8 ("break itDown");
   Backwards::Input::Lexer lexer8 (string8, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer8, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string9 ("while 9 do break itDown end");
   Backwards::Input::Lexer lexer9 (string9, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer9, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string10
   (
      "select 9 from "
      "   case 12 is "
      "   also case 13 is "
      "   case hello is "
      "   case from 14 to 15 is "
      "   case from 14 to monster is "
      "   case above 90 is "
      "   case below 04 is "
      "   case else is "
      "end"
   );
   Backwards::Input::Lexer lexer10 (string10, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer10, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string11 ("while 9 break itDown end");
   Backwards::Input::Lexer lexer11 (string11, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer11, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string12 ("select 10 from case else is set money to dollar end");
   Backwards::Input::Lexer lexer12 (string12, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer12, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string13 ("for x from 2 to dollar do end");
   Backwards::Input::Lexer lexer13 (string13, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer13, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string14 ("for x from 2 to 10 step dollar do end");
   Backwards::Input::Lexer lexer14 (string14, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer14, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string15 ("while 5 call bob do for x in {} call bob set mickey to ultra end end");
   Backwards::Input::Lexer lexer15 (string15, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer15, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string16 ("set x to function bob () is set bob to 3 return bob end");
   Backwards::Input::Lexer lexer16 (string16, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer16, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string17 ("set x to function [] bob () is return 0 end");
   Backwards::Input::Lexer lexer17 (string17, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer17, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string18 ("set x to function bob () [] is return 0 end");
   Backwards::Input::Lexer lexer18 (string18, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer18, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string19 ("set x to function [2] bob () is return 0 end");
   Backwards::Input::Lexer lexer19 (string19, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer19, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string20 ("set x to function [2] bob () [x; y] is return 0 end");
   Backwards::Input::Lexer lexer20 (string20, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer20, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string21 ("set x to function [2; 3] bob () [x] is return 0 end");
   Backwards::Input::Lexer lexer21 (string21, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer21, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string22 ("set x to function [3] fib () [x] is return fib[2; 3](5) end");
   Backwards::Input::Lexer lexer22 (string22, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer22, table, logger);
   ASSERT_EQ(nullptr, parse.get());

   Backwards::Input::StringInput string23 ("set x to function [3] fib () [x] is return fib(5) end");
   Backwards::Input::Lexer lexer23 (string23, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer23, table, logger);
   ASSERT_EQ(nullptr, parse.get());
 }

TEST(ParserTests, testStatementParses)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;

   context.logger = &logger;
   context.debugger = nullptr;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse;

   Backwards::Input::StringInput string1 ("set Info[12].hello,bob['hi'] to 19");
   Backwards::Input::Lexer lexer1 (string1, "InputString");
   parse = Backwards::Parser::Parser::ParseStatement(lexer1, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string2 ("while 12 do end");
   Backwards::Input::Lexer lexer2 (string2, "InputString");
   parse = Backwards::Parser::Parser::ParseStatement(lexer2, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string3 ("while 12 call larry do break break larry continue continue larry end");
   Backwards::Input::Lexer lexer3 (string3, "InputString");
   parse = Backwards::Parser::Parser::ParseStatement(lexer3, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string4 ("");
   Backwards::Input::Lexer lexer4 (string4, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer4, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string5
   (
      "select 9 from "
      "   case 12 is "
      "   also case 13 is "
      "   case else is "
      "end"
   );
   Backwards::Input::Lexer lexer5 (string5, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer5, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string6 ("for x in {} call bob do end");
   Backwards::Input::Lexer lexer6 (string6, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer6, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string7 ("for x from 3 to 9 step 2 do end");
   Backwards::Input::Lexer lexer7 (string7, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer7, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string8 ("for x from 10 downto 1 do end");
   Backwards::Input::Lexer lexer8 (string8, "InputString");
   parse = Backwards::Parser::Parser::Parse(lexer8, table, logger);
   EXPECT_NE(nullptr, parse.get());

   Backwards::Input::StringInput string9 ("set Info to function [Info] (a) [b] is set b to 0 return a end");
   Backwards::Input::Lexer lexer9 (string9, "InputString");
   parse = Backwards::Parser::Parser::ParseStatement(lexer9, table, logger);
   EXPECT_NE(nullptr, parse.get());
 }

TEST(ParserTests, testIDontKnowHowToProgram)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);

   EXPECT_THROW(table.getVariableGetter("lucy"), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(table.getVariableSetter("lucy"), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(table.buildPushBack(Backwards::Input::Token(), std::shared_ptr<Backwards::Engine::Expression>(),
      std::shared_ptr<Backwards::Engine::Expression>()), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(table.buildInsert(Backwards::Input::Token(), std::shared_ptr<Backwards::Engine::Expression>(),
      std::shared_ptr<Backwards::Engine::Expression>(), std::shared_ptr<Backwards::Engine::Expression>()), Backwards::Engine::ProgrammingException);

   try
    {
      table.getVariableGetter("lucy");
      FAIL() << "Didn't throw.";
    }
   catch (const Backwards::Engine::ProgrammingException& e)
    {
      EXPECT_STREQ("If you see this, then the programmer is wrong: Request for non existent variable lucy.", e.what());
    }
 }
