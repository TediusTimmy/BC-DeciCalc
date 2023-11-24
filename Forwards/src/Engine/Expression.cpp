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
#include "Forwards/Engine/Expression.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/CellRefEval.h"
#include "Forwards/Engine/CellRangeExpand.h"

#include "Forwards/Types/FloatValue.h"
#include "Forwards/Types/StringValue.h"
#include "Forwards/Types/NilValue.h"
#include "Forwards/Types/CellRefValue.h"
#include "Forwards/Types/CellRangeValue.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/NilValue.h"
#include "Backwards/Types/CellRefValue.h"
#include "Backwards/Types/CellRangeValue.h"

#include "Backwards/Engine/ProgrammingException.h"

#include <sstream>

namespace Forwards
 {

namespace Engine
 {

   static std::string wrapInParens(const std::string& me, int prevLevel, int myLevel)
    {
      if (prevLevel < 0)
       {
         if (std::abs(prevLevel) >= myLevel)
          {
            return "(" + me + ")";
          }
       }
      else
       {
         if (prevLevel > myLevel)
          {
            return "(" + me + ")";
          }
       }
      return me;
    }

   Expression::Expression(const Input::Token& token) : token(token)
    {
    }

   std::string Expression::constructMessage(const std::string& e) const
    {
      return constructMessage(e, token);
    }

   std::string Expression::constructMessage(const std::string& e, const Input::Token& token)
    {
      std::stringstream str;
      str << e << " at " << token.location;
      throw Backwards::Types::TypedOperationException(str.str());
    }

   std::shared_ptr<Types::FloatValue> Expression::FLOAT_ONE()
    {
      static std::shared_ptr<Types::FloatValue> one = std::make_shared<Types::FloatValue>(BigInt::Fixed("1"));
      return one;
    }

   std::shared_ptr<Types::FloatValue> Expression::FLOAT_ZERO()
    {
      static std::shared_ptr<Types::FloatValue> zero = std::make_shared<Types::FloatValue>(BigInt::Fixed("0"));
      return zero;
    }


   Constant::Constant(const Input::Token& token, const std::shared_ptr<Types::ValueType>& value) : Expression(token), value(value)
    {
    }

   std::shared_ptr<Types::ValueType> Constant::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> result = value;
      if (Types::CELL_REF == result->getType())
       {
         result = finalConst(std::static_pointer_cast<Types::CellRefValue>(result), context);
       }
      return result;
    }

   std::string Constant::toString(size_t col, size_t row, int) const
    {
      return value->toString(col, row);
    }

   std::shared_ptr<Types::ValueType> Constant::finalConst (std::shared_ptr<Types::CellRefValue> value, CallingContext& context)
    {
         // Determine column and row.
      int64_t col, row;
      if ((true == value->colAbsolute) && (true == value->rowAbsolute))
       {
         col = value->colRef;
         row = value->rowRef;
       }
      else if (true == value->colAbsolute)
       {
         col = value->colRef;
         row = Types::CellRefValue::getRow(context.topCell()->row, value->rowRef);
       }
      else if (true == value->rowAbsolute)
       {
         col = Types::CellRefValue::getColumn(context.topCell()->col, value->colRef);
         row = value->rowRef;
       }
      else
       {
         col = Types::CellRefValue::getColumn(context.topCell()->col, value->colRef);
         row = Types::CellRefValue::getRow(context.topCell()->row, value->rowRef);
       }

      Cell* cell = context.theSheet->getCellAt(col, row);
         // If no cell, Nil.
      if (nullptr == cell)
       {
         return std::make_shared<Types::NilValue>();
       }

         // If we are currently evaluating this cell, stop.
      if (true == cell->inEvaluation)
       {
         std::shared_ptr<Types::ValueType> result = cell->previousValue;
         if (nullptr == result.get())
          {
            result = std::make_shared<Types::NilValue>();
          }
         cell->recursed = true;
         return result;
       }

         // Guess we need to do work.
      std::shared_ptr<Types::ValueType> result;
      (void) context.theSheet->computeCell(context, result, col, row, true);
      if (nullptr == result.get())
       {
         result = std::make_shared<Types::NilValue>();
       }
      return result;
    }


#define OperationConstructor(x) \
   x::x(const Input::Token& token, const std::shared_ptr<Expression>& lhs, const std::shared_ptr<Expression>& rhs) : \
      Expression(token), lhs(lhs), rhs(rhs) \
    { \
    }

   OperationConstructor(Plus)

   std::shared_ptr<Types::ValueType> Plus::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::FloatValue>(static_cast<Types::FloatValue*>(LHS.get())->value + static_cast<Types::FloatValue*>(RHS.get())->value);
            break;
         case Types::NIL:
            result = LHS; // This is why we do math this way, instead of the better Backwards way.
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error adding " + LHS->getTypeName() + " to " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = RHS;
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error adding " + LHS->getTypeName() + " to " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error adding " + LHS->getTypeName() + " to " + RHS->getTypeName());
       }
      return result;
    }

   std::string Plus::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 2) + "+" + rhs->toString(col, row, 2), level, 2);
    }

   OperationConstructor(Minus)

   std::shared_ptr<Types::ValueType> Minus::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::FloatValue>(static_cast<Types::FloatValue*>(LHS.get())->value - static_cast<Types::FloatValue*>(RHS.get())->value);
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error subtracting " + RHS->getTypeName() + " from " + LHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::FloatValue>(-static_cast<Types::FloatValue*>(RHS.get())->value);
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error subtracting " + RHS->getTypeName() + " from " + LHS->getTypeName());
          }
         break;
      case Types::STRING:
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error subtracting " + RHS->getTypeName() + " from " + LHS->getTypeName());
       }
      return result;
    }

   std::string Minus::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 2) + "-" + rhs->toString(col, row, -2), level, 2);
    }

   OperationConstructor(Multiply)

   std::shared_ptr<Types::ValueType> Multiply::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::FloatValue>(static_cast<Types::FloatValue*>(LHS.get())->value * static_cast<Types::FloatValue*>(RHS.get())->value);
            break;
         case Types::NIL:
            result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error multiplying " + LHS->getTypeName() + " by " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error multiplying " + LHS->getTypeName() + " by " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error multiplying " + LHS->getTypeName() + " by " + RHS->getTypeName());
       }
      return result;
    }

   std::string Multiply::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 3) + "*" + rhs->toString(col, row, 3), level, 3);
    }

   OperationConstructor(Divide)

   std::shared_ptr<Types::ValueType> Divide::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::FloatValue>(static_cast<Types::FloatValue*>(LHS.get())->value / static_cast<Types::FloatValue*>(RHS.get())->value);
            break;
         case Types::NIL: // Nil is a positive zero, and preserve the sign of infinity.
            result = std::make_shared<Types::FloatValue>(static_cast<Types::FloatValue*>(LHS.get())->value / FLOAT_ZERO()->value);
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error dividing " + LHS->getTypeName() + " by " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error dividing " + LHS->getTypeName() + " by " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error dividing " + LHS->getTypeName() + " by " + RHS->getTypeName());
       }
      return result;
    }

   std::string Divide::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 3) + "/" + rhs->toString(col, row, 3), level, 3);
    }

   OperationConstructor(Cat)

   std::shared_ptr<Types::ValueType> Cat::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            // We will abuse the fact that FLOAT and STRING don't care what cell they are in.
            result = std::make_shared<Types::StringValue>(LHS->toString(0U, 0U) + RHS->toString(0U, 0U));
            break;
         case Types::NIL:
            result = std::make_shared<Types::StringValue>(LHS->toString(0U, 0U));
            break;
         case Types::STRING:
            result = std::make_shared<Types::StringValue>(LHS->toString(0U, 0U) + RHS->toString(0U, 0U));
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error catenating " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::StringValue>(LHS->toString(0U, 0U) + RHS->toString(0U, 0U));
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
            result = std::make_shared<Types::StringValue>(LHS->toString(0U, 0U) + RHS->toString(0U, 0U));
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error catenating " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            result = std::make_shared<Types::StringValue>(RHS->toString(0U, 0U));
            break;
         case Types::NIL:
            result = LHS;
            break;
         case Types::STRING:
            result = RHS;
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error catenating " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error catenating " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string Cat::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 2) + "&" + rhs->toString(col, row, 2), level, 2);
    }

   OperationConstructor(MakeRange)

   std::shared_ptr<Types::ValueType> MakeRange::evaluate (CallingContext& context) const
    {
         // Pull out the Cell Refs that OUGHT to be our arguments. We need the RAW references.
      std::shared_ptr<Constant> LHSc = std::dynamic_pointer_cast<Constant>(lhs);
      std::shared_ptr<Constant> RHSc = std::dynamic_pointer_cast<Constant>(rhs);
      if ((nullptr == LHSc.get()) || (nullptr == RHSc.get()))
       {
         throw Backwards::Engine::ProgrammingException("The arguments to MakeRange weren't Cell Refs.");
       }
      std::shared_ptr<Types::CellRefValue> LHS = std::dynamic_pointer_cast<Types::CellRefValue>(LHSc->value);
      std::shared_ptr<Types::CellRefValue> RHS = std::dynamic_pointer_cast<Types::CellRefValue>(RHSc->value);
      if ((nullptr == LHS.get()) || (nullptr == RHS.get()))
       {
         throw Backwards::Engine::ProgrammingException("The arguments to MakeRange weren't Cell Refs.");
       }

         // Determine column and row.
      int64_t col1, row1;
      if ((true == LHS->colAbsolute) && (true == LHS->rowAbsolute))
       {
         col1 = LHS->colRef;
         row1 = LHS->rowRef;
       }
      else if (true == LHS->colAbsolute)
       {
         col1 = LHS->colRef;
         row1 = context.topCell()->row + LHS->rowRef;
       }
      else if (true == LHS->rowAbsolute)
       {
         col1 = context.topCell()->col + LHS->colRef;
         row1 = LHS->rowRef;
       }
      else
       {
         col1 = context.topCell()->col + LHS->colRef;
         row1 = context.topCell()->row + LHS->rowRef;
       }
      int64_t col2, row2;
      if ((true == RHS->colAbsolute) && (true == RHS->rowAbsolute))
       {
         col2 = RHS->colRef;
         row2 = RHS->rowRef;
       }
      else if (true == RHS->colAbsolute)
       {
         col2 = RHS->colRef;
         row2 = context.topCell()->row + RHS->rowRef;
       }
      else if (true == RHS->rowAbsolute)
       {
         col2 = context.topCell()->col + RHS->colRef;
         row2 = RHS->rowRef;
       }
      else
       {
         col2 = context.topCell()->col + RHS->colRef;
         row2 = context.topCell()->row + RHS->rowRef;
       }

         // Validate
      if ((col1 < 0) || (col2 < 0) || (row1 < 0) || (row2 < 0))
       {
         constructMessage("Invalid cell reference");
       }

      if (col1 > col2)
       {
         std::swap(col1, col2);
       }
      if (row1 > row2)
       {
         std::swap(row1, row2);
       }

      return std::make_shared<Types::CellRangeValue>(col1, row1, col2, row2);
    }

   std::string MakeRange::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 5) + ":" + rhs->toString(col, row, 5), level, 5);
    }

   OperationConstructor(Equals)

   std::shared_ptr<Types::ValueType> Equals::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(LHS.get())->value == static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::FloatValue*>(LHS.get())->value.isZero())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::STRING:
            if (static_cast<Types::StringValue*>(LHS.get())->value == static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::StringValue*>(LHS.get())->value.empty())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::FLOAT:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(RHS.get())->value.isZero())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = FLOAT_ONE();
            break;
         case Types::STRING:
            if (static_cast<Types::StringValue*>(RHS.get())->value.empty())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string Equals::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 1) + "=" + rhs->toString(col, row, 1), level, 1);
    }

   OperationConstructor(NotEqual)

   std::shared_ptr<Types::ValueType> NotEqual::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(LHS.get())->value != static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (!static_cast<Types::FloatValue*>(LHS.get())->value.isZero())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::STRING:
            if (static_cast<Types::StringValue*>(LHS.get())->value != static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (!static_cast<Types::StringValue*>(LHS.get())->value.empty())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::FLOAT:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (!static_cast<Types::FloatValue*>(RHS.get())->value.isZero())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = FLOAT_ZERO();
            break;
         case Types::STRING:
            if (!static_cast<Types::StringValue*>(RHS.get())->value.empty())
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string NotEqual::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 1) + "<>" + rhs->toString(col, row, 1), level, 1);
    }

   OperationConstructor(Greater)

   std::shared_ptr<Types::ValueType> Greater::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(LHS.get())->value > static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::FloatValue*>(LHS.get())->value > FLOAT_ZERO()->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::STRING:
            if (static_cast<Types::StringValue*>(LHS.get())->value > static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::StringValue*>(LHS.get())->value > "")
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::FLOAT:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (FLOAT_ZERO()->value > static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
         case Types::STRING:
            result = FLOAT_ZERO();
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string Greater::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 1) + ">" + rhs->toString(col, row, 1), level, 1);
    }

   OperationConstructor(Less)

   std::shared_ptr<Types::ValueType> Less::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(LHS.get())->value < static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::FloatValue*>(LHS.get())->value < FLOAT_ZERO()->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::STRING:
            if (static_cast<Types::StringValue*>(LHS.get())->value < static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = FLOAT_ZERO();
            break;
         case Types::FLOAT:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (FLOAT_ZERO()->value < static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = FLOAT_ZERO();
            break;
         case Types::STRING:
            if ("" < static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string Less::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 1) + "<" + rhs->toString(col, row, 1), level, 1);
    }

   OperationConstructor(GEQ)

   std::shared_ptr<Types::ValueType> GEQ::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(LHS.get())->value >= static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::FloatValue*>(LHS.get())->value >= FLOAT_ZERO()->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::STRING:
            if (static_cast<Types::StringValue*>(LHS.get())->value >= static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = FLOAT_ONE();
            break;
         case Types::FLOAT:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (FLOAT_ZERO()->value >= static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            result = FLOAT_ONE();
            break;
         case Types::STRING:
            if ("" >= static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string GEQ::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 1) + ">=" + rhs->toString(col, row, 1), level, 1);
    }

   OperationConstructor(LEQ)

   std::shared_ptr<Types::ValueType> LEQ::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (LHS->getType())
       {
      case Types::FLOAT:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (static_cast<Types::FloatValue*>(LHS.get())->value <= static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::FloatValue*>(LHS.get())->value <= FLOAT_ZERO()->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::STRING:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::STRING:
         switch (RHS->getType())
          {
         case Types::STRING:
            if (static_cast<Types::StringValue*>(LHS.get())->value <= static_cast<Types::StringValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
            if (static_cast<Types::StringValue*>(LHS.get())->value <= "")
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::FLOAT:
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::NIL:
         switch (RHS->getType())
          {
         case Types::FLOAT:
            if (FLOAT_ZERO()->value <= static_cast<Types::FloatValue*>(RHS.get())->value)
               result = FLOAT_ONE();
            else
               result = FLOAT_ZERO();
            break;
         case Types::NIL:
         case Types::STRING:
            result = FLOAT_ONE();
            break;
         case Types::CELL_REF:
         case Types::CELL_RANGE:
            constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
          }
         break;
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error comparing " + LHS->getTypeName() + " with " + RHS->getTypeName());
       }
      return result;
    }

   std::string LEQ::toString(size_t col, size_t row, int level) const
    {
      return wrapInParens(lhs->toString(col, row, 1) + "<=" + rhs->toString(col, row, 1), level, 1);
    }


   Negate::Negate(const Input::Token& token, const std::shared_ptr<Expression>& arg) : Expression(token), arg(arg)
    {
    }

   std::shared_ptr<Types::ValueType> Negate::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> ARG = arg->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      switch (ARG->getType())
       {
      case Types::FLOAT:
         result = std::make_shared<Types::FloatValue>(-static_cast<Types::FloatValue*>(ARG.get())->value);
         break;
      case Types::NIL:
         result = ARG;
         break;
      case Types::STRING:
      case Types::CELL_REF:
      case Types::CELL_RANGE:
         constructMessage("Error negating " + ARG->getTypeName());
       }
      return result;
    }

   std::string Negate::toString(size_t col, size_t row, int) const
    {
      return "-" + arg->toString(col, row, 4);
    }


   FunctionCall::FunctionCall(const Input::Token& token, const std::shared_ptr<Backwards::Engine::Expression>& location, const std::vector<std::shared_ptr<Expression> >& args) :
      Expression(token), location(location), args(args)
    {
    }

   std::shared_ptr<Types::ValueType> FunctionCall::evaluate (CallingContext& context) const
    {
      std::shared_ptr<Backwards::Types::ArrayValue> newArg = std::make_shared<Backwards::Types::ArrayValue>();
      for (std::shared_ptr<Expression> expr : args)
       {
         newArg->value.emplace_back(
            std::make_shared<Backwards::Types::CellRefValue>(
               std::make_shared<CellRefEval>(expr)));
       }
      std::vector<std::shared_ptr<Backwards::Engine::Expression> > newArgs;
      newArgs.push_back(std::make_shared<Backwards::Engine::Constant>(Backwards::Input::Token(), newArg));

      Backwards::Engine::FunctionCall call (Backwards::Input::Token(), location, newArgs);

      std::shared_ptr<Backwards::Types::ValueType> returned = call.evaluate(context);

      if (typeid(Backwards::Types::CellRefValue) == typeid(*returned.get()))
       {
         std::shared_ptr<CellRefEval> temp = std::dynamic_pointer_cast<CellRefEval>(static_cast<Backwards::Types::CellRefValue*>(returned.get())->value);
         if (nullptr == temp.get())
          {
            throw Backwards::Engine::ProgrammingException("CellRefHolder was not a Forward CellRefEval.");
          }
         returned = temp->evaluate(context);
       }

      std::shared_ptr<Types::ValueType> result;
      if (typeid(Backwards::Types::FloatValue) == typeid(*returned.get()))
       {
         result = std::make_shared<Types::FloatValue>(static_cast<Backwards::Types::FloatValue*>(returned.get())->value);
       }
      else if (typeid(Backwards::Types::StringValue) == typeid(*returned.get()))
       {
         result = std::make_shared<Types::StringValue>(static_cast<Backwards::Types::StringValue*>(returned.get())->value);
       }
      else if (typeid(Backwards::Types::NilValue) == typeid(*returned.get()))
       {
         result = std::make_shared<Types::NilValue>();
       }
      else if (typeid(Backwards::Types::CellRangeValue) == typeid(*returned.get()))
       {
         std::shared_ptr<CellRangeExpand> temp = std::dynamic_pointer_cast<CellRangeExpand>(static_cast<Backwards::Types::CellRangeValue*>(returned.get())->value);
         if (nullptr == temp.get())
          {
            throw Backwards::Engine::ProgrammingException("CellRangeHolder was not a Forward CellRangeExpand.");
          }
         result = temp->value;
       }
      else
       {
         throw Backwards::Engine::ProgrammingException("Call to function " + token.text + " returned invalid type");
       }
      return result;
    }

   std::string FunctionCall::toString(size_t col, size_t row, int) const
    {
      std::string result = "@" + token.text;
      if (false == args.empty())
       {
         result += "(";
         for (size_t i = 0U; i < args.size(); ++i)
          {
            if (0U != i)
               result += ";";
            result += args[i]->toString(col, row, 0);
          }
         result += ")";
       }
      return result;
    }

 } // namespace Forwards

 } // namespace Backwards
