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

   std::shared_ptr<ValueType> ValueType::neg() const
    {
      throw TypedOperationException("Error negating " + getTypeName());
    }

   bool ValueType::logical() const
    {
      throw TypedOperationException("Error converting " + getTypeName() + " to a boolean value.");
    }

#define DEFINE1(x,y,z) \
   std::shared_ptr<ValueType> ValueType::x (const FloatValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const StringValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const ArrayValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const DictionaryValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const FunctionValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const NilValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const CellRefValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); } \
   std::shared_ptr<ValueType> ValueType::x (const CellRangeValue& lhs) const \
      { throw TypedOperationException("Error " y " " + lhs.getTypeName() + " " z " " + getTypeName()); }

#define DEFINEBOOL(x,y) \
   bool ValueType::x (const FloatValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const StringValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const ArrayValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const DictionaryValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const FunctionValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const NilValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const CellRefValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); } \
   bool ValueType::x (const CellRangeValue& lhs) const \
      { throw TypedOperationException("Error comparing " + lhs.getTypeName() + " to " + getTypeName() + " (" y ")"); }

   DEFINE1(add, "adding", "to")
   DEFINE1(sub, "subtracting", "from")
   DEFINE1(mul, "multiplying", "by")
   DEFINE1(div, "dividing", "by")

   DEFINEBOOL(greater, ">")
   DEFINEBOOL(less, "<")
   DEFINEBOOL(geq, ">=")
   DEFINEBOOL(leq, "<=")
   DEFINEBOOL(equal, "=")
   DEFINEBOOL(notEqual, "<>")

   bool ValueType::compare (const ValueType& rhs) const
    {
      if (typeid(*this) != typeid(rhs))
       {
         return false;
       }
      return equal(rhs);
    }

 } // namespace Types

 } // namespace DCTL
