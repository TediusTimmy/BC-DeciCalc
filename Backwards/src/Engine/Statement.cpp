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
#include "Backwards/Engine/Statement.h"
#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/StdLib.h"
#include "Backwards/Engine/StackFrame.h"

#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"

#include "Backwards/Engine/ConstantsSingleton.h"
#include "Backwards/Engine/DebuggerHook.h"

#include <sstream>

namespace Backwards
 {

namespace Engine
 {

   FlowControl::FlowControl(const Input::Token& source, Type type, size_t target, const std::shared_ptr<Types::ValueType>& value) : source(source), type(type), target(target), value(value)
    {
    }

   const size_t FlowControl::NO_TARGET = 0U;

   Statement::Statement(const Input::Token& token) : token(token)
    {
    }


   NOP::NOP(const Input::Token& token) : Statement(token)
    {
    }

   std::shared_ptr<FlowControl> NOP::execute (CallingContext&) const
    {
      return std::shared_ptr<FlowControl>();
    }


   Expr::Expr(const Input::Token& token, const std::shared_ptr<Expression>& expr) : Statement(token), expr(expr)
    {
    }

   std::shared_ptr<FlowControl> Expr::execute (CallingContext& context) const
    {
      (void) expr->evaluate(context);
      return std::shared_ptr<FlowControl>();
    }


   StatementSeq::StatementSeq(const Input::Token& token, const std::vector<std::shared_ptr<Statement> >& statements) :
      Statement(token), statements(statements)
    {
    }

   std::shared_ptr<FlowControl> StatementSeq::execute (CallingContext& context) const
    {
      for (std::vector<std::shared_ptr<Statement> >::const_iterator iter = statements.begin();
         statements.end() != iter; ++iter)
       {
         std::shared_ptr<FlowControl> temp = (*iter)->execute(context);
         if (nullptr != temp.get())
          {
            return temp;
          }
       }
      return std::shared_ptr<FlowControl>();
    }


   RecAssignState::RecAssignState(const Input::Token& token, const std::shared_ptr<Expression>& index) :
      token(token), index(index)
    {
    }

   std::shared_ptr<Types::ValueType> RecAssignState::evaluate
      (CallingContext& context, const std::shared_ptr<Types::ValueType>& lhs, const std::shared_ptr<Expression>& rhs) const
    {
      std::shared_ptr<Types::ValueType> result;
      if (nullptr == next.get())
       {
         result = setIndex(lhs, index->evaluate(context), rhs->evaluate(context), context);
       }
      else
       {
         std::shared_ptr<Types::ValueType> arrayIndex = index->evaluate(context);
          {
            result = setIndex(lhs, arrayIndex, next->evaluate(context, getIndex(lhs, arrayIndex, context), rhs), context);
          }
       }
      return result;
    }

   std::shared_ptr<Types::ValueType> RecAssignState::getIndex (std::shared_ptr<Types::ValueType> container, std::shared_ptr<Types::ValueType> index,
      CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> result;
      try
       {
         if (typeid(Types::ArrayValue) == typeid(*container))
          {
            result = GetIndex(container, index);
          }
         else if (typeid(Types::DictionaryValue) == typeid(*container))
          {
            result = GetValue(container, index);
          }
         else
          {
            throw Types::TypedOperationException("Error indexing non-Collection.");
          }
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = Expression::constructMessage(e, token);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return result;
    }

   std::shared_ptr<Types::ValueType> RecAssignState::setIndex (std::shared_ptr<Types::ValueType> container, std::shared_ptr<Types::ValueType> index,
      std::shared_ptr<Types::ValueType> value, CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> result;
      try
       {
         if (typeid(Types::ArrayValue) == typeid(*container))
          {
            result = SetIndex(container, index, value);
          }
         else if (typeid(Types::DictionaryValue) == typeid(*container))
          {
            result = Insert(container, index, value);
          }
         else
          {
            throw Types::TypedOperationException("Error indexing non-Collection.");
          }
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = Expression::constructMessage(e, token);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return result;
    }


   Assignment::Assignment(const Input::Token& token, const std::shared_ptr<Getter>& getter, const std::shared_ptr<Setter>& setter,
      const std::shared_ptr<RecAssignState>& index, const std::shared_ptr<Expression>& rhs) :
      Statement(token), getter(getter), setter(setter), index(index), rhs(rhs)
    {
    }

   std::shared_ptr<FlowControl> Assignment::execute (CallingContext& context) const
    {
      if (nullptr == index.get())
       {
         setter->set(context, rhs->evaluate(context));
       }
      else
       {
         setter->set(context, index->evaluate(context, getter->get(context), rhs));
       }
      return std::shared_ptr<FlowControl>();
    }


   IfStatement::IfStatement(const Input::Token& token, const std::shared_ptr<Expression>& condition,
      const std::shared_ptr<Statement>& thenSeq, const std::shared_ptr<Statement>& elseSeq) :
      Statement(token), condition(condition), thenSeq(thenSeq), elseSeq(elseSeq)
    {
    }

   std::shared_ptr<FlowControl> IfStatement::execute (CallingContext& context) const
    {
      bool conditional = true;
      try
       {
         conditional = condition->evaluate(context)->logical();
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = Expression::constructMessage(e, token);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      if (true == conditional)
       {
         return thenSeq->execute(context);
       }
      else
       {
         return elseSeq->execute(context);
       }
    }


   WhileStatement::WhileStatement(const Input::Token& token, const std::shared_ptr<Expression>& condition, const std::shared_ptr<Statement>& seq, size_t id) :
      Statement(token), condition(condition), seq(seq), id(id)
    {
    }

   std::shared_ptr<FlowControl> WhileStatement::execute (CallingContext& context) const
    {
      bool conditional = true;
      try
       {
         conditional = condition->evaluate(context)->logical();
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = Expression::constructMessage(e, token);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      while (true == conditional)
       {
         std::shared_ptr<FlowControl> temp = seq->execute(context);

         if (nullptr != temp.get())
          {
            switch (temp->type)
             {
            case FlowControl::RETURN:
               return temp; // Pass it up.
            case FlowControl::BREAK:
               if (id == temp->target)
                {
                  return std::shared_ptr<FlowControl>(); // Loop is done.
                }
               else
                {
                  return temp; // Not for me, pass it up.
                }
            case FlowControl::CONTINUE:
               if (id != temp->target)
                {
                  return temp; // Not for me, pass it up.
                }
               // Else do nothing: the previous iteration has stopped and we will move on to the next.
             }
          }

         try
          {
            conditional = condition->evaluate(context)->logical();
          }
         catch (const Types::TypedOperationException& e)
          {
            std::string msg = Expression::constructMessage(e, token);
            if (nullptr != context.debugger)
             {
               context.debugger->EnterDebugger(msg, context);
             }
            throw Types::TypedOperationException(msg);
          }
       }
      return std::shared_ptr<FlowControl>();
    }


   CaseContainer::CaseContainer(const Input::Token& token, bool breaking, CaseType type, const std::shared_ptr<Expression>& condition, const std::shared_ptr<Expression>& lower, const std::shared_ptr<Statement>& seq) :
      token(token), breaking(breaking), type(type), condition(condition), lower(lower), seq(seq)
    {
    }

   bool CaseContainer::evaluate (CallingContext& context, const std::shared_ptr<Types::ValueType>& controlVal) const
    {
      bool result = false;
      try
       {
         if (nullptr == condition.get()) // case else
          {
            result = true;
          }
         else if (nullptr == lower.get()) // one comparison
          {
            switch (type)
             {
            case AT:
               result = condition->evaluate(context)->equal(*controlVal);
               break;
            case ABOVE: // This is inverted because we have inverted the condition.
               result = condition->evaluate(context)->leq(*controlVal);
               break;
            case BELOW: // This is inverted because we have inverted the condition.
               result = condition->evaluate(context)->geq(*controlVal);
               break;
             }
          }
         else // two comparisons
          {
            std::shared_ptr<Types::ValueType> TOP = condition->evaluate(context);
            std::shared_ptr<Types::ValueType> BOTTOM = lower->evaluate(context);
            result = BOTTOM->leq(*controlVal) && TOP->geq(*controlVal);
          }
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = Expression::constructMessage(e, token);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return result;
    }

   SelectStatement::SelectStatement(const Input::Token& token, const std::shared_ptr<Expression>& control, const std::vector<std::shared_ptr<CaseContainer> >& cases) :
      Statement(token), control(control), cases(cases)
    {
    }

   std::shared_ptr<FlowControl> SelectStatement::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> controlVal = control->evaluate(context);

      bool end = false;
      for (std::vector<std::shared_ptr<CaseContainer> >::const_iterator iter = cases.begin();
         (false == end) && (cases.end() != iter); ++iter)
       {
         if (true == (*iter)->evaluate(context, controlVal))
          {
            do
             {
               std::shared_ptr<FlowControl> temp = (*iter)->seq->execute(context);
               if (nullptr != temp.get())
                {
                  return temp;
                }
               ++iter;
             }
            while ((cases.end() != iter) && (false == (*iter)->breaking));
            end = true;
          }
       }
      return std::shared_ptr<FlowControl>();
    }


   ForStatement::ForStatement(const Input::Token& token, const std::shared_ptr<Getter>& getter, const std::shared_ptr<Setter>& setter,
         const std::shared_ptr<Expression>& lower, bool to, const std::shared_ptr<Expression>& upper,
         const std::shared_ptr<Expression>& step, const std::shared_ptr<Statement>& seq, size_t id) :
      Statement(token), getter(getter), setter(setter), lower(lower), to(to), upper(upper), step(step), seq(seq), id(id)
    {
    }

   std::shared_ptr<FlowControl> ForStatement::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> currentValue = lower->evaluate(context);

      if (nullptr == upper.get())
       {
         return collIter(context, currentValue);
       }
      else
       {
         return loopIter(context, currentValue);
       }
    }

   std::shared_ptr<FlowControl> ForStatement::loopIter (CallingContext& context, std::shared_ptr<Types::ValueType> currentValue) const
    {
      std::shared_ptr<Types::ValueType> UPPER = upper->evaluate(context);
      std::shared_ptr<Types::ValueType> STEP;
      if (nullptr == step.get())
       {
         if (true == to)
          {
            STEP = ConstantsSingleton::getInstance().FLOAT_ONE;
          }
         else
          {
            STEP = std::make_shared<Types::FloatValue>(BigInt::Fixed("-1"));
          }
       }
      else
       {
         STEP = step->evaluate(context);
       }
      std::shared_ptr<Expression> del = std::make_shared<Constant>(token, STEP);

      while (true)
       {
         setter->set(context, currentValue);

         std::shared_ptr<Expression> lcv = std::make_shared<Constant>(token, currentValue);
         std::shared_ptr<Expression> limit = std::make_shared<Constant>(token, UPPER);
         std::shared_ptr<Expression> comparator;
         if (true == to)
          {
            comparator = std::make_shared<LEQ>(token, lcv, limit);
          }
         else
          {
            comparator = std::make_shared<GEQ>(token, lcv, limit);
          }

         bool conditional = false;
         try
          {
            conditional = comparator->evaluate(context)->logical();
          }
         catch (const Types::TypedOperationException& e)
          {
            std::string msg = Expression::constructMessage(e, token);
            if (nullptr != context.debugger)
             {
               context.debugger->EnterDebugger(msg, context);
             }
            throw Types::TypedOperationException(msg);
          }

         if (false == conditional)
          {
            break;
          }

         std::shared_ptr<FlowControl> temp = seq->execute(context);

         if (nullptr != temp.get())
          {
            switch (temp->type)
             {
            case FlowControl::RETURN:
               return temp; // Pass it up.
            case FlowControl::BREAK:
               if (id == temp->target)
                {
                  return std::shared_ptr<FlowControl>(); // Loop is done.
                }
               else
                {
                  return temp; // Not for me, pass it up.
                }
            case FlowControl::CONTINUE:
               if (id != temp->target)
                {
                  return temp; // Not for me, pass it up.
                }
               // Else do nothing: the previous iteration has stopped and we will move on to the next.
             }
          }

         Plus plus (token, lcv, del);
         currentValue = plus.evaluate(context);
       }
      return std::shared_ptr<FlowControl>();
    }

   static std::shared_ptr<FlowControl> arrayIter(CallingContext& context, std::shared_ptr<Types::ArrayValue> currentValue, const std::shared_ptr<Setter>& setter, const std::shared_ptr<Statement>& seq, size_t id)
    {
      for (std::shared_ptr<Types::ValueType> iter : currentValue->value)
       {
         setter->set(context, iter);

         std::shared_ptr<FlowControl> temp = seq->execute(context);

         if (nullptr != temp.get())
          {
            switch (temp->type)
             {
            case FlowControl::RETURN:
               return temp; // Pass it up.
            case FlowControl::BREAK:
               if (id == temp->target)
                {
                  return std::shared_ptr<FlowControl>(); // Loop is done.
                }
               else
                {
                  return temp; // Not for me, pass it up.
                }
            case FlowControl::CONTINUE:
               if (id != temp->target)
                {
                  return temp; // Not for me, pass it up.
                }
               // Else do nothing: the previous iteration has stopped and we will move on to the next.
             }
          }
       }
      return std::shared_ptr<FlowControl>();
    }

   static std::shared_ptr<FlowControl> dictIter(CallingContext& context, std::shared_ptr<Types::DictionaryValue> currentValue, const std::shared_ptr<Setter>& setter, const std::shared_ptr<Statement>& seq, size_t id)
    {
      for (auto iter : currentValue->value)
       {
         std::shared_ptr<Types::ArrayValue> currIter = std::make_shared<Types::ArrayValue>();
         currIter->value.push_back(iter.first);
         currIter->value.push_back(iter.second);
         setter->set(context, currIter);

         std::shared_ptr<FlowControl> temp = seq->execute(context);

         if (nullptr != temp.get())
          {
            switch (temp->type)
             {
            case FlowControl::RETURN:
               return temp; // Pass it up.
            case FlowControl::BREAK:
               if (id == temp->target)
                {
                  return std::shared_ptr<FlowControl>(); // Loop is done.
                }
               else
                {
                  return temp; // Not for me, pass it up.
                }
            case FlowControl::CONTINUE:
               if (id != temp->target)
                {
                  return temp; // Not for me, pass it up.
                }
               // Else do nothing: the previous iteration has stopped and we will move on to the next.
             }
          }
       }
      return std::shared_ptr<FlowControl>();
    }

   std::shared_ptr<FlowControl> ForStatement::collIter (CallingContext& context, std::shared_ptr<Types::ValueType> currentValue) const
    {
      if (typeid(Types::ArrayValue) == typeid(*currentValue.get()))
       {
         return arrayIter(context, std::dynamic_pointer_cast<Types::ArrayValue>(currentValue), setter, seq, id);
       }
      else if (typeid(Types::DictionaryValue) == typeid(*currentValue.get()))
       {
         return dictIter(context, std::dynamic_pointer_cast<Types::DictionaryValue>(currentValue), setter, seq, id);
       }
      else
       {
         Types::TypedOperationException notThrown ("Error iterating over non-Collection.");
         std::string msg = Expression::constructMessage(notThrown, token);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
    }


   FlowControlStatement::FlowControlStatement(const Input::Token& token, FlowControl::Type type, size_t target, const std::shared_ptr<Expression>& value) :
      Statement(token), type(type), target(target), value(value)
    {
    }

   std::shared_ptr<FlowControl> FlowControlStatement::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> VALUE;
      if (nullptr != value.get())
       {
         try
          {
            VALUE = value->evaluate(context);
          }
         catch (const Types::TypedOperationException& e)
          {
            std::string msg = Expression::constructMessage(e, token);
            if (nullptr != context.debugger)
             {
               context.debugger->EnterDebugger(msg, context);
             }
            throw Types::TypedOperationException(msg);
          }
       }
      return std::make_shared<FlowControl>(token, type, target, VALUE);
    }


   StandardConstantFunction::StandardConstantFunction(ConstantFunctionPointer function) : Statement(Input::Token()), function(function)
    {
    }

   std::shared_ptr<FlowControl> StandardConstantFunction::execute (CallingContext&) const
    {
      return std::make_shared<FlowControl>(token, FlowControl::RETURN, FlowControl::NO_TARGET, function());
    }


   StandardConstantFunctionWithContext::StandardConstantFunctionWithContext(ConstantFunctionPointerWithContext function) : Statement(Input::Token()), function(function)
    {
    }

   std::shared_ptr<FlowControl> StandardConstantFunctionWithContext::execute (CallingContext& context) const
    {
      return std::make_shared<FlowControl>(token, FlowControl::RETURN, FlowControl::NO_TARGET, function(context));
    }


   StandardUnaryFunction::StandardUnaryFunction(UnaryFunctionPointer function) : Statement(Input::Token()), function(function)
    {
    }

   std::shared_ptr<FlowControl> StandardUnaryFunction::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> arg = context.currentFrame->args[0U];
      try
       {
         return std::make_shared<FlowControl>(token, FlowControl::RETURN, FlowControl::NO_TARGET, function(arg));
       }
      catch (const Types::TypedOperationException& e)
       {
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(e.what(), context);
          }
         throw;
       }
    }


   StandardUnaryFunctionWithContext::StandardUnaryFunctionWithContext(UnaryFunctionPointerWithContext function) : Statement(Input::Token()), function(function)
    {
    }

   std::shared_ptr<FlowControl> StandardUnaryFunctionWithContext::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> arg = context.currentFrame->args[0U];
      try
       {
         return std::make_shared<FlowControl>(token, FlowControl::RETURN, FlowControl::NO_TARGET, function(context, arg));
       }
      catch (const Types::TypedOperationException& e)
       {
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(e.what(), context);
          }
         throw;
       }
    }


   StandardBinaryFunction::StandardBinaryFunction(BinaryFunctionPointer function) : Statement(Input::Token()), function(function)
    {
    }

   std::shared_ptr<FlowControl> StandardBinaryFunction::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> lhs = context.currentFrame->args[0U];
      std::shared_ptr<Types::ValueType> rhs = context.currentFrame->args[1U];
      try
       {
         return std::make_shared<FlowControl>(token, FlowControl::RETURN, FlowControl::NO_TARGET, function(lhs, rhs));
       }
      catch (const Types::TypedOperationException& e)
       {
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(e.what(), context);
          }
         throw;
       }
    }


   StandardTernaryFunction::StandardTernaryFunction(TernaryFunctionPointer function) : Statement(Input::Token()), function(function)
    {
    }

   std::shared_ptr<FlowControl> StandardTernaryFunction::execute (CallingContext& context) const
    {
      std::shared_ptr<Types::ValueType> first = context.currentFrame->args[0U];
      std::shared_ptr<Types::ValueType> second = context.currentFrame->args[1U];
      std::shared_ptr<Types::ValueType> third = context.currentFrame->args[2U];
      try
       {
         return std::make_shared<FlowControl>(token, FlowControl::RETURN, FlowControl::NO_TARGET, function(first, second, third));
       }
      catch (const Types::TypedOperationException& e)
       {
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(e.what(), context);
          }
         throw;
       }
    }

 } // namespace Engine

 } // namespace Backwards
