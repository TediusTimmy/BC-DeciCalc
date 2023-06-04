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
#include "Backwards/Engine/StdLib.h"

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"

#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/CallingContext.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"

namespace Backwards
 {

namespace Engine
 {

   STDLIB_UNARY_DECL_WITH_CONTEXT(Eval)
    {
      if (typeid(Types::StringValue) == typeid(*arg))
       {
         Input::StringInput string (static_cast<const Types::StringValue&>(*arg).value);
         Input::Lexer lexer (string, "Eval Argument");

         Parser::GetterSetter gs;
         Parser::SymbolTable table (gs, *context.globalScope);
         if (nullptr != context.topScope())
          {
            table.pushScope(context.topScope());
          }

         std::shared_ptr<Expression> res = Parser::Parser::ParseFullExpression(lexer, table, *context.logger);

         if (nullptr != res.get())
          {
            return res->evaluate(context);
          }
         else
          {
            throw Types::TypedOperationException("Error evaluating String.");
          }
       }
      else
       {
         throw Types::TypedOperationException("Error evaluating non-String.");
       }
    }

 } // namespace Engine

 } // namespace Backwards
