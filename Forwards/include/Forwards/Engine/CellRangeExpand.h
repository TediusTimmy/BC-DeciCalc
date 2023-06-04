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
#ifndef FORWARDS_ENGINE_CELLRANGEEXPAND_H
#define FORWARDS_ENGINE_CELLRANGEEXPAND_H

#include "Backwards/Engine/CellRangeExpand.h"
#include "Forwards/Types/CellRangeValue.h"

namespace Forwards
 {

namespace Engine
 {

   class CellRangeExpand final : public Backwards::Engine::CellRangeExpand
    {
   public:
      std::shared_ptr<Types::CellRangeValue> value;

      CellRangeExpand();
      CellRangeExpand(const std::shared_ptr<Types::CellRangeValue>&);

      virtual std::shared_ptr<Backwards::Types::ValueType> expand (Backwards::Engine::CallingContext&) const;

      virtual bool equal (const Backwards::Types::CellRangeValue& lhs) const;
      virtual bool notEqual (const Backwards::Types::CellRangeValue& lhs) const;
      virtual bool sort (const Backwards::Types::CellRangeValue& lhs) const;
      virtual size_t hash() const;
    };

 } // namespace Engine

 } // namespace Forwards

#endif /* FORWARDS_ENGINE_CELLRANGEEXPAND_H */
