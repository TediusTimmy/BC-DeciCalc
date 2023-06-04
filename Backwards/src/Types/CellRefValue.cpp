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

   CellRefValue::CellRefValue()
    {
    }

   CellRefValue::CellRefValue(const std::shared_ptr<CellRefHolder>& value) : value(value)
    {
    }

   const std::string& CellRefValue::getTypeName() const
    {
      static const std::string name ("CellRef");
      return name;
    }

   bool CellRefValue::equal (const CellRefValue& lhs) const
    {
      return value->equal(lhs);
    }

   bool CellRefValue::notEqual (const CellRefValue& lhs) const
    {
      return value->notEqual(lhs);
    }

   IMPLEMENTVISITOR(CellRefValue)

   bool CellRefValue::sort (const FloatValue&) const
    {
      return true;
    }

   bool CellRefValue::sort (const StringValue&) const
    {
      return true;
    }

   bool CellRefValue::sort (const ArrayValue&) const
    {
      return true;
    }

   bool CellRefValue::sort (const DictionaryValue&) const
    {
      return true;
    }

   bool CellRefValue::sort (const FunctionValue&) const
    {
      return true;
    }

   bool CellRefValue::sort (const NilValue&) const
    {
      return true;
    }

   bool CellRefValue::sort (const CellRefValue& lhs) const
    {
      return value->sort(lhs);
    }

   bool CellRefValue::sort (const CellRangeValue&) const
    {
      return false;
    }

   size_t CellRefValue::hash() const
    {
      return value->hash();
    }

 } // namespace Types

 } // namespace Backwards
