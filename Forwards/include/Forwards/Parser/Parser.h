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
#ifndef FORWARDS_PARSER_PARSER_H
#define FORWARDS_PARSER_PARSER_H

#include "Forwards/Input/Lexer.h"

#include <memory>
#include <map>

namespace Backwards
 {

namespace Engine
 {
   class Logger;
   class Getter;
 }

 }

namespace Forwards
 {

namespace Engine
 {
   class Expression;
   typedef std::map<std::string, std::shared_ptr<Backwards::Engine::Getter> > GetterMap;
 }

namespace Parser
 {

   class Parser /* Syntax Analyzer */ /* Analyser for the Bri'ish */ final
    {
   public:

      static std::shared_ptr<Engine::Expression> ParseFullExpression (Input::Lexer& src, const Engine::GetterMap&, Backwards::Engine::Logger&, size_t, size_t);

   private:

      static void expect (Input::Lexer& src, Input::Lexeme expected, const std::string& name);

      static std::shared_ptr<Engine::Expression> expression (Input::Lexer& src, const Engine::GetterMap&, Backwards::Engine::Logger&, size_t, size_t);
      static std::shared_ptr<Engine::Expression> simple (Input::Lexer& src, const Engine::GetterMap&, Backwards::Engine::Logger&, size_t, size_t);
      static std::shared_ptr<Engine::Expression> term (Input::Lexer& src, const Engine::GetterMap&, Backwards::Engine::Logger&, size_t, size_t);
      static std::shared_ptr<Engine::Expression> unary (Input::Lexer& src, const Engine::GetterMap&, Backwards::Engine::Logger&, size_t, size_t);
      static std::shared_ptr<Engine::Expression> primary (Input::Lexer& src, const Engine::GetterMap&, Backwards::Engine::Logger&, size_t, size_t);

      static std::shared_ptr<Engine::Expression> cellref (const Input::Token&, size_t, size_t);
    };

 } // namespace Parser

 } // namespace Forwards

#endif /* FORWARDS_PARSER_PARSER_H */
