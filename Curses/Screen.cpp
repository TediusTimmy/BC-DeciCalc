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
#include <ncurses.h>

#include <chrono>
#include <thread>

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"

#include "Forwards/Types/ValueType.h"
#include "Forwards/Types/StringValue.h"
#include "Forwards/Types/FloatValue.h"

#include "Screen.h"
#include "GetAndSet.h"

const int RECALC_POLL_MILLIS = 40; // 25 Hz
const int NO_INPUT_SLEEP_MILLIS = 10; // 100 Hz (when no input; always AFAP when processing input)

const size_t MAX_ROW = 999999998U; // Yes, minus one.
const size_t MAX_COL = 18277U;

volatile bool blinky = true;
std::thread updateThread;

void GetRC(const std::string& from, int64_t& col, int64_t& row)
 {
   const char * iter = from.c_str();
   int alphas = 1;
   if (!std::isalpha(*iter))
    {
      row = -1;
      col = -1;
      return;
    }
   col = (*iter & ~' ') - 'A';
   ++iter;
   if (std::isalpha(*iter))
    {
      col = (col * 26) + ((*iter & ~' ') - 'A');
      ++iter;
      ++alphas;
    }
   if (std::isalpha(*iter))
    {
      col = (col * 26) + ((*iter & ~' ') - 'A');
      ++iter;
      ++alphas;
    }
   if (3 == alphas)
    {
      col += 26 * 26 + 26;
    }
   else if (2 == alphas)
    {
      col += 26;
    }
   if (!std::isdigit(*iter))
    {
      row = -1;
      col = -1;
      return;
    }
   row = std::atoll(iter) - 1;
   if (static_cast<size_t>(row) > MAX_ROW)
    {
      row = -1;
      col = -1;
    }
 }

std::string setComma(const std::string& str, bool useComma)
 {
   std::string result = str;
   if (true == useComma)
    {
      size_t c = result.find('.');
      while (std::string::npos != c)
       {
         result[c] = ',';
         c = result.find('.', c);
       }
    }
   return result;
 }

std::string getStringPreviousValue(Forwards::Engine::Cell* curCell, SharedData& data)
 {
   std::string content = curCell->previousValue->toString(data.c_col, data.c_row);
   if (Forwards::Engine::VALUE == curCell->type) content = setComma(content, data.useComma);
   return content;
 }

std::string getStringDisplayValue(Forwards::Engine::Cell* curCell, SharedData& data)
 {
   std::string content ("ERROR");
   if (Forwards::Engine::VALUE == curCell->type) content = setComma(curCell->value->toString(data.c_col, data.c_row), data.useComma);
   else if (Forwards::Engine::LABEL == curCell->type) content = curCell->value->evaluate(*data.context)->toString(data.c_col, data.c_row);
   return content;
 }

void threadrun (SharedData& data)
 {
   std::chrono::system_clock::time_point last;
   for (;;)
    {
      if (true == blinky)
       {
         data.context->theSheet->recalc(*data.context);
         blinky = false;
       }
      last = std::chrono::system_clock::now() + std::chrono::milliseconds(RECALC_POLL_MILLIS);
      std::this_thread::sleep_until(last);
    }
 }

void InitScreen(SharedData& data)
 {
   initscr();
   start_color();

   cbreak();
   keypad(stdscr, TRUE);
   noecho();
   nonl();
   nodelay(stdscr, TRUE);

   init_pair(1, COLOR_WHITE, COLOR_BLUE);
   init_pair(2, COLOR_BLACK, COLOR_WHITE);
   init_pair(3, COLOR_WHITE, COLOR_BLACK);
   init_pair(4, COLOR_BLUE, COLOR_BLACK);
   init_pair(5, COLOR_WHITE, COLOR_RED);

   updateThread = std::thread(threadrun, std::ref(data));
   updateThread.detach();
 }

