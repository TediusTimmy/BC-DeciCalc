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
#include "Forwards/Input/Lexer.h"

#include <cctype>
#include <string>

namespace Forwards
 {

namespace Input
 {

   Lexer::Lexer (Backwards::Input::GenericInput& input, size_t location) :
      input(input), curChar(location), nextToken()
    {
      get_NextToken();
    }

   void Lexer::consume (void)
    {
      if (Backwards::Input::ENDOFFILE != input.consume())
       {
         ++curChar;
       }
    }

   void Lexer::consumeWhiteSpace (void)
    {
      for(;;)
       {
         switch (input.peek())
          {
         case ' ':
         case '\t':
            consume();
            break;
         default:
            return;
          }
       }
    }

   void Lexer::get_NextToken (void)
    {
      consumeWhiteSpace();

      int charNo = curChar;

      Lexeme tokenType = END_OF_FILE;
      std::string text;

      if (std::isalpha(input.peek())|| ('$' == input.peek()))
       { // Read in a cell reference
         text = static_cast<char>(std::toupper(input.peek()));
         int alphas = 0;
         if (std::isalpha(input.peek()))
          {
            ++alphas;
          }
         consume();
         while (std::isalpha(input.peek()))
          {
            text += static_cast<char>(std::toupper(input.peek()));
            ++alphas;
            consume();
          }

         int nums = 0;
         if ('$' == input.peek())
          {
            text += "$";
            consume();
          }

         std::string number;
         while (std::isdigit(input.peek()))
          {
            text += static_cast<char>(input.peek());
            number += static_cast<char>(input.peek());
            ++nums;
            consume();
          }

         if ((alphas > 0) && (alphas < 4) && (nums > 0) && (nums < 10) && (0 != std::stoi(number)))
          {
            tokenType = CELL_REFERENCE;
          }
         else
          {
            tokenType = INVALID;
          }
       }
      else if ('@' == input.peek())
       { // Read in an identifier.
         consume();
         while (std::isalpha(input.peek()))
          {
            text += static_cast<char>(std::toupper(input.peek()));
            consume();
          }
         if (false == text.empty())
          {
            tokenType = IDENTIFIER;
          }
         else
          {
            tokenType = INVALID;
          }
       }
      else if (std::isdigit(input.peek()) || ('.' == input.peek()) || (',' == input.peek()))
       { // Read in a number
         while (std::isdigit(input.peek()))
          {
            text += static_cast<char>(input.peek());
            consume();
          }
         if (('.' == input.peek()) || (',' == input.peek()))
          {
            text += ".";
            consume();
          }
         while (std::isdigit(input.peek()))
          {
            text += static_cast<char>(input.peek());
            consume();
          }

         if (("." == text) || ("," == text))
          {
            tokenType = INVALID;
          }
         else
          {
            if (('e' == input.peek()) || ('E' == input.peek()))
             {
               std::string temp = "e";
               size_t advance = 1U;

               if (('-' == input.peek(advance)) || ('+' == input.peek(advance)))
                {
                  temp += static_cast<char>(input.peek(advance));
                  ++advance;
                }
               while (std::isdigit(input.peek(advance)))
                {
                  temp += static_cast<char>(input.peek(advance));
                  ++advance;
                }

               if (("e-" != temp) && ("e+" != temp) && ("e" != temp))
                {
                  text += temp;
                  while (0U < advance)
                   {
                     consume();
                     --advance;
                   }
                }
             }

            tokenType = NUMBER;
          }
       }
      else
       { //DFA for all other tokens
         switch (input.peek())
          {
         case Backwards::Input::ENDOFFILE:
            text = "END-OF-INPUT";
            tokenType = END_OF_FILE;
            break;
         case ';':
            consume();
            text = ";";
            tokenType = SEMICOLON;
            break;
         case '(':
            consume();
            text = "(";
            tokenType = OPEN_PARENS;
            break;
         case ')':
            consume();
            text = ")";
            tokenType = CLOSE_PARENS;
            break;
         case ':':
            consume();
            text = ":";
            tokenType = RANGE;
            break;
         case '=':
            consume();
            text = "=";
            tokenType = EQUALITY;
            break;
         case '<':
            consume();
            if ('>' == input.peek())
             {
               consume();
               text = "<>";
               tokenType = INEQUALITY;
             }
            else if ('=' == input.peek())
             {
               consume();
               text = "<=";
               tokenType = LESS_THAN_OR_EQUAL_TO;
             }
            else
             {
               text = "<";
               tokenType = LESS_THAN;
             }
            break;
         case '>':
            consume();
            if ('=' == input.peek())
             {
               consume();
               text = ">=";
               tokenType = GREATER_THAN_OR_EQUAL_TO;
             }
            else
             {
               text = ">";
               tokenType = GREATER_THAN;
             }
            break;
         case '+':
            consume();
            text = "+";
            tokenType = PLUS;
            break;
         case '-':
            consume();
            text = "-";
            tokenType = MINUS;
            break;
         case '*':
            consume();
            text = "*";
            tokenType = MULTIPLY;
            break;
         case '/':
            consume();
            text = "/";
            tokenType = DIVIDE;
            break;
         case '&':
            consume();
            text = "&";
            tokenType = CAT;
            break;
         default:
            text = static_cast<char>(input.peek());
            consume();
            tokenType = INVALID;
            break;
          }
       }

      nextToken = Token(tokenType, text, charNo);
    }

   Token Lexer::getNextToken (void)
    {
      Token result (nextToken);
      get_NextToken();
      return result;
    }

 } // namespace Input

 } // namespace Forwards
