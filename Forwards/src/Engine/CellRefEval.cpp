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
#include "Forwards/Engine/CellRefEval.h"
#include "Forwards/Types/CellRefValue.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Types/FloatValue.h"
#include "Forwards/Types/StringValue.h"
#include "Forwards/Types/CellRangeValue.h"
#include "Forwards/Engine/CellRangeExpand.h"

#include "Backwards/Types/CellRefValue.h"
#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/NilValue.h"
#include "Backwards/Types/CellRangeValue.h"

#include "Backwards/Engine/ProgrammingException.h"

namespace Forwards
 {

namespace Engine
 {

   static const Types::CellRefValue* getReferencedCell(const std::shared_ptr<Expression>& expr)
    {
      if (typeid(Constant) == typeid(*expr.get()))
       {
         const Constant& temp1 = static_cast<const Constant&>(*expr.get());
         if (typeid(Types::CellRefValue) == typeid(*temp1.value.get()))
          {
            return static_cast<const Types::CellRefValue*>(temp1.value.get());
          }
       }
      return nullptr;
    }

   static const Types::CellRefValue* getReferencedCell(const Backwards::Types::CellRefValue& val)
    {
      if (typeid(CellRefEval) == typeid(*val.value.get()))
       {
         return getReferencedCell(static_cast<const CellRefEval&>(*val.value.get()).value);
       }
      throw Backwards::Engine::ProgrammingException("CellRefHolder is not a Forward CellRefValue.");
    }

   CellRefEval::CellRefEval()
    {
    }

   CellRefEval::CellRefEval(const std::shared_ptr<Expression>& value) : value(value)
    {
    }

   std::shared_ptr<Backwards::Types::ValueType> CellRefEval::evaluate (Backwards::Engine::CallingContext& context) const
    {
      try
       {
         std::shared_ptr<Types::ValueType> result = value->evaluate(dynamic_cast<Forwards::Engine::CallingContext&>(context));
         switch (result->getType())
          {
         case Types::FLOAT:
            return std::make_shared<Backwards::Types::FloatValue>(static_cast<Types::FloatValue&>(*result.get()).value);
         case Types::STRING:
            return std::make_shared<Backwards::Types::StringValue>(static_cast<Types::StringValue&>(*result.get()).value);
         case Types::NIL:
            return std::make_shared<Backwards::Types::NilValue>();
         case Types::CELL_REF:
            throw Backwards::Engine::ProgrammingException("CellRefEval::evaluate did not resolve to a Backwards Type.");
         case Types::CELL_RANGE:
            return std::make_shared<Backwards::Types::CellRangeValue>(std::make_shared<CellRangeExpand>(std::static_pointer_cast<Types::CellRangeValue>(result)));
          }
         throw Backwards::Engine::ProgrammingException("Forward getType returned invalid type.");
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards context wasn't Forwards context.");
       }
    }

   bool CellRefEval::equal (const Backwards::Types::CellRefValue& lhs) const
    {
      const Types::CellRefValue* LHS = getReferencedCell(lhs);
      const Types::CellRefValue* RHS = getReferencedCell(value);

      if ((nullptr != LHS) && (nullptr != RHS))
       {
         return (LHS->colAbsolute == RHS->colAbsolute) && (LHS->rowAbsolute == RHS->rowAbsolute) &&
            (LHS->colRef == RHS->colRef) && (LHS->rowRef == RHS->rowRef);
       }
      return lhs.value.get() == this;
    }

   bool CellRefEval::notEqual (const Backwards::Types::CellRefValue& lhs) const
    {
      return !equal(lhs);
    }

   bool CellRefEval::sort (const Backwards::Types::CellRefValue& lhs) const
    {
      const Types::CellRefValue* LHS = getReferencedCell(lhs);
      const Types::CellRefValue* RHS = getReferencedCell(value);

      if ((nullptr != LHS) && (nullptr != RHS))
       {
         if (LHS->colAbsolute != RHS->colAbsolute) return LHS->colAbsolute < RHS->colAbsolute;
         if (LHS->rowAbsolute != RHS->rowAbsolute) return LHS->rowAbsolute < RHS->rowAbsolute;
         if (LHS->colRef != RHS->colRef) return LHS->colRef < RHS->colRef;
         return LHS->rowRef < RHS->rowRef;
       }
      return lhs.value.get() < this;
    }

   size_t CellRefEval::hash() const
    {
      const Types::CellRefValue* THIS = getReferencedCell(value);

      if (nullptr != THIS)
       {
         size_t result = std::hash<bool>()(THIS->colAbsolute);
         Backwards::Types::boost_hash_combine(result, std::hash<size_t>()(THIS->colRef));
         Backwards::Types::boost_hash_combine(result, std::hash<bool>()(THIS->rowAbsolute));
         Backwards::Types::boost_hash_combine(result, std::hash<size_t>()(THIS->rowRef));
         return result;
       }
      return std::hash<void*>()(value.get());
    }

 } // namespace Engine

 } // namespace Forwards
