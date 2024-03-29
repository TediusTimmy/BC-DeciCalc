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
#ifndef SCREEN_H
#define SCREEN_H

enum MODE
 {
   CELL_MODIFICATION,
   GOTO_CELL
 };

class SharedData final
 {
public:
   size_t c_col;
   size_t c_row;
   size_t tr_col;
   size_t tr_row;

   bool inputMode;
   bool insertMode;
   size_t baseChar;
   size_t editChar;
   bool useComma;

   std::string tempString;
   std::string origString;
   MODE mode;

   std::deque<int> inputBuffer;

   size_t def_col_width;
   std::vector<int> col_widths;

   Forwards::Engine::CellType yankedType;
   std::shared_ptr<Forwards::Engine::Expression> yanked;

   Forwards::Engine::CallingContext* context;

   bool saveRequested;
 };

void InitScreen(SharedData&);
void UpdateScreen(SharedData&);
int ProcessInput(SharedData&);
void WaitToSave(void);
void DestroyScreen(void);

#endif /* SCREEN_H */
