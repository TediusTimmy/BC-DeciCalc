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
#include "Forwards/Engine/StdLib.h"

#include "Forwards/Engine/Expression.h"
#include "Forwards/Engine/CallingContext.h"

#include "Forwards/Engine/CellRefEval.h"

#include "Backwards/Types/CellRefValue.h"
#include "Backwards/Types/StringValue.h"

#include "Backwards/Engine/StackFrame.h"
#include "Backwards/Engine/DebuggerHook.h"

#include "Backwards/Engine/ProgrammingException.h"

namespace Forwards
 {

namespace Engine
 {

   STDLIB_BINARY_DECL_WITH_CONTEXT(Let)
    {
      try
       {
         CallingContext& text = dynamic_cast<CallingContext&>(context);
         if (typeid(Backwards::Types::StringValue) == typeid(*first))
          {
            if (typeid(Backwards::Types::CellRefValue) == typeid(*second))
             {
               const std::string& name = static_cast<const Backwards::Types::StringValue&>(*first).value;
               const std::shared_ptr<Backwards::Types::CellRefHolder>& expr = static_cast<const Backwards::Types::CellRefValue&>(*second).value;
               const std::shared_ptr<CellRefEval>& last = std::dynamic_pointer_cast<CellRefEval>(expr);

               if (nullptr == last.get())
                {
                  throw Backwards::Engine::ProgrammingException("Cell Reference wasn't the right Cell Reference.");
                }

               text.names->insert(std::make_pair(name, last->value));
             }
            else
             {
               throw Backwards::Types::TypedOperationException("Error creating name: target not a Cell Reference.");
             }
          }
         else
          {
            throw Backwards::Types::TypedOperationException("Error creating name: name not String.");
          }
       }
      catch (const std::bad_cast&)
       {
         throw Backwards::Engine::ProgrammingException("Backwards context wasn't a Forwards context.");
       }
      return first;
    }

   StandardBinaryFunctionWithContext::StandardBinaryFunctionWithContext(BinaryFunctionPointerWithContext function) : Backwards::Engine::Statement(Backwards::Input::Token()), function(function)
    {
    }

   std::shared_ptr<Backwards::Engine::FlowControl> StandardBinaryFunctionWithContext::execute (Backwards::Engine::CallingContext& context) const
    {
      std::shared_ptr<Backwards::Types::ValueType> lhs = context.currentFrame->args[0U];
      std::shared_ptr<Backwards::Types::ValueType> rhs = context.currentFrame->args[1U];
      try
       {
         return std::make_shared<Backwards::Engine::FlowControl>(token, Backwards::Engine::FlowControl::RETURN, Backwards::Engine::FlowControl::NO_TARGET, function(context, lhs, rhs));
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(e.what(), context);
          }
         throw;
       }
    }

 } // namespace Engine

 } // namespace Forwards
