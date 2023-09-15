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
#ifndef BACKWARDS_TYPES_FUNCTIONVALUE_H
#define BACKWARDS_TYPES_FUNCTIONVALUE_H

#include "Backwards/Types/ValueType.h"

namespace Backwards
 {

namespace Types
 {

    /*
      I don't want Types dependent on Engine, but a function is a pointer to a Statement.
      This shenanigan hides that dependency. I will also try to ensure a 1:1 map between
      FunctionValues and FunctionObjectHolders.
    */
   class FunctionObjectHolder
    {
   public:
      FunctionObjectHolder() = default;
      virtual ~FunctionObjectHolder() = default;
    };

   class FunctionValue final : public ValueType
    {

   public:
      std::shared_ptr<FunctionObjectHolder> value;
      std::weak_ptr<FunctionObjectHolder> valueToo;
      std::vector<std::shared_ptr<ValueType> > captures;

      FunctionValue();
      FunctionValue(const std::shared_ptr<FunctionObjectHolder>& value, const std::vector<std::shared_ptr<ValueType> >& captures);
      FunctionValue(const std::vector<std::shared_ptr<ValueType> >& captures, const std::weak_ptr<FunctionObjectHolder>& value);
      FunctionValue(const FunctionValue&) = delete;

      FunctionValue& operator=(const FunctionValue&) = delete;

      const std::string& getTypeName() const override;

      bool equal (const FunctionValue& lhs) const override;
      bool notEqual (const FunctionValue& lhs) const override;

      DECLAREVISITOR

    };

 } // namespace Types

 } // namespace Backwards

#endif /* BACKWARDS_TYPES_FUNCTIONVALUE_H */
