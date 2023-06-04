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
#ifndef BACKWARDS_TYPES_FLOATVALUE_H
#define BACKWARDS_TYPES_FLOATVALUE_H

#include "Fixed.hpp"
#include "Backwards/Types/ValueType.h"

namespace Backwards
 {

namespace Types
 {

   class FloatValue final : public ValueType
    {

   public:
      BigInt::Fixed value;

      FloatValue();
      FloatValue(const BigInt::Fixed& value);

      const std::string& getTypeName() const;

      std::shared_ptr<ValueType> neg() const;
      bool logical() const;

      std::shared_ptr<ValueType> add (const FloatValue& lhs) const;
      std::shared_ptr<ValueType> sub (const FloatValue& lhs) const;
      std::shared_ptr<ValueType> mul (const FloatValue& lhs) const;
      std::shared_ptr<ValueType> div (const FloatValue& lhs) const;

      bool greater (const FloatValue& lhs) const;
      bool less (const FloatValue& lhs) const;
      bool geq (const FloatValue& lhs) const;
      bool leq (const FloatValue& lhs) const;
      bool equal (const FloatValue& lhs) const;
      bool notEqual (const FloatValue& lhs) const;

      DECLAREVISITOR

    };

 } // namespace Types

 } // namespace Backwards

#endif /* BACKWARDS_TYPES_FLOATVALUE_H */
