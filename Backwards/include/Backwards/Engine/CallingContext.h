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
#ifndef BACKWARDS_ENGINE_CALLINGCONTEXT_H
#define BACKWARDS_ENGINE_CALLINGCONTEXT_H

#include "Backwards/Types/ValueType.h"
#include "Backwards/Engine/GetterSetter.h"
#include "Backwards/Engine/Scope.h"

#include <vector>

namespace Backwards
 {

namespace Input
 {
   class Token;
 }

namespace Engine
 {

   class DebuggerHook;
   class Logger;
   class StackFrame;
   class Statement;

   class CallingContext
    {
   public:
      CallingContext();
      virtual ~CallingContext();

      void pushContext(StackFrame* newFrame);
      void popContext();

      Logger* logger;
      DebuggerHook* debugger;

      StackFrame* currentFrame;
      Scope* globalScope;

      Scope* topScope();
      void pushScope(Scope* scope);
      void popScope();

      virtual std::shared_ptr<CallingContext> duplicate(); // This function exists for the debugger.

   private:
      std::vector<Scope*> scopes;

   protected:
      void duplicate(std::shared_ptr<CallingContext>);
    };

   class GlobalGetter final : public Getter
    {
   private:
      size_t location;
   public:
      explicit GlobalGetter(size_t location);
      std::shared_ptr<Types::ValueType> get(CallingContext&) const override;
    };

   class GlobalSetter final : public Setter
    {
   private:
      size_t location;
   public:
      explicit GlobalSetter(size_t location);
      void set(CallingContext&, const std::shared_ptr<Types::ValueType>&) const override;
    };

   class ScopeGetter final : public Getter
    {
   private:
      size_t location;
   public:
      explicit ScopeGetter(size_t location);
      std::shared_ptr<Types::ValueType> get(CallingContext&) const override;
    };

   class ScopeSetter final : public Setter
    {
   private:
      size_t location;
   public:
      explicit ScopeSetter(size_t location);
      void set(CallingContext&, const std::shared_ptr<Types::ValueType>&) const override;
    };

   typedef std::shared_ptr<Types::ValueType> (*ConstantFunctionPointer)(void);
   typedef std::shared_ptr<Types::ValueType> (*ConstantFunctionPointerWithContext) (CallingContext&);
   typedef std::shared_ptr<Types::ValueType> (*UnaryFunctionPointer) (const std::shared_ptr<Types::ValueType>&);
   typedef std::shared_ptr<Types::ValueType> (*UnaryFunctionPointerWithContext) (CallingContext&, const std::shared_ptr<Types::ValueType>&);
   typedef std::shared_ptr<Types::ValueType> (*BinaryFunctionPointer)
      (const std::shared_ptr<Types::ValueType>&, const std::shared_ptr<Types::ValueType>&);
   typedef std::shared_ptr<Types::ValueType> (*TernaryFunctionPointer)
      (const std::shared_ptr<Types::ValueType>&, const std::shared_ptr<Types::ValueType>&, const std::shared_ptr<Types::ValueType>&);

 } // namespace Engine

 } // namespace Backwards

#endif /* BACKWARDS_ENGINE_CALLINGCONTEXT_H */
