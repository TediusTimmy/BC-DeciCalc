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
#ifndef BACKWARDS_INPUT_LEXER_H
#define BACKWARDS_INPUT_LEXER_H

#include "Backwards/Input/Lexemes.h"
#include "Backwards/Input/Token.h"
#include "Backwards/Input/BufferedGenericInput.h"

#include <string>
#include <map>

namespace Backwards
 {

namespace Input
 {

   class Lexer /* Lex Me Up, Scotty */ final
    {

   private:
      BufferedGenericInput input; // Input mechanism.
      std::string sourceName; // Name of the current data source.
      size_t lineNumber; // Current line number in the input.
      size_t curChar; // Current character in the current line.

      Token nextToken; // The next token that will be returned.

      static const std::map<std::string, Lexeme>& keyWords(); // The map of keywords to Lexemes.

       /*
         Internal functions for operating on buffered input.
       */
      void consume (void);
      void consumeWhiteSpace (void);

      void get_NextToken (void); // Updates nextToken.

      Lexer();

   public:

      const Token& peekNextToken (void) const { return nextToken; }
      Token getNextToken (void); // Returns nextToken and then updates nextToken.

      Lexer (GenericInput& input, const std::string& sourceName, size_t lineNumber = 1U, size_t lineLocation = 1U);

    };

 } // namespace Input

 } // namespace Backwards

#endif /* BACKWARDS_INPUT_LEXER_H */
