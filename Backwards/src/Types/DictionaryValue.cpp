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

   bool ChristHowHorrifying::operator() (const std::shared_ptr<ValueType>& lhs, const std::shared_ptr<ValueType>& rhs) const
    {
      return lhs->sort(*rhs);
    }

   const std::string& DictionaryValue::getTypeName() const
    {
      static const std::string name ("Dictionary");
      return name;
    }

   std::shared_ptr<ValueType> DictionaryValue::neg() const
    {
      std::shared_ptr<DictionaryValue> result = std::make_shared<DictionaryValue>();
      for (std::map<std::shared_ptr<ValueType>, std::shared_ptr<ValueType>, ChristHowHorrifying>::const_iterator iter = value.begin();
         value.end() != iter; ++iter)
       {
         result->value.emplace(std::make_pair(iter->first, iter->second->neg()));
       }
      return result;
    }

#define COMMUTEDICTIONARY(x,y) \
   std::shared_ptr<ValueType> DictionaryValue::x (const y& lhs) const \
    { \
      std::shared_ptr<DictionaryValue> result = std::make_shared<DictionaryValue>(); \
      for (std::map<std::shared_ptr<ValueType>, std::shared_ptr<ValueType>, ChristHowHorrifying>::const_iterator iter = value.begin(); \
         value.end() != iter; ++iter) \
       { \
         result->value.emplace(std::make_pair(iter->first, lhs.x(*(iter->second)))); \
       } \
      return result; \
    }

   COMMUTEDICTIONARY(add, FloatValue)
   COMMUTEDICTIONARY(add, StringValue)
   COMMUTEDICTIONARY(add, ArrayValue)
   COMMUTEDICTIONARY(add, DictionaryValue)
   COMMUTEDICTIONARY(sub, FloatValue)
   COMMUTEDICTIONARY(sub, ArrayValue)
   COMMUTEDICTIONARY(sub, DictionaryValue)
   COMMUTEDICTIONARY(mul, FloatValue)
   COMMUTEDICTIONARY(mul, ArrayValue)
   COMMUTEDICTIONARY(mul, DictionaryValue)
   COMMUTEDICTIONARY(div, FloatValue)
   COMMUTEDICTIONARY(div, ArrayValue)
   COMMUTEDICTIONARY(div, DictionaryValue)


   bool DictionaryValue::equal (const DictionaryValue& lhs) const
    {
      bool are_equal = false;
      if (lhs.value.size() == value.size())
       {
         are_equal = true;
         for (std::map<std::shared_ptr<ValueType>, std::shared_ptr<ValueType>, ChristHowHorrifying>::const_iterator iter1 = lhs.value.begin(),
            iter2 = value.begin(); (lhs.value.end() != iter1) && (true == are_equal); ++iter1, ++iter2)
          {
            are_equal &= (iter1->first->compare(*(iter2->first)));
            if (true == are_equal)
             {
               are_equal &= (iter1->second->compare(*(iter2->second)));
             }
          }
       }
      return are_equal;
    }

   bool DictionaryValue::notEqual (const DictionaryValue& lhs) const
    {
      return (false == equal(lhs));
    }


#define DICTIONARYCOMMUTE(x) \
   std::shared_ptr<ValueType> DictionaryValue::x (const ValueType& rhs) const \
    { \
      std::shared_ptr<DictionaryValue> result = std::make_shared<DictionaryValue>(); \
      for (std::map<std::shared_ptr<ValueType>, std::shared_ptr<ValueType>, ChristHowHorrifying>::const_iterator iter = value.begin(); \
         value.end() != iter; ++iter) \
       { \
         result->value.emplace(std::make_pair(iter->first, iter->second->x(rhs))); \
       } \
      return result; \
    }

   DICTIONARYCOMMUTE(add)
   DICTIONARYCOMMUTE(sub)
   DICTIONARYCOMMUTE(mul)
   DICTIONARYCOMMUTE(div)
   IMPLEMENTBOOL(DictionaryValue, greater)
   IMPLEMENTBOOL(DictionaryValue, less)
   IMPLEMENTBOOL(DictionaryValue, geq)
   IMPLEMENTBOOL(DictionaryValue, leq)
   IMPLEMENTBOOL(DictionaryValue, equal)
   IMPLEMENTBOOL(DictionaryValue, notEqual)
   IMPLEMENTBOOL(DictionaryValue, sort)

   bool DictionaryValue::sort (const FloatValue&) const
    {
      return true;
    }

   bool DictionaryValue::sort (const StringValue&) const
    {
      return true;
    }

   bool DictionaryValue::sort (const ArrayValue&) const
    {
      return true;
    }

   bool DictionaryValue::sort (const DictionaryValue& lhs) const
    {
      bool is_less = false;
      if (lhs.value.size() == value.size())
       {
         for (std::map<std::shared_ptr<ValueType>, std::shared_ptr<ValueType>, ChristHowHorrifying>::const_iterator iter1 = lhs.value.begin(),
            iter2 = value.begin(); lhs.value.end() != iter1; ++iter1, ++iter2)
          {
            if (false == (iter1->first->compare(*(iter2->first))))
             {
               is_less = (iter1->first->sort(*(iter2->first)));
               break;
             }
            if (false == (iter1->second->compare(*(iter2->second))))
             {
               is_less = (iter1->second->sort(*(iter2->second)));
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

   bool DictionaryValue::sort (const FunctionValue&) const
    {
      return false;
    }

   bool DictionaryValue::sort (const NilValue&) const
    {
      return false;
    }

   bool DictionaryValue::sort (const CellRefValue&) const
    {
      return false;
    }

   bool DictionaryValue::sort (const CellRangeValue&) const
    {
      return false;
    }

   size_t DictionaryValue::hash() const
    {
                      // B E E F C A K E
      size_t result = 0x4245454643414B45;
      for (std::map<std::shared_ptr<ValueType>, std::shared_ptr<ValueType>, ChristHowHorrifying>::const_iterator iter = value.begin();
         value.end() != iter; ++iter)
       {
         size_t temp = iter->first->hash();
         boost_hash_combine(temp, iter->second->hash());

         // We can't do anything special here because the final hash needs to be independent of iteration order.
         result ^= temp;
       }
      return result;
    }

 } // namespace Types

 } // namespace Backwards
