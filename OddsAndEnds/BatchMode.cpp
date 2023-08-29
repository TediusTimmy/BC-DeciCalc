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
#include <list>

#include "Backwards/Input/StringInput.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Expression.h"
#include "Forwards/Parser/Parser.h"

int ReadBatches (int argc, char ** argv, int libEnd, std::list<std::string>& batches)
 {
   int i = libEnd;
   while (i < argc)
    {
      if (std::string("-b") == argv[i])
       {
         ++i;
         if (i < argc)
          {
            batches.push_back(argv[i]);
          }
         ++i;
       }
      else
       {
         break;
       }
    }

   return i;
 }

void dumpLog(Backwards::Engine::Logger& logger); // From LibraryLoader

void RunBatches (const std::list<std::string>& batches, Forwards::Engine::CallingContext& context)
 {
   for (const std::string& batch : batches)
    {
      Forwards::Engine::CellFrame newFrame (nullptr, 0U, 0U);
      std::shared_ptr<Forwards::Engine::Expression> value;

      Backwards::Input::StringInput interlinked (batch);
      Forwards::Input::Lexer lexer (interlinked);
      value = Forwards::Parser::Parser::ParseFullExpression(lexer, *context.map, *context.logger, 0U, 0U);

      if (nullptr == value.get())
       {
         std::cerr << "Error processing batch: " << batch << std::endl;
         dumpLog(*context.logger);
         continue;
       }

      std::shared_ptr<Forwards::Types::ValueType> result;
      try
       {
         context.pushCell(&newFrame);
         result = value->evaluate(context);
         context.popCell();
       }
      catch (const std::exception& e)
       {
         std::cerr << "Error processing batch: " << batch << std::endl;
         std::cerr << "Exception thrown: " << e.what() << std::endl;
         context.popCell();
       }
      catch (...)
       {
         std::cerr << "Error processing batch: " << batch << std::endl;
         std::cerr << "Unknown exception thrown." << std::endl;
         context.popCell();
       }

      if (nullptr != result.get())
       {
         std::cout << result->toString(0U, 0U) << std::endl;
       }

      dumpLog(*context.logger);
    }
 }
