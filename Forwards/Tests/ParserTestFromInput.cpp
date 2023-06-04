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
#include <functional>

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/LineBufferedStreamInput.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"
#include "Backwards/Parser/ContextBuilder.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/Statement.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"

#include "Forwards/Types/ValueType.h"

class ConsoleLogger final : public Backwards::Engine::Logger
 {
public:
   void log (const std::string& message) { std::cout << message << std::endl; }
   std::string get () { std::string result; std::getline(std::cin, result); return result; }
 };

int main (int argc, char ** argv)
 {
   Forwards::Engine::GetterMap map;

   Backwards::Engine::Scope global;
   Backwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);

   Forwards::Engine::CallingContext context;

   context.globalScope = &global;
   ConsoleLogger logger;
   context.logger = &logger;

   if (argc > 1)
    {
      for (int i = 1; i < argc; ++i)
       {
         Backwards::Input::FileInput console (argv[i]);
         Backwards::Input::Lexer lexer (console, argv[i]);

         std::shared_ptr<Backwards::Engine::Statement> res = Backwards::Parser::Parser::ParseFunctions(lexer, table, logger);
         if (nullptr == res.get())
          {
            std::cerr << "Error processing file: " << argv[i] << std::endl;
          }
         else
          {
            try
             {
               res->execute(context);
             }
            catch (const Backwards::Types::TypedOperationException& e)
             {
               std::cerr << "Caught runtime exception: " << e.what() << std::endl;
               std::cerr << "Error processing file: " << argv[i] << std::endl;
             }
            catch (const Backwards::Engine::FatalException& e)
             {
               std::cerr << "Caught Fatal Error: " << e.what() << std::endl;
               std::cerr << "Error processing file: " << argv[i] << std::endl;
             }
          }
       }
    }

   for (const std::string& name : global.names)
    {
      std::string temp = name;
      std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){ return std::toupper(c); });
      if (name == temp)
       {
         map.insert(std::make_pair(name, table.getVariableGetter(name)));
       }
    }

   Forwards::Engine::SpreadSheet theSheet;
   theSheet.sheet.emplace_back(std::vector<std::unique_ptr<Forwards::Engine::Cell> >());
   theSheet.sheet[0].emplace_back(std::make_unique<Forwards::Engine::Cell>());
   context.theSheet = &theSheet;

   Forwards::Engine::CellFrame frame (theSheet.sheet[0][0].get(), 0U, 0U);
   context.pushCell(&frame);

   std::string inLine;

   std::getline(std::cin, inLine);
   while (inLine != "")
    {
      Backwards::Input::StringInput console (inLine);
      Forwards::Input::Lexer lexer (console);
      std::shared_ptr<Forwards::Engine::Expression> res = Forwards::Parser::Parser::ParseFullExpression(lexer, map, logger, 0U, 0U);

      if (nullptr != res.get())
       {

         std::cout << res->toString(0U, 0U) << std::endl;
         try
          {
            std::shared_ptr<Forwards::Types::ValueType> val = res->evaluate(context);

            if (nullptr != val.get())
             {
               std::cout << val->toString(0U, 0U) << std::endl;
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
       }

      inLine = "";
      std::getline(std::cin, inLine);
    }

   return 0;
 }
