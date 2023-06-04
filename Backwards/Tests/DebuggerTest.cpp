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

#include "Backwards/Input/Token.h"

#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/Scope.h"
#include "Backwards/Engine/StackFrame.h"
#include "Backwards/Engine/StdLib.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"

class ContollerLogger final : public Backwards::Engine::Logger
 {
public:
   std::vector<std::string> logs;
   std::vector<std::string> commands;
   size_t command;
   void log (const std::string& message) { logs.emplace_back(message); }
   std::string get () { if (command >= commands.size()) return ""; return commands[command++]; }
 };

TEST(ParserTests, testEnterDebugger)
 {
   Backwards::Engine::Scope global;

   Backwards::Engine::CallingContext context;
   Backwards::Engine::DefaultDebugger debugger;
   ContollerLogger logger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;

   std::shared_ptr<Backwards::Engine::FunctionContext> fun = std::make_shared<Backwards::Engine::FunctionContext>();
   fun->name = "EnterDebugger";
   fun->nargs = 0U;
   fun->nlocals = 0U;

   Backwards::Input::Token token (Backwards::Input::INVALID, "EnterDebugger", "test", 1, 1);
   Backwards::Engine::StackFrame frame(fun, token, nullptr);
   context.currentFrame = &frame;

   EXPECT_EQ(0U, logger.logs.size());

   logger.command = 0U;
   logger.commands.emplace_back("quit");

   context.debugger->EnterDebugger("", context);

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("In function #1: >EnterDebugger< from line 1 in test", logger.logs[0]);


   logger.logs.clear();
   logger.command = 0U;
   context.debugger->EnterDebugger("blah", context);
   ASSERT_EQ(2U, logger.logs.size());
   EXPECT_EQ("Entered debugger with message: blah", logger.logs[0]);
   EXPECT_EQ("In function #1: >EnterDebugger< from line 1 in test", logger.logs[1]);

   logger.logs.clear();
   logger.command = 0U;
   logger.commands.clear();
   logger.commands.emplace_back("up");
   logger.commands.emplace_back("down");
   logger.commands.emplace_back("");      // Empty lines repeat the previous command.
   logger.commands.emplace_back("blah");
   logger.commands.emplace_back("quit");
   context.debugger->EnterDebugger("", context);
   ASSERT_EQ(5U, logger.logs.size());
   EXPECT_EQ("Already in top-most frame.", logger.logs[1]);
   EXPECT_EQ("Already in bottom-most frame.", logger.logs[2]);
   EXPECT_EQ("Already in bottom-most frame.", logger.logs[3]);
   EXPECT_EQ("Did not understand >blah<.", logger.logs[4].substr(0, 26));
 }

