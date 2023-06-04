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
#include "Backwards/Types/FloatValue.h"
#include "Backwards/Engine/CallingContext.h"

#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/Statement.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/StdLib.h"
#include "Backwards/Types/FunctionValue.h"
#include "Backwards/Types/StringValue.h"

#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/ArrayValue.h"

#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FatalException.h"

#include "Backwards/Engine/StackFrame.h"

class StringLogger final : public Backwards::Engine::Logger
 {
public:
   std::vector<std::string> logs;
   void log (const std::string& message) { logs.emplace_back(message); }
   std::string get () { return ""; }
 };

static std::shared_ptr<Backwards::Types::FloatValue> makeFloatValue (const char * str)
 {
   return std::make_shared<Backwards::Types::FloatValue>(BigInt::Fixed(str));
 }

TEST(EngineTests, testBase)
 {
   std::shared_ptr<Backwards::Engine::Constant> one = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Backwards::Engine::Constant> two = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("9"));
   std::shared_ptr<Backwards::Engine::Constant> six = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("0"));
   Backwards::Engine::Plus plus (Backwards::Input::Token(), one, two);
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   std::shared_ptr<Backwards::Types::ValueType> res = plus.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("15"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Minus minus (Backwards::Input::Token(), one, two);
   res = minus.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("-3"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Multiply multiply (Backwards::Input::Token(), one, two);
   res = multiply.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("54"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   BigInt::Fixed::setDefaultPrecision(1U);
   Backwards::Engine::Divide divide (Backwards::Input::Token(), two, one); // Flipped args
   res = divide.evaluate(context);
   BigInt::Fixed::setDefaultPrecision(0U);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1.5"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::ShortAnd shortAndTT (Backwards::Input::Token(), one, two);
   res = shortAndTT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::ShortAnd shortAndTF (Backwards::Input::Token(), one, six);
   res = shortAndTF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::ShortAnd shortAndF (Backwards::Input::Token(), six, one);
   res = shortAndF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::ShortOr shortOrT (Backwards::Input::Token(), one, two);
   res = shortOrT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::ShortOr shortOrFT (Backwards::Input::Token(), six, two);
   res = shortOrFT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::ShortOr shortOrFF (Backwards::Input::Token(), six, six);
   res = shortOrFF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Equals equalsT (Backwards::Input::Token(), one, one);
   res = equalsT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Equals equalsF (Backwards::Input::Token(), one, two);
   res = equalsF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::NotEqual notEqualT (Backwards::Input::Token(), one, two);
   res = notEqualT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::NotEqual notEqualF (Backwards::Input::Token(), one, one);
   res = notEqualF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Greater greaterT (Backwards::Input::Token(), two, one);
   res = greaterT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Greater greaterF (Backwards::Input::Token(), one, two);
   res = greaterF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Less lessT (Backwards::Input::Token(), one, two);
   res = lessT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Less lessF (Backwards::Input::Token(), two, one);
   res = lessF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::GEQ geqT (Backwards::Input::Token(), two, one);
   res = geqT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::GEQ geqF (Backwards::Input::Token(), one, two);
   res = geqF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::LEQ leqT (Backwards::Input::Token(), one, two);
   res = leqT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::LEQ leqF (Backwards::Input::Token(), two, one);
   res = leqF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Not notF (Backwards::Input::Token(), one);
   res = notF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Not notT (Backwards::Input::Token(), six);
   res = notT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::Negate neg (Backwards::Input::Token(), one);
   res = neg.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("-6"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::TernaryOperation toT (Backwards::Input::Token(), one, one, two);
   res = toT.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("6"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   Backwards::Engine::TernaryOperation toF (Backwards::Input::Token(), six, one, two);
   res = toF.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("9"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);


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

   Backwards::Engine::FunctionCall call (Backwards::Input::Token(), infos, args);

   res = call.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("INFO: hello", logger.logs[0]);


   std::shared_ptr<Backwards::Types::DictionaryValue> dict = std::make_shared<Backwards::Types::DictionaryValue>();
   dict->value.insert(std::make_pair(message, makeFloatValue("6")));
   std::shared_ptr<Backwards::Engine::Constant> dicts = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), dict);

   Backwards::Engine::DerefVar derefDict (Backwards::Input::Token(), dicts, messages);
   res = derefDict.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("6"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   std::shared_ptr<Backwards::Types::ArrayValue> arr = std::make_shared<Backwards::Types::ArrayValue>();
   arr->value.push_back(makeFloatValue("6"));
   arr->value.push_back(makeFloatValue("9"));
   std::shared_ptr<Backwards::Engine::Constant> arrs = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), arr);

   Backwards::Engine::DerefVar derefArr (Backwards::Input::Token(), arrs, six);
   res = derefArr.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("6"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
 }

class DummyDebugger final : public Backwards::Engine::DebuggerHook
 {
public:
   bool entered;
   virtual void EnterDebugger(const std::string&, Backwards::Engine::CallingContext&) { entered = true; }
 };

TEST(EngineTests, testExceptionBonanza)
 {
   std::shared_ptr<Backwards::Engine::Constant> one = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Backwards::Engine::Constant> two = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("0"));

   std::shared_ptr<Backwards::Engine::FunctionContext> fun = std::make_shared<Backwards::Engine::FunctionContext>();
   fun->nargs = 1;
   fun->nlocals = 0;
   fun->function = std::make_shared<Backwards::Engine::StandardUnaryFunctionWithContext>(Backwards::Engine::Info);

   std::shared_ptr<Backwards::Types::FunctionValue> infos = std::make_shared<Backwards::Types::FunctionValue>(fun, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> info = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), infos);

   Backwards::Engine::CallingContext context;
   DummyDebugger debugger;
   context.debugger = &debugger;

   debugger.entered = false;
   Backwards::Engine::Plus plus (Backwards::Input::Token(), one, info);
   EXPECT_THROW(plus.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Minus minus (Backwards::Input::Token(), one, info);
   EXPECT_THROW(minus.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Multiply multiply (Backwards::Input::Token(), one, info);
   EXPECT_THROW(multiply.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Divide divide (Backwards::Input::Token(), one, info);
   EXPECT_THROW(divide.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::ShortAnd sandTE (Backwards::Input::Token(), one, info);
   EXPECT_THROW(sandTE.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::ShortAnd sandE (Backwards::Input::Token(), info, one);
   EXPECT_THROW(sandE.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::ShortOr sorFE (Backwards::Input::Token(), two, info);
   EXPECT_THROW(sorFE.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::ShortOr sorE (Backwards::Input::Token(), info, one);
   EXPECT_THROW(sorE.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Equals eq (Backwards::Input::Token(), one, info);
   EXPECT_THROW(eq.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::NotEqual neq (Backwards::Input::Token(), one, info);
   EXPECT_THROW(neq.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Greater gre (Backwards::Input::Token(), one, info);
   EXPECT_THROW(gre.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Less les (Backwards::Input::Token(), one, info);
   EXPECT_THROW(les.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::GEQ geq (Backwards::Input::Token(), one, info);
   EXPECT_THROW(geq.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::LEQ leq (Backwards::Input::Token(), one, info);
   EXPECT_THROW(leq.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Not lnot (Backwards::Input::Token(), info);
   EXPECT_THROW(lnot.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::Negate neg (Backwards::Input::Token(), info);
   EXPECT_THROW(neg.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::TernaryOperation top (Backwards::Input::Token(), info, info, one);
   EXPECT_THROW(top.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::DerefVar dref (Backwards::Input::Token(), info, one);
   EXPECT_THROW(dref.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);


   std::vector<std::shared_ptr<Backwards::Engine::Expression> > args;

   debugger.entered = false;
   Backwards::Engine::FunctionCall callNF (Backwards::Input::Token(), one, args);
   EXPECT_THROW(callNF.evaluate(context), Backwards::Engine::FatalException);
   EXPECT_TRUE(debugger.entered);

   debugger.entered = false;
   Backwards::Engine::FunctionCall callBA (Backwards::Input::Token(), info, args);
   EXPECT_THROW(callBA.evaluate(context), Backwards::Engine::FatalException);
   EXPECT_TRUE(debugger.entered);

   std::shared_ptr<Backwards::Engine::FunctionContext> funThrow = std::make_shared<Backwards::Engine::FunctionContext>();
   funThrow->nargs = 0;
   funThrow->nlocals = 0;
   funThrow->function = std::make_shared<Backwards::Engine::Expr>(Backwards::Input::Token(), std::make_shared<Backwards::Engine::Plus>(Backwards::Input::Token(), one, info));

   std::shared_ptr<Backwards::Types::FunctionValue> funThrows = std::make_shared<Backwards::Types::FunctionValue>(funThrow, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> funThrowsC = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), funThrows);

   debugger.entered = false;
   Backwards::Engine::FunctionCall callTH (Backwards::Input::Token(), funThrowsC, args);
   EXPECT_THROW(callTH.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_TRUE(debugger.entered);

   std::shared_ptr<Backwards::Engine::FunctionContext> funNoRet = std::make_shared<Backwards::Engine::FunctionContext>();
   funNoRet->nargs = 0;
   funNoRet->nlocals = 0;
   funNoRet->function = std::make_shared<Backwards::Engine::Expr>(Backwards::Input::Token(), std::make_shared<Backwards::Engine::Plus>(Backwards::Input::Token(), one, one));

   std::shared_ptr<Backwards::Types::FunctionValue> funNR = std::make_shared<Backwards::Types::FunctionValue>(funNoRet, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> funNRC = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), funNR);

   debugger.entered = false;
   Backwards::Engine::FunctionCall callNR (Backwards::Input::Token(), funNRC, args);
   EXPECT_THROW(callNR.evaluate(context), Backwards::Engine::FatalException);
   EXPECT_FALSE(debugger.entered);

   std::shared_ptr<Backwards::Engine::FunctionContext> funBreaks = std::make_shared<Backwards::Engine::FunctionContext>();
   funBreaks->nargs = 0;
   funBreaks->nlocals = 0;
   funBreaks->function = std::make_shared<Backwards::Engine::FlowControlStatement>(Backwards::Input::Token(), Backwards::Engine::FlowControl::BREAK, 0U, std::shared_ptr<Backwards::Engine::Expression>());

   std::shared_ptr<Backwards::Types::FunctionValue> funBreak = std::make_shared<Backwards::Types::FunctionValue>(funBreaks, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> funBeak = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), funBreak);

   debugger.entered = false;
   Backwards::Engine::FunctionCall callBR (Backwards::Input::Token(), funBeak, args);
   EXPECT_THROW(callBR.evaluate(context), Backwards::Engine::FatalException);
   EXPECT_TRUE(debugger.entered);

   try
    {
      (void) callNR.evaluate(context);
      FAIL() << "Didn't throw exception.";
    }
   catch(Backwards::Engine::FatalException& e)
    {
      EXPECT_STREQ("Function failed to return a value at 0 on line 0 in file ", e.what());
    }
 }

TEST(EngineTests, testCaCall)
 {
   Backwards::Engine::CallingContext context;
   std::vector<std::shared_ptr<Backwards::Engine::Expression> > args;
   std::shared_ptr<Backwards::Engine::Constant> one = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("9"));

   std::shared_ptr<Backwards::Engine::FunctionContext> funRetNine = std::make_shared<Backwards::Engine::FunctionContext>();
   funRetNine->nargs = 0;
   funRetNine->nlocals = 0;
   funRetNine->function = std::make_shared<Backwards::Engine::FlowControlStatement>(Backwards::Input::Token(), Backwards::Engine::FlowControl::RETURN, 0U, one);

   std::shared_ptr<Backwards::Types::FunctionValue> funRetNineF = std::make_shared<Backwards::Types::FunctionValue>(funRetNine, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> funR9 = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), funRetNineF);

   std::shared_ptr<Backwards::Engine::FunctionContext> funRetRet = std::make_shared<Backwards::Engine::FunctionContext>();
   funRetRet->nargs = 0;
   funRetRet->nlocals = 0;
   funRetRet->function = std::make_shared<Backwards::Engine::FlowControlStatement>(Backwards::Input::Token(), Backwards::Engine::FlowControl::RETURN, 0U,
      std::make_shared<Backwards::Engine::FunctionCall>(Backwards::Input::Token(), funR9, args));

   std::shared_ptr<Backwards::Types::FunctionValue> funRetRetF = std::make_shared<Backwards::Types::FunctionValue>(funRetRet, std::vector<std::shared_ptr<Backwards::Types::ValueType> >());
   std::shared_ptr<Backwards::Engine::Constant> funRR = std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), funRetRetF);

   Backwards::Engine::FunctionCall call (Backwards::Input::Token(), funRR, args);

   std::shared_ptr<Backwards::Types::ValueType> res = call.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("9"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
 }

TEST(EngineTests, testGetterSetter)
 {
   Backwards::Engine::CallingContext context;
   Backwards::Engine::Scope scope;
   context.globalScope = &scope;

   scope.vars.push_back(makeFloatValue("6"));
   scope.vars.push_back(std::shared_ptr<Backwards::Types::ValueType>());

   std::shared_ptr<Backwards::Engine::GlobalGetter> getter1 = std::make_shared<Backwards::Engine::GlobalGetter>(0U);
   std::shared_ptr<Backwards::Engine::GlobalGetter> getter2 = std::make_shared<Backwards::Engine::GlobalGetter>(1U);
   std::shared_ptr<Backwards::Engine::GlobalSetter> setter2 = std::make_shared<Backwards::Engine::GlobalSetter>(1U);

   Backwards::Engine::Variable getGud (Backwards::Input::Token(), getter1);
   std::shared_ptr<Backwards::Types::ValueType> res = getGud.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("6"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(getter2->get(context), Backwards::Engine::FatalException);
   setter2->set(context, std::make_shared<Backwards::Types::StringValue>("A"));
   res = getter2->get(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("A", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);


   EXPECT_EQ(nullptr, context.topScope());
   context.pushScope(&scope);
   EXPECT_NE(nullptr, context.topScope());
   EXPECT_EQ(&scope, context.topScope());
   context.popScope();
   EXPECT_EQ(nullptr, context.topScope());


   std::shared_ptr<Backwards::Engine::FunctionContext> fun = std::make_shared<Backwards::Engine::FunctionContext>();
   fun->nargs = 1;
   fun->nlocals = 1;
   fun->ncaptures = 1;

   Backwards::Engine::StackFrame frame (fun, Backwards::Input::Token(), nullptr);
   context.pushContext(&frame);
   frame.captures.resize(1U);

   Backwards::Engine::LocalGetter getterL1 (0U);
   Backwards::Engine::LocalSetter setterL1 (0U);

   EXPECT_THROW(getterL1.get(context), Backwards::Engine::FatalException);
   setterL1.set(context, std::make_shared<Backwards::Types::StringValue>("A"));
   res = getterL1.get(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("A", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   Backwards::Engine::ArgGetter getterA1 (0U);
   Backwards::Engine::ArgSetter setterA1 (0U);

   // Linguistically, it ought to be impossible for an argument to be uninitialized.
   setterA1.set(context, std::make_shared<Backwards::Types::StringValue>("B"));
   res = getterA1.get(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("B", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   Backwards::Engine::CaptureGetter getterC1 (0U);
   Backwards::Engine::CaptureSetter setterC1 (0U);

   // Linguistically, it ought to be impossible for a capture to be uninitialized.
   setterC1.set(context, std::make_shared<Backwards::Types::StringValue>("C"));
   res = getterC1.get(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("C", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);


   Backwards::Engine::ScopeGetter getterS1 (0U);
   Backwards::Engine::ScopeSetter setterS1 (0U);
   Backwards::Engine::ScopeGetter getterS2 (3U);
   Backwards::Engine::ScopeSetter setterS2 (3U);

   EXPECT_THROW(getterS1.get(context), Backwards::Engine::FatalException);
   EXPECT_THROW(setterS1.set(context, std::make_shared<Backwards::Types::StringValue>("A")), Backwards::Engine::FatalException);

   context.pushScope(&scope);
   scope.vars[0] = std::shared_ptr<Backwards::Types::ValueType>();

   EXPECT_THROW(getterS1.get(context), Backwards::Engine::FatalException);
   setterS1.set(context, std::make_shared<Backwards::Types::StringValue>("Z"));
   res = getterS1.get(context);
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("Z", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   EXPECT_THROW(getterS2.get(context), Backwards::Engine::FatalException);
   EXPECT_THROW(setterS2.set(context, std::make_shared<Backwards::Types::StringValue>("A")), Backwards::Engine::FatalException);
 }

TEST(EngineTests, testBuildFunction)
 {
   Backwards::Engine::CallingContext context;

   std::shared_ptr<Backwards::Engine::FunctionContext> funDummy = std::make_shared<Backwards::Engine::FunctionContext>();

   Backwards::Engine::BuildFunction dummy1 (Backwards::Input::Token(), funDummy, std::vector<std::shared_ptr<Backwards::Engine::Expression> >());

   std::shared_ptr<Backwards::Types::ValueType> res = dummy1.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FunctionValue) == typeid(*res.get()));
   EXPECT_EQ(funDummy.get(), std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->value.get());
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->captures.size());

   std::vector<std::shared_ptr<Backwards::Engine::Expression> > captures;
   captures.emplace_back(std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), makeFloatValue("9")));

   Backwards::Engine::BuildFunction dummy2 (Backwards::Input::Token(), funDummy, captures);

   res = dummy2.evaluate(context);

   ASSERT_TRUE(typeid(Backwards::Types::FunctionValue) == typeid(*res.get()));
   EXPECT_EQ(funDummy.get(), std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->value.get());
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->captures.size());
   ASSERT_NE(nullptr, std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->captures[0].get());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->captures[0].get()));
   EXPECT_EQ(BigInt::Fixed("9"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(res)->captures[0])->value);
 }
