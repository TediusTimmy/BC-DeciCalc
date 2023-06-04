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
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/DebuggerHook.h"

class StringLogger final : public Backwards::Engine::Logger
 {
public:
   std::vector<std::string> logs;
   void log (const std::string& message) { logs.emplace_back(message); }
   std::string get () { return ""; }
 };

class DummyDebugger final : public Backwards::Engine::DebuggerHook
 {
public:
   bool entered;
   virtual void EnterDebugger(const std::string&, Backwards::Engine::CallingContext&) { entered = true; }
 };

TEST(AllTests, testICanParseExecute)
 {
   Backwards::Input::StringInput string
      (
      "if 7 < 5 then "
      "   call Info('True') "
      "else "
      "   call Info('False') "
      "end "
      "if 5 < 7 then "
      "   call Info('True') "
      "else "
      "   call Info('False') "
      "end "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(2U, logger.logs.size());
   EXPECT_EQ("INFO: False", logger.logs[0]);
   EXPECT_EQ("INFO: True", logger.logs[1]);
 }

TEST(AllTests, testGoodAssignments)
 {
   Backwards::Input::StringInput string
      (
      "set Barbara to 'Walters' \n"
      "set ThisThing to NewArrayDefault(3; { 'hello' : 'world' }) \n"
      "set ThisThing[2],hello to { 'Bye' : 'Cruel World' } \n"
      "set ThisThing[2],hello.Bye to NewArrayDefault(3; 'More') \n"
      "set ThisThing[2],hello.Bye[1] to NewArrayDefault(3; 'Less') \n"
      "set ThisThing[2],hello.Bye[1][2] to 'Poor Unfortunate Souls' \n"
      "call Warn(ThisThing[2],hello.Bye[1][2])"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());
   for (auto log : logger.logs) std::cerr << log << std::endl;

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("WARN: Poor Unfortunate Souls", logger.logs[0]);
   ASSERT_FALSE(debugger.entered);
 }

TEST(AllTests, testGoodFor)
 {
   Backwards::Input::StringInput string
      (
      "for x from 1 to 10 do "
      "   call Info('Nope') "
      "end "
      "for x from 10 downto 1 do "
      "   call Info('Nope') "
      "end "
      "for x from 2 to 10 step 2 do "
      "   call Info('Nope') "
      "end "
      "for x in {1; 2; 3} do "
      "   call Info('Nope') "
      "end "
      "for x in {1 : 1; 2 : 2; 3 : 3} do "
      "   call Info('Nope') "
      "end "
      "for x from 1 to 10 call Bob do "
      "   for y from 1 to 10 do "
      "      select x from "
      "         case 1 is "
      "            continue Bob "
      "         case 2 is "
      "            break Bob "
      "      end "
      "   end "
      "end "
      "for x in {1; 2; 3} call Bob do "
      "   for y in {1; 2; 3} do "
      "      select x from "
      "         case 1 is "
      "            continue Bob "
      "         case 2 is "
      "            break Bob "
      "      end "
      "   end "
      "end "
      "for x in {1 : 1; 2 : 2; 3 : 3} call Bob do "
      "   for y in {1 : 1; 2 : 2; 3 : 3} do "
      "      select x[0] from "
      "         case 1 is "
      "            continue Bob "
      "         case 2 is "
      "            break Bob "
      "      end "
      "   end "
      "end "
      "set y to function () is "
      "   for x from 1 to 10 do "
      "      return 1 "
      "   end "
      "end "
      "call y() "
      "set y to function () is "
      "   for x in { 1; 2; 3} do "
      "      return 1 "
      "   end "
      "end "
      "call y() "
      "set y to function () is "
      "   for x in {1 : 1; 2 : 2; 3 : 3} do "
      "      return 1 "
      "   end "
      "end "
      "call y() "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());
   for (auto log : logger.logs) std::cerr << log << std::endl;

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(31U, logger.logs.size());
 }

TEST(AllTests, testGoodSelects)
 {
   Backwards::Input::StringInput string
      (
      "select 27 from "
      "   case below 3 is "
      "      call Info('Nope') "
      "   case above 100 is "
      "      call Info('Nope') "
      "   case from 50 to 100 is "
      "      call Info('Nope') "
      "   case 25 is "
      "      call Info('Nope') "
      "   case else is "
      "      call Info('Yep') "
      "end "
      "select 27 from "
      "   case below 3 is "
      "      call Info('Nope') "
      "   case above 100 is "
      "      call Info('Nope') "
      "   case from 50 to 100 is "
      "      call Info('Nope') "
      "   case 27 is "
      "      call Info('Yeppers') "
      "   also case else is "
      "      call Info('Yeps') "
      "end "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());
   for (auto log : logger.logs) std::cerr << log << std::endl;

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(3U, logger.logs.size());
   EXPECT_EQ("INFO: Yep", logger.logs[0]);
   EXPECT_EQ("INFO: Yeppers", logger.logs[1]);
   EXPECT_EQ("INFO: Yeps", logger.logs[2]);
 }

TEST(AllTests, testGoodWhiles)
 {
   Backwards::Input::StringInput string
      (
      "set x to 12 \n"
      "while x < 15 call Bob do"
      "   call Info('Yeppers') "
      "   set x to x + 1 "
      "   while x > 10 do "
      "      select x from "
      "         case 13 is "
      "            continue Bob "
      "         case 14 is "
      "            break Bob "
      "      end "
      "   end "
      "end "
      "while 3 > 5 do "
      "end "
      "set y to function () is "
      "   while 1 do "
      "      return 1 "
      "   end "
      "end "
      "call y() "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());
   for (auto log : logger.logs) std::cerr << log << std::endl;

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(2U, logger.logs.size());
   EXPECT_EQ("INFO: Yeppers", logger.logs[1]);
 }

TEST(AllTests, testThrowExceptions_AssignGet)
 {
   Backwards::Input::StringInput string
      (
      "set x to 3"
      "set x[2][3] to 5"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testThrowExceptions_AssignSet)
 {
   Backwards::Input::StringInput string
      (
      "set x to 3"
      "set x[2] to 5"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testThrowExceptions_If)
 {
   Backwards::Input::StringInput string
      (
      "if 3 < 'hello' then end"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

 // The internal structure of the While implementation means that there are two locations where the
 // condition is evaluated.
TEST(AllTests, testThrowExceptions_While_First)
 {
   Backwards::Input::StringInput string
      (
      "while 3 < 'hello' do end"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testThrowExceptions_While_Follow)
 {
   Backwards::Input::StringInput string
      (
      "set x to 3 "
      "while x < 4 do set x to 'hello' end "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testThrowExceptions_Select)
 {
   Backwards::Input::StringInput string
      (
      "select 3 from "
      "   case 'hello' is "
      "   case 14 is "
      "end "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testThrowExceptions_ForBadUpper)
 {
   Backwards::Input::StringInput string
      (
      "for x from 3 to 'hello' do "
      "end "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testThrowExceptions_ForBadCollection)
 {
   Backwards::Input::StringInput string
      (
      "for x in 3 do "
      "end "
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());

   if (nullptr != parse.get())
    {
      try
       {
         parse->execute(context);
         FAIL() << "Didn't throw.";
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
       }
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(debugger.entered);
 }

TEST(AllTests, testICanRecursiveFib)
 {
   Backwards::Input::StringInput string
      (
      "call Info(ToString(function fib (y) is if y > 1 then return fib(y - 1) * y else return 1 end end (5)))"
      );
   Backwards::Input::Lexer lexer (string, "InputString");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   DummyDebugger debugger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::Statement> parse = Backwards::Parser::Parser::Parse(lexer, table, logger);

   debugger.entered = false;
   EXPECT_EQ(0U, logger.logs.size());
   for (const auto& log : logger.logs) std::cout << log << std::endl;

   if (nullptr != parse.get())
    {
      parse->execute(context);
    }
   else
    {
      FAIL() << "Parse returned NULL.";
    }

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("INFO: 120", logger.logs[0]);
 }
