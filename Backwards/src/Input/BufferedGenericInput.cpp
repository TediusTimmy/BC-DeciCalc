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
#include "Backwards/Input/BufferedGenericInput.h"

namespace Backwards
 {

namespace Input
 {

   BufferedGenericInput::BufferedGenericInput (GenericInput& input) :
      input(input), buffer(), endOfFile(false)
    {
    }

   void BufferedGenericInput::fill (int count)
    {
      if (true == endOfFile)
       {
         return;
       }

      for (int i = 0; i < count; ++i)
       {
         int it = input.getNextCharacter();

         if (ENDOFFILE == it)
          {
            endOfFile = true;
            return;
          }
         else
          {
            buffer.push_back(it);
          }
       }
    }

   int BufferedGenericInput::peek (size_t lookahead)
    {
      // Do we have the data already?
      if (buffer.size() > lookahead)
       {
         return buffer[lookahead];
       }

      // Try to add the amount of needed data
      fill(lookahead - buffer.size() + 1);

      // Did we get enough input?
      if (buffer.size() > lookahead)
       {
         return buffer[lookahead];
       }

      // We must be at the End of the File.
      return ENDOFFILE;
    }

   int BufferedGenericInput::consume ()
    {
      // Do we have the data already?
      if (false == buffer.empty())
       {
         int front = buffer.front();
         buffer.pop_front();
         return front;
       }

      // Try to add a character
      fill(1);

      // Did we get new input?
      if (false == buffer.empty())
       {
         int front = buffer.front();
         buffer.pop_front();
         return front;
       }

      // We must be at the End of the File.
      return ENDOFFILE;
    }

 } // namespace Input

 } // namespace Backwards
