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
#ifndef FORWARDS_TYPES_VALUETYPE_H
#define FORWARDS_TYPES_VALUETYPE_H

#include <string>
#include <cstdint>

namespace Forwards
 {

namespace Types
 {

   inline void boost_hash_combine(size_t& seed, size_t value)
    {
      seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

   enum ValueTypes
    {
      FLOAT,
      STRING,
      NIL,
      CELL_REF,
      CELL_RANGE
    };

   class ValueType
    {

   public:
      virtual ~ValueType() = default;

      virtual const std::string& getTypeName() const = 0;
      virtual std::string toString(size_t column, size_t row) const = 0;
      virtual ValueTypes getType() const = 0;

      static std::string columnToString(size_t column);
    };

 } // namespace Types

 } // namespace Forwards

#endif /* FORWARDS_TYPES_VALUETYPE_H */
