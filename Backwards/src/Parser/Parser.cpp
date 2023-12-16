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
#include "Backwards/Parser/Parser.h"

#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/Statement.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Parser/SymbolTable.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/FunctionValue.h"

#include "Backwards/Engine/ConstantsSingleton.h"

#include <sstream>

namespace Backwards
 {

namespace Parser
 {

   class ParserException final : public std::exception
    {
   private:
      std::string message;

   public:
      explicit ParserException(const std::string& message) : message(message) { }

      ~ParserException() throw() { }

      const char * what() const throw() { return message.c_str(); }
    };

   std::shared_ptr<Engine::Expression> Parser::ParseExpression (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> result;
      try
       {
         result = expression(src, table, logger);
       }
      catch (const ParserException& e)
       {
         logger.log(e.what());
       }
      return result;
    }

   std::shared_ptr<Engine::Expression> Parser::ParseFullExpression (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> result;
      try
       {
         result = expression(src, table, logger);
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
         str << "Expected >" << name << "< but found >" << src.peekNextToken().text << "<" << std::endl
             << "\tFrom " << src.peekNextToken().lineLocation << " on line " << src.peekNextToken().lineNumber << " in file " << src.peekNextToken().sourceFile;
         throw ParserException(str.str());
       }
      src.getNextToken();
    }

   std::shared_ptr<Engine::Expression> Parser::expression (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> condition = predicate(src, table, logger);

      if (Input::CONDITIONAL == src.peekNextToken().lexeme)
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> thenCase = expression(src, table, logger);

         expect(src, Input::ALTERNATIVE, ":");

         std::shared_ptr<Engine::Expression> elseCase = expression(src, table, logger);

         condition = std::make_shared<Engine::TernaryOperation>(buildToken, condition, thenCase, elseCase);
       }

      return condition;
    }

   std::shared_ptr<Engine::Expression> Parser::predicate (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> lhs (relation(src, table, logger));

      while ((Input::SHORT_OR == src.peekNextToken().lexeme) ||
         (Input::SHORT_AND == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = relation(src, table, logger);

         switch (buildToken.lexeme)
          {
         case Input::SHORT_OR:
            lhs = std::make_shared<Engine::ShortOr>(buildToken, lhs, rhs);
            break;
         case Input::SHORT_AND:
            lhs = std::make_shared<Engine::ShortAnd>(buildToken, lhs, rhs);
            break;
         default:
            break;
          }
       }

      return lhs;
    }

   std::shared_ptr<Engine::Expression> Parser::relation (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> lhs (simple(src, table, logger));

      if ((Input::EQUALITY == src.peekNextToken().lexeme) || (Input::INEQUALITY == src.peekNextToken().lexeme) || 
          (Input::GREATER_THAN == src.peekNextToken().lexeme) || (Input::LESS_THAN_OR_EQUAL_TO == src.peekNextToken().lexeme) ||
          (Input::GREATER_THAN_OR_EQUAL_TO == src.peekNextToken().lexeme) || (Input::LESS_THAN == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = simple(src, table, logger);

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

   std::shared_ptr<Engine::Expression> Parser::simple (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> lhs (term(src, table, logger));

      while ((Input::PLUS == src.peekNextToken().lexeme) || (Input::MINUS == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = term(src, table, logger);

         switch(buildToken.lexeme)
          {
         case Input::PLUS:
            lhs = std::make_shared<Engine::Plus>(buildToken, lhs, rhs);
            break;
         case Input::MINUS:
            lhs = std::make_shared<Engine::Minus>(buildToken, lhs, rhs);
            break;
         default:
            break;
          }
       }

      return lhs;
    }

   std::shared_ptr<Engine::Expression> Parser::term (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> lhs (unary(src, table, logger));

      while ((Input::MULTIPLY == src.peekNextToken().lexeme) || (Input::DIVIDE == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs = unary(src, table, logger);

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

   std::shared_ptr<Engine::Expression> Parser::unary (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> ret;

      if ((Input::NOT == src.peekNextToken().lexeme) || (Input::MINUS == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> arg (unary(src, table, logger));

         switch(buildToken.lexeme)
          {
         case Input::NOT:
            ret = std::make_shared<Engine::Not>(buildToken, arg);
            break;
         case Input::MINUS:
            ret = std::make_shared<Engine::Negate>(buildToken, arg);
            break;
         default:
            break;
          }
       }
      else
       {
         ret = referent(src, table, logger);
       }

      return ret;
    }

   std::shared_ptr<Engine::Expression> Parser::referent (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> lhs (builder(src, table, logger));

      while ((Input::OPEN_BRACKET == src.peekNextToken().lexeme) || (Input::PERIOD == src.peekNextToken().lexeme))
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> rhs;
         if (Input::PERIOD == buildToken.lexeme)
          {
            Input::Token memberToken = src.peekNextToken();
            expect(src, Input::IDENTIFIER, "Identifier");
            rhs = std::make_shared<Engine::Constant>(memberToken, std::make_shared<Types::StringValue>(memberToken.text));
          }
         else
          {
            rhs = builder(src, table, logger);
            expect(src, Input::CLOSE_BRACKET, "]");
          }

         lhs = std::make_shared<Engine::DerefVar>(buildToken, lhs, rhs);
       }

      return lhs;
    }

   std::shared_ptr<Engine::Expression> Parser::builder (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> ret;

      if (Input::OPEN_BRACE == src.peekNextToken().lexeme)
       {
         Input::Token buildToken = src.getNextToken();

         if (Input::CLOSE_BRACE != src.peekNextToken().lexeme)
          {
            std::shared_ptr<Engine::Expression> key (expression(src, table, logger));

            // Is it a Dictionary?
            if (Input::ALTERNATIVE == src.peekNextToken().lexeme)
             {
               src.getNextToken();
               std::shared_ptr<Engine::Expression> newColl = std::make_shared<Engine::Constant>(buildToken, Engine::ConstantsSingleton::getInstance().EMPTY_DICTIONARY);
               std::shared_ptr<Engine::Expression> value (expression(src, table, logger));
               ret = std::shared_ptr<Engine::Expression>(table.buildInsert(buildToken, newColl, key, value));
               while (Input::SEMICOLON == src.peekNextToken().lexeme)
                {
                  src.getNextToken();
                  std::shared_ptr<Engine::Expression> nextKey (expression(src, table, logger));
                  expect(src, Input::ALTERNATIVE, ":");
                  std::shared_ptr<Engine::Expression> nextValue (expression(src, table, logger));
                  ret = std::shared_ptr<Engine::Expression>(table.buildInsert(buildToken, ret, nextKey, nextValue));
                }
             }
            else
            // Assume an Array
             {
               std::shared_ptr<Engine::Expression> newColl = std::make_shared<Engine::Constant>(buildToken, Engine::ConstantsSingleton::getInstance().EMPTY_ARRAY);
               ret = std::shared_ptr<Engine::Expression>(table.buildPushBack(buildToken, newColl, key));
               while (Input::SEMICOLON == src.peekNextToken().lexeme)
                {
                  src.getNextToken();
                  std::shared_ptr<Engine::Expression> next (expression(src, table, logger));
                  ret = std::shared_ptr<Engine::Expression>(table.buildPushBack(buildToken, ret, next));
                }
             }
            expect(src, Input::CLOSE_BRACE, "}");
          }
         else
         // Empty Array
          {
            src.getNextToken();
            ret = std::make_shared<Engine::Constant>(buildToken, Engine::ConstantsSingleton::getInstance().EMPTY_ARRAY);
          }
       }
      else
       {
         ret = functionCall(src, table, logger);
       }

      return ret;
    }

    std::shared_ptr<Engine::Expression> Parser::functionCall (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> ret = primary(src, table, logger);

      while (Input::OPEN_PARENS == src.peekNextToken().lexeme)
       {
         Input::Token buildToken = src.getNextToken();

         std::vector<std::shared_ptr<Engine::Expression> > args;
         if (Input::CLOSE_PARENS != src.peekNextToken().lexeme)
          {
            args.emplace_back(expression(src, table, logger));

            while (Input::SEMICOLON == src.peekNextToken().lexeme)
             {
               src.getNextToken();
               args.emplace_back(expression(src, table, logger));
             }
          }

         expect(src, Input::CLOSE_PARENS, ")");

         ret = std::make_shared<Engine::FunctionCall>(buildToken, ret, args);
      }

      return ret;
    }

   std::shared_ptr<Engine::Expression> Parser::primary (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> ret;

      switch(src.peekNextToken().lexeme)
       {
      case Input::IDENTIFIER:
         switch(table.lookup(src.peekNextToken().text))
          {
         case SymbolTable::GLOBAL_VARIABLE:
         case SymbolTable::SCOPE_VARIABLE:
         case SymbolTable::LOCAL_VARIABLE:
          {
            Input::Token buildToken = src.getNextToken();

            ret = std::make_shared<Engine::Variable>(buildToken, table.getVariableGetter(buildToken.text));
          }
            break;
         case SymbolTable::FUNCTION:
          {
            Input::Token buildToken = src.getNextToken();

            std::vector<std::shared_ptr<Engine::Expression> > captures;
            if (0U != table.activeFunctions[buildToken.text].lock()->ncaptures)
             {
               expect(src, Input::OPEN_BRACKET, "[ function parameters");
               captures.emplace_back(expression(src, table, logger));
               while (Input::SEMICOLON == src.peekNextToken().lexeme)
                {
                  src.getNextToken();
                  captures.emplace_back(expression(src, table, logger));
                }
               if (captures.size() != table.activeFunctions[buildToken.text].lock()->ncaptures)
                {
                  std::stringstream str;
                  str << "Function >" << buildToken.text << "< called with " << captures.size() << " of " << table.activeFunctions[buildToken.text].lock()->ncaptures << " captured values provided." << std::endl
                      << "\tFrom " << src.peekNextToken().lineLocation << " on line " << src.peekNextToken().lineNumber << " in file " << src.peekNextToken().sourceFile;
                  throw ParserException(str.str());
                }
               expect(src, Input::CLOSE_BRACKET, "]");
             }

            if (true == captures.empty())
             {
               ret = std::make_shared<Engine::Constant>(buildToken, std::make_shared<Types::FunctionValue>(std::vector<std::shared_ptr<Types::ValueType> >(), table.activeFunctions[buildToken.text]));
             }
            else
             {
               ret = std::make_shared<Engine::BuildFunction>(buildToken, captures, table.activeFunctions[buildToken.text]);
             }
          }
            break;
         case SymbolTable::UNDEFINED:
          {
            std::stringstream str;
            str << "Undefined identifier >" << src.peekNextToken().text << "< used." << std::endl
                << "\tFrom " << src.peekNextToken().lineLocation << " on line " << src.peekNextToken().lineNumber << " in file " << src.peekNextToken().sourceFile;
            throw ParserException(str.str());
          }
            break;
          }
         break;
      case Input::FUNCTION:
       {
         Input::Token buildToken = src.getNextToken();

         std::vector<std::shared_ptr<Engine::Expression> > captures;
         if (Input::OPEN_BRACKET == src.peekNextToken().lexeme)
          {
            src.getNextToken();
            captures.emplace_back(expression(src, table, logger));
            while (Input::SEMICOLON == src.peekNextToken().lexeme)
             {
               src.getNextToken();
               captures.emplace_back(expression(src, table, logger));
             }
            expect(src, Input::CLOSE_BRACKET, "]");
          }

         table.pushContext();
         try
          {
            bool badWrong = false;
            if (Input::IDENTIFIER == src.peekNextToken().lexeme)
             {
               Input::Token identToken = src.getNextToken();
               badWrong |= enforceUnique(identToken, table, "function name", logger);
               table.getContext()->name = identToken.text;
             }

            expect(src, Input::OPEN_PARENS, "(");
            table.activeFunctions.emplace(table.getContext()->name, table.getContext());
            table.getContext()->ncaptures = captures.size();

            if (Input::CLOSE_PARENS != src.peekNextToken().lexeme)
             {
               Input::Token argName = src.peekNextToken();
               expect(src, Input::IDENTIFIER, "Argument Identifier");
               badWrong |= enforceUnique(argName, table, "function argument", logger);
               table.addArgument(argName.text);

               while (Input::SEMICOLON == src.peekNextToken().lexeme)
                {
                  src.getNextToken();

                  Input::Token argNext = src.peekNextToken();
                  expect(src, Input::IDENTIFIER, "Argument Identifier");
                  badWrong |= enforceUnique(argNext, table, "function argument", logger);
                  table.addArgument(argNext.text);
                }
             }

            table.getContext()->nargs = table.getContext()->args.size();

            try
             {
               expect(src, Input::CLOSE_PARENS, ")");

               if (false == captures.empty())
                {
                  expect(src, Input::OPEN_BRACKET, "[ function parameter names");

                  Input::Token captureName = src.peekNextToken();
                  expect(src, Input::IDENTIFIER, "Capture Identifier");
                  badWrong |= enforceUnique(captureName, table, "function capture", logger);
                  table.addCapture(captureName.text);

                  while (Input::SEMICOLON == src.peekNextToken().lexeme)
                   {
                     src.getNextToken();

                     Input::Token captureNext = src.peekNextToken();
                     expect(src, Input::IDENTIFIER, "Capture Identifier");
                     badWrong |= enforceUnique(captureNext, table, "function capture", logger);
                     table.addCapture(captureNext.text);
                   }

                  if (captures.size() != table.getContext()->captures.size())
                   {
                     std::stringstream str;
                     str << "Function parameterized with " << captures.size() << " values but given " << table.getContext()->captures.size() << " value names." << std::endl
                         << "\tFrom " << src.peekNextToken().lineLocation << " on line " << src.peekNextToken().lineNumber << " in file " << src.peekNextToken().sourceFile;
                     throw ParserException(str.str());
                   }

                  expect(src, Input::CLOSE_BRACKET, "]");
                }

               expect(src, Input::IS, "is");
             }
            catch (const ParserException& e)
             {
               logger.log(e.what());
               badWrong = true;
               recoverStatement(src);
             }

            std::shared_ptr<Engine::Statement> block = innerStatementSeq(src, table, logger);

            expect(src, Input::END, "end");

            if ((nullptr != block.get()) && (false == badWrong))
             {
               table.getContext()->function = block;
               table.getContext()->nlocals = table.getContext()->locals.size();
                // Nota bene : we are being very loosey-goosey with the functions.
               table.activeFunctions.erase(table.getContext()->name);
               if (true == captures.empty())
                {
                  ret = std::make_shared<Engine::Constant>(buildToken, std::make_shared<Types::FunctionValue>(table.getContext(), std::vector<std::shared_ptr<Types::ValueType> >()));
                }
               else
                {
                  ret = std::make_shared<Engine::BuildFunction>(buildToken, table.getContext(), captures);
                }
             }
            else
             {
               throw ParserException("Parse failed. See previous messages.");
             }
          }
         catch(...)
          {
            table.activeFunctions.erase(table.getContext()->name);
            table.popContext();
            throw;
          }
         table.popContext();
       }
         break;
      case Input::NUMBER:
       {
         Input::Token buildToken = src.getNextToken();

         ret = std::make_shared<Engine::Constant>(buildToken, std::make_shared<Types::FloatValue>(BigInt::Fixed(buildToken.text)));
       }
         break;
      case Input::STRING:
       {
         Input::Token buildToken = src.getNextToken();

         ret = std::make_shared<Engine::Constant>(buildToken, std::make_shared<Types::StringValue>(buildToken.text));
       }
         break;
      case Input::OPEN_PARENS:
         src.getNextToken();
         ret = expression(src, table, logger);
         expect(src, Input::CLOSE_PARENS, ")");
         break;
      default:
       {
         std::stringstream str;
         str << "Expected >primary expression< but found >" << src.peekNextToken().text << "<" << std::endl
             << "\tFrom " << src.peekNextToken().lineLocation << " on line " << src.peekNextToken().lineNumber << " in file " << src.peekNextToken().sourceFile;
         throw ParserException(str.str());
       }
       }

      return ret;
    }







   bool Parser::enforceUnique(const Input::Token& token, const SymbolTable& table, const std::string& use, Engine::Logger& logger)
    {
      bool errorOccurred = false;
      if (SymbolTable::UNDEFINED != table.lookup(token.text))
       {
         errorOccurred = true;
         std::stringstream str;
         str << "Identifier >" << token.text << "< for " << use << " is already defined." << std::endl <<
             "\tFrom " << token.lineLocation << " on line " << token.lineNumber << " in file " << token.sourceFile;
         logger.log(str.str());
       }
      return errorOccurred;
    }

   void Parser::recoverExpression(Input::Lexer& src)
    {
      bool tuberculosis = true; // consumption
      while (true == tuberculosis)
       {
         // This is the follow set of an expression
         // As an expression can end a statement,
         // its follow set is the follow set of a statement:
         switch (src.peekNextToken().lexeme)
          {
         case Input::FUNCTION:
         case Input::SET:
         case Input::CALL:
         case Input::IF:
         case Input::WHILE:
         case Input::SELECT:
         case Input::BREAK:
         case Input::CONTINUE:
         case Input::RETURN:
         case Input::FOR:

         case Input::ELSE:
         case Input::ELSEIF:
         case Input::CASE:
         case Input::ALSO:
         case Input::END:

         case Input::TO:
         case Input::THEN:
         case Input::DO:
         case Input::FROM:
         case Input::IS:
         case Input::DOWNTO:
         case Input::STEP:
         case Input::IN:

         case Input::END_OF_FILE:
            tuberculosis = false;
            break;

         default:
            src.getNextToken();
            break;
          }
       }
    }

   void Parser::recoverStatement(Input::Lexer& src)
    {
      bool tuberculosis = true; // consumption
      while (true == tuberculosis)
       {
         // This is the follow set of a statement:
         // most of this is the first set of a statement.
         switch (src.peekNextToken().lexeme)
          {
         case Input::FUNCTION:
         case Input::SET:
         case Input::CALL:
         case Input::IF:
         case Input::WHILE:
         case Input::SELECT:
         case Input::BREAK:
         case Input::CONTINUE:
         case Input::RETURN:
         case Input::FOR:

         case Input::ELSE:
         case Input::ELSEIF:
         case Input::CASE:
         case Input::ALSO:
         case Input::END:

         case Input::END_OF_FILE:
            tuberculosis = false;
            break;

         default:
            src.getNextToken();
            break;
          }
       }
    }

   std::shared_ptr<Engine::Statement> Parser::ParseFunctions (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::vector<std::shared_ptr<Engine::Statement> > statements;
      bool badWrong = false;
      Input::Token token = src.peekNextToken();
      try
       {
         while (Input::SET == src.peekNextToken().lexeme)
          {
            std::shared_ptr<Engine::Statement> ret (statement(src, table, true, logger));
            statements.emplace_back(ret); // A SET statement is incapable of returning NULL. It MUST either throw or return a valid pointer.
          }
         if (Input::END_OF_FILE != src.peekNextToken().lexeme)
          {
            logger.log("Something other than a function definition was found while parsing only functions.");
            badWrong = true;
          }
       }
      catch (const ParserException& e)
       {
         logger.log(std::string("Could not parse functions: ") + e.what());
         badWrong = true;
       }
      if (false == badWrong)
       {
         return std::make_shared<Engine::StatementSeq>(token, statements);
       }
      return std::shared_ptr<Engine::Statement>();
    }

   std::shared_ptr<Engine::Statement> Parser::Parse (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      return outerStatementSeq(src, table, logger); // Currently, outerStatementSeq will never throw an exception.
    }

   std::shared_ptr<Engine::Statement> Parser::ParseStatement (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      try
       {
         return statement(src, table, false, logger);
       }
      catch (const ParserException& e)
       {
         logger.log(e.what());
       }
      return std::shared_ptr<Engine::Statement>();
    }





   std::shared_ptr<Engine::Statement> Parser::statement (Input::Lexer& src, SymbolTable& table, bool shadow, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Statement> ret;
      bool badWrong = false;

      switch (src.peekNextToken().lexeme)
       {
      case Input::SET:
       {
         bool defined = false;
         Input::Token buildToken = src.getNextToken();
         Input::Token identToken = src.peekNextToken();
         expect(src, Input::IDENTIFIER, "Identifier");
         switch(table.lookup(identToken.text))
          {
         case SymbolTable::GLOBAL_VARIABLE:
             // YES! THIS IS AS WONKY AS IT LOOKS. Bet you didn't know that was valid C++?
             // If Scope variables shadow Global variables, treat the global as undefined and create it.
             // This can only happen at the top-most level parse of a "functions" block.
            if (true == shadow)
             {
                {
               std::stringstream str;
               str << "Assignment to >" << identToken.text << "< will create a local variable that shadows the global variable." << std::endl
                   << "\tFrom " << identToken.lineLocation << " on line " << identToken.lineNumber << " in file " << identToken.sourceFile;
               logger.log(str.str());
                }

         case SymbolTable::UNDEFINED: // Assignment Statement
            // Add this to the symbol table, defining it as a local.
            table.addLocal(identToken.text);
            defined = true;

             }

            /* !!! FALL THROUGH !!! */
         case SymbolTable::SCOPE_VARIABLE: // Assignment Statement
         case SymbolTable::LOCAL_VARIABLE:
          {
            std::shared_ptr<Engine::RecAssignState> base;
            std::shared_ptr<Engine::RecAssignState> current;

            if ((true == defined) && ((Input::OPEN_BRACKET == src.peekNextToken().lexeme) || (Input::PERIOD == src.peekNextToken().lexeme)))
             {
               std::stringstream str;
               str << "Identifier >" << identToken.text << "< cannot be a Dictionary or Array in this context." << std::endl
                   << "\tFrom " << identToken.lineLocation << " on line " << identToken.lineNumber << " in file " << identToken.sourceFile;
               throw ParserException(str.str());
             }

            while ((Input::OPEN_BRACKET == src.peekNextToken().lexeme) || (Input::PERIOD == src.peekNextToken().lexeme))
             {
               Input::Token openToken = src.getNextToken();

               std::shared_ptr<Engine::Expression> index;
               if (Input::PERIOD == openToken.lexeme)
                {
                  Input::Token memberToken = src.peekNextToken();
                  expect(src, Input::IDENTIFIER, "Identifier");
                  index = std::make_shared<Engine::Constant>(memberToken, std::make_shared<Types::StringValue>(memberToken.text));
                }
               else
                {
                  index = expression(src, table, logger);
                  expect(src, Input::CLOSE_BRACKET, "]");
                }

               if (nullptr == base.get())
                {
                  base = std::make_shared<Engine::RecAssignState>(openToken, index);
                  current = base;
                }
               else
                {
                  current->next = std::make_shared<Engine::RecAssignState>(openToken, index);
                  current = current->next;
                }
             }

            expect(src, Input::TO, "to");

            std::shared_ptr<Engine::Expression> rhs = expression(src, table, logger);

            ret = std::make_shared<Engine::Assignment>(buildToken, table.getVariableGetter(identToken.text), table.getVariableSetter(identToken.text), base, rhs);
          }
            break;
         case SymbolTable::FUNCTION:
          {
            std::stringstream str;
            str << "Identifier >" << src.peekNextToken().text << "< is not allowed in this context." << std::endl
                << "\tFrom " << src.peekNextToken().lineLocation << " on line " << src.peekNextToken().lineNumber << " in file " << src.peekNextToken().sourceFile;
            throw ParserException(str.str());
          }
            break;
          }
       }
         break;

      case Input::CALL: // Expression-as-statement.
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> exprey = expression(src, table, logger);

         ret = std::make_shared<Engine::Expr>(buildToken, exprey);
       }
         break;

      case Input::IF:
         ret = innerIF(src, table, logger);
         break;

      case Input::WHILE:
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> condition = expressionRecover(src, table, logger);

         size_t id = table.newLoop();
         try
          {
            if (Input::CALL == src.peekNextToken().lexeme)
             {
               src.getNextToken();

               Input::Token name = src.getNextToken();

               if (0U != table.getLoop(name.text))
                {
                  badWrong = true;
                  std::stringstream str;
                  str << "Label >" << name.text << "< is already defined." << std::endl
                      << "\tFrom " << name.lineLocation << " on line " << name.lineNumber << " in file " << name.sourceFile;
                  logger.log(str.str());
                }
               else
                {
                  table.nameLoop(name.text);
                }
             }

            expect(src, Input::DO, "do");

            std::shared_ptr<Engine::Statement> block = innerStatementSeq(src, table, logger);

            expect(src, Input::END, "end");

            if ((nullptr != condition.get()) && (nullptr != block.get()) && (false == badWrong))
             {
               ret = std::make_shared<Engine::WhileStatement>(buildToken, condition, block, id);
             }
          }
         catch (...)
          {
            table.popLoop();
            throw;
          }
         table.popLoop();
       }
         break;

      case Input::FOR:
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Getter> getter;
         std::shared_ptr<Engine::Setter> setter;
         bool to = true;
         std::shared_ptr<Engine::Expression> condition;
         std::shared_ptr<Engine::Expression> upper;
         std::shared_ptr<Engine::Expression> step;

         Input::Token identToken = src.peekNextToken();
         expect(src, Input::IDENTIFIER, "Identifier");
         if (SymbolTable::UNDEFINED == table.lookup(identToken.text))
          {
            table.addLocal(identToken.text);
          }
         getter = table.getVariableGetter(identToken.text);
         setter = table.getVariableSetter(identToken.text);

         if (Input::IN == src.peekNextToken().lexeme)
          {
            src.getNextToken();
            condition = expressionRecover(src, table, logger);
          }
         else
          {
            expect(src, Input::FROM, "from");

            condition = expressionRecover(src, table, logger);

            if (Input::DOWNTO == src.peekNextToken().lexeme)
             {
               src.getNextToken();
               to = false;
             }
            else
             {
               expect(src, Input::TO, "to");
             }
            upper = expressionRecover(src, table, logger);
            if (nullptr == upper.get())
             {
               badWrong = true;
             }

            if (Input::STEP == src.peekNextToken().lexeme)
             {
               src.getNextToken();
               step = expressionRecover(src, table, logger);
               if (nullptr == step.get())
                {
                  badWrong = true;
                }
             }
          }

         size_t id = table.newLoop();
         try
          {
            if (Input::CALL == src.peekNextToken().lexeme)
             {
               src.getNextToken();

               Input::Token name = src.getNextToken();

               if (0U != table.getLoop(name.text))
                {
                  badWrong = true;
                  std::stringstream str;
                  str << "Label >" << name.text << "< is already defined." << std::endl
                      << "\tFrom " << name.lineLocation << " on line " << name.lineNumber << " in file " << name.sourceFile;
                  logger.log(str.str());
                }
               else
                {
                  table.nameLoop(name.text);
                }
             }

            expect(src, Input::DO, "do");

            std::shared_ptr<Engine::Statement> block = innerStatementSeq(src, table, logger);

            expect(src, Input::END, "end");

            if ((nullptr != condition.get()) && (nullptr != block.get()) && (false == badWrong))
             {
               ret = std::make_shared<Engine::ForStatement>(buildToken, getter, setter, condition, to, upper, step, block, id);
             }
          }
         catch (...)
          {
            table.popLoop();
            throw;
          }
         table.popLoop();
       }
         break;

      case Input::BREAK:
      case Input::CONTINUE:
       {
         Input::Token buildToken = src.getNextToken();
         size_t id = table.currentLoop();
         if (0U == id)
          {
            std::stringstream str;
            str << "Statement >" << buildToken.text << "<, but not in loop." << std::endl
                << "\tFrom " << buildToken.lineLocation << " on line " << buildToken.lineNumber << " in file " << buildToken.sourceFile;
            throw ParserException(str.str());
          }

         if (Input::IDENTIFIER == src.peekNextToken().lexeme)
         {
            Input::Token name = src.getNextToken();
            id = table.getLoop(name.text);
            if (0U == id)
             {
               std::stringstream str;
               str << "Loop label >" + name.text + "< has not been defined." << std::endl
                   << "\tFrom " << name.lineLocation << " on line " << name.lineNumber << " in file " << name.sourceFile;
               throw ParserException(str.str());
             }
         }

         ret = std::make_shared<Engine::FlowControlStatement>(buildToken, (("break" == buildToken.text) ? Engine::FlowControl::BREAK : Engine::FlowControl::CONTINUE), id, std::shared_ptr<Engine::Expression>());
       }
         break;

      case Input::RETURN:
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> expry = expression(src, table, logger);

         ret = std::make_shared<Engine::FlowControlStatement>(buildToken, Engine::FlowControl::RETURN, Engine::FlowControl::NO_TARGET, expry);
       }
         break;

      case Input::SELECT:
       {
         Input::Token buildToken = src.getNextToken();

         std::shared_ptr<Engine::Expression> control = expressionRecover(src, table, logger);

         expect(src, Input::FROM, "from");

         std::vector<std::shared_ptr<Engine::CaseContainer> > cases;
         bool elseFound = false;
         while ((false == elseFound) && ((Input::CASE == src.peekNextToken().lexeme) || (Input::ALSO == src.peekNextToken().lexeme)))
          {
            bool breaking = true;
            if (Input::ALSO == src.peekNextToken().lexeme)
             {
               breaking = false;
               src.getNextToken();
             }
            Input::Token caseToken = src.peekNextToken();
            expect(src, Input::CASE, "case");

            if (Input::FROM == src.peekNextToken().lexeme)
             {
               src.getNextToken();

               std::shared_ptr<Engine::Expression> lower = expressionRecover(src, table, logger);

               expect(src, Input::TO, "to");

               std::shared_ptr<Engine::Expression> upper = expressionRecover(src, table, logger);

               expect(src, Input::IS, "is");

               std::shared_ptr<Engine::Statement> block = innerStatementSeq(src, table, logger);

               if ((nullptr != lower.get()) && (nullptr != upper.get()) && (nullptr != block.get()))
                {
                  cases.emplace_back(std::make_shared<Engine::CaseContainer>(caseToken, breaking, Engine::CaseContainer::AT, lower, upper, block));
                }
               else
                {
                  badWrong = true;
                }
             }
            else if (Input::ELSE != src.peekNextToken().lexeme)
             {
               Engine::CaseContainer::CaseType type = Engine::CaseContainer::AT;
               if (Input::ABOVE == src.peekNextToken().lexeme)
               {
                  src.getNextToken();
                  type = Engine::CaseContainer::ABOVE;
               }
               else if (Input::BELOW == src.peekNextToken().lexeme)
               {
                  src.getNextToken();
                  type = Engine::CaseContainer::BELOW;
               }

               std::shared_ptr<Engine::Expression> current = expressionRecover(src, table, logger);

               expect(src, Input::IS, "is");

               std::shared_ptr<Engine::Statement> block = innerStatementSeq(src, table, logger);

               if ((nullptr != current.get()) && (nullptr != block.get()))
                {
                  cases.emplace_back(std::make_shared<Engine::CaseContainer>(caseToken, breaking, type, current, std::shared_ptr<Engine::Expression>(), block));
                }
               else
                {
                  badWrong = true;
                }
             }
            else // ELSE! Ah ha ha!
             {
               elseFound = true;
               src.getNextToken();
               expect(src, Input::IS, "is");

               std::shared_ptr<Engine::Statement> block = innerStatementSeq(src, table, logger);

               if (nullptr != block.get())
                {
                  cases.emplace_back(std::make_shared<Engine::CaseContainer>(caseToken, breaking, Engine::CaseContainer::AT,
                     std::shared_ptr<Engine::Expression>(), std::shared_ptr<Engine::Expression>(), block));
                }
               else
                {
                  badWrong = true;
                }
             }
          }

         expect(src, Input::END, "end");

         if ((nullptr != control.get()) && (false == badWrong))
          {
            ret = std::make_shared<Engine::SelectStatement>(buildToken, control, cases);
          }
       }
         break;

      case Input::ELSE:
      case Input::ELSEIF:
      case Input::CASE:
      case Input::ALSO:
      case Input::END:
         // I expected a statement, but got something in the follow set for a statement.
         // This implies a malformed construct somewhere: if it weren't malformed, it would have seen this.
         // Because this is in the follow set of a statement, consume it before continuing to prevent a loop.
         src.getNextToken();

      /* !!! FALL THROUGH !!! */
      default:
         // The purpose of this is to throw an exception with the correct error string.
         // We also wish to invoke error recovery, so that we don't get stuck in a bad input loop.
         expect(src, Input::LEXER_NEVER_RETURNS_THIS, "Statement");
         break;
       }

      return ret;
    }

   std::shared_ptr<Engine::Statement> Parser::innerIF (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Statement> ret;
      Input::Token buildToken = src.getNextToken();

      std::shared_ptr<Engine::Expression> condition = expressionRecover(src, table, logger);

      expect(src, Input::THEN, "then");

      std::shared_ptr<Engine::Statement> thenStat = innerStatementSeq(src, table, logger);

      std::shared_ptr<Engine::Statement> elseStat = Engine::ConstantsSingleton::getInstance().ONE_TRUE_NOP;
      if (Input::ELSEIF == src.peekNextToken().lexeme)
       {
         elseStat = innerIF(src, table, logger);
       }
      else
       {
         if (Input::ELSE == src.peekNextToken().lexeme)
          {
            src.getNextToken();
            elseStat = innerStatementSeq(src, table, logger);
          }

         expect(src, Input::END, "end");
       }

      if ((nullptr != condition.get()) && (nullptr != thenStat.get()) && (nullptr != elseStat.get()))
       {
         ret = std::make_shared<Engine::IfStatement>(buildToken, condition, thenStat, elseStat);
       }

      return ret;
    }

   std::shared_ptr<Engine::Statement> Parser::outerStatementSeq (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Statement> ret;
      bool badWrong = false;
      Input::Token token = src.peekNextToken();
      std::vector<std::shared_ptr<Engine::Statement> > statements;
      while (Input::END_OF_FILE != src.peekNextToken().lexeme)
       {
         try
          {
            std::shared_ptr<Engine::Statement> state = statement(src, table, false, logger);
            if (nullptr != state.get())
             {
               if (Engine::ConstantsSingleton::getInstance().ONE_TRUE_NOP.get() != state.get())
                {
                  statements.emplace_back(state);
                }
             }
            else
             {
               badWrong = true;
             }
          }
         catch (const ParserException& e)
          {
            logger.log(e.what());
            badWrong = true;
            recoverStatement(src);
          }
       }
      if (false == badWrong)
       {
         if (true == statements.empty())
          {
            ret = Engine::ConstantsSingleton::getInstance().ONE_TRUE_NOP;
          }
         else if (1U == statements.size())
          {
            ret = statements[0];
          }
         else
          {
            ret = std::make_shared<Engine::StatementSeq>(token, statements);
          }
       }
      return ret;
    }

   std::shared_ptr<Engine::Statement> Parser::innerStatementSeq (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Statement> ret;
      bool badWrong = false;
      Input::Token token = src.peekNextToken();
      std::vector<std::shared_ptr<Engine::Statement> > statements;
      // This is the follow set of a statement sequence within an 'if', 'while', 'switch', or 'function' statement.
      // With the exception of end-of-file, which is an erroneous condition (that will be caught by the caller).
      while ((Input::ELSE != src.peekNextToken().lexeme) &&
         (Input::ELSEIF != src.peekNextToken().lexeme) &&
         (Input::END != src.peekNextToken().lexeme) &&
         (Input::CASE != src.peekNextToken().lexeme) &&
         (Input::ALSO != src.peekNextToken().lexeme) &&
         (Input::END_OF_FILE != src.peekNextToken().lexeme))
       {
         try
          {
            std::shared_ptr<Engine::Statement> state = statement(src, table, false, logger);
            if (nullptr != state.get())
             {
               if (Engine::ConstantsSingleton::getInstance().ONE_TRUE_NOP.get() != state.get())
                {
                  statements.emplace_back(state);
                }
             }
            else
             {
               badWrong = true;
             }
          }
         catch (const ParserException& e)
          {
            logger.log(e.what());
            badWrong = true;
            recoverStatement(src);
          }
       }
      if (false == badWrong)
       {
         if (true == statements.empty())
          {
            ret = Engine::ConstantsSingleton::getInstance().ONE_TRUE_NOP;
          }
         else if (1U == statements.size())
          {
            ret = statements[0];
          }
         else
          {
            ret = std::make_shared<Engine::StatementSeq>(token, statements);
          }
       }
      return ret;
    }

   std::shared_ptr<Engine::Expression> Parser::expressionRecover (Input::Lexer& src, SymbolTable& table, Engine::Logger& logger)
    {
      std::shared_ptr<Engine::Expression> result;
      try
       {
         result = expression(src, table, logger);
       }
      catch (const ParserException& e)
       {
         logger.log(e.what());
         recoverExpression(src);
       }
      return result;
    }

 } // namespace Parser

 } // namespace Backwards
