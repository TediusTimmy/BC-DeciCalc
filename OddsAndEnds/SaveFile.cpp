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

static void replaceAll(std::string& in, char ch, const std::string& with)
 {
   size_t c;
   c = in.rfind(ch, std::string::npos);
   while (std::string::npos != c)
    {
      in = in.substr(0U, c) + with + in.substr(c + 1U, std::string::npos);
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
   return result;
 }

void SaveFile(const std::string& fileName, Forwards::Engine::SpreadSheet* theSheet)
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
   file << "<html><head><style>td { border: 1px solid black; }</style></head><body><table>" << std::endl;
   size_t col = 0U;
   for (auto& column : theSheet->sheet)
    {
      file << "   <tr>";
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
               if (nullptr != cell->value.get()) toPrint = cell->value->toString(col, row, 0);
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
      in = in.substr(0U, c) + with + in.substr(c + ent.length(), std::string::npos);
      if (0U != c)
         c = in.rfind(ent, c - 1U);
      else
         c = std::string::npos;
    }
 }

static std::string soften(const std::string& in)
 {
   std::string result = in;
   replaceAllEntities(result, "&gt;", ">");
   replaceAllEntities(result, "&lt;", "<");
   replaceAllEntities(result, "&amp;", "&");
   return result;
 }

void LoadFile(const std::string& fileName, Forwards::Engine::SpreadSheet* sheet)
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
      curCol = curCol.substr(0U, curCol.size() - 1U);
    }
   if ("<html><head><style>td { border: 1px solid black; }</style></head><body><table>" != curCol) // I WILL REGRET THIS!
    {
      sheet->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = sheet->getCellAt(0U, 0U);
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName;
      return;
    }

   curCol = "";
   std::getline(file, curCol);
   if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
    {
      curCol = curCol.substr(0U, curCol.size() - 1U);
    }
   size_t col = 0U;
   while (("</table></body></html>" != curCol) && (true == file.good()))
    {
      size_t row = 0U;
      size_t n = curCol.find("<tr>");

      if (std::string::npos != n)
       {
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
                  // Skip junk.
               n = curCol.find('<', n);
             }
          }
       }

      ++col;
      curCol = "";
      std::getline(file, curCol);
      if ((0U != curCol.size()) && ('\r' == curCol[curCol.size() - 1]))
       {
         curCol = curCol.substr(0U, curCol.size() - 1U);
       }
    }
 }
