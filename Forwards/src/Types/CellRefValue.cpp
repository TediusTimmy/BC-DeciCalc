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
#include "Forwards/Types/CellRefValue.h"

#define MAX_COL 18278U
#define MAX_ROW 999999999U

namespace Forwards
 {

namespace Types
 {

   CellRefValue::CellRefValue() : colAbsolute(false), colRef(0), rowAbsolute(false), rowRef(0)
    {
    }

   CellRefValue::CellRefValue(bool colAbsolute, int64_t colRef, bool rowAbsolute, int64_t rowRef) : colAbsolute(colAbsolute), colRef(colRef), rowAbsolute(rowAbsolute), rowRef(rowRef)
    {
    }

   const std::string& CellRefValue::getTypeName() const
    {
      static const std::string name ("CellRef");
      return name;
    }

   std::string CellRefValue::toString(size_t column, size_t row) const
    {
      size_t finalCol = getColumn(column, colRef);
      size_t finalRow = getRow(row, rowRef);

      if (colAbsolute && rowAbsolute)
       {
         return "$" + columnToString(colRef) + "$" + std::to_string(rowRef + 1);
       }
      else if (colAbsolute)
       {
         return "$" + columnToString(colRef) + std::to_string(static_cast<size_t>(finalRow + 1));
       }
      else if (rowAbsolute)
       {
         return columnToString(static_cast<size_t>(finalCol)) + "$" + std::to_string(rowRef + 1);
       }
      else
       {
         return columnToString(static_cast<size_t>(finalCol)) + std::to_string(static_cast<size_t>(finalRow + 1));
       }
    }

   size_t CellRefValue::getColumn(size_t fromColumn, int64_t offset)
    {
      size_t finalCol = fromColumn + offset;
      if ((offset < 0) && (static_cast<size_t>(-offset) > fromColumn))
       {
         finalCol = fromColumn + offset + MAX_COL;
       }
      else if (fromColumn + offset >= MAX_COL)
       {
         finalCol = fromColumn + offset - MAX_COL;
       }
      return finalCol;
    }

   size_t CellRefValue::getRow(size_t fromRow, int64_t offset)
    {
      size_t finalRow = fromRow + offset;
      if ((offset < 0) && (static_cast<size_t>(-offset) > fromRow))
       {
         finalRow = fromRow + offset + MAX_ROW;
       }
      else if (fromRow + offset >= MAX_ROW)
       {
         finalRow = fromRow + offset - MAX_ROW;
       }
      return finalRow;
    }

   std::string ValueType::columnToString(size_t column)
    {
      std::string temp;
      if (column < 26)
       {
         return temp + static_cast<char>('A' + column);
       }
      else if (column < (26 + 26 * 26))
       {
         size_t temp2 = column - 26;
         char A = 'A' + temp2 / 26;
         char B = 'A' + temp2 % 26;
         return temp + A + B;
       }
      else
       {
         size_t temp2 = column - (26 + 26 * 26);
         char A = 'A' + temp2 / (26 * 26);
         char B = 'A' + (temp2 / 26) % 26;
         char C = 'A' + temp2 % 26;
         return temp + A + B + C;
       }
    }

   ValueTypes CellRefValue::getType() const
    {
      return CELL_REF;
    }

 } // namespace Types

 } // namespace Forwards
