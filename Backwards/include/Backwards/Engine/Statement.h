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
#ifndef BACKWARDS_ENGINE_STATEMENT_H
#define BACKWARDS_ENGINE_STATEMENT_H

#include "Backwards/Engine/CallingContext.h"

#include "Backwards/Input/Token.h"
#include "Backwards/Types/ValueType.h"

#include <string>
#include <vector>
#include <memory>

namespace Backwards
 {

namespace Engine
 {

   class Expression;

   class FlowControl final
    {
   public:

      enum Type
       {
         RETURN,
         BREAK,
         CONTINUE
       };

      static const size_t NO_TARGET;

      const Input::Token& source;
      Type type;
      size_t target;
      std::shared_ptr<Types::ValueType> value;

      FlowControl(const Input::Token&, Type, size_t, const std::shared_ptr<Types::ValueType>&);
      ~FlowControl() = default;
    };

   class Statement
    {
   public:
      Input::Token token;

      Statement(const Input::Token&);
      virtual ~Statement() = default;

       /* CallingContext can't be const, because if we propagate it
          to a function call, the function call is allowed to modify it. */
      virtual std::shared_ptr<FlowControl> execute (CallingContext&) const = 0;
    };

   class NOP final : public Statement
    {
   public:
      explicit NOP(const Input::Token&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class Expr final : public Statement
    {
   public:
      std::shared_ptr<Expression> expr;

      Expr(const Input::Token&, const std::shared_ptr<Expression>&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StatementSeq final : public Statement
    {
   public:
      std::vector<std::shared_ptr<Statement> > statements;

      StatementSeq(const Input::Token&, const std::vector<std::shared_ptr<Statement> >&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class RecAssignState final
    {
   public:
      Input::Token token;

      std::shared_ptr<Expression> index;
      std::shared_ptr<RecAssignState> next;

      RecAssignState(const Input::Token&, const std::shared_ptr<Expression>&);

      std::shared_ptr<Types::ValueType> evaluate (CallingContext&, const std::shared_ptr<Types::ValueType>& lhs, const std::shared_ptr<Expression>& rhs) const;

      std::shared_ptr<Types::ValueType> getIndex (std::shared_ptr<Types::ValueType> container, std::shared_ptr<Types::ValueType> index, CallingContext&) const;
      std::shared_ptr<Types::ValueType> setIndex (std::shared_ptr<Types::ValueType> container, std::shared_ptr<Types::ValueType> index,
         std::shared_ptr<Types::ValueType> value, CallingContext&) const;
    };

   class Assignment final : public Statement
    {
   public:
      std::shared_ptr<Getter> getter;
      std::shared_ptr<Setter> setter;
      std::shared_ptr<RecAssignState> index;
      std::shared_ptr<Expression> rhs;

      Assignment(const Input::Token&, const std::shared_ptr<Getter>&, const std::shared_ptr<Setter>&,
         const std::shared_ptr<RecAssignState>&, const std::shared_ptr<Expression>&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class IfStatement final : public Statement
    {
   public:
      std::shared_ptr<Expression> condition;
      std::shared_ptr<Statement> thenSeq;
      std::shared_ptr<Statement> elseSeq;

      IfStatement(const Input::Token&, const std::shared_ptr<Expression>&, const std::shared_ptr<Statement>&, const std::shared_ptr<Statement>&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class WhileStatement final : public Statement
    {
   public:
      std::shared_ptr<Expression> condition;
      std::shared_ptr<Statement> seq;
      size_t id;

      WhileStatement(const Input::Token&, const std::shared_ptr<Expression>&, const std::shared_ptr<Statement>&, size_t);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class CaseContainer final
    {
   public:

      enum CaseType
       {
         AT,
         ABOVE,
         BELOW
       };

      Input::Token token;

      bool breaking;
      CaseType type;
      std::shared_ptr<Expression> condition;
      std::shared_ptr<Expression> lower;
      std::shared_ptr<Statement> seq;

      CaseContainer(const Input::Token&, bool, CaseType, const std::shared_ptr<Expression>&, const std::shared_ptr<Expression>&, const std::shared_ptr<Statement>&);

      bool evaluate (CallingContext&, const std::shared_ptr<Types::ValueType>&) const;
    };

   class SelectStatement final : public Statement
    {
   public:
      std::shared_ptr<Expression> control;
      std::vector<std::shared_ptr<CaseContainer> > cases;

      SelectStatement(const Input::Token&, const std::shared_ptr<Expression>&, const std::vector<std::shared_ptr<CaseContainer> >&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class ForStatement final : public Statement
    {
   private:
      std::shared_ptr<FlowControl> loopIter (CallingContext&, std::shared_ptr<Types::ValueType>) const;
      std::shared_ptr<FlowControl> collIter (CallingContext&, std::shared_ptr<Types::ValueType>) const;

   public:
      std::shared_ptr<Getter> getter;
      std::shared_ptr<Setter> setter;
      std::shared_ptr<Expression> lower;
      bool to;
      std::shared_ptr<Expression> upper;
      std::shared_ptr<Expression> step;
      std::shared_ptr<Statement> seq;
      size_t id;

      ForStatement(const Input::Token&, const std::shared_ptr<Getter>&, const std::shared_ptr<Setter>&,
         const std::shared_ptr<Expression>&, bool, const std::shared_ptr<Expression>&,
         const std::shared_ptr<Expression>&, const std::shared_ptr<Statement>&, size_t);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class FlowControlStatement final : public Statement
    {
   public:
      FlowControl::Type type;
      size_t target;
      std::shared_ptr<Expression> value;

      FlowControlStatement(const Input::Token&, FlowControl::Type, size_t, const std::shared_ptr<Expression>&);

      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StandardConstantFunction final : public Statement
    {
   public:
      ConstantFunctionPointer function;
      explicit StandardConstantFunction(ConstantFunctionPointer);
      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StandardConstantFunctionWithContext final : public Statement
    {
   public:
      ConstantFunctionPointerWithContext function;
      explicit StandardConstantFunctionWithContext(ConstantFunctionPointerWithContext);
      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StandardUnaryFunction final : public Statement
    {
   public:
      UnaryFunctionPointer function;
      explicit StandardUnaryFunction(UnaryFunctionPointer);
      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StandardUnaryFunctionWithContext final : public Statement
    {
   public:
      UnaryFunctionPointerWithContext function;
      explicit StandardUnaryFunctionWithContext(UnaryFunctionPointerWithContext);
      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StandardBinaryFunction final : public Statement
    {
   public:
      BinaryFunctionPointer function;
      explicit StandardBinaryFunction(BinaryFunctionPointer);
      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

   class StandardTernaryFunction final : public Statement
    {
   public:
      TernaryFunctionPointer function;
      explicit StandardTernaryFunction(TernaryFunctionPointer);
      std::shared_ptr<FlowControl> execute (CallingContext&) const override;
    };

 } // namespace Engine

 } // namespace Backwards

#endif /* BACKWARDS_ENGINE_STATEMENT_H */
