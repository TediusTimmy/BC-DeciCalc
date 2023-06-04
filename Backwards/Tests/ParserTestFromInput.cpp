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
#include <iostream>
#include <iomanip>

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/LineBufferedStreamInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"
#include "Backwards/Parser/ContextBuilder.h"

#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/Logger.h"

#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/StackFrame.h"
#include "Backwards/Input/Token.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"

class ConsoleLogger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string& message) { std::cout << message << std::endl; }
   std::string get () { std::string result; std::getline(std::cin, result); return result; }
 };

int main (void)
 {
   Backwards::Input::ConsoleInput console;
   Backwards::Input::Lexer lexer (console, "Console");

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);

   Backwards::Engine::CallingContext context;

   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   Backwards::Engine::DefaultDebugger debugger;
   context.debugger = &debugger;

   std::shared_ptr<Backwards::Engine::FunctionContext> nil = std::make_shared<Backwards::Engine::FunctionContext>();
   nil->name = "main";
   Backwards::Engine::StackFrame frame (nil, Backwards::Input::Token(), nullptr);
   context.pushContext(&frame);

   while (lexer.peekNextToken().lexeme != Backwards::Input::END_OF_FILE)
    {
      std::shared_ptr<Backwards::Engine::Expression> res = Backwards::Parser::Parser::ParseExpression(lexer, table, logger);

      if (nullptr != res.get())
       {

         try
          {
            std::shared_ptr<Backwards::Types::ValueType> val = res->evaluate(context);

            if (nullptr != val.get())
             {
               Backwards::Engine::DefaultDebugger::printValue(std::cout, val);
               std::cout << std::endl;
             }
            else
             {
               std::cout << "Evaluate returned NULL." << std::endl;
             }
          }
         catch (const Backwards::Types::TypedOperationException& e)
          {
            std::cerr << "Caught runtime exception: " << e.what() << std::endl;
          }
         catch (const Backwards::Engine::FatalException& e)
          {
            std::cerr << "Caught Fatal Error: " << e.what() << std::endl;
          }
       }
      else
       {
         std::cerr << "Parse returned NULL." << std::endl;
         Backwards::Parser::Parser::recoverExpression(lexer);
       }

      lexer.getNextToken();
    }

   return 0;
 }
