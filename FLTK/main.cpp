/*
BSD 3-Clause License

Copyright (c) 2024, Thomas DiModica
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
// Include these first, so that we don't have to undefine IN and ERROR
#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

// Don't put OddsAndEnds on the include path, because stdlib.h and StdLib.h are the same on Windows,
// and FLTK includes stdlib.h somehow, probably through windows.h.
#include "../OddsAndEnds/GetAndSet.h"
#include "../OddsAndEnds/LibraryLoader.h"
#include "../OddsAndEnds/SaveFile.h"

// Actually, we use ERROR. So, redefine it for this file.
static const Forwards::Engine::CellType UNREAL_ERROR = Forwards::Engine::ERROR;

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>

#include <cstring>
#include <fstream>

#include <chrono>
#include <thread>
#include <atomic>

const int RECALC_POLL_MILLIS = 40; // 25 Hz
const size_t MAX_ROW = 999999998U; // Yes, minus one.
const size_t MAX_COL = 18277U;

std::atomic<bool> blinky {true};
std::thread updateThread;

#define COLUMN_SCALE 8
#define SIZE_VIEW_COLS 100
#define SIZE_VIEW_ROWS 500

void lr_cb      (Fl_Widget*, void*);
void td_cb      (Fl_Widget*, void*);
void cr_cb      (Fl_Widget*, void*);
void open_cb    (Fl_Widget*, void*);
void import_cb  (Fl_Widget*, void*);
void save_cb    (Fl_Widget*, void*);
void exit_cb    (Fl_Widget*, void*);
void input_cb   (Fl_Widget*, void*);
void library_cb (Fl_Widget*, void*);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
Fl_Menu_Item static_menu_array [] =
 {
   {"&File",         0, nullptr, nullptr, FL_SUBMENU},
   {"&Open",         0, open_cb},
   {"&Load Library", 0, library_cb},
   {"&Import CSV",   0, import_cb},
   {"&Save",         0, save_cb},
   {"E&xit",         0, exit_cb},
   {nullptr},
   {"Settings",      0, nullptr, nullptr, FL_SUBMENU},
   {"Left-Right",    0, lr_cb,   nullptr, FL_MENU_TOGGLE | FL_MENU_VALUE},
   {"Top-Bottom",    0, td_cb,   nullptr, FL_MENU_TOGGLE | FL_MENU_VALUE},
   {"Column-Row",    0, cr_cb,   nullptr, FL_MENU_TOGGLE | FL_MENU_VALUE},
   {nullptr},
   {nullptr}
 };
#pragma GCC diagnostic pop

class SharedData final
 {
public:
   size_t c_col;
   size_t c_row;
   size_t tr_col;
   size_t tr_row;

   bool inputMode;
   bool useComma;

   std::string origString;

   size_t def_col_width;
   std::vector<int> col_widths;

   std::vector<Forwards::Engine::CellType> yankedType;
   std::vector<std::shared_ptr<Forwards::Engine::Expression> > yanked;
   size_t yankedCols;

   Forwards::Engine::CallingContext* context;

   size_t m_col;
   size_t m_row;

   std::vector<std::pair<std::string, std::string> > otherLibs;
   std::vector<std::pair<std::string, std::string> > fileLibs;
 };



static Fl_Input* G_location = nullptr;
static Fl_Output* G_result = nullptr;
static Fl_Output* G_interpretation = nullptr;
static Fl_Input* G_input = nullptr;
static SharedData* G_shared = nullptr;
static Fl_Widget* G_table = nullptr;



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

std::string getStringPreviousValuePtr(const Forwards::Engine::Cell* const curCell, const std::shared_ptr<Forwards::Types::ValueType>& previousValue, SharedData& data)
 {
   std::string content = previousValue->toString(data.c_col, data.c_row);
   if (Forwards::Engine::VALUE == curCell->type) content = setComma(content, data.useComma);
   return content;
 }

std::string getStringPreviousValue(const Forwards::Engine::Cell* const curCell, SharedData& data)
 {
   return getStringPreviousValuePtr(curCell, curCell->previousValue, data);
 }

std::string getStringDisplayValue(Forwards::Engine::Cell* curCell, SharedData& data)
 {
   std::string content ("ERROR");
   if (Forwards::Engine::VALUE == curCell->type) content = setComma(curCell->value->toString(data.c_col, data.c_row), data.useComma);
   else if (Forwards::Engine::LABEL == curCell->type) content = curCell->value->evaluate(*data.context)->toString(data.c_col, data.c_row);
   return content;
 }

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

void sheetrun (void)
 {
   std::chrono::system_clock::time_point last;
   for (;;)
    {
      if (true == blinky)
       {
         G_shared->context->theSheet->recalc(*G_shared->context);
         G_table->damage(FL_DAMAGE_ALL);
         Fl::awake();
         blinky = false;
       }
      last = std::chrono::system_clock::now() + std::chrono::milliseconds(RECALC_POLL_MILLIS);
      std::this_thread::sleep_until(last);
    }
 }



class Spreadsheet : public Fl_Table
 {
protected:
   void draw_cell(TableContext context, int, int, int, int, int, int) override;
   void real_callback();

   static void event_cb(Fl_Widget*, void *v)
    {
      static_cast<Spreadsheet*>(v)->real_callback();
    }

public:
   Spreadsheet(int X, int Y, int W, int H, const char* L = 0) : Fl_Table(X, Y, W, H, L)
    {
      callback(event_cb, reinterpret_cast<void*>(this));
      when(FL_WHEN_NOT_CHANGED|when());
      end();
      set_selection(0,0,0,0);
   }

   ~Spreadsheet() { }

   void start_editing(int R, int C)
    {
      if ((-1 == R) || (-1 == C)) return;
      G_shared->c_col = C + G_shared->tr_col;
      G_shared->c_row = R + G_shared->tr_row;
      Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
      if (nullptr != curCell)
       {
         if (("" == curCell->currentInput) && (nullptr != curCell->value.get()))
          {
            curCell->currentInput = getStringDisplayValue(curCell, *G_shared);
            curCell->value.reset();
          }
         G_shared->origString = curCell->currentInput;

         G_shared->inputMode = true;
         G_input->value(curCell->currentInput.c_str());
         G_input->take_focus();
       }
      else
       {
         G_input->value("");
       }
      set_selection(R, C, R, C);
      input_cb(nullptr, this);
    }

   void cancel_editing()
    {
      if (G_shared->inputMode)
       {
         Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
         curCell->currentInput = G_shared->origString;
         curCell->value.reset();
         curCell->previousValue.reset();
         G_shared->inputMode = false;
         damage(FL_DAMAGE_ALL);
         blinky = true;
       }
    }

   void done_editing()
    {
      if (G_shared->inputMode)
       {
         Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
         curCell->currentInput = G_input->value();
         curCell->value.reset();
         curCell->previousValue.reset();
         G_shared->inputMode = false;
         damage(FL_DAMAGE_ALL);
         blinky = true;
       }
    }

   void update_fields(int R, int C)
    {
      if ((-1 == R) || (-1 == C)) return;
      G_shared->c_col = C + G_shared->tr_col;
      G_shared->c_row = R + G_shared->tr_row;
      Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
      if (nullptr != curCell)
       {
         std::string type = "(E) ";
         if (Forwards::Engine::VALUE == curCell->type) type = "(V) ";
         else if (Forwards::Engine::LABEL == curCell->type) type = "(L) ";
         if (nullptr != curCell->previousValue.get())
            G_result->value((type + getStringPreviousValue(curCell, *G_shared)).c_str());
         else
            G_result->value(type.c_str());
         if (nullptr != curCell->value.get())
            G_interpretation->value(getStringDisplayValue(curCell, *G_shared).c_str());
         else
            G_interpretation->value(curCell->currentInput.c_str());
         G_input->value("");
       }
      else
       {
         G_result->value("");
         G_interpretation->value("");
         G_input->value("");
       }
      G_result->damage(FL_DAMAGE_ALL);
      G_interpretation->damage(FL_DAMAGE_ALL);
      G_input->damage(FL_DAMAGE_ALL);
    }
 };

// Handle drawing all cells in table
void Spreadsheet::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
 {
   switch (context)
    {
   case CONTEXT_STARTPAGE: // table about to redraw
      break;

   case CONTEXT_COL_HEADER: // table wants us to draw a column heading (C is column)
      fl_font(FL_HELVETICA | FL_BOLD, 14);
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
      fl_color(FL_BLACK);
      fl_draw(Forwards::Types::ValueType::columnToString(C + G_shared->tr_col).c_str(), X, Y, W, H, FL_ALIGN_CENTER);
      fl_pop_clip();
      break;

   case CONTEXT_ROW_HEADER: // table wants us to draw a row heading (R is row)
      fl_font(FL_HELVETICA | FL_BOLD, 14);
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
      fl_color(FL_BLACK);
      fl_draw(std::to_string(R + G_shared->tr_row + 1).c_str(), X,Y,W,H, FL_ALIGN_CENTER);
      fl_pop_clip();
      break;

   case CONTEXT_CELL: // table wants us to draw a cell
    {
      Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
      std::string content;
      Fl_Color back = is_selected(R, C) ? FL_YELLOW : FL_WHITE;
      if (nullptr != curCell)
       {
         if (true == curCell->recursed)
          {
            back = is_selected(R, C) ? FL_MAGENTA : FL_RED;
          }
         if (nullptr != curCell->previousValue)
          {
            content = getStringPreviousValue(curCell, *G_shared);
          }
         else if (("" != curCell->currentInput) || (nullptr != curCell->value.get()))
          {
            content = "***";
            back = is_selected(R, C) ? FL_MAGENTA : FL_RED;
          }
       }
      fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, back);
      fl_push_clip(X + 3, Y + 3, W - 6, H - 6);
      fl_color(FL_BLACK);
      fl_font(FL_HELVETICA, 14);
      fl_draw(content.c_str(), X + 3, Y + 3, W - 6, H - 6, FL_ALIGN_RIGHT);
      fl_pop_clip();
    }
      break;

   default:
      break;
    }
 }

static char hiddenState = '\0';
// Callback whenever someone clicks on different parts of the table
void Spreadsheet::real_callback()
 {
   int R = callback_row();
   int C = callback_col();
   TableContext context = callback_context(); 

   switch ( context )
    {
   case CONTEXT_CELL: // A table event occurred on a cell
      switch (Fl::event()) // see what FLTK event caused it
       {
      case FL_PUSH:
         hiddenState = '\0';
         if (blinky) break;
         done_editing();
         take_focus();
         update_fields(R, C);
         start_editing(R, C);
         break;

      case FL_KEYBOARD:
         done_editing();
         if ('\0' != hiddenState)
          {
            switch (hiddenState)
             {
            case 'd':
               switch (Fl::e_text[0])
                {
               case 'd':
                  G_shared->context->theSheet->clearCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
                  break;
               case 'c':
                  G_shared->context->theSheet->clearColumn(C + G_shared->tr_col);
                  break;
               case 'r':
                  G_shared->context->theSheet->clearRow(R + G_shared->tr_row);
                  break;
               case 'm':
                {
                  size_t bc = std::min(C + G_shared->tr_col, G_shared->m_col);
                  size_t mc = std::max(C + G_shared->tr_col, G_shared->m_col);
                  size_t br = std::min(R + G_shared->tr_row, G_shared->m_row);
                  size_t mr = std::max(R + G_shared->tr_row, G_shared->m_row);
                  for (size_t _c = bc; _c <= mc; ++_c)
                     for (size_t _r = br; _r <= mr; ++_r)
                        G_shared->context->theSheet->clearCellAt(_c, _r);
                }
                  break;
                }
               blinky = true;
               update_fields(R, C);
               damage(FL_DAMAGE_ALL);
               break;
            case 'x':
               switch (Fl::e_text[0])
                {
               case 'x':
                  G_shared->context->theSheet->removeCellShiftUp(C + G_shared->tr_col, R + G_shared->tr_row);
                  break;
               case 'z':
                  G_shared->context->theSheet->removeCellShiftLeft(C + G_shared->tr_col, R + G_shared->tr_row);
                  break;
               case 'c':
                  G_shared->context->theSheet->removeColumn(C + G_shared->tr_col);
                  removeColumn(G_shared->col_widths, C + G_shared->tr_col);
                  break;
               case 'r':
                  G_shared->context->theSheet->removeRow(R + G_shared->tr_row);
                  break;
                }
               blinky = true;
               update_fields(R, C);
               damage(FL_DAMAGE_ALL);
               break;
            case 'i':
               switch (Fl::e_text[0])
                {
               case 'i':
                  G_shared->context->theSheet->insertCellBeforeShiftDown(C + G_shared->tr_col, R + G_shared->tr_row);
                  break;
               case 'c':
                  G_shared->context->theSheet->insertColumnBefore(C + G_shared->tr_col);
                  insertColumnBefore(G_shared->col_widths, C + G_shared->tr_col, G_shared->def_col_width);
                  break;
               case 'r':
                  G_shared->context->theSheet->insertRowBefore(R + G_shared->tr_row);
                  break;
                }
               blinky = true;
               update_fields(R, C);
               damage(FL_DAMAGE_ALL);
               break;
            case 'o':
               switch (Fl::e_text[0])
                {
               case 'o':
                  G_shared->context->theSheet->insertCellBeforeShiftRight(C + G_shared->tr_col, R + G_shared->tr_row);
                  break;
               case 'c':
                  G_shared->context->theSheet->insertColumnBefore(C + G_shared->tr_col + 1U);
                  insertColumnBefore(G_shared->col_widths, C + G_shared->tr_col + 1U, G_shared->def_col_width);
                  break;
               case 'r':
                  G_shared->context->theSheet->insertRowBefore(R + G_shared->tr_row + 1U);
                  break;
                }
               blinky = true;
               update_fields(R, C);
               damage(FL_DAMAGE_ALL);
               break;
            case 'y':
               G_shared->c_col = C + G_shared->tr_col;
               G_shared->c_row = R + G_shared->tr_row;
               switch (Fl::e_text[0])
                {
               case 'y':
                {
                  Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
                  if ((nullptr != curCell) && (nullptr != curCell->value.get()))
                   {
                     G_shared->yankedType.resize(1U);
                     G_shared->yankedType[0] = curCell->type;
                     G_shared->yanked.resize(1U);
                     G_shared->yanked[0] = curCell->value;
                     G_shared->yankedCols = 1U;
                   }
                }
                  break;
               case 'c':
                {
                  size_t maxRow = 0U;
                  if (G_shared->c_col < G_shared->context->theSheet->sheet.size())
                   {
                     maxRow = G_shared->context->theSheet->sheet[G_shared->c_col].size();
                   }
                  while (nullptr == G_shared->context->theSheet->getCellAt(G_shared->c_col, maxRow))
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
                  G_shared->yankedCols = 1U;
                  G_shared->yankedType.resize(maxRow + 1U);
                  G_shared->yanked.resize(maxRow + 1U);
                  for (size_t i = 0U; i <= maxRow; ++i)
                   {
                     Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, i);
                     if ((nullptr != tempCell) && (nullptr != tempCell->value.get()))
                      {
                        G_shared->yankedType[i] = tempCell->type;
                        G_shared->yanked[i] = tempCell->value;
                      }
                     else
                      {
                        G_shared->yankedType[i] = UNREAL_ERROR;
                      }
                   }
                }
                  break;
               case 'r':
                {
                  size_t maxCol = G_shared->context->theSheet->sheet.size();
                  while (nullptr == G_shared->context->theSheet->getCellAt(maxCol, G_shared->c_row))
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
                  G_shared->yankedCols = maxCol + 1U;
                  G_shared->yankedType.resize(maxCol + 1U);
                  G_shared->yanked.resize(maxCol + 1U);
                  for (size_t i = 0U; i <= maxCol; ++i)
                   {
                     Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(i, G_shared->c_row);
                     if ((nullptr != tempCell) && (nullptr != tempCell->value.get()))
                      {
                        G_shared->yankedType[i] = tempCell->type;
                        G_shared->yanked[i] = tempCell->value;
                      }
                     else
                      {
                        G_shared->yankedType[i] = UNREAL_ERROR;
                      }
                   }
                }
                  break;
               case 'm':
                {
                  size_t bc = std::min(G_shared->c_col, G_shared->m_col);
                  size_t mc = std::max(G_shared->c_col, G_shared->m_col);
                  size_t br = std::min(G_shared->c_row, G_shared->m_row);
                  size_t mr = std::max(G_shared->c_row, G_shared->m_row);
                  G_shared->yankedType.clear();
                  G_shared->yanked.clear();
                  for (size_t _c = bc; _c <= mc; ++_c)
                     for (size_t _r = br; _r <= mr; ++_r)
                      {
                        Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                        if ((nullptr != tempCell) && (nullptr != tempCell->value.get()))
                         {
                           G_shared->yankedType.push_back(tempCell->type);
                           G_shared->yanked.push_back(tempCell->value);
                         }
                        else
                         {
                           G_shared->yankedType.push_back(UNREAL_ERROR);
                           G_shared->yanked.push_back(std::shared_ptr<Forwards::Engine::Expression>());
                         }
                      }
                  G_shared->yankedCols = mc - bc + 1U;
                }
                  break;
               case 'd':
                  G_shared->yankedType.clear();
                  G_shared->yanked.clear();
                  G_shared->yankedCols = 0U;
                  break;
                }
               break;
            case 'p':
               if (0U == G_shared->yankedCols) break;
               G_shared->c_col = C + G_shared->tr_col;
               G_shared->c_row = R + G_shared->tr_row;
               switch (Fl::e_text[0])
                {
               case 'p':
                  if (UNREAL_ERROR != G_shared->yankedType[0])
                   {
                     Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
                     if (nullptr == curCell)
                      {
                        G_shared->context->theSheet->initCellAt(G_shared->c_col, G_shared->c_row);
                        curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
                      }
                     curCell->type = G_shared->yankedType[0];
                     curCell->value = G_shared->yanked[0];
                   }
                  break;
               case 'c':
                  for (size_t i = 0U; i < G_shared->yanked.size(); ++i)
                   {
                     if (UNREAL_ERROR != G_shared->yankedType[i])
                      {
                        Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, i);
                        if (nullptr == tempCell)
                         {
                           G_shared->context->theSheet->initCellAt(G_shared->c_col, i);
                           tempCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, i);
                         }
                        tempCell->type = G_shared->yankedType[i];
                        tempCell->value = G_shared->yanked[i];
                      }
                   }
                  break;
               case 'r':
                {
                  size_t maxCol = std::min(G_shared->yanked.size(), MAX_COL + 1U);
                  for (size_t i = 0U; i < maxCol; ++i)
                   {
                     if (UNREAL_ERROR != G_shared->yankedType[i])
                      {
                        Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(i, G_shared->c_row);
                        if (nullptr == tempCell)
                         {
                           G_shared->context->theSheet->initCellAt(i, G_shared->c_row);
                           tempCell = G_shared->context->theSheet->getCellAt(i, G_shared->c_row);
                         }
                        tempCell->type = G_shared->yankedType[i];
                        tempCell->value = G_shared->yanked[i];
                      }
                   }
                }
                  break;
               case 'm':
                {
                  size_t rs = G_shared->yanked.size() / G_shared->yankedCols;
                  size_t i = 0U;
                  for (size_t _c = G_shared->c_col; _c < G_shared->c_col + G_shared->yankedCols; ++_c)
                     for (size_t _r = G_shared->c_row; _r < G_shared->c_row + rs; ++_r)
                      {
                        if ((UNREAL_ERROR != G_shared->yankedType[i]) && (_c <= MAX_COL) && (_r <= MAX_ROW))
                         {
                           Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                           if (nullptr == tempCell)
                            {
                              G_shared->context->theSheet->initCellAt(_c, _r);
                              tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                            }
                           tempCell->type = G_shared->yankedType[i];
                           tempCell->value = G_shared->yanked[i];
                         }
                        ++i;
                      }
                }
                  break;
               case 'n':
                {
                  size_t rs = G_shared->yanked.size() / G_shared->yankedCols;
                  size_t i = 0U;
                  for (size_t _r = G_shared->c_row; _r < G_shared->c_row + G_shared->yankedCols; ++_r)
                     for (size_t _c = G_shared->c_col; _c < G_shared->c_col + rs; ++_c)
                      {
                        if ((UNREAL_ERROR != G_shared->yankedType[i]) && (_c <= MAX_COL) && (_r <= MAX_ROW))
                         {
                           Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                           if (nullptr == tempCell)
                            {
                              G_shared->context->theSheet->initCellAt(_c, _r);
                              tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                            }
                           tempCell->type = G_shared->yankedType[i];
                           tempCell->value = G_shared->yanked[i];
                         }
                        ++i;
                      }
                }
                  break;
               case 'f':
                {
                  size_t bc = std::min(G_shared->c_col, G_shared->m_col);
                  size_t mc = std::max(G_shared->c_col, G_shared->m_col);
                  size_t br = std::min(G_shared->c_row, G_shared->m_row);
                  size_t mr = std::max(G_shared->c_row, G_shared->m_row);
                  size_t i = 0U;
                  for (size_t _c = bc; (_c <= mc) && (i < G_shared->yankedType.size()); ++_c)
                     for (size_t _r = br; (_r <= mr) && (i < G_shared->yankedType.size()); ++_r)
                      {
                        if (UNREAL_ERROR != G_shared->yankedType[i])
                         {
                           Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                           if (nullptr == tempCell)
                            {
                              G_shared->context->theSheet->initCellAt(_c, _r);
                              tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                            }
                           tempCell->type = G_shared->yankedType[i];
                           tempCell->value = G_shared->yanked[i];
                         }
                        ++i;
                      }
                }
                  break;
               case 't':
                {
                  size_t bc = std::min(G_shared->c_col, G_shared->m_col);
                  size_t mc = std::max(G_shared->c_col, G_shared->m_col);
                  size_t br = std::min(G_shared->c_row, G_shared->m_row);
                  size_t mr = std::max(G_shared->c_row, G_shared->m_row);
                  size_t i = 0U;
                  for (size_t _r = br; (_r <= mr) && (i < G_shared->yankedType.size()); ++_r)
                     for (size_t _c = bc; (_c <= mc) && (i < G_shared->yankedType.size()); ++_c)
                      {
                        if (UNREAL_ERROR != G_shared->yankedType[i])
                         {
                           Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                           if (nullptr == tempCell)
                            {
                              G_shared->context->theSheet->initCellAt(_c, _r);
                              tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                            }
                           tempCell->type = G_shared->yankedType[i];
                           tempCell->value = G_shared->yanked[i];
                         }
                        ++i;
                      }
                }
                  break;
                }
               blinky = true;
               update_fields(R, C);
               damage(FL_DAMAGE_ALL);
               break;
            case 'v':
               G_shared->c_col = C + G_shared->tr_col;
               G_shared->c_row = R + G_shared->tr_row;
               switch (Fl::e_text[0])
                {
               case 'v':
                {
                  Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
                  if (nullptr != curCell)
                   {
                     if (("" == curCell->currentInput) && (nullptr != curCell->value.get()) && (nullptr != curCell->previousValue.get()))
                      {
                        curCell->currentInput = getStringPreviousValue(curCell, *G_shared);
                        curCell->value.reset();
                      }
                   }
                }
                  break;
               case 'm':
                {
                  size_t bc = std::min(G_shared->c_col, G_shared->m_col);
                  size_t mc = std::max(G_shared->c_col, G_shared->m_col);
                  size_t br = std::min(G_shared->c_row, G_shared->m_row);
                  size_t mr = std::max(G_shared->c_row, G_shared->m_row);
                  for (size_t _c = bc; _c <= mc; ++_c)
                     for (size_t _r = br; _r <= mr; ++_r)
                      {
                        Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                        if (nullptr != tempCell)
                         {
                           if (("" == tempCell->currentInput) && (nullptr != tempCell->value.get()) && (nullptr != tempCell->previousValue.get()))
                            {
                              tempCell->currentInput = getStringPreviousValue(tempCell, *G_shared);
                              tempCell->value.reset();
                            }
                         }
                      }
                }
                  break;
               case '=':
                {
                  Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
                  if (nullptr != curCell)
                   {
                     if (("" == curCell->currentInput) && (nullptr != curCell->value.get()))
                      {
                        curCell->currentInput = getStringDisplayValue(curCell, *G_shared);
                        curCell->value.reset();
                      }
                     if (Forwards::Engine::VALUE == curCell->type)
                      {
                        curCell->type = Forwards::Engine::LABEL;
                      }
                     else if (Forwards::Engine::LABEL == curCell->type)
                      {
                        curCell->type = Forwards::Engine::VALUE;
                      }
                   }
                }
                  break;
               case '-':
                {
                  size_t bc = std::min(G_shared->c_col, G_shared->m_col);
                  size_t mc = std::max(G_shared->c_col, G_shared->m_col);
                  size_t br = std::min(G_shared->c_row, G_shared->m_row);
                  size_t mr = std::max(G_shared->c_row, G_shared->m_row);
                  for (size_t _c = bc; _c <= mc; ++_c)
                     for (size_t _r = br; _r <= mr; ++_r)
                      {
                        Forwards::Engine::Cell* tempCell = G_shared->context->theSheet->getCellAt(_c, _r);
                        if (nullptr != tempCell)
                         {
                           if (("" == tempCell->currentInput) && (nullptr != tempCell->value.get()))
                            {
                              tempCell->currentInput = getStringDisplayValue(tempCell, *G_shared);
                              tempCell->value.reset();
                            }
                           if (Forwards::Engine::VALUE == tempCell->type)
                            {
                              tempCell->type = Forwards::Engine::LABEL;
                            }
                           else if (Forwards::Engine::LABEL == tempCell->type)
                            {
                              tempCell->type = Forwards::Engine::VALUE;
                            }
                         }
                      }
                }
                  break;
                }
               blinky = true;
               update_fields(R, C);
               damage(FL_DAMAGE_ALL);
               break;
            default:
               break;
             }
            hiddenState = '\0';
          }
         else
          {
            switch (Fl::e_text[0])
             {
            case '=':
               if (blinky) break;
             {
               Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
               if (nullptr == curCell)
                {
                  G_shared->context->theSheet->initCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
                  curCell = G_shared->context->theSheet->getCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
                }
               curCell->type = Forwards::Engine::VALUE;
               curCell->currentInput = "";
               curCell->value.reset();
               start_editing(R, C);
             }
               break;
            case '<':
               if (blinky) break;
             {
               Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
               if (nullptr == curCell)
                {
                  G_shared->context->theSheet->initCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
                  curCell = G_shared->context->theSheet->getCellAt(C + G_shared->tr_col, R + G_shared->tr_row);
                }
               curCell->type = Forwards::Engine::LABEL;
               curCell->currentInput = "";
               curCell->value.reset();
               start_editing(R, C);
             }
               break;
            case 'd':
               hiddenState = 'd';
               break;
            case 'm':
               G_shared->m_row = R + G_shared->tr_row;
               G_shared->m_col = C + G_shared->tr_col;
               break;
            case 'x':
               hiddenState = 'x';
               break;
            case 'i':
               hiddenState = 'i';
               break;
            case 'o':
               hiddenState = 'o';
               break;
            case 'y':
               hiddenState = 'y';
               break;
            case 'p':
               hiddenState = 'p';
               break;
            case 'v':
               hiddenState = 'v';
               break;
            case '!':
               blinky = true;
               damage(FL_DAMAGE_ALL);
               break;
            case ',':
               G_shared->useComma = !G_shared->useComma;
               damage(FL_DAMAGE_ALL);
               break;
            case '\r':
            case '\n':
               if (!blinky) start_editing(R, C);
               break;
            default:
               update_fields(R, C);
               break;
             }
          }
       }
      break;

   case CONTEXT_TABLE: // A table event occurred on dead zone in table
   case CONTEXT_ROW_HEADER: // A table event occurred on row/column header
   case CONTEXT_COL_HEADER:
      hiddenState = '\0';
      cancel_editing();
      break;

   default:
      hiddenState = '\0';
      break;
    }
 }


void setTableWidths ()
 {
   for (int i = 0; i < SIZE_VIEW_COLS; ++i)
    {
      int width = getWidth(G_shared->col_widths, G_shared->tr_col + i, G_shared->def_col_width);
      static_cast<Spreadsheet*>(G_table)->col_width(i, width * COLUMN_SCALE);
    }
 }

void saveTableWidths ()
 {
   for (int i = 0; i < SIZE_VIEW_COLS; ++i)
    {
      int width = static_cast<Spreadsheet*>(G_table)->col_width(i);
      int residue = width % COLUMN_SCALE;
      width = width / COLUMN_SCALE + ((0 != residue) ? 1 : 0);
      setWidth(G_shared->col_widths, G_shared->tr_col + i, width, G_shared->def_col_width);
    }
 }



void lr_cb (Fl_Widget*, void*)
 {
   G_shared->context->theSheet->left_right = !G_shared->context->theSheet->left_right;
 }
void td_cb (Fl_Widget*, void*)
 {
   G_shared->context->theSheet->top_down = !G_shared->context->theSheet->top_down;
 }
void cr_cb (Fl_Widget*, void*)
 {
   G_shared->context->theSheet->c_major = !G_shared->context->theSheet->c_major;
 }

void open_cb (Fl_Widget*, void*)
 {
   const char* fileName = fl_file_chooser("Open file...", nullptr, nullptr, 0);
   if (nullptr != fileName)
    {
      G_shared->fileLibs.clear();
      LoadFile(fileName, G_shared->context->theSheet, G_shared->col_widths, G_shared->def_col_width, G_shared->fileLibs);
      std::vector<std::pair<std::string, std::string> > allLibs (G_shared->fileLibs);
      allLibs.insert(allLibs.end(), G_shared->otherLibs.begin(), G_shared->otherLibs.end());
      LoadLibraries(allLibs, *G_shared->context);
      blinky = true;
      setTableWidths();
      G_table->damage(FL_DAMAGE_ALL);
    }
 }

void library_cb (Fl_Widget*, void*)
 {
   const char* fileName = fl_file_chooser("Load Library File...", nullptr, nullptr, 0);
   if (nullptr != fileName)
    {
      std::ifstream file (fileName, std::ios_base::in);
      if (file)
       {
         std::string lib {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
         G_shared->otherLibs.push_back(std::make_pair(fileName, lib));
       }
      std::vector<std::pair<std::string, std::string> > allLibs (G_shared->fileLibs);
      allLibs.insert(allLibs.end(), G_shared->otherLibs.begin(), G_shared->otherLibs.end());
      LoadLibraries(allLibs, *G_shared->context);
    }
 }

void import_cb (Fl_Widget*, void*)
 {
   const char* fileName = fl_file_chooser("Import CSV...", nullptr, nullptr, 0);
   if (nullptr != fileName)
    {
      ImportCSV(fileName, G_shared->context->theSheet);
      blinky = true;
      G_table->damage(FL_DAMAGE_ALL);
    }
 }

void save_cb (Fl_Widget*, void*)
 {
   const char* fileName = fl_file_chooser("Save as...", nullptr, nullptr, 0);
   if (nullptr != fileName)
    {
      std::vector<std::pair<std::string, std::string> > allLibs (G_shared->fileLibs);
      allLibs.insert(allLibs.end(), G_shared->otherLibs.begin(), G_shared->otherLibs.end());
      SaveFile(fileName, G_shared->context->theSheet, G_shared->col_widths, G_shared->def_col_width, allLibs);
    }
 }

void exit_cb (Fl_Widget*, void*)
 {
   std::exit(0);
 }

void input_cb(Fl_Widget*, void*)
 {
   Forwards::Engine::Cell* curCell = G_shared->context->theSheet->getCellAt(G_shared->c_col, G_shared->c_row);
   if (nullptr != curCell)
    {
         // unfinished VALUE : parse current contents
      if ((Forwards::Engine::VALUE == curCell->type) && (nullptr == curCell->value))
       {
         G_shared->context->inUserInput = true;
         --G_shared->context->generation;
         curCell->currentInput = G_input->value();
         std::shared_ptr<Forwards::Types::ValueType> result;
         std::string content = G_shared->context->theSheet->computeCell(*G_shared->context, result, G_shared->c_col, G_shared->c_row);
         ++G_shared->context->generation;
         if (nullptr != result.get())
          {
            content = result->toString(G_shared->c_col, G_shared->c_row);
          }
         content = setComma(content, G_shared->useComma);
         G_interpretation->value(content.c_str());
         G_interpretation->damage(FL_DAMAGE_ALL);
       }
         // finished VALUE or LABEL
      else if (nullptr != curCell->value)
       {
         G_interpretation->value(getStringDisplayValue(curCell, *G_shared).c_str());
         G_interpretation->damage(FL_DAMAGE_ALL);
       }
    }
   else
    {
      G_interpretation->value("");
      G_interpretation->damage(FL_DAMAGE_ALL);
    }

   G_table->damage(FL_DAMAGE_ALL);
 }

void location_cb(Fl_Widget*, void*)
 {
   int64_t row, col;
   GetRC(G_location->value(), col, row);
   if ((-1 != col) && (-1 != row))
    {
      saveTableWidths();
      G_shared->tr_col = col;
      G_shared->tr_row = row;
      if ((MAX_COL - SIZE_VIEW_COLS + 1) < static_cast<size_t>(col)) G_shared->tr_col = MAX_COL - SIZE_VIEW_COLS + 1;
      if ((MAX_ROW - SIZE_VIEW_ROWS + 1) < static_cast<size_t>(row)) G_shared->tr_row = MAX_ROW - SIZE_VIEW_ROWS + 1;
      setTableWidths();
      G_table->damage(FL_DAMAGE_ALL);
    }
   std::string location = Forwards::Types::ValueType::columnToString(G_shared->tr_col) + std::to_string(G_shared->tr_row + 1);
   G_location->value(location.c_str());
 }

int dontclose_hand(int event)
 {
   if ((FL_SHORTCUT == event) && (FL_Escape == Fl::event_key()))
    {
      if (G_shared->inputMode)
       {
         G_input->value(G_shared->origString.c_str());
         G_table->take_focus();
       }
      static_cast<Spreadsheet*>(G_table)->cancel_editing();
      static_cast<Spreadsheet*>(G_table)->update_fields(G_shared->c_row - G_shared->tr_row, G_shared->c_col - G_shared->tr_col);
      return 1; // Don't close the window when someone presses ESC!
    }
   return 0;
 }

void close_cb(Fl_Widget*, void*)
 {
   if (FL_REASON_CLOSED == Fl::callback_reason())
    {
      if (fl_choice("Did you mean to click exit?", "No", "Yes", NULL))
       {
         std::exit(0);
       }
    }
 }



int main(void)
 {
   Fl_Double_Window win (800, 600, "BC-DeciCalc FLTK");

   Fl_Menu_Bar bar (0, 0, 800, 30);
   bar.menu(static_menu_array);

   Fl_Input location (10, 40, 110, 20);
   Fl_Output result (130, 40, 660, 20);
   Fl_Output interpretation (10, 70, 780, 20);
   Fl_Input input (10, 100, 780, 20);

   location.value("A1");
   location.callback(location_cb, nullptr);

   G_location = &location;
   G_result = &result;
   G_interpretation = &interpretation;
   G_input = &input;

   Spreadsheet table (10, 130, 780, 460);

   table.tooltip("Use '=' or '<' to open a new cell for editing.");

   table.row_header(1);
   table.row_header_width((DEF_COLUMN_WIDTH + 1) * COLUMN_SCALE);
   table.rows(SIZE_VIEW_ROWS);
   table.row_height_all(20);
   table.col_header(1);
   table.col_header_height(20);
   table.col_resize(1);
   table.cols(SIZE_VIEW_COLS);
   table.col_width_all(DEF_COLUMN_WIDTH * COLUMN_SCALE);

   input.callback(input_cb, &table);
   input.when(FL_WHEN_CHANGED);

   win.callback(close_cb);
   win.end();
   win.resizable(&table);
   win.show();

   G_table = &table;

   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   context.globalScope = &global;
   Forwards::Parser::StringLogger logger;
   context.logger = &logger;
   Forwards::Engine::SpreadSheet sheet;
   context.theSheet = &sheet;
   Forwards::Engine::GetterMap map;
   context.map = &map;
   Forwards::Engine::NameMap names;
   context.names = &names;

   LoadLibraries(std::vector<std::pair<std::string, std::string> >(), context);

   SharedData state;

   state.c_row = 0U;
   state.c_col = 0U;
   state.tr_row = 0U;
   state.tr_col = 0U;

   state.inputMode = false;
   state.useComma = false;

   state.def_col_width = DEF_COLUMN_WIDTH;

   state.yankedCols = 0U;

   state.context = &context;

   G_shared = &state;

   updateThread = std::thread(sheetrun);
   updateThread.detach();

   Fl::add_handler(dontclose_hand);

   return Fl::run();
 }
