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
#include "Forwards/Engine/CellRangeExpand.h"
#include "Forwards/Engine/CellRefEval.h"
#include "Forwards/Engine/Expression.h"
#include "Forwards/Types/CellRefValue.h"

#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/CellRefValue.h"
#include "Backwards/Types/CellRangeValue.h"

#include "Backwards/Engine/ProgrammingException.h"

namespace Forwards
 {

namespace Engine
 {

   CellRangeExpand::CellRangeExpand()
    {
    }

   CellRangeExpand::CellRangeExpand(const std::shared_ptr<Types::CellRangeValue>& value) : value(value)
    {
    }

   std::shared_ptr<Backwards::Types::ValueType> CellRangeExpand::expand (Backwards::Engine::CallingContext&) const
    {
      std::shared_ptr<Backwards::Types::ArrayValue> result = std::make_shared<Backwards::Types::ArrayValue>();

         // This should never be true, but if it is...
      if ((value->col1 == value->col2) && (value->row1 == value->row2))
       {
         result->value.emplace_back(
            std::make_shared<Backwards::Types::CellRefValue>(
               std::make_shared<CellRefEval>(
                  std::make_shared<Constant>(Input::Token(),
                     std::make_shared<Types::CellRefValue>(true, value->col1, true, value->row1)))));
       }
      else if (value->col1 == value->col2)
       {
         for (size_t row = value->row1; row <= value->row2; ++row)
          {
            result->value.emplace_back(
               std::make_shared<Backwards::Types::CellRefValue>(
                  std::make_shared<CellRefEval>(
                     std::make_shared<Constant>(Input::Token(),
                        std::make_shared<Types::CellRefValue>(true, value->col1, true, row)))));
          }
       }
      else if (value->row1 == value->row2)
       {
         for (size_t col = value->col1; col <= value->col2; ++col)
          {
            result->value.emplace_back(
               std::make_shared<Backwards::Types::CellRefValue>(
                  std::make_shared<CellRefEval>(
                     std::make_shared<Constant>(Input::Token(),
                        std::make_shared<Types::CellRefValue>(true, col, true, value->row1)))));
          }
       }
      else
       {
         for (size_t col = value->col1; col <= value->col2; ++col)
          {
            result->value.emplace_back(
               std::make_shared<Backwards::Types::CellRangeValue>(
                  std::make_shared<CellRangeExpand>(
                     std::make_shared<Types::CellRangeValue>(col, value->row1, col, value->row2))));
          }
       }

      return result;
    }

   bool CellRangeExpand::equal (const Backwards::Types::CellRangeValue& lhs) const
    {
      try
       {
         const CellRangeExpand& val = dynamic_cast<const CellRangeExpand&>(*lhs.value.get());
         return (value->col1 == val.value->col1) && (value->col2 == val.value->col2) &&
            (value->row1 == val.value->row1) && (value->row2 == val.value->row2);
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("CellRangeHolder is not a Forward CellRangeExpand.");
       }
    }

   bool CellRangeExpand::notEqual (const Backwards::Types::CellRangeValue& lhs) const
    {
      return !equal(lhs);
    }

   bool CellRangeExpand::sort (const Backwards::Types::CellRangeValue& lhs) const
    {
      try
       {
         const CellRangeExpand& val = dynamic_cast<const CellRangeExpand&>(*lhs.value.get());

         if (value->col1 != val.value->col1) return val.value->col1 < value->col1;
         if (value->col2 != val.value->col2) return val.value->col2 < value->col2;
         if (value->row1 != val.value->row1) return val.value->row1 < value->row1;
         return val.value->row2 < value->row2;
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("CellRangeHolder is not a Forward CellRangeExpand.");
       }
    }

   size_t CellRangeExpand::hash() const
    {
      size_t result = std::hash<size_t>()(value->col1);
      Backwards::Types::boost_hash_combine(result, std::hash<size_t>()(value->col2));
      Backwards::Types::boost_hash_combine(result, std::hash<size_t>()(value->row1));
      Backwards::Types::boost_hash_combine(result, std::hash<size_t>()(value->row2));
      return result;
    }

 } // namespace Engine

 } // namespace Forwards
