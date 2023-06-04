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
#ifndef FORWARDS_ENGINE_CALLINGCONTEXT_H
#define FORWARDS_ENGINE_CALLINGCONTEXT_H

#include "Backwards/Engine/CallingContext.h"

#include <vector>

namespace Forwards
 {

namespace Engine
 {

   class Cell;
   class SpreadSheet;
   typedef std::map<std::string, std::shared_ptr<Backwards::Engine::Getter> > GetterMap;

   class CellFrame
    {
   public:
      CellFrame(Cell* cell, size_t col, size_t row) : cell(cell), col(col), row(row) { }

      Cell* cell;
      size_t col;
      size_t row;
    };

   class CallingContext : public Backwards::Engine::CallingContext
    {
   public:
      CallingContext();

      bool inUserInput;
      size_t generation;
      SpreadSheet* theSheet;
      GetterMap* map;

      CellFrame* topCell();
      void pushCell(CellFrame* cell);
      void popCell();

      virtual std::shared_ptr<Backwards::Engine::CallingContext> duplicate(); // This function exists for the debugger.

   private:
      std::vector<CellFrame*> cells;

   protected:
      virtual void duplicate(std::shared_ptr<CallingContext>);
    };

 } // namespace Engine

 } // namespace Forwards

#endif /* FORWARDS_ENGINE_CALLINGCONTEXT_H */
