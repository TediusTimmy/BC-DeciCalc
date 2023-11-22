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
#include "Backwards/Input/Lexer.h"

#include <cctype>
#include <string>

namespace Backwards
 {

namespace Input
 {

   std::map<std::string, Lexeme> buildKeywords()
    {
      std::map<std::string, Lexeme> result;
      result.emplace(std::make_pair("function", FUNCTION));
      result.emplace(std::make_pair("end",      END));
      result.emplace(std::make_pair("set",      SET));
      result.emplace(std::make_pair("to",       TO));
      result.emplace(std::make_pair("call",     CALL));
      result.emplace(std::make_pair("if",       IF));
      result.emplace(std::make_pair("then",     THEN));
      result.emplace(std::make_pair("else",     ELSE));
      result.emplace(std::make_pair("elseif",   ELSEIF));
      result.emplace(std::make_pair("while",    WHILE));
      result.emplace(std::make_pair("do",       DO));
      result.emplace(std::make_pair("select",   SELECT));
      result.emplace(std::make_pair("from",     FROM));
      result.emplace(std::make_pair("case",     CASE));
      result.emplace(std::make_pair("is",       IS));
      result.emplace(std::make_pair("also",     ALSO));
      result.emplace(std::make_pair("above",    ABOVE));
      result.emplace(std::make_pair("below",    BELOW));
      result.emplace(std::make_pair("break",    BREAK));
      result.emplace(std::make_pair("continue", CONTINUE));
      result.emplace(std::make_pair("return",   RETURN));
      result.emplace(std::make_pair("for",      FOR));
      result.emplace(std::make_pair("downto",   DOWNTO));
      result.emplace(std::make_pair("step",     STEP));
      result.emplace(std::make_pair("in",       IN));
      return result;
    }

   const std::map<std::string, Lexeme>& Lexer::keyWords()
    {
      static const std::map<std::string, Lexeme> keyWordsMap (buildKeywords());
      return keyWordsMap;
    }

   Lexer::Lexer (GenericInput& input, const std::string& sourceName, size_t lineNumber, size_t lineLocation) :
      input(input), sourceName(sourceName), lineNumber(lineNumber), curChar(lineLocation), nextToken()
    {
      get_NextToken();
    }

   void Lexer::consume (void)
    {
      int next = input.consume();
      if ('\n' == next)
       {
         curChar = 1;
         ++lineNumber;
       }
      else if (ENDOFFILE != next)
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
         case '\r':
         case '\n':
            consume();
            break;
         case '(':
            // Old Pascal-style comment. Read until *)
            if ('*' == input.peek(1))
             {
               consume(); // Consume the '(' and '*'
               consume(); // so that "(*)" is not a valid comment.
               while ((ENDOFFILE != input.peek()) && (('*' != input.peek()) || (')' != input.peek(1))))
                {
                  consume();
                }
               consume(); // Finish off the comment.
               consume();
             }
            else
             {
               return;
             }
            break;
         default:
            return;
          }
       }
    }

   void Lexer::get_NextToken (void)
    {
      consumeWhiteSpace();

      int lineNo = lineNumber;
      int charNo = curChar;

      Lexeme tokenType = END_OF_FILE;
      std::string text;

      if (std::isalpha(input.peek()) || ('_' == input.peek())) // Will read a letter or underscore
       { // Read in an identifier or keyword -- loop until not letter, number, or '_'
         text = static_cast<char>(input.peek());
         consume();
         while (std::isalnum(input.peek()) || ('_' == input.peek()))
          {
            text += static_cast<char>(input.peek());
            consume();
          }

         std::map<std::string, Lexeme>::const_iterator keyWord = keyWords().find(text);

         if (keyWords().end() == keyWord)
          {
            tokenType = IDENTIFIER;
          }
         else
          {
            tokenType = keyWord->second;
          }
       }
      else if (std::isdigit(input.peek()) || ('.' == input.peek()) || (',' == input.peek())) // Will read a number or the begining of one
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
            tokenType = PERIOD;
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
         case ENDOFFILE:
            text = "END-OF-INPUT";
            tokenType = END_OF_FILE;
            break;
         case ';':
            consume();
            text = ";";
            tokenType = SEMICOLON;
            break;
         case '\'':
            consume();
            tokenType = STRING;
            while ((ENDOFFILE != input.peek()) && ('\'' != input.peek()))
             {
               text += static_cast<char>(input.peek());
               consume();
             }
            if (ENDOFFILE == input.peek())
             {
               tokenType = INVALID;
             }
            consume();
            break;
         case '"':
            consume();
            tokenType = STRING;
            while ((ENDOFFILE != input.peek()) && ('"' != input.peek()))
             {
               text += static_cast<char>(input.peek());
               consume();
             }
            if (ENDOFFILE == input.peek())
             {
               tokenType = INVALID;
             }
            consume();
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
         case '[':
            consume();
            text = "[";
            tokenType = OPEN_BRACKET;
            break;
         case ']':
            consume();
            text = "]";
            tokenType = CLOSE_BRACKET;
            break;
         case '{':
            consume();
            text = "{";
            tokenType = OPEN_BRACE;
            break;
         case '}':
            consume();
            text = "}";
            tokenType = CLOSE_BRACE;
            break;
         case ':':
            consume();
            text = ":";
            tokenType = ALTERNATIVE;
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
         case '?':
            consume();
            text = "?";
            tokenType = CONDITIONAL;
            break;
         case '&':
            consume();
            text = "&";
            tokenType = SHORT_AND;
            break;
         case '|':
            consume();
            text = "|";
            tokenType = SHORT_OR;
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
         case '!':
            consume();
            text = "!";
            tokenType = NOT;
            break;
         default:
            text = static_cast<char>(input.peek());
            consume();
            tokenType = INVALID;
            break;
          }
       }

      nextToken = Token(tokenType, text, sourceName, lineNo, charNo);
    }

   Token Lexer::getNextToken (void)
    {
      Token result (nextToken);
      get_NextToken();
      return result;
    }

 } // namespace Input

 } // namespace Backwards
