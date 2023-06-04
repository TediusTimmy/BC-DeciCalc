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
#ifndef BACKWARDS_ENGINE_STACKFRAME_H
#define BACKWARDS_ENGINE_STACKFRAME_H

#include "Backwards/Types/ValueType.h"
#include "Backwards/Engine/GetterSetter.h"

#include <vector>

namespace Backwards
 {

namespace Engine
 {

   class CallingContext;
   class FunctionContext;

   class StackFrame final
    {
   public:
      std::shared_ptr<FunctionContext> function;

      std::vector<std::shared_ptr<Types::ValueType> > args;
      std::vector<std::shared_ptr<Types::ValueType> > locals;
      std::vector<std::shared_ptr<Types::ValueType> > captures;

      StackFrame* prev;
      StackFrame* next;

      const Input::Token& callingToken;
      size_t depth;

      StackFrame(std::shared_ptr<FunctionContext> function, const Input::Token& callingToken, StackFrame* prev);
    };

   class LocalGetter final : public Getter
    {
   private:
      size_t location;
   public:
      LocalGetter(size_t location);
      std::shared_ptr<Types::ValueType> get(CallingContext&) const;
    };

   class LocalSetter final : public Setter
    {
   private:
      size_t location;
   public:
      LocalSetter(size_t location);
      void set(CallingContext&, const std::shared_ptr<Types::ValueType>&) const;
    };

   class ArgGetter final : public Getter
    {
   private:
      size_t location;
   public:
      ArgGetter(size_t location);
      std::shared_ptr<Types::ValueType> get(CallingContext&) const;
    };

   class ArgSetter final : public Setter
    {
   private:
      size_t location;
   public:
      ArgSetter(size_t location);
      void set(CallingContext&, const std::shared_ptr<Types::ValueType>&) const;
    };

   class CaptureGetter final : public Getter
    {
   private:
      size_t location;
   public:
      CaptureGetter(size_t location);
      std::shared_ptr<Types::ValueType> get(CallingContext&) const;
    };

   class CaptureSetter final : public Setter
    {
   private:
      size_t location;
   public:
      CaptureSetter(size_t location);
      void set(CallingContext&, const std::shared_ptr<Types::ValueType>&) const;
    };

 } // namespace Engine

 } // namespace Backwards

#endif /* BACKWARDS_ENGINE_STACKFRAME_H */
