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

#include "Backwards/Engine/StdLib.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/DebuggerHook.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h" // One function uses it!
#include "Backwards/Types/NilValue.h"
#include "Backwards/Types/CellRefValue.h"
#include "Backwards/Types/CellRangeValue.h"

#include "Backwards/Engine/CellRefEval.h"
#include "Backwards/Engine/CellRangeExpand.h"
#include "Backwards/Engine/ProgrammingException.h"

#include <cmath>

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

TEST(EngineTests, testFunctionsWithContext)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   res = Backwards::Engine::Info(context, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("INFO: hello", logger.logs[0]);
   logger.logs.clear();

   res = Backwards::Engine::Warn(context, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("WARN: hello", logger.logs[0]);
   logger.logs.clear();

   res = Backwards::Engine::Error(context, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("ERROR: hello", logger.logs[0]);
   logger.logs.clear();

   res = Backwards::Engine::DebugPrint(context, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("hello", logger.logs[0]);
   logger.logs.clear();

   try
    {
      (void) Backwards::Engine::Fatal(context, std::make_shared<Backwards::Types::StringValue>("hello"));
      FAIL() << "Didn't throw exception.";
    }
   catch(Backwards::Engine::FatalException& e)
    {
      EXPECT_STREQ("hello", e.what());
    }
   ASSERT_EQ(1U, logger.logs.size());
   EXPECT_EQ("FATAL: hello", logger.logs[0]);
   logger.logs.clear();

   EXPECT_THROW(Backwards::Engine::Info(context, makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::Warn(context, makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::Error(context, makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::DebugPrint(context, makeFloatValue("1")), Backwards::Types::TypedOperationException);
      // Fatal still throws a FatalException when given bad input.
   EXPECT_THROW(Backwards::Engine::Fatal(context, makeFloatValue("1")), Backwards::Engine::FatalException);
 }

TEST(EngineTests, testBasicEight) // The seven functions needed for the parser and ToString.
 {
   std::shared_ptr<Backwards::Types::ValueType> res, left, right;

   res = Backwards::Engine::NewArray();
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*res.get()));
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value.size());

   res = Backwards::Engine::NewDictionary();
   ASSERT_TRUE(typeid(Backwards::Types::DictionaryValue) == typeid(*res.get()));
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(res)->value.size());

      // Result has dependencies below.
   res = Backwards::Engine::PushBack(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*res.get()));
   ASSERT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

   EXPECT_THROW(Backwards::Engine::PushBack(std::make_shared<Backwards::Types::StringValue>("hello"), Backwards::Engine::NewArray()), Backwards::Types::TypedOperationException);

      // Depend on res above.
   EXPECT_THROW(Backwards::Engine::GetIndex(res, makeFloatValue("-3")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::GetIndex(res, makeFloatValue("-1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::GetIndex(res, makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::GetIndex(res, makeFloatValue("3")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::GetIndex(res, std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

      // Depends on res above.
   left = Backwards::Engine::GetIndex(res, makeFloatValue("0"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

   EXPECT_THROW(Backwards::Engine::GetIndex(Backwards::Engine::NewDictionary(), makeFloatValue("0")), Backwards::Types::TypedOperationException);

      // Depend on res above.
   EXPECT_THROW(Backwards::Engine::SetIndex(res, makeFloatValue("-3"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetIndex(res, makeFloatValue("-1"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetIndex(res, makeFloatValue("1"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetIndex(res, makeFloatValue("3"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetIndex(res, std::make_shared<Backwards::Types::StringValue>("world"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

      // Depends on res above.
   right = Backwards::Engine::SetIndex(res, makeFloatValue("0"), std::make_shared<Backwards::Types::StringValue>("world"));
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*right.get()));
   ASSERT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(right)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(right)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

   EXPECT_THROW(Backwards::Engine::SetIndex(Backwards::Engine::NewDictionary(), makeFloatValue("0"), makeFloatValue("0")), Backwards::Types::TypedOperationException);

      // Result has dependencies below.
   res = Backwards::Engine::Insert(Backwards::Engine::NewDictionary(), std::make_shared<Backwards::Types::StringValue>("hello"), std::make_shared<Backwards::Types::StringValue>("world"));
   ASSERT_TRUE(typeid(Backwards::Types::DictionaryValue) == typeid(*res.get()));
   ASSERT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(res)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(res)->value.begin()->first;
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);
   right = std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(res)->value.begin()->second;
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*right.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(right)->value);

   EXPECT_THROW(Backwards::Engine::Insert(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("hello"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

      // Depends on res above.
   EXPECT_THROW(Backwards::Engine::GetValue(res, std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

      // Depends on res above.
   res = Backwards::Engine::GetValue(res, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::GetValue(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::ToString(makeFloatValue("314"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("314", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::ToString(std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testJustCalls) // Don't bother validating that the output is correct.
 {
   std::shared_ptr<Backwards::Types::ValueType> res; // I KNOW WHAT I JUST SAID!

   (void) Backwards::Engine::Abs(makeFloatValue("30"));
   (void) Backwards::Engine::Abs(makeFloatValue("-30"));
   EXPECT_THROW(Backwards::Engine::Abs(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   (void) Backwards::Engine::Round(makeFloatValue("30"));
   EXPECT_THROW(Backwards::Engine::Round(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   (void) Backwards::Engine::Floor(makeFloatValue("30"));
   EXPECT_THROW(Backwards::Engine::Floor(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   (void) Backwards::Engine::Ceil(makeFloatValue("30"));
   EXPECT_THROW(Backwards::Engine::Ceil(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   (void) Backwards::Engine::Sqr(makeFloatValue("30"));
   EXPECT_THROW(Backwards::Engine::Sqr(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   EXPECT_EQ(BigInt::ROUND_TIES_EVEN, BigInt::Fixed::getRoundMode());
   (void) Backwards::Engine::SetRoundMode(makeFloatValue("4"));
   (void) Backwards::Engine::SetRoundMode(makeFloatValue("7"));
   EXPECT_EQ(BigInt::ROUND_AWAY, BigInt::Fixed::getRoundMode());
   EXPECT_THROW(Backwards::Engine::SetRoundMode(makeFloatValue("30")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetRoundMode(makeFloatValue("-1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetRoundMode(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

      // Nasty assumption that previous part of test has been run.
   res = Backwards::Engine::GetRoundMode();
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("7"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
   BigInt::Fixed::setRoundMode(BigInt::ROUND_TIES_EVEN);


   (void) Backwards::Engine::SetDefaultPrecision(makeFloatValue("4"));

   res = Backwards::Engine::GetDefaultPrecision();
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("4"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
   EXPECT_THROW(Backwards::Engine::SetDefaultPrecision(makeFloatValue("10000000000")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetDefaultPrecision(makeFloatValue("-1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetDefaultPrecision(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   (void) Backwards::Engine::SetDefaultPrecision(makeFloatValue("0"));


   res = Backwards::Engine::GetPrecision(makeFloatValue("2.00"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("2"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::GetPrecision(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);


   res = Backwards::Engine::SetPrecision(makeFloatValue("2.00"), makeFloatValue("4"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(4U, std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value.getPrecision());

   EXPECT_THROW(Backwards::Engine::SetPrecision(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("4")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetPrecision(makeFloatValue("2.00"), std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetPrecision(makeFloatValue("2.00"), makeFloatValue("-4")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SetPrecision(makeFloatValue("2.00"), makeFloatValue("10000000000")), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testSimpleCalls)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;

   res = Backwards::Engine::IsFloat(makeFloatValue("0"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsFloat(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsString(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsString(makeFloatValue("0"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsArray(std::make_shared<Backwards::Types::ArrayValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsArray(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsDictionary(std::make_shared<Backwards::Types::DictionaryValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsDictionary(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsFunction(std::make_shared<Backwards::Types::FunctionValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsFunction(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsNil(std::make_shared<Backwards::Types::NilValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsNil(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsCellRef(std::make_shared<Backwards::Types::CellRefValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsCellRef(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsCellRange(std::make_shared<Backwards::Types::CellRangeValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::IsCellRange(std::make_shared<Backwards::Types::StringValue>());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::Length(std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("5"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::Length(makeFloatValue("1")), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::Size(Backwards::Engine::NewArray());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   res = Backwards::Engine::Size(Backwards::Engine::NewDictionary());
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::Size(makeFloatValue("1")), Backwards::Types::TypedOperationException);

   std::shared_ptr<Backwards::Types::FloatValue> one = makeFloatValue("6");
   std::shared_ptr<Backwards::Types::FloatValue> two = makeFloatValue("9");

   res = Backwards::Engine::Min(one, two);
   EXPECT_EQ(one.get(), res.get()); // Yes : compare the POINTERS.
   res = Backwards::Engine::Min(two, one);
   EXPECT_EQ(one.get(), res.get());
   EXPECT_THROW(Backwards::Engine::Min(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::Min(makeFloatValue("1"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::Max(one, two);
   EXPECT_EQ(two.get(), res.get());
   res = Backwards::Engine::Max(two, one);
   EXPECT_EQ(two.get(), res.get());
   EXPECT_THROW(Backwards::Engine::Max(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::Max(makeFloatValue("1"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::ValueOf(std::make_shared<Backwards::Types::StringValue>("12"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("12"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::ValueOf(std::make_shared<Backwards::Types::StringValue>("12h")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::ValueOf(std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::ValueOf(makeFloatValue("1")), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::FromCharacter(std::make_shared<Backwards::Types::StringValue>("H"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("72"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::FromCharacter(std::make_shared<Backwards::Types::StringValue>("Hi")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::FromCharacter(makeFloatValue("1")), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::ToCharacter(makeFloatValue("72"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("H", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   EXPECT_THROW(Backwards::Engine::ToCharacter(makeFloatValue("-1024")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::ToCharacter(makeFloatValue("512")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::ToCharacter(std::make_shared<Backwards::Types::StringValue>("Hi")), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testArrayFunctions)
 {
   std::shared_ptr<Backwards::Types::ValueType> res, left;

      // Result has dependencies below.
   res = Backwards::Engine::PushFront(Backwards::Engine::PushBack(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("world")), std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*res.get()));
   ASSERT_EQ(2U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value[1];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

      // Depend on res above.
   left = Backwards::Engine::PopFront(res);
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*left.get()));
   ASSERT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

      // Depend on res above.
   left = Backwards::Engine::PopBack(res);
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*left.get()));
   ASSERT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

   EXPECT_THROW(Backwards::Engine::PushFront(Backwards::Engine::NewDictionary(), Backwards::Engine::NewArray()), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::PopFront(Backwards::Engine::NewDictionary()), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::PopBack(Backwards::Engine::NewDictionary()), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::PopFront(Backwards::Engine::NewArray()), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::PopBack(Backwards::Engine::NewArray()), Backwards::Types::TypedOperationException);

   res = Backwards::Engine::NewArrayDefault(makeFloatValue("2"), std::make_shared<Backwards::Types::StringValue>("world"));
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*res.get()));
   ASSERT_EQ(2U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value.size());
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);
   left = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(res)->value[1];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*left.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(left)->value);

   EXPECT_THROW(Backwards::Engine::NewArrayDefault(std::make_shared<Backwards::Types::StringValue>("world"), makeFloatValue("2")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::NewArrayDefault(makeFloatValue("-1"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::NewArrayDefault(makeFloatValue("10000000000"), std::make_shared<Backwards::Types::StringValue>("world")), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testDictionaryFunctions)
 {
   std::shared_ptr<Backwards::Types::ValueType> res, left, right;
   res = Backwards::Engine::Insert(Backwards::Engine::NewDictionary(), std::make_shared<Backwards::Types::StringValue>("hello"), std::make_shared<Backwards::Types::StringValue>("world"));
   res = Backwards::Engine::Insert(res, std::make_shared<Backwards::Types::StringValue>("world"), std::make_shared<Backwards::Types::StringValue>("hello"));
      // Result has dependencies below.
   res = Backwards::Engine::Insert(res, std::make_shared<Backwards::Types::StringValue>("world"), std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::DictionaryValue) == typeid(*res.get()));
   ASSERT_EQ(2U, std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(res)->value.size());

   left = Backwards::Engine::ContainsKey(res, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*left.get()));
   EXPECT_EQ(BigInt::Fixed("1"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(left)->value);

   left = Backwards::Engine::ContainsKey(res, std::make_shared<Backwards::Types::StringValue>("shenanigans"));
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*left.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(left)->value);

   EXPECT_THROW(Backwards::Engine::ContainsKey(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   left = Backwards::Engine::RemoveKey(res, std::make_shared<Backwards::Types::StringValue>("hello"));
   ASSERT_TRUE(typeid(Backwards::Types::DictionaryValue) == typeid(*left.get()));
   ASSERT_EQ(1U, std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(left)->value.size());
   right = std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(left)->value.begin()->first;
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*right.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(right)->value);
   right = std::dynamic_pointer_cast<Backwards::Types::DictionaryValue>(left)->value.begin()->second;
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*right.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(right)->value);

   EXPECT_THROW(Backwards::Engine::RemoveKey(Backwards::Engine::NewArray(), std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::RemoveKey(res, std::make_shared<Backwards::Types::StringValue>("shenanigans")), Backwards::Types::TypedOperationException);

   left = Backwards::Engine::GetKeys(res);
   ASSERT_TRUE(typeid(Backwards::Types::ArrayValue) == typeid(*left.get()));
   ASSERT_EQ(2U, std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value.size());
   right = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value[0];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*right.get()));
   EXPECT_EQ("hello", std::dynamic_pointer_cast<Backwards::Types::StringValue>(right)->value);
   right = std::dynamic_pointer_cast<Backwards::Types::ArrayValue>(left)->value[1];
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*right.get()));
   EXPECT_EQ("world", std::dynamic_pointer_cast<Backwards::Types::StringValue>(right)->value);

   EXPECT_THROW(Backwards::Engine::GetKeys(Backwards::Engine::NewArray()), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testSubstring)
 {
   EXPECT_THROW(Backwards::Engine::SubString(Backwards::Engine::NewArray(), makeFloatValue("1"), makeFloatValue("2")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("-1"), makeFloatValue("2")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("1"), makeFloatValue("8")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("8"), makeFloatValue("10")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("2"), makeFloatValue("1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("2"), makeFloatValue("-1")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("2")), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("-1"), std::make_shared<Backwards::Types::StringValue>("hello")), Backwards::Types::TypedOperationException);

   std::shared_ptr<Backwards::Types::ValueType> res;

   res = Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("1"), makeFloatValue("1"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value.length());

   res = Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("0"), makeFloatValue("1"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("h", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   res = Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("0"), makeFloatValue("2"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("he", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   res = Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("4"), makeFloatValue("5"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("o", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   res = Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("3"), makeFloatValue("5"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("lo", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);

   res = Backwards::Engine::SubString(std::make_shared<Backwards::Types::StringValue>("hello"), makeFloatValue("1"), makeFloatValue("4"));
   ASSERT_TRUE(typeid(Backwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("ell", std::dynamic_pointer_cast<Backwards::Types::StringValue>(res)->value);
 }

class DummyDebugger final : public Backwards::Engine::DebuggerHook
 {
public:
   bool entered;
   virtual void EnterDebugger(const std::string&, Backwards::Engine::CallingContext&) { entered = true; }
 };

TEST(EngineTests, testEnterDebugger)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   Backwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   res = Backwards::Engine::EnterDebugger(context);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
   EXPECT_TRUE(debugger.entered);
   EXPECT_EQ(0U, logger.logs.size());

   context.debugger = nullptr;

   res = Backwards::Engine::EnterDebugger(context);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(BigInt::Fixed("0"), std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);
   EXPECT_EQ(0U, logger.logs.size());
 }

class Crusty : public Backwards::Engine::CellRefEval
 {
public:
   mutable bool entered;

   virtual std::shared_ptr<Backwards::Types::ValueType> evaluate (Backwards::Engine::CallingContext&) const { entered = true; return makeFloatValue("1"); }

   virtual bool equal (const Backwards::Types::CellRefValue&) const { return false; }
   virtual bool notEqual (const Backwards::Types::CellRefValue&) const { return false; }
   virtual bool sort (const Backwards::Types::CellRefValue&) const { return false; }
   virtual size_t hash() const { return 0U; }
 };

class Bozo : public Backwards::Engine::CellRangeExpand
 {
public:
   mutable bool entered;

   virtual std::shared_ptr<Backwards::Types::ValueType> expand (Backwards::Engine::CallingContext&) const { entered = true; return makeFloatValue("1"); }

   virtual bool equal (const Backwards::Types::CellRangeValue&) const { return false; }
   virtual bool notEqual (const Backwards::Types::CellRangeValue&) const { return false; }
   virtual bool sort (const Backwards::Types::CellRangeValue&) const { return false; }
   virtual size_t hash() const { return 0U; }
 };

class Bingo : public Backwards::Types::CellRefHolder
 {
public:

   virtual bool equal (const Backwards::Types::CellRefValue&) const { return true; }
   virtual bool notEqual (const Backwards::Types::CellRefValue&) const { return false; }
   virtual bool sort (const Backwards::Types::CellRefValue&) const { return false; }
   virtual size_t hash() const { return 0U; }
 };

class Pennywise : public Backwards::Types::CellRangeHolder
 {
public:

   virtual bool equal (const Backwards::Types::CellRangeValue&) const { return true; }
   virtual bool notEqual (const Backwards::Types::CellRangeValue&) const { return false; }
   virtual bool sort (const Backwards::Types::CellRangeValue&) const { return false; }
   virtual size_t hash() const { return 0U; }
 };

TEST(EngineTests, testCellFunctions)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   std::shared_ptr<Crusty> clown1 = std::make_shared<Crusty>();
   std::shared_ptr<Backwards::Types::CellRefValue> perfectCell = std::make_shared<Backwards::Types::CellRefValue>(clown1);
   std::shared_ptr<Backwards::Types::CellRefValue> imperfectCell = std::make_shared<Backwards::Types::CellRefValue>(std::make_shared<Bingo>());
   std::shared_ptr<Backwards::Types::StringValue> what = std::make_shared<Backwards::Types::StringValue>("Cell");
   std::shared_ptr<Bozo> clown2 = std::make_shared<Bozo>();
   std::shared_ptr<Backwards::Types::CellRangeValue> perfectRange = std::make_shared<Backwards::Types::CellRangeValue>(clown2);
   std::shared_ptr<Backwards::Types::CellRangeValue> imperfectRange = std::make_shared<Backwards::Types::CellRangeValue>(std::make_shared<Pennywise>());

   Backwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;
   DummyDebugger debugger;
   context.debugger = &debugger;

   clown1->entered = false;
   debugger.entered = false;
   res = Backwards::Engine::EvalCell(context, perfectCell);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(clown1->entered);
   ASSERT_FALSE(debugger.entered);

   EXPECT_THROW(Backwards::Engine::EvalCell(context, what), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::EvalCell(context, imperfectCell), Backwards::Engine::ProgrammingException);

   clown2->entered = false;
   debugger.entered = false;
   res = Backwards::Engine::ExpandRange(context, perfectRange);
   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   ASSERT_EQ(0U, logger.logs.size());
   ASSERT_TRUE(clown2->entered);
   ASSERT_FALSE(debugger.entered);

   EXPECT_THROW(Backwards::Engine::ExpandRange(context, what), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Backwards::Engine::ExpandRange(context, imperfectRange), Backwards::Engine::ProgrammingException);
 }
