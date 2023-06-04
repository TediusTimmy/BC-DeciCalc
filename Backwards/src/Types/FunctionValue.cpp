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

#include <functional>

namespace Backwards
 {

namespace Types
 {

   FunctionValue::FunctionValue() : value(nullptr), captures()
    {
    }

   FunctionValue::FunctionValue(const std::shared_ptr<FunctionObjectHolder>& value, const std::vector<std::shared_ptr<ValueType> >& captures) : value(value), captures(captures)
    {
    }

   FunctionValue::FunctionValue(const std::vector<std::shared_ptr<ValueType> >& captures, const std::weak_ptr<FunctionObjectHolder>& value) : valueToo(value), captures(captures)
    {
    }

   const std::string& FunctionValue::getTypeName() const
    {
      static const std::string name ("Function");
      return name;
    }

   bool FunctionValue::equal (const FunctionValue& lhs) const
    {
      bool are_equal = lhs.value.get() == value.get();
      if (true == are_equal)
       {
         for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter1 = lhs.captures.begin(),
            iter2 = captures.begin(); (lhs.captures.end() != iter1) && (true == are_equal); ++iter1, ++iter2)
          {
            are_equal &= ((*iter1)->compare(**iter2));
          }
       }
      return are_equal;
    }

   bool FunctionValue::notEqual (const FunctionValue& lhs) const
    {
      return (false == equal(lhs));
    }

   IMPLEMENTVISITOR(FunctionValue)

   bool FunctionValue::sort (const FloatValue&) const
    {
      return true;
    }

   bool FunctionValue::sort (const StringValue&) const
    {
      return true;
    }

   bool FunctionValue::sort (const ArrayValue&) const
    {
      return true;
    }

   bool FunctionValue::sort (const DictionaryValue&) const
    {
      return true;
    }

   bool FunctionValue::sort (const FunctionValue& lhs) const
    {
      bool is_less = false;
      if (lhs.value.get() == value.get())
       {
         for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter1 = lhs.captures.begin(),
            iter2 = captures.begin(); lhs.captures.end() != iter1; ++iter1, ++iter2)
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
         is_less = std::less<void*>()(lhs.value.get(), value.get());
       }
      return is_less;
    }

   bool FunctionValue::sort (const NilValue&) const
    {
      return false;
    }

   bool FunctionValue::sort (const CellRefValue&) const
    {
      return false;
    }

   bool FunctionValue::sort (const CellRangeValue&) const
    {
      return false;
    }

   size_t FunctionValue::hash() const
    {
      size_t result = std::hash<void*>()(value.get());
      for (std::vector<std::shared_ptr<ValueType> >::const_iterator iter = captures.begin();
         captures.end() != iter; ++iter)
       {
         boost_hash_combine(result, (*iter)->hash());
       }
      return result;
    }

 } // namespace Types

 } // namespace Backwards
