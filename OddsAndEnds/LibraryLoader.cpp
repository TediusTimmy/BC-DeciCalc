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
#include <functional>
#include <algorithm>

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
#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

#include "StdLib.h"

static void dumpLog(Backwards::Engine::Logger& logger)
 {
   try
    {
      Forwards::Parser::StringLogger& realLogger = dynamic_cast<Forwards::Parser::StringLogger&>(logger);
      if (false == realLogger.logs.empty())
       {
         std::cerr << "These messages were logged:" << std::endl;
         for (const auto& bob : realLogger.logs)
          {
            std::cerr << bob << std::endl;
          }
         realLogger.logs.clear();
       }
    }
   catch (const std::bad_cast&)
    {
      std::cerr << "There is a bug and the logger isn't the right type." << std::endl;
    }
 }

int LoadLibraries (int argc, char ** argv, Forwards::Engine::CallingContext& context)
 {
   Backwards::Parser::ContextBuilder::createGlobalScope(*context.globalScope); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, *context.globalScope);

         // We assume that this cannot fail.
    {
      Backwards::Input::StringInput stdlib (STDLIB);
      Backwards::Input::Lexer lexer (stdlib, "Standard Library");
      std::shared_ptr<Backwards::Engine::Statement> res = Backwards::Parser::Parser::ParseFunctions(lexer, table, *context.logger);
      res->execute(context);
    }

   int i = 1;
   if (argc > 1)
    {
      while (i < argc)
       {
         if (std::string("-l") == argv[i])
          {
            ++i;
            if (i < argc)
             {
               Backwards::Input::FileInput console (argv[i]);
               Backwards::Input::Lexer lexer (console, argv[i]);

               std::shared_ptr<Backwards::Engine::Statement> res = Backwards::Parser::Parser::ParseFunctions(lexer, table, *context.logger);
               if (nullptr == res.get())
                {
                  std::cerr << "Error processing file: " << argv[i] << std::endl;
                  dumpLog(*context.logger);
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
                     dumpLog(*context.logger);
                   }
                  catch (const Backwards::Engine::FatalException& e)
                   {
                     std::cerr << "Caught Fatal Error: " << e.what() << std::endl;
                     std::cerr << "Error processing file: " << argv[i] << std::endl;
                     dumpLog(*context.logger);
                   }
                }
             }
            ++i;
          }
         else
          {
            break;
          }
       }
    }

   for (const std::string& name : context.globalScope->names)
    {
      std::string temp = name;
      std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){ return std::toupper(c); });
      if (name == temp)
       {
         context.map->insert(std::make_pair(name, table.getVariableGetter(name)));
       }
    }

   return i;
 }
