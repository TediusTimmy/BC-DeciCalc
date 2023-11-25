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

#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/StackFrame.h"
#include "Backwards/Engine/ProgrammingException.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/CellRefValue.h"

#include "Forwards/Engine/StdLib.h"
#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/CellRefEval.h"

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

TEST(EngineTests, testStdLibHolder)
 {
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   Forwards::Engine::CallingContext text;
   text.logger = &logger;
   text.debugger = &debugger;
   Forwards::Engine::NameMap names;
   text.names = &names;

   std::shared_ptr<Backwards::Engine::FunctionContext> func = std::make_shared<Backwards::Engine::FunctionContext>();
   func->nargs = 3;
   func->nlocals = 0;

   Backwards::Engine::StackFrame frame (func, Backwards::Input::Token(), nullptr);
   context.pushContext(&frame);
   text.pushContext(&frame);

   Forwards::Engine::StandardBinaryFunctionWithContext Atan2 (Forwards::Engine::Let);
   EXPECT_THROW(Atan2.execute(context), Backwards::Engine::ProgrammingException);

   frame.args[0] = std::make_shared<Backwards::Types::StringValue>("hello");
   std::shared_ptr<Forwards::Engine::CellRefEval> bob = std::make_shared<Forwards::Engine::CellRefEval>(std::shared_ptr<Forwards::Engine::Expression>());
   frame.args[1] = std::make_shared<Backwards::Types::CellRefValue>(bob);
   EXPECT_NO_THROW(Atan2.execute(text));
   frame.args[0] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("6"));
   debugger.entered = false;
   EXPECT_THROW(Atan2.execute(text), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);
   frame.args[0] = std::make_shared<Backwards::Types::StringValue>("hello");
   frame.args[1] = std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed("9"));
   debugger.entered = false;
   EXPECT_THROW(Atan2.execute(text), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   EXPECT_EQ(1U, names.size());
   EXPECT_TRUE(names.end() != names.find("hello"));
 }
