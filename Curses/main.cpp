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

#include "Backwards/Engine/Logger.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

#include "Forwards/Types/ValueType.h"

#include "BatchMode.h"
#include "GetAndSet.h"
#include "LibraryLoader.h"
#include "SaveFile.h"

#include "Screen.h"

int main (int argc, char ** argv)
 {
   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   context.globalScope = &global;
   Forwards::Parser::StringLogger logger;
   context.logger = &logger;
   Forwards::Engine::SpreadSheet sheet;
   context.theSheet = &sheet;
   Forwards::Engine::GetterMap map;
   context.map = &map;
   Forwards::Engine::NameMap names;
   context.names = &names;

   std::list<std::string> batches;
   std::vector<std::pair<std::string, std::string> > argLibs;
   std::vector<std::pair<std::string, std::string> > fileLibs;

   int file = PreLoadLibraries(argc, argv, argLibs);
   file = ReadBatches(argc, argv, file, batches);


   SharedData state;

   state.c_row = 0U;
   state.c_col = 0U;
   state.tr_row = 0U;
   state.tr_col = 0U;

   state.inputMode = false;
   state.insertMode = true;
   state.useComma = false;

   state.def_col_width = DEF_COLUMN_WIDTH;

   state.yankedType = Forwards::Engine::ERROR;

   state.context = &context;

   state.saveRequested = false;


   std::string saveFileName = "untitled.html";
   if (file < argc)
    {
      if ((file + 1) < argc)
       {
         saveFileName = argv[file + 1];
       }
      else
       {
         saveFileName = argv[file];
       }

      LoadFile(argv[file], &sheet, state.col_widths, state.def_col_width, fileLibs);
    }


   fileLibs.insert(fileLibs.end(), argLibs.begin(), argLibs.end());
   LoadLibraries(fileLibs, context);


   if (false == batches.empty())
    {
      if (0U != sheet.max_row) // We loaded saved data, so recalculate the sheet.
       {
         sheet.recalc(context);
       }
      RunBatches(batches, context);
      return 0;
    }


   InitScreen(state);
   UpdateScreen(state);
   while (ProcessInput(state))
    {
      UpdateScreen(state);
      if (true == state.saveRequested)
       {
         SaveFile(saveFileName, &sheet, state.col_widths, state.def_col_width, fileLibs);
         state.saveRequested = false;
       }
    }
   DestroyScreen();

   if (true == state.saveRequested)
    {
      WaitToSave();
      SaveFile(saveFileName, &sheet, state.col_widths, state.def_col_width, fileLibs);
    }

   if (false == logger.logs.empty())
    {
      std::cerr << "These messages were logged:" << std::endl;
      for (const auto& bob : logger.logs)
       {
         std::cerr << bob << std::endl;
       }
      logger.logs.clear();
    }

   return 0;
 }
