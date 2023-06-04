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

   int file = LoadLibraries(argc, argv, context);

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

      LoadFile(argv[file], &sheet);
    }

   SharedData state;


   state.c_row = 0U;
   state.c_col = 0U;
   state.tr_row = 0U;
   state.tr_col = 0U;

   state.inputMode = false;
   state.insertMode = true;
   state.useComma = false;

   state.def_col_width = 9;

   state.yankedType = Forwards::Engine::ERROR;

   state.context = &context;

   state.saveRequested = false;


   if (0U != sheet.max_row) // We loaded saved data, so recalculate the sheet.
    {
      sheet.recalc(context);
    }


   InitScreen();
   UpdateScreen(state);
   while (ProcessInput(state))
    {
      UpdateScreen(state);
      if (true == state.saveRequested)
       {
         SaveFile(saveFileName, &sheet);
         state.saveRequested = false;
       }
    }
   DestroyScreen();

   if (true == state.saveRequested)
    {
      SaveFile(saveFileName, &sheet);
      state.saveRequested = false;
    }

   return 0;
 }