void UpdateScreen(SharedData& data)
 {
   int x, y, mx, my;
   getmaxyx(stdscr, y, x); // CODING HORROR!!!
   mx = 0;
   my = 2;

   move(0, 0);
         // Line 1
    {
      attron(COLOR_PAIR(2));
      std::string location = Forwards::Types::ValueType::columnToString(data.c_col) + std::to_string(data.c_row + 1);
      printw("%s", location.c_str());
      for (int i = 12 - location.size(); i > 0; --i) addch(' ');
      addch(' ');
      Forwards::Engine::Cell* curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
      if (nullptr != curCell)
       {
         if (Forwards::Engine::VALUE == curCell->type)
          {
            printw("VALUE ");
          }
         else // Must be a label.
          {
            printw("LABEL ");
          }

         if (nullptr != curCell->previousValue)
          {
            std::string content = getStringPreviousValue(curCell, data);
            if (content.size() > static_cast<size_t>(x - 23)) content.resize(x - 23);
            printw("%s", content.c_str());
            for (int i = (x - 22 - content.size()); i > 0; --i) addch(' ');
          }
         else if (nullptr == curCell->value.get())
          {
            for (int i = x - 22; i > 0; --i) addch(' ');
          }
         else
          {
            attron(COLOR_PAIR(5));
            for (int i = x - 22; i > 0; --i) addch(' ');
            attron(COLOR_PAIR(2));
          }
       }
      else
       {
         for (int i = x - 16; i > 0; --i) addch(' ');
       }
      if (true == blinky)
       {
         addch('#');
       }
      else
       {
         addch(' ');
       }
      if (data.context->theSheet->c_major)
       {
         addch(data.context->theSheet->top_down ? 'T' : 'B');
         addch(data.context->theSheet->left_right ? 'L' : 'R');
       }
      else
       {
         addch(data.context->theSheet->left_right ? 'L' : 'R');
         addch(data.context->theSheet->top_down ? 'T' : 'B');
       }
    }
         // Line 2
    {
      Forwards::Engine::Cell* curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
      if (nullptr != curCell)
       {
            // unfinished VALUE : parse current contents
         if ((false == blinky) && (Forwards::Engine::VALUE == curCell->type) && (nullptr == curCell->value))
          {
            data.context->inUserInput = true;
            --data.context->generation;
            if (false == data.tempString.empty()) curCell->currentInput = data.tempString;
            std::shared_ptr<Forwards::Types::ValueType> result;
            std::string content = data.context->theSheet->computeCell(*data.context, result, data.c_col, data.c_row);
            ++data.context->generation;
            if (nullptr != result.get())
             {
               content = result->toString(data.c_col, data.c_row);
             }
            content = setComma(content, data.useComma);
            if (content.size() > static_cast<size_t>(x - 1)) content.resize(x - 1);
            printw("%s", content.c_str());
            for (int i = (x - content.size()); i > 0; --i) addch(' ');
            if (false == data.tempString.empty())
             {
               curCell->value.reset();
               curCell->previousValue.reset();
             }
          }
            // finished VALUE or LABEL
         else if (nullptr != curCell->value)
          {
            std::string content = getStringDisplayValue(curCell, data);
            if (content.size() > static_cast<size_t>(x - 1)) content.resize(x - 1);
            printw("%s", content.c_str());
            for (int i = (x - content.size()); i > 0; --i) addch(' ');
          }
            // unfinished LABEL : show prior contents
         else
          {
            std::string content = data.origString;
            if (content.size() > static_cast<size_t>(x - 1)) content.resize(x - 1);
            printw("%s", content.c_str());
            for (int i = (x - content.size()); i > 0; --i) addch(' ');
          }
       }
      else
       {
         for (int i = 0; i < x; ++i) addch(' ');
       }
    }
         // Line 3
    {
      attron(COLOR_PAIR(1));
      Forwards::Engine::Cell* curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
      if (true == data.inputMode)
       {
         std::string content = data.tempString.substr(data.baseChar, std::string::npos);
         if (content.size() > static_cast<size_t>(x))
          {
            content.resize(x);
          }
         mx = data.editChar - data.baseChar;
         printw("%s", content.c_str());
         for (int i = (x - content.size()); i > 0; --i) addch(' ');
       }
      else if (nullptr != curCell)
       {
         if (nullptr != curCell->value.get())
          {
            std::string content = getStringDisplayValue(curCell, data);
            if (content.size() > static_cast<size_t>(x - 1)) content.resize(x - 1);
            printw("%s", content.c_str());
            for (int i = (x - content.size()); i > 0; --i) addch(' ');
          }
         else if ("" != curCell->currentInput)
          {
            std::string content = curCell->currentInput;
            if (content.size() > static_cast<size_t>(x - 5))
             {
               content = content.substr(content.size() - x + 5, std::string::npos);
             }
            printw("%s", content.c_str());
            for (int i = (x - content.size()); i > 0; --i) addch(' ');
          }
         else
          {
            for (int i = 0; i < x; ++i) addch(' ');
          }
       }
      else
       {
         for (int i = 0; i < x; ++i) addch(' ');
       }
    }
         // Line 4
    {
      attron(COLOR_PAIR(2));
      printw("   ");
      int cx = 3;
      size_t cc = 0U + data.tr_col;
      for (; cc <= MAX_COL;)
       {
         int nextWidth = getWidth(data.col_widths, cc, data.def_col_width);
         if (cx + nextWidth <= x)
          {
            if (cc == data.c_col) attron(COLOR_PAIR(4));
            cx += nextWidth;
            std::string colName = Forwards::Types::ValueType::columnToString(cc);
            while (static_cast<int>(colName.size()) > nextWidth) colName = colName.substr(1U, std::string::npos);
            for (int i = 0; i < static_cast<int>((nextWidth - colName.size()) >> 1); ++i) addch(' ');
            printw("%s", colName.c_str());
            for (int i = 0; i < static_cast<int>((nextWidth - colName.size()) >> 1); ++i) addch(' ');
            if ((nextWidth - colName.size()) & 1U) addch(' ');
            if (cc == data.c_col) attron(COLOR_PAIR(2));
          }
         else
          {
            break;
          }
         ++cc;
       }
      attron(COLOR_PAIR(3));
      for (; cx < x; ++cx) addch(' ');
    }

      // All other lines.
   for (int j = 4; j < y; ++j)
    {
      size_t cr = data.tr_row + j - 4;
      if (cr == data.c_row)
       {
         attron(COLOR_PAIR(4));
       }
      else
       {
         attron(COLOR_PAIR(2));
       }
      std::string rowName = std::to_string(cr + 1);
      while (rowName.size() < 3U) rowName = " " + rowName;
      if (rowName.size() > 3U) rowName = rowName.substr(rowName.size() - 3U, std::string::npos);
      printw("%s", rowName.c_str());
      int cx = 3;
      size_t cc = data.tr_col;
      for (; cc <= MAX_COL;)
       {
         int nextWidth = getWidth(data.col_widths, cc, data.def_col_width);
         if (cx + nextWidth <= x)
          {
            if ((data.c_col == cc) && (data.c_row == cr))
             {
               attron(COLOR_PAIR(1));
               if (false == data.inputMode)
                {
                  mx = (2 * cx + nextWidth) / 2; // Middle of the cell
                  my = j;
                }
             }
            else if ((data.c_col == cc) || (data.c_row == cr))
             {
               attron(COLOR_PAIR(2));
             }
            else
             {
               attron(COLOR_PAIR(1));
             }
            cx += nextWidth;
            Forwards::Engine::Cell* curCell = data.context->theSheet->getCellAt(cc, cr);
            if (nullptr != curCell)
             {
               if (true == curCell->recursed)
                {
                  attron(COLOR_PAIR(5));
                }
               if (nullptr != curCell->previousValue)
                {
                  std::string content = getStringPreviousValue(curCell, data);
                  if (content.size() > static_cast<size_t>(nextWidth))
                   {
                     if (Forwards::Types::FLOAT == curCell->previousValue->getType()) // Make numbers note that they are truncated.
                      {
                        content.resize(nextWidth - 1);
                        content += "#";
                      }
                     else // Truncate strings
                      {
                        content.resize(nextWidth);
                      }
                   }
                  if (content.size() < static_cast<size_t>(nextWidth))
                   {
                     if (Forwards::Types::FLOAT == curCell->previousValue->getType()) // Left pad numbers
                      {
                        while (content.size() < static_cast<size_t>(nextWidth)) content = " " + content;
                      }
                     else // Right pad strings
                      {
                        while (content.size() < static_cast<size_t>(nextWidth)) content += " ";
                      }
                   }
                  printw("%s", content.c_str());
                }
               else if (("" != curCell->currentInput) || (nullptr != curCell->value.get()))
                {
                  attron(COLOR_PAIR(5));
                  for (int i = 0; i < static_cast<int>((nextWidth - 3) >> 1); ++i) addch(' ');
                  std::string temp = "***";
                  if (temp.size() > static_cast<size_t>(nextWidth)) temp.resize(nextWidth);
                  printw("%s", temp.c_str());
                  for (int i = 0; i < static_cast<int>((nextWidth - 3) >> 1); ++i) addch(' ');
                  if ((nextWidth > 3) && ((nextWidth - 3) & 1U)) addch(' ');
                }
               else
                {
                  for (int i = 0; i < nextWidth; ++i) addch(' ');
                }
             }
            else
             {
               for (int i = 0; i < nextWidth; ++i) addch(' ');
             }
          }
         else
          {
            break;
          }
         ++cc;
       }
      attron(COLOR_PAIR(3));
      for (; cx < x; ++cx) addch(' ');
    }

   move(my, mx);
   refresh();
 }

