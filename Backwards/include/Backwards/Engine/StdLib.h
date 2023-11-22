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
#ifndef BACKWARDS_ENGINE_STDLIB_H
#define BACKWARDS_ENGINE_STDLIB_H

#include "Backwards/Types/ValueType.h"

namespace Backwards
 {

namespace Engine
 {

   class CallingContext;

#define STDLIB_CONSTANT_DECL(x) \
   std::shared_ptr<Types::ValueType> x (void)

   STDLIB_CONSTANT_DECL(NaN);
   STDLIB_CONSTANT_DECL(NewArray);
   STDLIB_CONSTANT_DECL(NewDictionary);
   STDLIB_CONSTANT_DECL(GetRoundMode);
   STDLIB_CONSTANT_DECL(GetDefaultPrecision);

#define STDLIB_CONSTANT_DECL_WITH_CONTEXT(x) \
   std::shared_ptr<Types::ValueType> x (CallingContext& context)

   STDLIB_CONSTANT_DECL_WITH_CONTEXT(EnterDebugger);

#define STDLIB_UNARY_DECL(x) \
   std::shared_ptr<Types::ValueType> x (const std::shared_ptr<Types::ValueType>& arg)

   STDLIB_UNARY_DECL(Sqr);
   STDLIB_UNARY_DECL(Abs);
   STDLIB_UNARY_DECL(Round);
   STDLIB_UNARY_DECL(Floor);
   STDLIB_UNARY_DECL(Ceil);
   STDLIB_UNARY_DECL(ToString);
   STDLIB_UNARY_DECL(Length);
   STDLIB_UNARY_DECL(IsNaN);
   STDLIB_UNARY_DECL(IsInfinity);
   STDLIB_UNARY_DECL(ValueOf);
   STDLIB_UNARY_DECL(ToCharacter);
   STDLIB_UNARY_DECL(FromCharacter);
   STDLIB_UNARY_DECL(IsFloat);
   STDLIB_UNARY_DECL(IsString);
   STDLIB_UNARY_DECL(IsArray);
   STDLIB_UNARY_DECL(IsDictionary);
   STDLIB_UNARY_DECL(IsFunction);
   STDLIB_UNARY_DECL(IsNil);
   STDLIB_UNARY_DECL(IsCellRef);
   STDLIB_UNARY_DECL(IsCellRange);
   STDLIB_UNARY_DECL(Size);
   STDLIB_UNARY_DECL(PopFront);
   STDLIB_UNARY_DECL(PopBack);
   STDLIB_UNARY_DECL(GetKeys);
   STDLIB_UNARY_DECL(SetRoundMode);
   STDLIB_UNARY_DECL(SetDefaultPrecision);
   STDLIB_UNARY_DECL(GetPrecision);

#define STDLIB_UNARY_DECL_WITH_CONTEXT(x) \
   std::shared_ptr<Types::ValueType> x (CallingContext& context, const std::shared_ptr<Types::ValueType>& arg)

   STDLIB_UNARY_DECL_WITH_CONTEXT(Fatal);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Error);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Warn);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Info);
   STDLIB_UNARY_DECL_WITH_CONTEXT(DebugPrint);
   STDLIB_UNARY_DECL_WITH_CONTEXT(Eval);

   STDLIB_UNARY_DECL_WITH_CONTEXT(EvalCell);
   STDLIB_UNARY_DECL_WITH_CONTEXT(ExpandRange);

#define STDLIB_BINARY_DECL(x) \
   std::shared_ptr<Types::ValueType> x (const std::shared_ptr<Types::ValueType>& first, const std::shared_ptr<Types::ValueType>& second)

   STDLIB_BINARY_DECL(Min);
   STDLIB_BINARY_DECL(Max);
   STDLIB_BINARY_DECL(GetIndex);
   STDLIB_BINARY_DECL(NewArrayDefault); // size, default_value
   STDLIB_BINARY_DECL(PushBack);
   STDLIB_BINARY_DECL(PushFront);
   STDLIB_BINARY_DECL(ContainsKey);
   STDLIB_BINARY_DECL(RemoveKey);
   STDLIB_BINARY_DECL(GetValue);
   STDLIB_BINARY_DECL(SetPrecision); // number, precision

#define STDLIB_TERNARY_DECL(x) \
   std::shared_ptr<Types::ValueType> x \
      (const std::shared_ptr<Types::ValueType>& first, const std::shared_ptr<Types::ValueType>& second, const std::shared_ptr<Types::ValueType>& third)

   STDLIB_TERNARY_DECL(SubString);
   STDLIB_TERNARY_DECL(SetIndex);
   STDLIB_TERNARY_DECL(Insert);

 } // namespace Engine

 } // namespace Backwards

#endif /* BACKWARDS_ENGINE_STDLIB_H */
