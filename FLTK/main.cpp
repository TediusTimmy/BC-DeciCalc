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

#include <chrono>
#include <thread>
#include <atomic>

const int RECALC_POLL_MILLIS = 40; // 25 Hz
const size_t MAX_ROW = 999999998U; // Yes, minus one.
const size_t MAX_COL = 18277U;

std::atomic<bool> blinky {true};
std::thread updateThread;

#define COLUMN_SCALE 8
#define SIZE_VIEW 100

void lr_cb (Fl_Widget*, void*);
void td_cb (Fl_Widget*, void*);
void cr_cb (Fl_Widget*, void*);
void open_cb (Fl_Widget*, void*);
void input_cb(Fl_Widget*, void*);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
Fl_Menu_Item static_menu_array [] =
 {
   {"&File",      0, nullptr, nullptr, FL_SUBMENU},
   {"&Open",      0, open_cb},
   {"&Save"},
   {"E&xit"},
   {nullptr},
   {"Settings",   0, nullptr, nullptr, FL_SUBMENU},
   {"Left-Right", 0, lr_cb,   nullptr, FL_MENU_TOGGLE | FL_MENU_VALUE},
   {"Top-Bottom", 0, td_cb,   nullptr, FL_MENU_TOGGLE | FL_MENU_VALUE},
   {"Column-Row", 0, cr_cb,   nullptr, FL_MENU_TOGGLE | FL_MENU_VALUE},
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

   Forwards::Engine::CellType yankedType;
   std::shared_ptr<Forwards::Engine::Expression> yanked;

   Forwards::Engine::CallingContext* context;
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
         blinky = false;
       }
      last = std::chrono::system_clock::now() + std::chrono::milliseconds(RECALC_POLL_MILLIS);
      std::this_thread::sleep_until(last);
    }
 }



class Spreadsheet : public Fl_Table
 {
protected:
   void draw_cell(TableContext context, int, int, int, int, int, int);
   void real_callback();

   static void event_cb(Fl_Widget*, void *v)
    {
      ((Spreadsheet*)v)->real_callback();
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
         if (blinky) break;
         done_editing();
         take_focus();
         update_fields(R, C);
         start_editing(R, C);
         break;

      case FL_KEYBOARD:
         done_editing();
         switch ( Fl::e_text[0] )
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
      break;

   case CONTEXT_TABLE: // A table event occurred on dead zone in table
   case CONTEXT_ROW_HEADER: // A table event occurred on row/column header
   case CONTEXT_COL_HEADER:
      cancel_editing();
      break;

   default:
      break;
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
   std::vector<std::pair<std::string, std::string> > fileLibs;
   const char* fileName = fl_file_chooser("Open file...", nullptr, nullptr, 0);
   LoadFile(fileName, G_shared->context->theSheet, G_shared->col_widths, G_shared->def_col_width, fileLibs);
   LoadLibraries(fileLibs, *G_shared->context);
   blinky = true;
   G_table->damage(FL_DAMAGE_ALL);
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
      G_shared->tr_col = col;
      G_shared->tr_row = row;
      if ((MAX_COL - SIZE_VIEW + 1) < static_cast<size_t>(col)) G_shared->tr_col = MAX_COL - SIZE_VIEW + 1;
      if ((MAX_ROW - SIZE_VIEW + 1) < static_cast<size_t>(row)) G_shared->tr_row = MAX_ROW - SIZE_VIEW + 1;
      G_table->damage(FL_DAMAGE_ALL);
    }
   std::string location = Forwards::Types::ValueType::columnToString(G_shared->tr_col) + std::to_string(G_shared->tr_row + 1);
   G_location->value(location.c_str());
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
   table.rows(SIZE_VIEW);
   table.row_height_all(20);
   table.col_header(1);
   table.col_header_height(20);
   table.col_resize(1);
   table.cols(SIZE_VIEW);
   table.col_width_all(DEF_COLUMN_WIDTH * COLUMN_SCALE);

   input.callback(input_cb, &table);
   input.when(FL_WHEN_CHANGED);

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

   state.yankedType = UNREAL_ERROR;

   state.context = &context;

   G_shared = &state;

   updateThread = std::thread(sheetrun);
   updateThread.detach();

   return Fl::run();
 }