size_t CountColumns(const SharedData& data, size_t fromHere, int x)
 {
   size_t tc = 0U;
   size_t cc = fromHere;
   int cx = 3;
   for (; cc <= MAX_COL;)
    {
      int nextWidth = getWidth(data.col_widths, cc, data.def_col_width);
      ++cc;
      if (cx + nextWidth <= x)
       {
         ++tc;
         cx += nextWidth;
       }
      else
       {
         break;
       }
    }
   return tc;
 }

size_t CountColumnsLeft(const SharedData& data, size_t fromHere, int x)
 {
   size_t tc = 0U;
   size_t cc = fromHere;
   int cx = 3;
   for (;;)
    {
      int nextWidth = getWidth(data.col_widths, cc, data.def_col_width);
      if (cx + nextWidth <= x)
       {
         ++tc;
         cx += nextWidth;
       }
      else
       {
         break;
       }
      if (0U != cc)
       {
         --cc;
       }
      else
       {
         break;
       }
    }
   return tc;
 }

void doMove(SharedData& data)
 {
   int x, y;
   getmaxyx(stdscr, y, x); // CODING HORROR!!!
   int64_t row, col;
   GetRC(data.tempString, col, row);
   if ((-1 == col) || (-1 == row))
      return;
   size_t cl = CountColumnsLeft(data, MAX_COL, x);
   data.c_col = col;
   data.tr_col = col;
   data.c_row = row;
   data.tr_row = row;
   if (((MAX_COL - cl) < static_cast<size_t>(col)) && (static_cast<size_t>(col) <= MAX_COL)) data.tr_col = MAX_COL - cl + 1;
   if ((data.tr_row + y - 4) > MAX_ROW) data.tr_row = MAX_ROW - y + 5;
 }

