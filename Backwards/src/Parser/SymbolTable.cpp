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
#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Engine/ProgrammingException.h"
#include "Backwards/Engine/Scope.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/StackFrame.h" // For Getters/Setters

namespace Backwards
 {

namespace Parser
 {

   SymbolTable::SymbolTable(GetterSetter& gs, Engine::Scope& globalScope) :
      globalScope(&globalScope), gs(gs)
    {
      if (globalScope.var.end() != globalScope.var.find("PushBack"))
       {
         pushBackFun = std::make_shared<Engine::Constant>(Input::Token(), globalScope.vars[globalScope.var.find("PushBack")->second]);
       }
      if (globalScope.var.end() != globalScope.var.find("Insert"))
       {
         insertFun = std::make_shared<Engine::Constant>(Input::Token(), globalScope.vars[globalScope.var.find("Insert")->second]);
       }
      while (globalScope.var.size() > gs.globalGetters.size())
       {
         gs.globalSetters.emplace_back(std::make_shared<Engine::GlobalSetter>(gs.globalSetters.size()));
         gs.globalGetters.emplace_back(std::make_shared<Engine::GlobalGetter>(gs.globalGetters.size()));
       }
    }

   void SymbolTable::pushScope(Engine::Scope* newScope)
    {
      scopes.push_back(newScope);
      while (scopes.back()->var.size() > gs.scopeGetters.size())
       {
         gs.scopeSetters.emplace_back(std::make_shared<Engine::ScopeSetter>(gs.scopeSetters.size()));
         gs.scopeGetters.emplace_back(std::make_shared<Engine::ScopeGetter>(gs.scopeGetters.size()));
       }
    }

   void SymbolTable::popScope()
    {
      scopes.pop_back();
    }

   void SymbolTable::pushContext()
    {
      frames.emplace_back(std::make_shared<Engine::FunctionContext>());
    }

   void SymbolTable::injectContext(const std::shared_ptr<Engine::FunctionContext>& context)
    {
      frames.push_back(context);
      while (frames.back()->args.size() > gs.argsGetters.size())
       {
         gs.argsSetters.emplace_back(std::make_shared<Engine::ArgSetter>(gs.argsSetters.size()));
         gs.argsGetters.emplace_back(std::make_shared<Engine::ArgGetter>(gs.argsGetters.size()));
       }
      while (frames.back()->locals.size() > gs.localsGetters.size())
       {
         gs.localsSetters.emplace_back(std::make_shared<Engine::LocalSetter>(gs.localsSetters.size()));
         gs.localsGetters.emplace_back(std::make_shared<Engine::LocalGetter>(gs.localsGetters.size()));
       }
      while (frames.back()->captures.size() > gs.capturesGetters.size())
       {
         gs.capturesSetters.emplace_back(std::make_shared<Engine::CaptureSetter>(gs.capturesSetters.size()));
         gs.capturesGetters.emplace_back(std::make_shared<Engine::CaptureGetter>(gs.capturesGetters.size()));
       }
    }

   std::shared_ptr<Engine::FunctionContext> SymbolTable::getContext()
    {
      return frames.back();
    }

   void SymbolTable::popContext()
    {
      frames.pop_back();
    }


   void SymbolTable::addVariable(const std::string& name)
    {
      if (false == scopes.empty())
       {
         scopes.back()->var.emplace(std::make_pair(name, scopes.back()->var.size()));
         scopes.back()->names.emplace_back(name);
         scopes.back()->vars.emplace_back(std::shared_ptr<Types::ValueType>());

         // Assume that these two arrays are in sync.
         if (scopes.back()->var.size() > gs.scopeGetters.size())
          {
            gs.scopeSetters.emplace_back(std::make_shared<Engine::ScopeSetter>(gs.scopeSetters.size()));
            gs.scopeGetters.emplace_back(std::make_shared<Engine::ScopeGetter>(gs.scopeGetters.size()));
          }
       }
      else
       {
         globalScope->var.emplace(std::make_pair(name, globalScope->var.size()));
         globalScope->names.emplace_back(name);
         globalScope->vars.emplace_back(std::shared_ptr<Types::ValueType>());

         // Assume that these two arrays are in sync.
         if (globalScope->var.size() > gs.globalGetters.size())
          {
            gs.globalSetters.emplace_back(std::make_shared<Engine::GlobalSetter>(gs.globalSetters.size()));
            gs.globalGetters.emplace_back(std::make_shared<Engine::GlobalGetter>(gs.globalGetters.size()));
          }
       }
    }

   void SymbolTable::addArgument(const std::string& name)
    {
      frames.back()->args.emplace(std::make_pair(name, frames.back()->args.size()));
      frames.back()->argNames.emplace_back(name);

      // Assume that these two arrays are in sync.
      if (frames.back()->args.size() > gs.argsGetters.size())
       {
         gs.argsSetters.emplace_back(std::make_shared<Engine::ArgSetter>(gs.argsSetters.size()));
         gs.argsGetters.emplace_back(std::make_shared<Engine::ArgGetter>(gs.argsGetters.size()));
       }
    }

   void SymbolTable::addLocal(const std::string& name)
    {
      if (true == frames.empty())
       {
         return addVariable(name);
       }

      frames.back()->locals.emplace(std::make_pair(name, frames.back()->locals.size()));
      frames.back()->localNames.emplace_back(name);

      // Assume that these two arrays are in sync.
      if (frames.back()->locals.size() > gs.localsGetters.size())
       {
         gs.localsSetters.emplace_back(std::make_shared<Engine::LocalSetter>(gs.localsSetters.size()));
         gs.localsGetters.emplace_back(std::make_shared<Engine::LocalGetter>(gs.localsGetters.size()));
       }
    }

   void SymbolTable::addCapture(const std::string& name)
    {
      frames.back()->captures.emplace(std::make_pair(name, frames.back()->captures.size()));
      frames.back()->captureNames.emplace_back(name);

      // Assume that these two arrays are in sync.
      if (frames.back()->captures.size() > gs.capturesGetters.size())
       {
         gs.capturesSetters.emplace_back(std::make_shared<Engine::CaptureSetter>(gs.capturesSetters.size()));
         gs.capturesGetters.emplace_back(std::make_shared<Engine::CaptureGetter>(gs.capturesGetters.size()));
       }
    }


   std::shared_ptr<Engine::Getter> SymbolTable::getVariableGetter(const std::string& name) const
    {
      std::map<std::string, size_t>::const_iterator test;
      if (false == frames.empty())
       {
         test = frames.back()->args.find(name);
         if (frames.back()->args.end() != test)
          {
            return gs.argsGetters[test->second];
          }

         test = frames.back()->locals.find(name);
         if (frames.back()->locals.end() != test)
          {
            return gs.localsGetters[test->second];
          }

         test = frames.back()->captures.find(name);
         if (frames.back()->captures.end() != test)
          {
            return gs.capturesGetters[test->second];
          }
       }

      if (false == scopes.empty())
       {
         test = scopes.back()->var.find(name);
         if (scopes.back()->var.end() != test)
          {
            return gs.scopeGetters[test->second];
          }
       }

      test = globalScope->var.find(name);
      if (globalScope->var.end() != test)
       {
         return gs.globalGetters[test->second];
       }

      throw Engine::ProgrammingException("Request for non existent variable " + name + ".");
    }

   std::shared_ptr<Engine::Setter> SymbolTable::getVariableSetter(const std::string& name) const
    {
      std::map<std::string, size_t>::const_iterator test;
      if (false == frames.empty())
       {
         test = frames.back()->args.find(name);
         if (frames.back()->args.end() != test)
          {
            return gs.argsSetters[test->second];
          }

         test = frames.back()->locals.find(name);
         if (frames.back()->locals.end() != test)
          {
            return gs.localsSetters[test->second];
          }

          /* I do not feel bad that this does not do what people think it does. */
         test = frames.back()->captures.find(name);
         if (frames.back()->captures.end() != test)
          {
            return gs.capturesSetters[test->second];
          }
       }

      if (false == scopes.empty())
       {
         test = scopes.back()->var.find(name);
         if (scopes.back()->var.end() != test)
          {
            return gs.scopeSetters[test->second];
          }
       }

      test = globalScope->var.find(name);
      if (globalScope->var.end() != test)
       {
         return gs.globalSetters[test->second];
       }

      throw Engine::ProgrammingException("Request for non existent variable " + name + ".");
    }

   SymbolTable::IdentifierType SymbolTable::lookup (const std::string& name) const
    {
      if (false == frames.empty())
       {
         if (frames.back()->args.end() != frames.back()->args.find(name)) return LOCAL_VARIABLE;
         if (frames.back()->locals.end() != frames.back()->locals.find(name)) return LOCAL_VARIABLE;
         if (frames.back()->captures.end() != frames.back()->captures.find(name)) return LOCAL_VARIABLE;
       }
      if (activeFunctions.end() != activeFunctions.find(name)) return FUNCTION;
      if ((false == scopes.empty()) && (scopes.back()->var.end() != scopes.back()->var.find(name))) return SCOPE_VARIABLE;
      if (globalScope->var.end() != globalScope->var.find(name)) return GLOBAL_VARIABLE;
      return UNDEFINED;
    }

   std::shared_ptr<Engine::Expression> SymbolTable::buildPushBack(const Input::Token& buildToken,
      const std::shared_ptr<Engine::Expression>& lhs, const std::shared_ptr<Engine::Expression>& rhs) const
    {
      std::shared_ptr<Engine::Expression> result;
      std::vector<std::shared_ptr<Engine::Expression> > args;
      args.push_back(lhs);
      args.push_back(rhs);
      if (nullptr == pushBackFun.get())
       {
         throw Engine::ProgrammingException("Cannot resolve request for PushBack.");
       }
      return std::make_shared<Engine::FunctionCall>(buildToken, pushBackFun, args);
    }

   std::shared_ptr<Engine::Expression> SymbolTable::buildInsert(const Input::Token& buildToken,
            const std::shared_ptr<Engine::Expression>& first, const std::shared_ptr<Engine::Expression>& second,
            const std::shared_ptr<Engine::Expression>& third) const
    {
      std::shared_ptr<Engine::Expression> result;
      std::vector<std::shared_ptr<Engine::Expression> > args;
      args.push_back(first);
      args.push_back(second);
      args.push_back(third);
      if (nullptr == insertFun.get())
       {
         throw Engine::ProgrammingException("Cannot resolve request for Insert.");
       }
      return std::make_shared<Engine::FunctionCall>(buildToken, insertFun, args);
    }

   size_t SymbolTable::newLoop()
    {
      loopCounter.emplace_back("");
      return loopCounter.size();
    }

   size_t SymbolTable::currentLoop() const
    {
      return loopCounter.size();
    }

   void SymbolTable::nameLoop(const std::string& name)
    {
      loopCounter.back() = name;
      loops.emplace(std::make_pair(name, loopCounter.size()));
    }

   size_t SymbolTable::getLoop(const std::string& name) const
    {
      if (loops.end() == loops.find(name))
       {
         return 0U;
       }
      return loops.find(name)->second;
    }

   void SymbolTable::popLoop()
    {
      std::string loop = loopCounter.back();
      if ("" != loop)
       {
         loops.erase(loop);
       }
      loopCounter.pop_back();
    }

 } // namespace Parser

 } // namespace Backwards
