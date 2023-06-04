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
#include "Backwards/Input/LineBufferedStreamInput.h"

#include <iostream>

namespace Backwards
 {

namespace Input
 {

   LineBufferedStreamInput::LineBufferedStreamInput(std::istream& input) : input(input), currentLine(""), index(1U)
    {
    }

   int LineBufferedStreamInput::getNextCharacter()
    {
      // Buffer a new line the next request after returning the end of line.
      if ((currentLine.length() + 1U) == index)
       {
         index = 0U;
         currentLine = "";
         std::getline(input, currentLine);
       }

      // Are we now at end of input?
      if ((true == input.eof()) && (true == currentLine.empty()))
       {
         return ENDOFFILE;
       }

      // Are we at the end of the line?
      if (currentLine.length() == index)
       {
         ++index; // Flag having given the newline
         return '\n';
       }

      // Return the next character.
      return currentLine[index++];
    }

   ConsoleInput::ConsoleInput() : LineBufferedStreamInput(std::cin)
    {
    }

   FileInput::FileInput(const std::string& fileName) : LineBufferedStreamInput(source), source(fileName.c_str())
    {
    }

   int FileInput::getNextCharacter()
    {
      if (false == source.is_open())
       {
         return ENDOFFILE;
       }
      return LineBufferedStreamInput::getNextCharacter();
    }

 } // namespace Input

 } // namespace Backwards