bool updateChOrFail(int& c, SharedData& data)
 {
   if (false == data.inputBuffer.empty())
    {
      c = data.inputBuffer.front();
      data.inputBuffer.pop_front();
      return true;
    }
   data.inputBuffer.push_front(c);
   return false;
 }

int ProcessInput(SharedData& data)
 {
   int returnValue = 1;
   int x, y;
   getmaxyx(stdscr, y, x); // CODING HORROR!!!

   size_t tc = CountColumns(data, data.tr_col, x);
   Forwards::Engine::Cell* curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);

   int c = getch();
   while (ERR != c)
    {
      data.inputBuffer.push_back(c);
      c = getch();
    }
   if (false == data.inputBuffer.empty())
    {
      c = data.inputBuffer.front();
      data.inputBuffer.pop_front();
    }

   if (ERR == c)
    {
      std::chrono::system_clock::time_point last;
      last = std::chrono::system_clock::now() + std::chrono::milliseconds(NO_INPUT_SLEEP_MILLIS);
      std::this_thread::sleep_until(last);
    }

   if (true == data.inputMode)
    {
      bool done = true;
      if ((c >= ' ') && (c <= '~'))
       {
         if (data.editChar == data.tempString.size())
          {
            data.tempString += c;
          }
         else
          {
            if (true == data.insertMode)
             {
               data.tempString = data.tempString.substr(0U, data.editChar) + static_cast<char>(c) + data.tempString.substr(data.editChar, std::string::npos);
             }
            else
             {
               data.tempString[data.editChar] = c;
             }
          }
         ++data.editChar;

         if (data.editChar > (x - 5U + data.baseChar))
          {
            ++data.baseChar;
          }

         if ((nullptr != curCell) && (Forwards::Engine::VALUE == curCell->type))
          {
            if ('.' == c) data.useComma = false;
            if (',' == c) data.useComma = true;
          }
       }
      else if ((c == KEY_BACKSPACE) || (c == '\b') || (c == 0177))
       {
         if (("" != data.tempString) && (0U != data.editChar))
          {
            if (data.editChar == data.tempString.size())
             {
               if (data.tempString.size() > 1U)
                {
                  data.tempString = data.tempString.substr(0U, data.tempString.size() - 1U);
                }
               else
                {
                  data.tempString = "";
                }
             }
            else
             {
               data.tempString = data.tempString.substr(0U, data.editChar - 1U) + data.tempString.substr(data.editChar, std::string::npos);
             }
            --data.editChar;

            if ((data.editChar + data.baseChar) > data.tempString.size())
             {
               --data.baseChar;
             }
          }
       }
      else if (c == KEY_DC)
       {
         if ("" != data.tempString)
          {
            if (data.editChar != data.tempString.size())
             {
               if (data.tempString.size() > 1U)
                {
                  data.tempString = data.tempString.substr(0U, data.editChar) + data.tempString.substr(data.editChar + 1U, std::string::npos);
                }
               else
                {
                  data.tempString = "";
                }

               if ((data.editChar + data.baseChar) > data.tempString.size())
                {
                  --data.baseChar;
                }
             }
          }
       }
      else if (c == KEY_LEFT)
       {
         if (0U != data.editChar)
          {
            --data.editChar;

            if ((data.editChar == (data.baseChar + 5U)) && (0U != data.baseChar))
             {
               --data.baseChar;
             }
          }
       }
      else if (c == KEY_RIGHT)
       {
         if (data.editChar != data.tempString.size())
          {
            ++data.editChar;

            if (data.editChar > (x - 5U + data.baseChar))
             {
               ++data.baseChar;
             }
          }
       }
      else if (c == KEY_IC)
       {
         data.insertMode = !data.insertMode;
         if (true == data.insertMode) // This doesn't work in Cygwin.
          {
            curs_set(1);
          }
         else
          {
            curs_set(2);
          }
       }
      else if (c == KEY_HOME)
       {
         data.baseChar = 0U;
         data.editChar = 0U;
       }
      else if (c == KEY_END)
       {
         data.editChar = data.tempString.size();
         if (data.editChar > (x - 5U))
          {
            data.baseChar = data.editChar - x + 5U;
          }
         else
          {
            data.baseChar = 0U;
          }
       }
      else if ((c == '\n') || (c == '\r') || (c == KEY_ENTER))
       {
         data.inputMode = false;
         if (CELL_MODIFICATION == data.mode)
          {
            curCell->currentInput = data.tempString;
            curCell->value.reset();
            curCell->previousValue.reset();
            blinky = true;
          }
         else if (GOTO_CELL == data.mode)
          {
            doMove(data);
          }
         data.tempString = "";
         data.origString = "";
       }
      else if ((KEY_DOWN == c) || (KEY_UP == c) || (KEY_NPAGE == c) || (KEY_PPAGE == c))
       {
         if (CELL_MODIFICATION == data.mode)
          {
            data.inputMode = false;
            curCell->currentInput = data.tempString;
            data.tempString = "";
            data.origString = "";
            blinky = true;
            done = false;
            if (KEY_NPAGE == c)
             {
               c = KEY_RIGHT;
             }
            else if (KEY_PPAGE == c)
             {
               c = KEY_LEFT;
             }
          }
       }
      else if (27 == c) // ESCape Key
       {
         data.inputMode = false;
         if (CELL_MODIFICATION == data.mode)
          {
            curCell->currentInput = data.origString;
          }
         data.tempString = "";
         data.origString = "";
       }

      if (true == done)
       {
         return returnValue;
       }
    }

   switch (c)
    {
   case 'g':
      data.inputMode = true;
      data.tempString = "";
      data.mode = GOTO_CELL;

      data.baseChar = 0U;
      data.editChar = 0U;
      break;
   case 'j':
   case KEY_DOWN:
      if (MAX_ROW != data.c_row)
       {
         ++data.c_row;
         if ((static_cast<int>(data.c_row - data.tr_row)) >= (y - 4)) ++data.tr_row;
       }
      break;
   case 'k':
   case KEY_UP:
      if (0U != data.c_row)
       {
         --data.c_row;
         if (data.c_row < data.tr_row) --data.tr_row;
       }
      break;
   case 'h':
   case KEY_LEFT:
      if (0U != data.c_col)
       {
         --data.c_col;
         if (data.c_col < data.tr_col) --data.tr_col;
       }
      break;
   case 'l':
   case KEY_RIGHT:
      if (MAX_COL != data.c_col)
       {
         ++data.c_col;
         if ((data.c_col - data.tr_col) >= tc)
          {
            size_t cl = CountColumnsLeft(data, data.c_col, x);
            data.tr_col = data.c_col - cl + 1U;
          }
       }
      break;
   case 'J':
   case KEY_NPAGE:
      data.c_row += (y - 4);
      data.tr_row += (y - 4);
      if (data.c_row > MAX_ROW)
       {
         data.c_row = MAX_ROW;
       }
      if ((data.tr_row + y - 4) > MAX_ROW)
       {
         data.tr_row = MAX_ROW - y + 5;
       }
      break;
   case 'K':
   case KEY_PPAGE:
      if (data.c_row < static_cast<size_t>(y - 4))
       {
         data.c_row = 0U;
       }
      else
       {
         data.c_row -= (y - 4);
       }
      if (data.tr_row < static_cast<size_t>(y - 4))
       {
         data.tr_row = 0U;
       }
      else
       {
         data.tr_row -= (y - 4);
       }
      break;
   case 'H':
    {
      size_t cl = CountColumnsLeft(data, data.tr_col, x);
      if (data.tr_col > cl)
       {
         data.tr_col -= cl;
         data.c_col = data.tr_col;
       }
      else
       {
         data.c_col = 0;
         data.tr_col = 0;
       }
    }
      break;
   case 'L':
      data.tr_col += tc;
      data.c_col = data.tr_col;
      if (data.tr_col > MAX_COL)
       {
         size_t cl = CountColumnsLeft(data, MAX_COL, x);
         data.c_col = MAX_COL;
         data.tr_col = MAX_COL - cl + 1;
       }
      break;
   case KEY_HOME:
      data.c_col = 0U;
      data.tr_col = 0U;
      data.c_row = 0U;
      data.tr_row = 0U;
      break;
   case '<':
      if (true == blinky) break;
    {
      if (nullptr == curCell)
       {
         data.context->theSheet->initCellAt(data.c_col, data.c_row);
         curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
       }
      if (("" == curCell->currentInput) && (nullptr != curCell->value.get()))
       {
         curCell->currentInput = getStringDisplayValue(curCell, data);
       }
      data.origString = curCell->currentInput;
      curCell->type = Forwards::Engine::LABEL;
      curCell->currentInput = "";
      curCell->value.reset();
      data.inputMode = true;
      data.tempString = "";
      data.mode = CELL_MODIFICATION;
      data.baseChar = 0U;
      data.editChar = 0U;
    }
      break;
   case '=':
      if (true == blinky) break;
    {
      if (nullptr == curCell)
       {
         data.context->theSheet->initCellAt(data.c_col, data.c_row);
         curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
       }
      if (("" == curCell->currentInput) && (nullptr != curCell->value.get()))
       {
         curCell->currentInput = getStringDisplayValue(curCell, data);
       }
      data.origString = curCell->currentInput;
      curCell->type = Forwards::Engine::VALUE;
      curCell->currentInput = "";
      curCell->value.reset();
      data.inputMode = true;
      data.tempString = "";
      data.mode = CELL_MODIFICATION;
      data.baseChar = 0U;
      data.editChar = 0U;
    }
      break;
   case 'q':
   case KEY_F(7):
      if (false == updateChOrFail(c, data)) break;
      if ('y' == c)
       {
         data.saveRequested = true;
         returnValue = 0;
       }
      else if ('n' == c)
       {
         returnValue = 0;
       }
      break;
   case '!':
      blinky = true;
      break;
   case 'd':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      switch (c)
       {
      case 'd':
         data.context->theSheet->clearCellAt(data.c_col, data.c_row);
         break;
      case 'c':
         data.context->theSheet->clearColumn(data.c_col);
         break;
      case 'r':
         data.context->theSheet->clearRow(data.c_row);
         break;
       }
      blinky = true;
      break;
   case 'y':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      if ('y' == c)
       {
         if ((nullptr != curCell) && (nullptr != curCell->value.get()))
          {
            data.yankedType = curCell->type;
            data.yanked = curCell->value;
          }
       }
      break;
   case 'p':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      if ('p' == c)
       {
         if (nullptr == curCell)
          {
            data.context->theSheet->initCellAt(data.c_col, data.c_row);
            curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
          }
         curCell->type = data.yankedType;
         curCell->value = data.yanked;
         blinky = true;
       }
      break;
   case 'e':
      if (true == blinky) break;
    {
      if (nullptr != curCell)
       {
         if (("" == curCell->currentInput) && (nullptr != curCell->value.get()))
          {
            curCell->currentInput = getStringDisplayValue(curCell, data);
            curCell->value.reset();
          }
         data.origString = curCell->currentInput;

         data.editChar = curCell->currentInput.size();
         if (data.editChar > (x - 5U))
          {
            data.baseChar = data.editChar - x + 5U;
          }
         else
          {
            data.baseChar = 0U;
          }

         data.inputMode = true;
         data.tempString = curCell->currentInput;
         data.mode = CELL_MODIFICATION;
       }
    }
      break;
   case 'W':
      data.saveRequested = true;
      break;
   case KEY_F(9):
   case KEY_SLEFT:
      decWidth(data.col_widths, data.c_col, data.def_col_width);
      break;
   case KEY_F(12):
   case KEY_SRIGHT:
      incWidth(data.col_widths, data.c_col, data.def_col_width);
      if ((CountColumns(data, data.tr_col, x) != tc) && (data.c_col == (data.tr_col +  tc - 1)))
       {
         data.tr_col++;
       }
      break;
   case '#':
      data.context->theSheet->c_major = !data.context->theSheet->c_major;
      break;
   case '$':
      data.context->theSheet->top_down = !data.context->theSheet->top_down;
      break;
   case '%':
      data.context->theSheet->left_right = !data.context->theSheet->left_right;
      break;
   case ',':
      data.useComma = !data.useComma;
      break;
   case '+':
      if (true == blinky) break;
    {
      if (nullptr == curCell)
       {
         data.context->theSheet->initCellAt(data.c_col, data.c_row);
         curCell = data.context->theSheet->getCellAt(data.c_col, data.c_row);
         curCell->type = Forwards::Engine::VALUE;

         data.origString = "";

         data.baseChar = 0U;
         data.editChar = 0U;
       }
      else
       {
         if (("" == curCell->currentInput) && (nullptr != curCell->value.get()))
          {
            curCell->currentInput = getStringDisplayValue(curCell, data);
            curCell->value.reset();
          }
         data.origString = curCell->currentInput;

         if (Forwards::Engine::VALUE == curCell->type)
          {
            curCell->currentInput += "+";
          }

         data.editChar = curCell->currentInput.size();
         if (data.editChar > (x - 5U))
          {
            data.baseChar = data.editChar - x + 5U;
          }
         else
          {
            data.baseChar = 0U;
          }
       }
      data.inputMode = true;
      data.tempString = curCell->currentInput;
      data.mode = CELL_MODIFICATION;
    }
      break;
   case ':':
      if (false == updateChOrFail(c, data)) break;
      switch (c)
       {
      case ')':
         data.c_col = 0U;
         data.tr_col = 0U;
         break;
      case '^':
         data.c_row = 0U;
         data.tr_row = 0U;
         break;
      case '$':
       {
         size_t maxCol = data.context->theSheet->sheet.size();
         while (nullptr == data.context->theSheet->getCellAt(maxCol, data.c_row))
          {
            if (0U != maxCol)
             {
               --maxCol;
             }
            else
             {
               break;
             }
          }
         data.c_col = maxCol;
         data.tr_col = data.c_col - CountColumnsLeft(data, data.c_col, x) + 1;
       }
         break;
      case '#':
       {
         size_t maxRow = 0U;
         if (data.c_col < data.context->theSheet->sheet.size())
          {
            maxRow = data.context->theSheet->sheet[data.c_col].size();
          }
         while (nullptr == data.context->theSheet->getCellAt(data.c_col, maxRow))
          {
            if (0 != maxRow)
             {
               --maxRow;
             }
            else
             {
               break;
             }
          }
         data.c_row = maxRow;
         if (maxRow < static_cast<size_t>(y - 4))
          {
            data.tr_row = 0U;
          }
         else
          {
            data.tr_row = maxRow - y + 5;
          }
       }
         break;
       }
      break;
   case 'x':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      switch (c)
       {
      case 'x':
         data.context->theSheet->removeCellShiftUp(data.c_col, data.c_row);
         break;
      case 'X':
         data.context->theSheet->removeCellShiftLeft(data.c_col, data.c_row);
         break;
      case 'c':
         data.context->theSheet->removeColumn(data.c_col);
         removeColumn(data.col_widths, data.c_col);
         break;
      case 'r':
         data.context->theSheet->removeRow(data.c_row);
         break;
       }
      blinky = true;
      break;
   case 'i':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      switch (c)
       {
      case 'i':
         data.context->theSheet->insertCellBeforeShiftDown(data.c_col, data.c_row);
         break;
      case 'c':
         data.context->theSheet->insertColumnBefore(data.c_col);
         insertColumnBefore(data.col_widths, data.c_col, data.def_col_width);
         break;
      case 'r':
         data.context->theSheet->insertRowBefore(data.c_row);
         break;
       }
      blinky = true;
      break;
   case 'o':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      switch (c)
       {
      case 'o':
         data.context->theSheet->insertCellBeforeShiftRight(data.c_col, data.c_row);
         break;
      case 'c':
         data.context->theSheet->insertColumnBefore(data.c_col + 1U);
         insertColumnBefore(data.col_widths, data.c_col + 1U, data.def_col_width);
         break;
      case 'r':
         data.context->theSheet->insertRowBefore(data.c_row + 1U);
         break;
       }
      blinky = true;
      break;
   case 'v':
      if (false == updateChOrFail(c, data)) break;
      if (true == blinky) break;
      if ('v' == c)
       {
         if (nullptr != curCell)
          {
            if (("" == curCell->currentInput) && (nullptr != curCell->value.get()) && (nullptr != curCell->previousValue.get()))
             {
               curCell->currentInput = getStringPreviousValue(curCell, data);
               curCell->value.reset();
             }
          }
       }
      break;
   case '`':
      endwin();
      break;
    }

   return returnValue;
 }

void WaitToSave(void)
 {
   std::chrono::system_clock::time_point last;
   while (true == blinky)
    {
      last = std::chrono::system_clock::now() + std::chrono::milliseconds(RECALC_POLL_MILLIS);
      std::this_thread::sleep_until(last);
    }
 }

void DestroyScreen(void)
 {
   endwin();
 }
