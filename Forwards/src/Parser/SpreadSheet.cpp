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
#include "Forwards/Engine/SpreadSheet.h"

#include "Backwards/Engine/Logger.h"
#include "Backwards/Input/StringInput.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

#include "Forwards/Types/ValueType.h"
#include "Forwards/Types/StringValue.h"

/*
   This is purposely in Parser because it depends on Parser.
   SpreadSheet creates a circular dependency between Parser and Engine, and I don't like it.
*/

namespace Forwards
 {

namespace Engine
 {

   SpreadSheet::SpreadSheet() : max_row(0U), c_major(true), top_down(true), left_right(true)
    {
    }

   Cell* SpreadSheet::getCellAt(size_t col, size_t row)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            return sheet[col][row].get();
          }
       }
      return nullptr;
    }

   void SpreadSheet::initCellAt(size_t col, size_t row)
    {
      if (col >= sheet.size())
       {
         sheet.resize(col + 1U);
       }
      if (row >= sheet[col].size())
       {
         sheet[col].resize(row + 1U);
         if (row >= max_row)
          {
            max_row = row + 1;
          }
       }
      sheet[col][row] = std::make_unique<Forwards::Engine::Cell>();
    }

   void SpreadSheet::removeCellAt(size_t col, size_t row)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            sheet[col][row].reset();
          }
       }
    }

   std::string SpreadSheet::computeCell(CallingContext& context, std::shared_ptr<Types::ValueType>& OUT, size_t col, size_t row, bool rethrow)
    {
      std::string result;
      OUT.reset(); // Ensure to clear OUT variable.

      Cell* cell = getCellAt(col, row);
      if (nullptr == cell)
       {
         return result;
       }
      CellFrame newFrame (cell, col, row);

         // If we have already evaluated this cell this generation, stop.
      if ((context.generation == cell->previousGeneration) && (context.inUserInput == rethrow))
       {
         OUT = cell->previousValue;
         return result;
       }

         // If this is a LABEL, then set the value.
      std::shared_ptr<Expression> value = cell->value;
      if ((LABEL == cell->type) && (nullptr == cell->value.get()))
       {
         value = std::make_shared<Constant>(Input::Token(), std::make_shared<Types::StringValue>(cell->currentInput));
       }
         // Else, this is a VALUE, and we need to parse it.
      if (nullptr == value.get())
       {
         Backwards::Input::StringInput interlinked (cell->currentInput);
         Input::Lexer lexer (interlinked);
         Backwards::Engine::Logger* temp = context.logger;
         Parser::StringLogger newLogger;
         context.logger = &newLogger;
         value = Parser::Parser::ParseFullExpression(lexer, *context.map, *context.logger, col, row);
         context.logger = temp;
         if (newLogger.logs.size() > 0U)
          {
            result = newLogger.logs[0U];
          }
       }

         // If the parse failed, leave. Result will have the first parser message.
      if (nullptr == value.get())
       {
         return result;
       }

         // If this is a regular update, update the cell. Eww....
      if (false == context.inUserInput)
       {
         cell->currentInput = "";
         cell->value = value;
       }

      try
       {
         context.pushCell(&newFrame);
            // Evaluate the new cell.
         context.topCell()->cell->inEvaluation = true;
         OUT = value->evaluate(context);
         context.topCell()->cell->inEvaluation = false;
            // If we are doing regular evaluation passes, set this as the current value.
         if (false == context.inUserInput)
          {
            context.topCell()->cell->previousGeneration = context.generation;
            context.topCell()->cell->previousValue = OUT;
          }
         context.popCell();
       }
      catch (const std::exception& e)
       {
         result = e.what();
         context.topCell()->cell->inEvaluation = false;
         context.popCell();
         if (true == rethrow)
          {
            throw;
          }
       }
      catch (...)
       {
         context.topCell()->cell->inEvaluation = false;
         context.popCell();
         if (true == rethrow)
          {
            throw;
          }
       }

      size_t c = result.find('\n');
      if (std::string::npos != c)
       {
         result = result.substr(0U, c);
       }
      return result;
    }

   void SpreadSheet::recalc(CallingContext& context)
    {
      context.inUserInput = false;
      ++context.generation;
      if (c_major) // Going in column-major order
       {
         if (left_right) // Going from left-to-right
          {
            if (top_down) // Going from top-to-bottom
             {
               for (size_t col = 0U; col < sheet.size(); ++col)
                {
                  for (size_t row = 0U; row < sheet[col].size(); ++row)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
            else // Going from bottom-to-top
             {
               for (size_t col = 0U; col < sheet.size(); ++col)
                {
                  for (size_t row = sheet[col].size() - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
          }
         else // Going from right-to-left
          {
            if (top_down) // Going from top-to-bottom
             {
               for (size_t col = sheet.size() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                {
                  for (size_t row = 0U; row < sheet[col].size(); ++row)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
            else // Going from bottom-to-top
             {
               for (size_t col = sheet.size() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                {
                  for (size_t row = sheet[col].size() - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
          }
       }
      else // Going in row major order
       {
         if (top_down) // Going from top-to-bottom
          {
            if (left_right) // Going from left-to-right
             {
               for (size_t row = 0U; row < max_row; ++row)
                {
                  for (size_t col = 0U; col < sheet.size(); ++col)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
            else // Going from right-to-left
             {
               for (size_t row = max_row - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                {
                  for (size_t col = 0U; col < sheet.size(); ++col)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
          }
         else // Going from bottom-to-top
          {
            if (left_right) // Going from left-to-right
             {
               for (size_t row = 0U; row < max_row; ++row)
                {
                  for (size_t col = sheet.size() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
            else // Going from right-to-left
             {
               for (size_t row = max_row - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                {
                  for (size_t col = sheet.size() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                   {
                     std::shared_ptr<Types::ValueType> trash;
                     (void) computeCell(context, trash, col, row, false);
                   }
                }
             }
          }
       }
      ++context.generation;
    }

 } // namespace Engine

 } // namespace Forwards
