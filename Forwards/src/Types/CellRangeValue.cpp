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
#include "Forwards/Types/ValueType.h"
#include "Forwards/Types/CellRangeValue.h"

namespace Forwards
 {

namespace Types
 {

   CellRangeValue::CellRangeValue() : col1(1U), row1(1U), col2(1U), row2(1U)
    {
    }

   CellRangeValue::CellRangeValue(size_t col1, size_t row1, size_t col2, size_t row2) : col1(col1), row1(row1), col2(col2), row2(row2)
    {
    }

   const std::string& CellRangeValue::getTypeName() const
    {
      static const std::string name ("CellRange");
      return name;
    }

      // The Range type is ephemeral, and this will only be called in an absolute sense: it isn't used to rebuild an expression.
   std::string CellRangeValue::toString(size_t, size_t) const
    {
      return columnToString(col1) + std::to_string(row1 + 1) + ":" + columnToString(col2) + std::to_string(row2 + 1);
    }

   ValueTypes CellRangeValue::getType() const
    {
      return CELL_RANGE;
    }

 } // namespace Types

 } // namespace Forwards
