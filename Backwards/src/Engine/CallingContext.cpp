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
#include "Backwards/Engine/CallingContext.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/StackFrame.h"

namespace Backwards
 {

namespace Engine
 {

   CallingContext::CallingContext() : logger(nullptr), debugger(nullptr), currentFrame(nullptr), globalScope(nullptr)
    {
    }

   CallingContext::~CallingContext()
    {
    }

   void CallingContext::pushContext(StackFrame* newFrame)
    {
      newFrame->prev = currentFrame;
      if (nullptr != currentFrame)
       {
         currentFrame->next = newFrame;
       }
      currentFrame = newFrame;
    }

   void CallingContext::popContext()
    {
      currentFrame = currentFrame->prev;
      if (nullptr != currentFrame)
       {
         currentFrame->next = nullptr;
       }
    }

   Scope* CallingContext::topScope()
    {
      if (false == scopes.empty())
       {
         return scopes.back();
       }
      return nullptr;
    }

   void CallingContext::pushScope(Scope* scope)
    {
      scopes.emplace_back(scope);
    }

   void CallingContext::popScope()
    {
      scopes.pop_back();
    }

   std::shared_ptr<CallingContext> CallingContext::duplicate()
    {
      std::shared_ptr<CallingContext> result = std::make_shared<CallingContext>();
      duplicate(result);
      return result;
    }

   void CallingContext::duplicate(std::shared_ptr<CallingContext> result)
    {
      result->logger = logger;
      result->debugger = nullptr; // Prevent Debugger-ception
      result->globalScope = globalScope;
      result->pushScope(topScope());
    }

   LocalGetter::LocalGetter(size_t location) : location(location)
    {
    }

   std::shared_ptr<Types::ValueType> LocalGetter::get(CallingContext& context) const
    {
      if (nullptr == context.currentFrame->locals[location].get())
       {
         throw FatalException("Read of value before set.");
       }
      return context.currentFrame->locals[location];
    }

   ArgGetter::ArgGetter(size_t location) : location(location)
    {
    }

   std::shared_ptr<Types::ValueType> ArgGetter::get(CallingContext& context) const
    {
      return context.currentFrame->args[location];
    }

   CaptureGetter::CaptureGetter(size_t location) : location(location)
    {
    }

   std::shared_ptr<Types::ValueType> CaptureGetter::get(CallingContext& context) const
    {
      return context.currentFrame->captures[location];
    }

   LocalSetter::LocalSetter(size_t location) : location(location)
    {
    }

   void LocalSetter::set(CallingContext& context, const std::shared_ptr<Types::ValueType>& value) const
    {
      context.currentFrame->locals[location] = value;
    }

   ArgSetter::ArgSetter(size_t location) : location(location)
    {
    }

   void ArgSetter::set(CallingContext& context, const std::shared_ptr<Types::ValueType>& value) const
    {
      context.currentFrame->args[location] = value;
    }

   CaptureSetter::CaptureSetter(size_t location) : location(location)
    {
    }

   void CaptureSetter::set(CallingContext& context, const std::shared_ptr<Types::ValueType>& value) const
    {
      context.currentFrame->captures[location] = value;
    }

   GlobalGetter::GlobalGetter(size_t location) : location(location)
    {
    }

   std::shared_ptr<Types::ValueType> GlobalGetter::get(CallingContext& context) const
    {
      if (nullptr == context.globalScope->vars[location].get())
       {
         throw FatalException("Read of value before set.");
       }
      return context.globalScope->vars[location];
    }

   ScopeGetter::ScopeGetter(size_t location) : location(location)
    {
    }

   std::shared_ptr<Types::ValueType> ScopeGetter::get(CallingContext& context) const
    {
      if (nullptr == context.topScope())
       {
         throw FatalException("Read of local variable with no location.");
       }
      if (location >= context.topScope()->vars.size())
       {
         throw FatalException("Read of local variable with bad location.");
       }
      if (nullptr == context.globalScope->vars[location].get())
       {
         throw FatalException("Read of local variable before set.");
       }
      return context.topScope()->vars[location];
    }

   GlobalSetter::GlobalSetter(size_t location) : location(location)
    {
    }

   void GlobalSetter::set(CallingContext& context, const std::shared_ptr<Types::ValueType>& value) const
    {
      context.globalScope->vars[location] = value;
    }

   ScopeSetter::ScopeSetter(size_t location) : location(location)
    {
    }

   void ScopeSetter::set(CallingContext& context, const std::shared_ptr<Types::ValueType>& value) const
    {
      if (nullptr == context.topScope())
       {
         throw FatalException("Write of local variable with no location.");
       }
      if (location >= context.topScope()->vars.size())
       {
         throw FatalException("Write of local variable with bad location.");
       }
      context.topScope()->vars[location] = value;
    }

 } // namespace Engine

 } // namespace Backwards
