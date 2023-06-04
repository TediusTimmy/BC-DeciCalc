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

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/LineBufferedStreamInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"
#include "Backwards/Parser/ContextBuilder.h"

#include "Backwards/Engine/Statement.h"
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
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/ConstantsSingleton.h"

class ConsoleLogger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string& message) { std::cout << message << std::endl; }
   std::string get () { std::string result; std::getline(std::cin, result); return result; }
 };

std::shared_ptr<Backwards::Types::ValueType> printValue(const std::shared_ptr<Backwards::Types::ValueType>& val)
 {
   Backwards::Engine::DefaultDebugger::printValue(std::cout, val);
   std::cout << std::endl;
   return Backwards::Engine::ConstantsSingleton::getInstance().FLOAT_ZERO;
 }

int main (int argc, char ** argv)
 {
   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);

   Backwards::Engine::CallingContext context;

   std::dynamic_pointer_cast<Backwards::Engine::FunctionContext>(std::dynamic_pointer_cast<Backwards::Types::FunctionValue>(global.vars[global.var["DebugPrint"]])->value)->function =
      std::make_shared<Backwards::Engine::StandardUnaryFunction>(printValue);

   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;
   Backwards::Engine::DefaultDebugger debugger;
   context.debugger = &debugger;

   std::shared_ptr<Backwards::Engine::FunctionContext> nil = std::make_shared<Backwards::Engine::FunctionContext>();
   nil->name = "main";
   Backwards::Engine::StackFrame frame (nil, Backwards::Input::Token(), nullptr);
   context.pushContext(&frame);

   // 0 is the invoking program.
   for (int i = 1; i < argc; ++i)
    {
      Backwards::Input::FileInput file (argv[i]);
      Backwards::Input::Lexer lexer (file, argv[i]);

      std::shared_ptr<Backwards::Engine::Statement> res = Backwards::Parser::Parser::ParseFunctions(lexer, table, logger);
      if (nullptr == res.get())
       {
         std::cerr << "Error processing file: " << argv[i] << std::endl << "Exiting...." << std::endl;
         return 1;
       }
      try
       {
         res->execute(context);
       }
      catch (const Backwards::Types::TypedOperationException& e)
       {
         std::cerr << "Caught runtime exception: " << e.what() << std::endl;
         std::cerr << "Error processing file: " << argv[i] << std::endl << "Exiting...." << std::endl;
         return 1;
       }
      catch (const Backwards::Engine::FatalException& e)
       {
         std::cerr << "Caught Fatal Error: " << e.what() << std::endl;
         std::cerr << "Error processing file: " << argv[i] << std::endl << "Exiting...." << std::endl;
         return 1;
       }
    }

   Backwards::Input::ConsoleInput console;
   Backwards::Input::Lexer lexer (console, "Console");

   while (lexer.peekNextToken().lexeme != Backwards::Input::END_OF_FILE)
    {
      try
       {
         std::shared_ptr<Backwards::Engine::Statement> res = Backwards::Parser::Parser::ParseStatement(lexer, table, logger);

         if (nullptr != res.get())
          {
            try
             {
               res->execute(context);
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
            Backwards::Parser::Parser::recoverStatement(lexer);
          }

         lexer.getNextToken();
       }
      catch (std::exception& e) // Parser Exceptions should never leave the Parser.
       {
         std::cerr << e.what() << std::endl;
         Backwards::Parser::Parser::recoverStatement(lexer);
       }
    }

   return 0;
 }