TEST(ParserTests, testMoreDebugger)
 {
   Backwards::Engine::Scope global;
   global.vars.emplace_back(std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("5")));
   global.vars.emplace_back(std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("6")));
   global.var.emplace("g", 0U);
   global.var.emplace("G", 1U);
   global.names.emplace_back("g");
   global.names.emplace_back("G");
   Backwards::Engine::Scope local;
   local.vars.emplace_back(std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("5")));
   local.vars.emplace_back(std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("4")));
   local.var.emplace("l", 0U);
   local.var.emplace("L", 1U);
   local.names.emplace_back("l");
   local.names.emplace_back("L");

   Backwards::Engine::CallingContext context;
   Backwards::Engine::DefaultDebugger debugger;
   ContollerLogger logger;

   context.logger = &logger;
   context.debugger = &debugger;
   context.globalScope = &global;
   context.pushScope(&local);

   std::shared_ptr<Backwards::Engine::FunctionContext> fun1 = std::make_shared<Backwards::Engine::FunctionContext>();
   fun1->name = "EnterDebugger";
   fun1->nargs = 0U;
   fun1->nlocals = 0U;

   std::shared_ptr<Backwards::Engine::FunctionContext> fun2 = std::make_shared<Backwards::Engine::FunctionContext>();
   fun2->name = "BuyMeAPony";
   fun2->nargs = 3U;
   fun2->nlocals = 3U;
   fun2->ncaptures = 2U;
   fun2->args.emplace("x", 0U);
   fun2->args.emplace("y", 1U);
   fun2->args.emplace("z", 2U);
   fun2->argNames.emplace_back("x");
   fun2->argNames.emplace_back("y");
   fun2->argNames.emplace_back("z");
   fun2->locals.emplace("a", 0U);
   fun2->locals.emplace("b", 1U);
   fun2->locals.emplace("c", 2U);
   fun2->localNames.emplace_back("a");
   fun2->localNames.emplace_back("b");
   fun2->localNames.emplace_back("c");
   fun2->captures.emplace("i", 0U);
   fun2->captures.emplace("j", 1U);
   fun2->captureNames.emplace_back("i");
   fun2->captureNames.emplace_back("j");

   std::shared_ptr<Backwards::Engine::FunctionContext> fun3 = std::make_shared<Backwards::Engine::FunctionContext>();
   fun3->name = "IWantTheWorld";
   fun3->nargs = 0U;
   fun3->nlocals = 2U;
   fun3->locals.emplace("n", 0U);
   fun3->locals.emplace("m", 1U);
   fun3->localNames.emplace_back("n");
   fun3->localNames.emplace_back("m");

   Backwards::Input::Token token3 (Backwards::Input::INVALID, "IWantTheWorld", "test", 1, 1);
   Backwards::Engine::StackFrame frame3 (fun3, token3, nullptr);
   Backwards::Input::Token token2 (Backwards::Input::INVALID, "BuyMeAPony", "test", 1, 2);
   Backwards::Engine::StackFrame frame2 (fun2, token2, &frame3);
   frame3.next = &frame2; // Would have been done in pushContext
   Backwards::Input::Token token1 (Backwards::Input::INVALID, "EnterDebugger", "test", 1, 2);
   Backwards::Engine::StackFrame frame1 (fun1, token1, &frame2);
   frame2.next = &frame1; // Would have been done in pushContext

   frame2.args[0] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("5"));
   frame2.args[1] = std::make_shared<Backwards::Types::StringValue>("Hello");
   frame2.args[2] = std::make_shared<Backwards::Types::FunctionValue>(fun1, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());

   frame2.captures.resize(2);
   frame2.captures[0] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("3"));
   std::vector<std::shared_ptr<Backwards::Types::ValueType> > caps;
   caps.emplace_back(std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("-2")));
   caps.emplace_back(std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("-3")));
   frame2.captures[1] = std::make_shared<Backwards::Types::FunctionValue>(fun2, caps);

   frame2.locals[0] = Backwards::Engine::Insert(
      Backwards::Engine::Insert(Backwards::Engine::NewDictionary(), std::make_shared<Backwards::Types::StringValue>("Hello"), std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("5"))),
      std::make_shared<Backwards::Types::StringValue>("World"), std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("6")));
   frame2.locals[1] = Backwards::Engine::PushBack(
      Backwards::Engine::PushBack(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("Hello")),
      std::make_shared<Backwards::Types::StringValue>("World"));

   EXPECT_EQ(0U, logger.logs.size());

   logger.command = 0U;
   logger.commands.emplace_back("bt");
   logger.commands.emplace_back("down");
   logger.commands.emplace_back("show");
   logger.commands.emplace_back("print x");
   logger.commands.emplace_back("print y");
   logger.commands.emplace_back("print z");
   logger.commands.emplace_back("print a");
   logger.commands.emplace_back("print b");
   logger.commands.emplace_back("print c");
   logger.commands.emplace_back("print i");
   logger.commands.emplace_back("print j");
   logger.commands.emplace_back("up");
   logger.commands.emplace_back("print 2 + 3 blah");
   logger.commands.emplace_back("print 2 + 'hello'");
   logger.commands.emplace_back("quit");

   context.currentFrame = &frame1;

   context.debugger->EnterDebugger("", context);

   ASSERT_EQ(16U, logger.logs.size());
   EXPECT_EQ("In function #3: >EnterDebugger< from line 1 in test", logger.logs[0]);
   EXPECT_EQ("#3: >EnterDebugger< from line 1 in test\n#2: >BuyMeAPony< from line 1 in test\n#1: >IWantTheWorld< from line 1 in test", logger.logs[1]);
   EXPECT_EQ("In function #2: >BuyMeAPony< from line 1 in test", logger.logs[2]);
   EXPECT_EQ("These variables are in the current stack frame: x, y, z, a, b, c, i, j\nThese variables are in the current scope: l, L\nThese variables are in the global scope: g, G", logger.logs[3]);
   EXPECT_EQ("5", logger.logs[4]);
   EXPECT_EQ("\"Hello\"", logger.logs[5]);
   EXPECT_EQ("Function : EnterDebugger", logger.logs[6]);
   EXPECT_EQ("{ \"Hello\":5; \"World\":6 }", logger.logs[7]);
   EXPECT_EQ("{ \"Hello\"; \"World\" }", logger.logs[8]);
   EXPECT_EQ("Error: Read of value before set.", logger.logs[9]);
   EXPECT_EQ("3", logger.logs[10]);
   EXPECT_EQ("Function : BuyMeAPony [ -2; -3 ]", logger.logs[11]);
   EXPECT_EQ("In function #3: >EnterDebugger< from line 1 in test", logger.logs[12]);
   // Skip
   EXPECT_EQ("Didn't understand that.", logger.logs[14]);
   EXPECT_EQ("Error: Error adding Float to String\n\tFrom file Print Argument on line 1 at 4", logger.logs[15]);
 }
