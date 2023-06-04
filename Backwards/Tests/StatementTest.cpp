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

#include "Backwards/Engine/Statement.h"
#include "Backwards/Engine/StdLib.h"

#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/StackFrame.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"

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

 /*
   For the meat-and-potatoes classes: they are just too involved to not test them in combination with
   the parser. It's not as clean, but we'll test them together with the parser.
 */

TEST(EngineTests, testBasic)
 {
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Backwards::Engine::NOP nop {Backwards::Input::Token()};

   std::shared_ptr<Backwards::Engine::FlowControl> res = nop.execute(context);
   EXPECT_EQ(nullptr, res.get());

   Backwards::Engine::StandardConstantFunction pi (Backwards::Engine::GetRoundMode);
   res = pi.execute(context);
   EXPECT_NE(nullptr, res.get());


   std::shared_ptr<Backwards::Engine::FunctionContext> fun = std::make_shared<Backwards::Engine::FunctionContext>();
   fun->nargs = 1;
   fun->nlocals = 0;
   fun->function = std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Backwards::Engine::Info);

   std::shared_ptr<Backwards::Types::FunctionValue> info = std::make_shared<Backwards::Types::FunctionValue>(fun, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> infos = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), info);
   std::shared_ptr<Backwards::Types::StringValue> message = std::make_shared<Backwards::Types::StringValue>("hello");
   std::shared_ptr<Backwards::Engine::Constant> messages = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), message);
   std::vector<std::shared_ptr<Backwards::Engine::Expression> > args;
   args.push_back(messages);

   std::shared_ptr<Backwards::Engine::FunctionCall> call = std::make_shared<Backwards::Engine::FunctionCall>(Backwards::Input::Token(), infos, args);

   std::shared_ptr<Backwards::Engine::Expr> expr = std::make_shared<Backwards::Engine::Expr>(Backwards::Input::Token(), call);
   std::vector<std::shared_ptr<Backwards::Engine::Statement> > states;
   states.push_back(expr);
   Backwards::Engine::StatementSeq seq1 (Backwards::Input::Token(), states);
   EXPECT_EQ(nullptr, seq1.execute(context).get());
   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("INFO: hello", logger.logs[0]);
   logger.logs.clear();

   states.clear();
   states.push_back(std::make_shared<Backwards::Engine::FlowControlStatement>(Backwards::Input::Token(), Backwards::Engine::FlowControl::RETURN, 0U, std::shared_ptr<Backwards::Engine::Expression>()));
   Backwards::Engine::StatementSeq seq2 (Backwards::Input::Token(), states);
   EXPECT_NE(nullptr, seq2.execute(context).get());

   states.clear();
   states.push_back(std::make_shared<Backwards::Engine::FlowControlStatement>(Backwards::Input::Token(), Backwards::Engine::FlowControl::RETURN, 0U, messages));
   Backwards::Engine::StatementSeq seq3 (Backwards::Input::Token(), states);
   std::shared_ptr<Backwards::Engine::FlowControl> ret = seq3.execute(context);
   ASSERT_NE(nullptr, ret.get());
   ASSERT_NE(nullptr, (ret->value).get());
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*(ret->value).get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(ret->value)->value);

   states.clear();
   states.push_back(std::make_shared<Backwards::Engine::FlowControlStatement>(Backwards::Input::Token(), Backwards::Engine::FlowControl::RETURN, 0U, std::make_shared<Backwards::Engine::Plus>(Backwards::Input::Token(), infos, messages)));
   Backwards::Engine::StatementSeq seq4 (Backwards::Input::Token(), states);
   debugger.entered = false;
   EXPECT_THROW(seq4.execute(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);


   std::shared_ptr<Backwards::Engine::FunctionContext> func = std::make_shared<Backwards::Engine::FunctionContext>();
   func->nargs = 3;
   func->nlocals = 0;

   Backwards::Engine::StackFrame frame (func, Backwards::Input::Token(), nullptr);
   context.pushContext(&frame);

   Backwards::Engine::StandardUnaryFunctionWithContext failInfo (Backwards::Engine::Info);
   frame.args[0] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("6"));

   debugger.entered = false;
   EXPECT_THROW(failInfo.execute(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);


   Backwards::Engine::StandardUnaryFunction Sine (Backwards::Engine::Abs);

   EXPECT_NO_THROW(Sine.execute(context));
   frame.args[0] = std::make_shared<Backwards::Types::StringValue>("hello");
   debugger.entered = false;
   EXPECT_THROW(Sine.execute(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   Backwards::Engine::StandardBinaryFunction Atan2 (Backwards::Engine::Min);

   frame.args[0] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("6"));
   frame.args[1] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("9"));
   EXPECT_NO_THROW(Atan2.execute(context));
   frame.args[0] = std::make_shared<Backwards::Types::StringValue>("hello");
   debugger.entered = false;
   EXPECT_THROW(Atan2.execute(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   Backwards::Engine::StandardTernaryFunction Insert (Backwards::Engine::Insert);

   frame.args[0] = Backwards::Engine::NewDictionary();
   frame.args[1] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("6"));
   frame.args[2] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("9"));
   EXPECT_NO_THROW(Insert.execute(context));
   frame.args[0] = Backwards::Engine::NewArray();
   debugger.entered = false;
   EXPECT_THROW(Insert.execute(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);


   Backwards::Engine::StandardConstantFunctionWithContext Debug (Backwards::Engine::EnterDebugger);

   debugger.entered = false;
   EXPECT_NO_THROW(Debug.execute(context));
   EXPECT_TRUE(debugger.entered);
 }
