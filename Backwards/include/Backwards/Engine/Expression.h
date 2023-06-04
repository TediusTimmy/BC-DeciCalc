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
#ifndef BACKWARDS_ENGINE_EXPRESSION_H
#define BACKWARDS_ENGINE_EXPRESSION_H

#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Input/Token.h"

namespace Backwards
 {

namespace Engine
 {

   class FunctionContext;

   class Expression
    {
   public:
      Input::Token token;

      Expression(const Input::Token&);
      virtual ~Expression() = default;

       /* CallingContext can't be const, because if we propagate it
          to a function call, the function call is allowed to modify it. */
      virtual std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const = 0;

      static std::string constructMessage(const Types::TypedOperationException&, const Input::Token&);
      std::string constructMessage(const Types::TypedOperationException&) const;
    };

   class Constant final : public Expression
    {
   public:
      std::shared_ptr<Types::ValueType> value;

      Constant(const Input::Token&, const std::shared_ptr<Types::ValueType>&);

      std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const;
    };

   class Variable final : public Expression
    {
   public:
      std::shared_ptr<Getter> getter;

      Variable(const Input::Token&, const std::shared_ptr<Getter>&);

      std::shared_ptr<Types::ValueType> evaluate (CallingContext& context) const;
    };

#define BinaryOperation(x) \
   class x final : public Expression \
    { \
   public: \
      std::shared_ptr<Expression> lhs, rhs; \
      x(const Input::Token&, const std::shared_ptr<Expression>&, const std::shared_ptr<Expression>&); \
      std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const; \
    };

   BinaryOperation(Plus)
   BinaryOperation(Minus)
   BinaryOperation(Multiply)
   BinaryOperation(Divide)
   BinaryOperation(ShortAnd)
   BinaryOperation(ShortOr)
   BinaryOperation(Equals)
   BinaryOperation(NotEqual)
   BinaryOperation(Greater)
   BinaryOperation(Less)
   BinaryOperation(GEQ)
   BinaryOperation(LEQ)
   BinaryOperation(DerefVar)


#define UnaryOperation(x) \
   class x final : public Expression \
    { \
   public: \
      std::shared_ptr<Expression> arg; \
      x(const Input::Token&, const std::shared_ptr<Expression>&); \
      std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const; \
    };

   UnaryOperation(Not)
   UnaryOperation(Negate)



   class FunctionCall final : public Expression
    {
   public:
      std::shared_ptr<Expression> location;
      std::vector<std::shared_ptr<Expression> > args;

      FunctionCall(const Input::Token&, const std::shared_ptr<Expression>&, const std::vector<std::shared_ptr<Expression> >&);

      std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const;
    };



   class BuildFunction final : public Expression
    {
   public:
      std::shared_ptr<FunctionContext> prototype;
      std::weak_ptr<FunctionContext> prototypeToo;
      std::vector<std::shared_ptr<Expression> > captures;

      BuildFunction(const Input::Token&, const std::shared_ptr<FunctionContext>&, const std::vector<std::shared_ptr<Expression> >&);
      BuildFunction(const Input::Token&, const std::vector<std::shared_ptr<Expression> >&, const std::weak_ptr<FunctionContext>&);

      std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const;
    };



   class TernaryOperation final : public Expression
    {
   public:
      std::shared_ptr<Expression> condition, thenCase, elseCase;

      TernaryOperation(const Input::Token&,
         const std::shared_ptr<Expression>&, const std::shared_ptr<Expression>&, const std::shared_ptr<Expression>&);

      std::shared_ptr<Types::ValueType> evaluate (CallingContext&) const;
    };

 } // namespace Engine

 } // namespace Backwards

#endif /* BACKWARDS_ENGINE_EXPRESSION_H */
