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

   void SpreadSheet::clearCellAt(size_t col, size_t row)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            sheet[col][row].reset();
          }
       }
    }

   void SpreadSheet::clearColumn(size_t col)
    {
      if (col < sheet.size())
       {
         sheet[col].clear();
       }
    }

   void SpreadSheet::clearRow(size_t row)
    {
      for (size_t i = 0U; i < sheet.size(); ++i)
       {
         if (row < sheet[i].size())
          {
            sheet[i][row].reset();
          }
       }
    }

   void SpreadSheet::insertColumnBefore(size_t col)
    {
      if (col < sheet.size())
       {
         sheet.insert(sheet.begin() + col, std::vector<std::unique_ptr<Cell> >());
       }
    }

   void SpreadSheet::insertRowBefore(size_t row)
    {
      bool didAnything = false;
      for (size_t i = 0U; i < sheet.size(); ++i)
       {
         if (row < sheet[i].size())
          {
            sheet[i].insert(sheet[i].begin() + row, std::unique_ptr<Cell>());
            didAnything = true;
          }
       }
      if (true == didAnything)
       {
         ++max_row;
       }
    }

   void SpreadSheet::swap(size_t col1, size_t col2, size_t row)
    {
      const Cell* one = getCellAt(col1, row);
      const Cell* two = getCellAt(col2, row);

      if ((nullptr != one) || (nullptr != two))
       {
         if (col2 >= sheet.size()) // col2 > col1, always
          {
            sheet.resize(col2 + 1U);
          }
         if (row >= sheet[col1].size())
          {
            sheet[col1].resize(row + 1U);
          }
         if (row >= sheet[col2].size())
          {
            sheet[col2].resize(row + 1U);
          }
         sheet[col1][row].swap(sheet[col2][row]);
       }
    }

   void SpreadSheet::insertCellBeforeShiftRight(size_t col, size_t row)
    {
      // Bubble in an empty cell from the far right.
      for (size_t i = sheet.size(); i > col; --i)
       {
         swap(i - 1U, i, row);
       }
    }

   void SpreadSheet::insertCellBeforeShiftDown(size_t col, size_t row)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            if (sheet[col].size() == max_row)
             {
               ++max_row;
             }
            sheet[col].insert(sheet[col].begin() + row, std::unique_ptr<Cell>());
          }
       }
    }

   void SpreadSheet::removeColumn(size_t col)
    {
      if (col < sheet.size())
       {
         sheet.erase(sheet.begin() + col);
       }
    }

   void SpreadSheet::removeRow(size_t row)
    {
      for (size_t i = 0U; i < sheet.size(); ++i)
       {
         if (row < sheet[i].size())
          {
            sheet[i].erase(sheet[i].begin() + row);
          }
       }
    }

   void SpreadSheet::removeCellShiftLeft(size_t col, size_t row)
    {
      // Clear the cell and bubble it out to the far right.
      clearCellAt(col, row);
      for (size_t i = col; i < sheet.size(); ++i) // Don't optimize to sheet.size() - 1
       {
         swap(i, i + 1U, row);
       }
    }

   void SpreadSheet::removeCellShiftUp(size_t col, size_t row)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            sheet[col].erase(sheet[col].begin() + row);
          }
       }
    }


   std::string SpreadSheet::computeCell(CallingContext& context, std::shared_ptr<Types::ValueType>& OUT, size_t col, size_t row)
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
      if ((context.generation == cell->previousGeneration) && (nullptr != cell->value.get()))
       {
         OUT = cell->previousValue;
         return result;
       }

         // If this is a LABEL, then set the value.
      std::shared_ptr<Expression> value = cell->value;
      if ((LABEL == cell->type) && (nullptr == value.get()))
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
         context.topCell()->cell->recursed = false;
         OUT = value->evaluate(context);
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
       }
      catch (const std::exception& e)
       {
         result = e.what();
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
       }
      catch (...)
       {
         context.topCell()->cell->inEvaluation = false;
         context.popCell();
       }

      size_t c = result.find('\n');
      if (std::string::npos != c)
       {
         result.resize(c);
       }
      return result;
    }


   std::shared_ptr<Types::ValueType> SpreadSheet::computeCell(CallingContext& context, size_t col, size_t row, bool rethrow)
    {
      std::shared_ptr<Types::ValueType> OUT;

      Cell* cell = getCellAt(col, row);
      if (nullptr == cell)
       {
         return OUT;
       }
      CellFrame newFrame (cell, col, row);

         // If we have already evaluated this cell this generation, stop.
      if (context.generation == cell->previousGeneration)
       {
         return cell->previousValue;
       }

         // If this is a LABEL, then set the value.
      std::shared_ptr<Expression> value = cell->value;
      if ((LABEL == cell->type) && (nullptr == value.get()))
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
       }

         // If the parse failed, leave. Result will have the first parser message.
      if (nullptr == value.get())
       {
         return OUT;
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
         context.topCell()->cell->recursed = false;
         OUT = value->evaluate(context);
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
       }
      catch (...)
       {
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
         if (true == rethrow)
          {
            throw;
          }
       }

      return OUT;
    }


   void SpreadSheet::recalc(CallingContext& context)
    {
      context.inUserInput = false;
      ++context.generation;
      context.names->clear();
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
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from bottom-to-top
             {
               for (size_t col = 0U; col < sheet.size(); ++col)
                {
                  for (size_t row = sheet[col].size() - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                   {
                     (void) computeCell(context, col, row, false);
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
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from bottom-to-top
             {
               for (size_t col = sheet.size() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                {
                  for (size_t row = sheet[col].size() - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                   {
                     (void) computeCell(context, col, row, false);
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
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from right-to-left
             {
               for (size_t row = max_row - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                {
                  for (size_t col = 0U; col < sheet.size(); ++col)
                   {
                     (void) computeCell(context, col, row, false);
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
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from right-to-left
             {
               for (size_t row = max_row - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                {
                  for (size_t col = sheet.size() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
          }
       }
      ++context.generation;
    }

 } // namespace Engine

 } // namespace Forwards
