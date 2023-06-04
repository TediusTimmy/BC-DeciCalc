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
#include "Forwards/Parser/Parser.h"

#include "Forwards/Engine/Expression.h"
#include "Backwards/Engine/Logger.h"

#include "Forwards/Types/FloatValue.h"
#include "Forwards/Types/CellRefValue.h"

#include <sstream>

namespace Forwards
 {

namespace Parser
 {

   class ParserException : public std::exception
    {
   private:
      std::string message;

   public:
      ParserException(const std::string& message) : message(message) { }

      ~ParserException() throw() { }

      const char * what() const throw() { return message.c_str(); }
    };

   std::shared_ptr<Engine::Expression> Parser::ParseFullExpression (Input::Lexer& src, Engine::GetterMap& scope, Backwards::Engine::Logger& logger, size_t col, size_t row)
    {
      std::shared_ptr<Engine::Expression> result;
      try
       {
         result = expression(src, scope, logger, col, row);
         expect(src, Input::END_OF_FILE, "End if Input");
       }
      catch (const ParserException& e)
       {
         logger.log(e.what());
         result = std::shared_ptr<Engine::Expression>();
       }
      return result;
    }

   void Parser::expect (Input::Lexer& src, Input::Lexeme expected, const std::string& name)
    {
      if (src.peekNextToken().lexeme != expected)
       {
         std::stringstream str;
         str << "Expected >" << name << "< but found >" << src.peekNextToken().text << "< at " << src.peekNextToken().location;
         throw ParserException(str.str());
       }
      src.getNextToken();
    }

   std::shared_ptr<Engine::Expression> Parser::expression (Input::Lexer& src, Engine::GetterMap& scope, Backwards::Engine::Logger& logger, size_t col, size_t row)
    {
      std::shared_ptr<Engine::Expression> lhs (simple(src, scope, logger, col, row));

      if ((Input::EQUALITY == src.peekNextToken().lexeme) || (Input::INEQUALITY == src.peekNextToken().lexeme) || 
          (Input::GREATER_THAN == src.peekNextToken().lexeme) || (Input::LESS_THAN_OR_EQUAL_TO == src.peekNextToken().lexeme) ||
          (Input::GREATER_THAN_OR_EQUAL_TO == src.peekNextToken().lexeme) || (Input::LESS_THAN == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = simple(src, scope, logger, col, row);

         switch(buildToken.lexeme)
          {
         case Input::EQUALITY:
            lhs = std::make_shared<Engine::Equals>(buildToken, lhs, rhs);
            break;
         case Input::INEQUALITY:
            lhs = std::make_shared<Engine::NotEqual>(buildToken, lhs, rhs);
            break;
         case Input::GREATER_THAN:
            lhs = std::make_shared<Engine::Greater>(buildToken, lhs, rhs);
            break;
         case Input::LESS_THAN:
            lhs = std::make_shared<Engine::Less>(buildToken, lhs, rhs);
            break;
         case Input::GREATER_THAN_OR_EQUAL_TO:
            lhs = std::make_shared<Engine::GEQ>(buildToken, lhs, rhs);
            break;
         case Input::LESS_THAN_OR_EQUAL_TO:
            lhs = std::make_shared<Engine::LEQ>(buildToken, lhs, rhs);
            break;
         default:
            break;
          }
       }

      return lhs;
    }

   std::shared_ptr<Engine::Expression> Parser::simple (Input::Lexer& src, Engine::GetterMap& scope, Backwards::Engine::Logger& logger, size_t col, size_t row)
    {
      std::shared_ptr<Engine::Expression> lhs (term(src, scope, logger, col, row));

      while ((Input::PLUS == src.peekNextToken().lexeme) || (Input::MINUS == src.peekNextToken().lexeme) ||
             (Input::CAT == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = term(src, scope, logger, col, row);

         switch(buildToken.lexeme)
          {
         case Input::PLUS:
            lhs = std::make_shared<Engine::Plus>(buildToken, lhs, rhs);
            break;
         case Input::MINUS:
            lhs = std::make_shared<Engine::Minus>(buildToken, lhs, rhs);
            break;
         case Input::CAT:
            lhs = std::make_shared<Engine::Cat>(buildToken, lhs, rhs);
            break;
         default:
            break;
          }
       }

      return lhs;
    }

   std::shared_ptr<Engine::Expression> Parser::term (Input::Lexer& src, Engine::GetterMap& scope, Backwards::Engine::Logger& logger, size_t col, size_t row)
    {
      std::shared_ptr<Engine::Expression> lhs (unary(src, scope, logger, col, row));

      while ((Input::MULTIPLY == src.peekNextToken().lexeme) || (Input::DIVIDE == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = unary(src, scope, logger, col, row);

         switch(buildToken.lexeme)
          {
         case Input::MULTIPLY:
            lhs = std::make_shared<Engine::Multiply>(buildToken, lhs, rhs);
            break;
         case Input::DIVIDE:
            lhs = std::make_shared<Engine::Divide>(buildToken, lhs, rhs);
            break;
         default:
            break;
          }
       }

      return lhs;
    }

   std::shared_ptr<Engine::Expression> Parser::unary (Input::Lexer& src, Engine::GetterMap& scope, Backwards::Engine::Logger& logger, size_t col, size_t row)
    {
      std::shared_ptr<Engine::Expression> ret;

      if (Input::MINUS == src.peekNextToken().lexeme)
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> arg (unary(src, scope, logger, col, row));

         switch(buildToken.lexeme)
          {
         case Input::MINUS:
            ret = std::make_shared<Engine::Negate>(buildToken, arg);
            break;
         default:
            break;
          }
       }
      else
       {
         ret = primary(src, scope, logger, col, row);
       }

      return ret;
    }

   std::shared_ptr<Engine::Expression> Parser::primary (Input::Lexer& src, Engine::GetterMap& scope, Backwards::Engine::Logger& logger, size_t col, size_t row)
    {
      std::shared_ptr<Engine::Expression> ret;

      switch(src.peekNextToken().lexeme)
       {
      case Input::CELL_REFERENCE:
         ret = cellref(src.getNextToken(), col, row);

         if (Input::RANGE == src.peekNextToken().lexeme)
          {
            Input::Token buildToken = src.getNextToken();

            Input::Token otherSide = src.peekNextToken();
            expect(src, Input::CELL_REFERENCE, "cell reference");
            std::shared_ptr<Engine::Expression> rhs = cellref(otherSide, col, row);

            ret = std::make_shared<Engine::MakeRange>(buildToken, ret, rhs);
          }
         break;
      case Input::IDENTIFIER:
       {
         Input::Token buildToken = src.getNextToken();
         std::vector<std::shared_ptr<Engine::Expression> > args;

         if (Input::OPEN_PARENS == src.peekNextToken().lexeme)
          {
            expect(src, Input::OPEN_PARENS, ")");
            if (Input::CLOSE_PARENS != src.peekNextToken().lexeme)
             {
               args.emplace_back(expression(src, scope, logger, col, row));

               while (Input::SEMICOLON == src.peekNextToken().lexeme)
                {
                  src.getNextToken();
                  args.emplace_back(expression(src, scope, logger, col, row));
                }
             }

            expect(src, Input::CLOSE_PARENS, ")");
          }

         if (scope.end() == scope.find(buildToken.text))
          {
            std::stringstream str;
            str << "Name >" << buildToken.text << "< is not a function at " << buildToken.location;
            throw ParserException(str.str());
          }

         ret = std::make_shared<Engine::FunctionCall>(buildToken, std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), scope[buildToken.text]), args);
       }
         break;
      case Input::NUMBER:
       {
         Input::Token buildToken = src.getNextToken();

         ret = std::make_shared<Engine::Constant>(buildToken, std::make_shared<Types::FloatValue>(BigInt::Fixed(buildToken.text)));
       }
         break;
      case Input::OPEN_PARENS:
         src.getNextToken();
         ret = expression(src, scope, logger, col, row);
         expect(src, Input::CLOSE_PARENS, ")");
         break;
      default:
       {
         std::stringstream str;
         str << "Expected >primary expression< but found >" << src.peekNextToken().text << "< at " << src.peekNextToken().location;
         throw ParserException(str.str());
       }
       }

      return ret;
    }

   std::shared_ptr<Engine::Expression> Parser::cellref (const Input::Token& ref, size_t col, size_t row)
    {
      bool colAbsolute = false, rowAbsolute = false;
      int64_t r_col, r_row;
      const char * iter = ref.text.c_str();
      if ('$' == *iter)
       {
         colAbsolute = true;
         ++iter;
       }
      int alphas = 1;
      r_col = *iter - 'A';
      ++iter;
      if (std::isalpha(*iter))
       {
         r_col = (r_col * 26) + (*iter - 'A');
         ++iter;
         ++alphas;
       }
      if (std::isalpha(*iter))
       {
         r_col = (r_col * 26) + (*iter - 'A');
         ++iter;
         ++alphas;
       }
      if (3 == alphas)
       {
         r_col += 26 * 26 + 26;
       }
      else if (2 == alphas)
       {
         r_col += 26;
       }
      if ('$' == *iter)
       {
         rowAbsolute = true;
         ++iter;
       }
      r_row = std::atoll(iter) - 1;
      if (colAbsolute && rowAbsolute)
       {
         return std::make_shared<Engine::Constant>(ref, std::make_shared<Types::CellRefValue>(colAbsolute, r_col, rowAbsolute, r_row));
       }
      else if (colAbsolute)
       {
         return std::make_shared<Engine::Constant>(ref, std::make_shared<Types::CellRefValue>(colAbsolute, r_col, rowAbsolute, r_row - row));
       }
      else if (rowAbsolute)
       {
         return std::make_shared<Engine::Constant>(ref, std::make_shared<Types::CellRefValue>(colAbsolute, r_col - col, rowAbsolute, r_row));
       }
      else
       {
         return std::make_shared<Engine::Constant>(ref, std::make_shared<Types::CellRefValue>(colAbsolute, r_col - col, rowAbsolute, r_row - row));
       }
    }

 } // namespace Parser

 } // namespace Forwards
