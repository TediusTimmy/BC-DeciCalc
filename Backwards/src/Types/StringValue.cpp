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

   StringValue::StringValue() : value()
    {
    }

   StringValue::StringValue(const std::string& value) : value(value)
    {
    }

   const std::string& StringValue::getTypeName() const
    {
      static const std::string name ("String");
      return name;
    }

   std::shared_ptr<ValueType> StringValue::add (const StringValue& lhs) const
    {
      return std::make_shared<StringValue>(lhs.value + value);
    }

   bool StringValue::greater (const StringValue& lhs) const
    {
      return lhs.value > value;
    }

   bool StringValue::less (const StringValue& lhs) const
    {
      return lhs.value < value;
    }

   bool StringValue::geq (const StringValue& lhs) const
    {
      return lhs.value >= value;
    }

   bool StringValue::leq (const StringValue& lhs) const
    {
      return lhs.value <= value;
    }

   bool StringValue::equal (const StringValue& lhs) const
    {
      return lhs.value == value;
    }

   bool StringValue::notEqual (const StringValue& lhs) const
    {
      return lhs.value != value;
    }

   IMPLEMENTVISITOR(StringValue)

   bool StringValue::sort (const FloatValue&) const
    {
      return true;
    }

   bool StringValue::sort (const StringValue& lhs) const
    {
      return lhs.value < value;
    }

   bool StringValue::sort (const ArrayValue&) const
    {
      return false;
    }

   bool StringValue::sort (const DictionaryValue&) const
    {
      return false;
    }

   bool StringValue::sort (const FunctionValue&) const
    {
      return false;
    }

   bool StringValue::sort (const NilValue&) const
    {
      return false;
    }

   bool StringValue::sort (const CellRefValue&) const
    {
      return false;
    }

   bool StringValue::sort (const CellRangeValue&) const
    {
      return false;
    }

   size_t StringValue::hash() const
    {
      return std::hash<std::string>()(value);
    }

 } // namespace Types

 } // namespace Backwards
