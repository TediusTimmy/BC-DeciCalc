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
#include <fstream>

#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "GetAndSet.h"

static void replaceAll(std::string& in, char ch, const std::string& with)
 {
   size_t c;
   c = in.rfind(ch, std::string::npos);
   while (std::string::npos != c)
    {
      in.replace(c, 1U, with);
      if (0U != c)
         c = in.rfind(ch, c - 1U);
      else
         c = std::string::npos;
    }
 }

static std::string harden(const std::string& in)
 {
   std::string result = in;
   replaceAll(result, '&', "&amp;");
   replaceAll(result, '<', "&lt;");
   replaceAll(result, '>', "&gt;");
   replaceAll(result, '\n', "&sect;");
   return result;
 }

void SaveFile(const std::string& fileName, Forwards::Engine::SpreadSheet* theSheet, const std::vector<int>& map, int def, const std::vector<std::pair<std::string, std::string> >& allLibs)
 {
   std::ofstream file (fileName.c_str(), std::ios::out);
   for (auto& column : theSheet->sheet)
    {
      size_t s = column.size();
      while ((s > 0U) && (nullptr == column[s - 1].get()))
       {
         --s;
       }
      if (s != column.size())
       {
         column.resize(s);
       }
    }
    {
      size_t s = theSheet->sheet.size();
      while ((s > 0U) && (0U == theSheet->sheet[s - 1].size()))
       {
         --s;
       }
      if (s != theSheet->sheet.size())
       {
         theSheet->sheet.resize(s);
       }
    }
   if (false == allLibs.empty())
    {
      file << "<html><head><style>td { border: 1px solid black; }</style></head><body>" << std::endl;
      for (const std::pair<std::string, std::string>& lib : allLibs)
       {
         file << "<b>" << harden(lib.first) << "</b><p>";
         std::string stripped;
         Backwards::Input::StringInput libText (lib.second);
         Backwards::Input::Lexer lexer (libText, lib.first);
         while (lexer.peekNextToken().lexeme != Backwards::Input::END_OF_FILE)
          {
            if (lexer.peekNextToken().lexeme != Backwards::Input::STRING)
             {
               stripped += lexer.getNextToken().text + " ";
             }
            else
             {
               stripped += "\"" + lexer.getNextToken().text + "\" ";
             }
          }
         file << harden(stripped) << "</p>" << std::endl;
       }
      file << "<table>" << std::endl;
    }
   else
    {
      file << "<html><head><style>td { border: 1px solid black; }</style></head><body><table>" << std::endl;
    }
   size_t col = 0U;
   for (auto& column : theSheet->sheet)
    {
      int width = getWidth(map, col, def);
      if (width == def)
       {
         file << "   <tr>";
       }
      else
       {
         file << "   <tr width=\"" << width << "\">";
       }
      if (0U == column.size())
       {
         file << "<td />"; // Insert one cell so that web browsers render the column.
       }
      size_t row = 0U;
      for (auto& cell : column)
       {
         if (nullptr == cell.get())
          {
            file << "<td />";
          }
         else if ((Forwards::Engine::VALUE == cell->type) && (nullptr == cell->value.get()))
          {
            file << "<td>=" << harden(cell->currentInput) << "</td>";
          }
         else
          {
            if (Forwards::Engine::VALUE == cell->type)
             {
               file << "<td>=" << harden(cell->value->toString(col, row)) << "</td>";
             }
            else
             {
               std::string toPrint;
               if (nullptr != cell->previousValue.get()) toPrint = cell->previousValue->toString(col, row);
               file << "<td>&lt;" << harden(toPrint) << "</td>";
             }
          }
         ++row;
       }
      file << "</tr>" << std::endl;
      ++col;
    }
   file << "</table></body></html>" << std::endl;
 }

static void replaceAllEntities(std::string& in, const std::string& ent, const std::string& with)
 {
   size_t c;
   c = in.rfind(ent, std::string::npos);
   while (std::string::npos != c)
    {
      in.replace(c, ent.length(), with);
      if (0U != c)
         c = in.rfind(ent, c - 1U);
      else
         c = std::string::npos;
    }
 }

static std::string soften(const std::string& in)
 {
   std::string result = in;
   replaceAllEntities(result, "&sect;", "\n");
   replaceAllEntities(result, "&gt;", ">");
   replaceAllEntities(result, "&lt;", "<");
   replaceAllEntities(result, "&amp;", "&");
   return result;
 }

