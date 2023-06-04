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
#ifndef BACKWARDS_PARSER_SYMBOLTABLE_H
#define BACKWARDS_PARSER_SYMBOLTABLE_H

#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/Expression.h"
#include "Backwards/Input/Token.h"

#include <string>
#include <vector>
#include <memory>

namespace Backwards
 {

namespace Engine
 {
   class FunctionContext;
 }

namespace Parser
 {

   class GetterSetter final
    {
   public:
      std::vector<std::shared_ptr<Engine::Getter> > globalGetters;
      std::vector<std::shared_ptr<Engine::Getter> > scopeGetters;
      std::vector<std::shared_ptr<Engine::Getter> > localsGetters;
      std::vector<std::shared_ptr<Engine::Getter> > argsGetters;
      std::vector<std::shared_ptr<Engine::Getter> > capturesGetters;
      std::vector<std::shared_ptr<Engine::Setter> > globalSetters;
      std::vector<std::shared_ptr<Engine::Setter> > scopeSetters;
      std::vector<std::shared_ptr<Engine::Setter> > localsSetters;
      std::vector<std::shared_ptr<Engine::Setter> > argsSetters;
      std::vector<std::shared_ptr<Engine::Setter> > capturesSetters;
    };

   class SymbolTable final
    {
   private:
      Engine::Scope* globalScope;
      std::vector<Engine::Scope*> scopes;
      std::vector<std::shared_ptr<Engine::FunctionContext> > frames;
      std::vector<std::string> loopCounter;
      std::map<std::string, size_t> loops;
      GetterSetter& gs;

   public:
      SymbolTable(GetterSetter&, Engine::Scope&);

      void pushScope(Engine::Scope*);
      void popScope();

      void pushContext();
      void injectContext(const std::shared_ptr<Engine::FunctionContext>&);
      std::shared_ptr<Engine::FunctionContext> getContext();
      void popContext();

      void addVariable(const std::string&);
      void addArgument(const std::string&);
      void addLocal(const std::string&);
      void addCapture(const std::string&);

      std::shared_ptr<Engine::Getter> getVariableGetter(const std::string&) const;
      std::shared_ptr<Engine::Setter> getVariableSetter(const std::string&) const;

      std::shared_ptr<Engine::Expression> buildPushBack(const Input::Token&, const std::shared_ptr<Engine::Expression>&, const std::shared_ptr<Engine::Expression>&) const;
      std::shared_ptr<Engine::Expression> buildInsert(const Input::Token&,
         const std::shared_ptr<Engine::Expression>&, const std::shared_ptr<Engine::Expression>&, const std::shared_ptr<Engine::Expression>&) const;

      std::shared_ptr<Engine::Expression> pushBackFun;
      std::shared_ptr<Engine::Expression> insertFun;

      enum IdentifierType
       {
         UNDEFINED,
         GLOBAL_VARIABLE,
         SCOPE_VARIABLE,
         LOCAL_VARIABLE,
         FUNCTION
       };

      std::map<std::string, std::weak_ptr<Engine::FunctionContext> > activeFunctions;
      IdentifierType lookup (const std::string&) const;

      size_t newLoop();
      size_t currentLoop() const;
      void nameLoop(const std::string&);
      size_t getLoop(const std::string&) const;
      void popLoop();
    };

 } // namespace Parser

 } // namespace Backwards

#endif /* BACKWARDS_PARSER_SYMBOLTABLE_H */
