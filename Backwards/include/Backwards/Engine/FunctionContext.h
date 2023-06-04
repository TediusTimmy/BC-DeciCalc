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
#ifndef BACKWARDS_ENGINE_FUNCTIONCONTEXT_H
#define BACKWARDS_ENGINE_FUNCTIONCONTEXT_H

#include "Backwards/Types/FunctionValue.h"

#include <map>
#include <vector>
#include <string>

namespace Backwards
 {

namespace Engine
 {

   class Statement;

   class FunctionContext final : public Types::FunctionObjectHolder
    {
   public:
      std::string name; // For debugging.

      size_t nargs;
      size_t nlocals;
      size_t ncaptures;
      std::shared_ptr<Statement> function;

      std::map<std::string, size_t> args;
      std::map<std::string, size_t> locals;
      std::map<std::string, size_t> captures;

      std::vector<std::string> argNames;
      std::vector<std::string> localNames;
      std::vector<std::string> captureNames;
    };

 } // namespace Engine

 } // namespace Backwards

#endif /* BACKWARDS_ENGINE_FUNCTIONCONTEXT_H */