void LoadFile(const std::string& fileName, Forwards::Engine::SpreadSheet* sheet, std::vector<int>& map, int def, std::vector<std::pair<std::string, std::string> >& fileLibs)
 {
   std::ifstream file (fileName.c_str(), std::ios::in);
   if (!file.good())
    {
      sheet->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = sheet->getCellAt(0U, 0U);
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName;
      return;
    }

   std::string curCol;
   
   std::getline(file, curCol);
   if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
    {
      curCol.resize(curCol.size() - 1U);
    }
   if (("<html><head><style>td { border: 1px solid black; }</style></head><body><table>" != curCol) &&
      ("<html><head><style>td { border: 1px solid black; }</style></head><body>" != curCol)) // I WILL REGRET THIS!
    {
      sheet->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = sheet->getCellAt(0U, 0U);
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName;
      return;
    }

   if ("<html><head><style>td { border: 1px solid black; }</style></head><body>" == curCol)
    {
      curCol = "";
      std::getline(file, curCol);
      if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
       {
         curCol.resize(curCol.size() - 1U);
       }
      while (("<table>" != curCol) && (true == file.good()))
       {
         size_t bn = curCol.find("<b>");
         size_t en = curCol.find("</b>");
         size_t bt = curCol.find("<p>");
         size_t et = curCol.find("</p>");

         if ((std::string::npos != bn) && (std::string::npos != en) && (std::string::npos != bt) && (std::string::npos != et))
          {
            fileLibs.push_back(std::make_pair(soften(curCol.substr(bn + 3U, en - bn - 3U)), soften(curCol.substr(bt + 3U, et - bt - 3U))));
          }

         curCol = "";
         std::getline(file, curCol);
         if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
          {
            curCol.resize(curCol.size() - 1U);
          }
       }
    }

   curCol = "";
   std::getline(file, curCol);
   if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
    {
      curCol.resize(curCol.size() - 1U);
    }
   size_t col = 0U;
   while (("</table></body></html>" != curCol) && (true == file.good()))
    {
      size_t n = curCol.find("<tr>");
      if (std::string::npos == n) // Does this column have attributes?
       {
         n = curCol.find("<tr ");
         if (std::string::npos != n) // Yes
          {
            size_t a = curCol.find("width=\"", n);
            if (std::string::npos != a)
             {
               try
                {
                  int width = std::stoi(curCol.substr(a + 7, curCol.find('"', a + 7)));
                  setWidth(map, col, width, def);
                }
               catch (const std::invalid_argument&)
                {
                }
               catch (const std::out_of_range&)
                {
                }
             }

               // Set the next read position after the end of the current tag.
            n = curCol.find('>', n);
            if (std::string::npos != n)
             {
               n -= 4U;
             }
          }
       }

      if (std::string::npos != n)
       {
         size_t row = 0U;
         n = n + 4U;
         while (std::string::npos != n)
          {
            if (n == curCol.find("</tr>", n))
             {
               n = std::string::npos;
             }
            else if (n == curCol.find("<td />", n))
             {
               n = n + 6U;
               ++row;
             }
            else if (n == curCol.find("<td>", n))
             {
               n = n + 4U;
               std::string content = soften(curCol.substr(n, curCol.find("</td>", n) - n));
               if (0U != content.length())
                {
                  if ('=' == content[0])
                   {
                     sheet->initCellAt(col, row);
                     Forwards::Engine::Cell* cell = sheet->getCellAt(col, row);
                     cell->type = Forwards::Engine::VALUE;
                     cell->currentInput = content.substr(1U, std::string::npos);
                   }
                  else if ('<' == content[0])
                   {
                     sheet->initCellAt(col, row);
                     Forwards::Engine::Cell* cell = sheet->getCellAt(col, row);
                     cell->type = Forwards::Engine::LABEL;
                     cell->currentInput = content.substr(1U, std::string::npos);
                   }
                  else
                   {
                     sheet->initCellAt(col, row);
                     Forwards::Engine::Cell* cell = sheet->getCellAt(col, row);
                     cell->type = Forwards::Engine::LABEL;
                     cell->currentInput = content;
                   }
                }
               n = curCol.find("</td>", n);
               if (std::string::npos != n) n = n + 5U;
               ++row;
             }
            else
             {
                  // Skip junk. And don't get stuck in an infinite loop on a tag we don't understand.
               size_t newn = curCol.find('<', n);
               if (n != newn)
                {
                  n = newn;
                }
               else
                {
                  ++n;
                }
             }
          }
       }

      ++col;
      curCol = "";
      std::getline(file, curCol);
      if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
       {
         curCol.resize(curCol.size() - 1U);
       }
    }
 }
