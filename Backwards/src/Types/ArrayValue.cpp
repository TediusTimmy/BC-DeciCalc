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
#include "Backwards/Types/ValueType.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"
#include "Backwards/Types/NilValue.h"
#include "Backwards/Types/CellRefValue.h"
#include "Backwards/Types/CellRangeValue.h"

namespace Backwards
 {

namespace Types
 {

   const std::string& ArrayValue::getTypeName() const
    {
      static const std::string name ("Array");
      return name;
    }

   std::shared_ptr<ValueType> ArrayValue::neg() const
    {
      std::shared_ptr<ArrayValue> result = std::make_shared<ArrayValue>();
      result->value.reserve(value.size());
      for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter = value.begin();
         value.end() != iter; ++iter)
       {
         result->value.emplace_back((*iter)->neg());
       }
      return result;
    }

#define COMMUTEARRAY(x,y) \
   std::shared_ptr<ValueType> ArrayValue::x (const y& lhs) const \
    { \
      std::shared_ptr<ArrayValue> result = std::make_shared<ArrayValue>(); \
      result->value.reserve(value.size()); \
      for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter = value.begin(); \
         value.end() != iter; ++iter) \
       { \
         result->value.emplace_back(lhs.x(**iter)); \
       } \
      return result; \
    }

   COMMUTEARRAY(add, FloatValue)
   COMMUTEARRAY(add, StringValue)
   COMMUTEARRAY(add, ArrayValue)
   COMMUTEARRAY(add, DictionaryValue)
   COMMUTEARRAY(sub, FloatValue)
   COMMUTEARRAY(sub, ArrayValue)
   COMMUTEARRAY(sub, DictionaryValue)
   COMMUTEARRAY(mul, FloatValue)
   COMMUTEARRAY(mul, ArrayValue)
   COMMUTEARRAY(mul, DictionaryValue)
   COMMUTEARRAY(div, FloatValue)
   COMMUTEARRAY(div, ArrayValue)
   COMMUTEARRAY(div, DictionaryValue)


   bool ArrayValue::equal (const ArrayValue& lhs) const
    {
      bool are_equal = false;
      if (lhs.value.size() == value.size())
       {
         are_equal = true;
         for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter1 = lhs.value.begin(),
            iter2 = value.begin(); (lhs.value.end() != iter1) && (true == are_equal); ++iter1, ++iter2)
          {
            are_equal &= ((*iter1)->compare(**iter2));
          }
       }
      return are_equal;
    }

   bool ArrayValue::notEqual (const ArrayValue& lhs) const
    {
      return (false == equal(lhs));
    }


#define ARRAYCOMMUTE(x) \
   std::shared_ptr<ValueType> ArrayValue::x (const ValueType& rhs) const \
    { \
      std::shared_ptr<ArrayValue> result = std::make_shared<ArrayValue>(); \
      result->value.reserve(value.size()); \
      for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter = value.begin(); \
         value.end() != iter; ++iter) \
       { \
         result->value.emplace_back((*iter)->x(rhs)); \
       } \
      return result; \
    }

   ARRAYCOMMUTE(add)
   ARRAYCOMMUTE(sub)
   ARRAYCOMMUTE(mul)
   ARRAYCOMMUTE(div)
   IMPLEMENTBOOL(ArrayValue, greater)
   IMPLEMENTBOOL(ArrayValue, less)
   IMPLEMENTBOOL(ArrayValue, geq)
   IMPLEMENTBOOL(ArrayValue, leq)
   IMPLEMENTBOOL(ArrayValue, equal)
   IMPLEMENTBOOL(ArrayValue, notEqual)
   IMPLEMENTBOOL(ArrayValue, sort)

   bool ArrayValue::sort (const FloatValue&) const
    {
      return true;
    }

   bool ArrayValue::sort (const StringValue&) const
    {
      return true;
    }

   bool ArrayValue::sort (const ArrayValue& lhs) const
    {
      bool is_less = false;
      if (lhs.value.size() == value.size())
       {
         for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter1 = lhs.value.begin(),
            iter2 = value.begin(); lhs.value.end() != iter1; ++iter1, ++iter2)
          {
            if (false == (*iter1)->compare(**iter2))
             {
               is_less = (*iter1)->sort(**iter2);
               break;
             }
          }
       }
      else
       {
         is_less = lhs.value.size() < value.size();
       }
      return is_less;
    }

   bool ArrayValue::sort (const DictionaryValue&) const
    {
      return false;
    }

   bool ArrayValue::sort (const FunctionValue&) const
    {
      return false;
    }

   bool ArrayValue::sort (const NilValue&) const
    {
      return false;
    }

   bool ArrayValue::sort (const CellRefValue&) const
    {
      return false;
    }

   bool ArrayValue::sort (const CellRangeValue&) const
    {
      return false;
    }

   size_t ArrayValue::hash() const
    {
                      // S H I A L A B E O U F
      size_t result = 0x534849414C414245;
      for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter = value.begin();
         value.end() != iter; ++iter)
       {
         boost_hash_combine(result, (*iter)->hash());
       }
      return result;
    }

 } // namespace Types

 } // namespace Backwards
