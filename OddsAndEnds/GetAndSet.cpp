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
#include <vector>

#include "GetAndSet.h"

int getWidth(const std::vector<int>& map, std::size_t col, int def)
 {
   if (map.size() <= col)
    {
      return def;
    }
   return map[col];
 }

void setWidth(std::vector<int>& map, std::size_t col, int width, int def)
 {
   if ((width >= MIN_COLUMN_WIDTH) && (width <= MAX_COLUMN_WIDTH))
    {
      if (map.size() <= col)
       {
         map.resize(col + 1U, def);
       }
      map[col] = width;
    }
 }

void incWidth(std::vector<int>& map, std::size_t col, int def)
 {
   if (map.size() <= col)
    {
      map.resize(col + 1U, def);
    }
   if (MAX_COLUMN_WIDTH != map[col])
    {
      ++map[col];
    }
 }

void decWidth(std::vector<int>& map, std::size_t col, int def)
 {
   if (map.size() <= col)
    {
      map.resize(col + 1U, def);
    }
   if (MIN_COLUMN_WIDTH != map[col])
    {
      --map[col];
    }
 }

void insertColumnBefore(std::vector<int>& map, std::size_t col, int def)
 {
   map.insert(map.begin() + col, def);
 }

void removeColumn(std::vector<int>& map, std::size_t col)
 {
   map.erase(map.begin() + col);
 }
