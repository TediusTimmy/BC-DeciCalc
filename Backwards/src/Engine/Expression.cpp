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
#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/StdLib.h"
#include "Backwards/Engine/Statement.h"
#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/ConstantsSingleton.h"
#include "Backwards/Engine/DebuggerHook.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/StackFrame.h"

#include <sstream>

namespace Backwards
 {

namespace Engine
 {

   Expression::Expression(const Input::Token& token) : token(token)
    {
    }

   std::string Expression::constructMessage(const Types::TypedOperationException& e) const
    {
      return constructMessage(e, token);
    }

   std::string Expression::constructMessage(const Types::TypedOperationException& e, const Input::Token& token)
    {
      std::stringstream str;
      str << e.what() << std::endl
          << "\tFrom file " << token.sourceFile << " on line " << token.lineNumber << " at " << token.lineLocation;
      return str.str();
    }


   Constant::Constant(const Input::Token& token, const std::shared_ptr<Types::ValueType>& value) : Expression(token), value(value)
    {
    }

   std::shared_ptr<Types::ValueType> Constant::evaluate (CallingContext&) const
    {
      return value;
    }


   Variable::Variable(const Input::Token& token, const std::shared_ptr<Getter>& getter) : Expression(token), getter(getter)
    {
    }

   std::shared_ptr<Types::ValueType> Variable::evaluate (CallingContext& context) const
    {
      return getter->get(context);
    }


#define FullBinaryOperation(x,y) \
   x::x(const Input::Token& token, const std::shared_ptr<Expression>& lhs, const std::shared_ptr<Expression>& rhs) : \
      Expression(token), lhs(lhs), rhs(rhs) \
    { \
    } \
   std::shared_ptr<Types::ValueType> x::evaluate (CallingContext& context) const \
    { \
      /* We don't want to catch an exception generated while evaluating the arguments, */ \
      /* just the one from performing this operation. */ \
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context); \
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context); \
      std::shared_ptr<Types::ValueType> result; \
      try \
       { \
         result = LHS->y(*RHS); \
       } \
      catch (const Types::TypedOperationException& e) \
       { \
         std::string msg = constructMessage(e); \
         if (nullptr != context.debugger) \
          { \
            context.debugger->EnterDebugger(msg, context); \
          } \
         throw Types::TypedOperationException(msg); \
       } \
      return result; \
    }

   FullBinaryOperation(Plus, add)
   FullBinaryOperation(Minus, sub)
   FullBinaryOperation(Multiply, mul)
   FullBinaryOperation(Divide, div)


#define FullBinaryOperationShort(x,y) \
   x::x(const Input::Token& token, const std::shared_ptr<Expression>& lhs, const std::shared_ptr<Expression>& rhs) : \
      Expression(token), lhs(lhs), rhs(rhs) \
    { \
    } \
   std::shared_ptr<Types::ValueType> x::evaluate (CallingContext& context) const \
    { \
      /* We don't want to catch an exception generated while evaluating the arguments, */ \
      /* just the one from performing this operation. */ \
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context); \
      bool value; \
      try \
       { \
         value = LHS->logical(); \
       } \
      catch (const Types::TypedOperationException& e) \
       { \
         std::string msg = constructMessage(e); \
         if (nullptr != context.debugger) \
          { \
            context.debugger->EnterDebugger(msg, context); \
          } \
         throw Types::TypedOperationException(msg); \
       } \
      if (y == value) \
       { \
         return (true == y) ? \
            ConstantsSingleton::getInstance().FLOAT_ONE : \
            ConstantsSingleton::getInstance().FLOAT_ZERO; \
       } \
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context); \
      try \
       { \
         value = RHS->logical(); \
       } \
      catch (const Types::TypedOperationException& e) \
       { \
         std::string msg = constructMessage(e); \
         if (nullptr != context.debugger) \
          { \
            context.debugger->EnterDebugger(msg, context); \
          } \
         throw Types::TypedOperationException(msg); \
       } \
      return (true == value) ? \
         ConstantsSingleton::getInstance().FLOAT_ONE : \
         ConstantsSingleton::getInstance().FLOAT_ZERO; \
    }

   FullBinaryOperationShort(ShortAnd, false)
   FullBinaryOperationShort(ShortOr, true)


#define FullBinaryOperationRel(x,y) \
   x::x(const Input::Token& token, const std::shared_ptr<Expression>& lhs, const std::shared_ptr<Expression>& rhs) : \
      Expression(token), lhs(lhs), rhs(rhs) \
    { \
    } \
   std::shared_ptr<Types::ValueType> x::evaluate (CallingContext& context) const \
    { \
      /* We don't want to catch an exception generated while evaluating the arguments, */ \
      /* just the one from performing this operation. */ \
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context); \
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context); \
      bool result; \
      try \
       { \
         result = LHS->y(*RHS); \
       } \
      catch (const Types::TypedOperationException& e) \
       { \
         std::string msg = constructMessage(e); \
         if (nullptr != context.debugger) \
          { \
            context.debugger->EnterDebugger(msg, context); \
          } \
         throw Types::TypedOperationException(msg); \
       } \
      return (true == result) ? \
         ConstantsSingleton::getInstance().FLOAT_ONE : \
         ConstantsSingleton::getInstance().FLOAT_ZERO; \
    }

   FullBinaryOperationRel(Equals, equal)
   FullBinaryOperationRel(NotEqual, notEqual)
   FullBinaryOperationRel(Greater, greater)
   FullBinaryOperationRel(Less, less)
   FullBinaryOperationRel(GEQ, geq)
   FullBinaryOperationRel(LEQ, leq)


   DerefVar::DerefVar(const Input::Token& token, const std::shared_ptr<Expression>& lhs, const std::shared_ptr<Expression>& rhs) :
      Expression(token), lhs(lhs), rhs(rhs)
    {
    }

   std::shared_ptr<Types::ValueType> DerefVar::evaluate (CallingContext& context) const
    {
      /* We don't want to catch an exception generated while evaluating the arguments, */
      /* just the one from performing this operation. */
      std::shared_ptr<Types::ValueType> LHS = lhs->evaluate(context);
      std::shared_ptr<Types::ValueType> RHS = rhs->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      try
       {
         if (typeid(Types::ArrayValue) == typeid(*LHS))
          {
            result = GetIndex(LHS, RHS);
          }
         else if (typeid(Types::DictionaryValue) == typeid(*LHS))
          {
            result = GetValue(LHS, RHS);
          }
         else
          {
            throw Types::TypedOperationException("Error indexing non-Collection.");
          }
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = constructMessage(e);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return result;
    }


   Not::Not(const Input::Token& token, const std::shared_ptr<Expression>& arg) : Expression(token), arg(arg)
    {
    }

   std::shared_ptr<Types::ValueType> Not::evaluate (CallingContext& context) const
    {
      /* We don't want to catch an exception generated while evaluating the arguments, */
      /* just the one from performing this operation. */
      std::shared_ptr<Types::ValueType> ARG = arg->evaluate(context);
      bool result;
      try
       {
         result = ARG->logical();
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = constructMessage(e);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return (true == result) ? // Yes, these are backwards for a reason. This is where the NOT happens.
         ConstantsSingleton::getInstance().FLOAT_ZERO :
         ConstantsSingleton::getInstance().FLOAT_ONE;
    }


   Negate::Negate(const Input::Token& token, const std::shared_ptr<Expression>& arg) : Expression(token), arg(arg)
    {
    }

   std::shared_ptr<Types::ValueType> Negate::evaluate (CallingContext& context) const
    {
      /* We don't want to catch an exception generated while evaluating the arguments, */
      /* just the one from performing this operation. */
      std::shared_ptr<Types::ValueType> ARG = arg->evaluate(context);
      std::shared_ptr<Types::ValueType> result;
      try
       {
         result = ARG->neg();
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = constructMessage(e);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return result;
    }


   StackFrame::StackFrame(std::shared_ptr<FunctionContext> function, const Input::Token& callingToken, StackFrame* prev) :
      function(function), args(function->nargs), locals(function->nlocals), prev(prev), next(nullptr), callingToken(callingToken), depth(1U)
    {
      if (nullptr != prev)
       {
         depth = prev->depth + 1U;
       }
    }


   FunctionCall::FunctionCall(const Input::Token& token, const std::shared_ptr<Expression>& location, const std::vector<std::shared_ptr<Expression> >& args) :
      Expression(token), location(location), args(args)
    {
    }

   std::shared_ptr<Types::ValueType> FunctionCall::evaluate (CallingContext& context) const
    {
      /* We don't want to catch an exception generated while evaluating the arguments, */
      /* just the one from performing this operation. */
      std::shared_ptr<Types::ValueType> LOC = location->evaluate(context);
      if (false == (typeid(Types::FunctionValue) == typeid(*LOC)))
       {
         std::stringstream str;
         str << "Call to not a Function at " << token.lineLocation << " on line " << token.lineNumber << " in file " << token.sourceFile;
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(str.str(), context);
          }
         throw FatalException(str.str());
       }
      std::shared_ptr<FunctionContext> function = std::dynamic_pointer_cast<FunctionContext>((std::dynamic_pointer_cast<Types::FunctionValue>(LOC)->valueToo).lock());
      if (nullptr == function.get())
       {
         function = std::dynamic_pointer_cast<FunctionContext>(std::dynamic_pointer_cast<Types::FunctionValue>(LOC)->value);
       }
      if (args.size() != function->nargs)
       {
         std::stringstream str;
         str << "Call to function with " << args.size() << " arguments, but function takes " << function->nargs <<
            " arguments at " << token.lineLocation << " on line " << token.lineNumber << " in file " << token.sourceFile;
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(str.str(), context);
          }
         throw FatalException(str.str());
       }
      StackFrame frame (function, token, context.currentFrame);
      frame.captures = std::dynamic_pointer_cast<Types::FunctionValue>(LOC)->captures;
      for (size_t i = 0U; i < args.size(); ++i)
       {
         frame.args[i] = args[i]->evaluate(context);
       }
      /* Can't link the frames until here, as we may use the current frame to compute the args, */
      /* and/or push multiple other frames onto the stack. */
      context.pushContext(&frame);
      try
       {
         std::shared_ptr<FlowControl> result;
         try
          {
            result = function->function->execute(context);
          }
         catch (const Types::TypedOperationException& e)
          {
            std::string msg = constructMessage(e);
            throw Types::TypedOperationException(msg);
          }
         if (nullptr == result.get())
          {
            std::stringstream str;
            str << "Function failed to return a value at " << token.lineLocation << " on line " << token.lineNumber << " in file " << token.sourceFile;
            throw FatalException(str.str());
          }
         if (FlowControl::RETURN != result->type)
          {
            std::stringstream str;
            str << "Function had a 'break' or 'continue' outside of a loop at " << token.lineLocation << " on line " << token.lineNumber << " in file " << token.sourceFile;
            if (nullptr != context.debugger)
             {
               context.debugger->EnterDebugger(str.str(), context);
             }
            throw FatalException(str.str());
          }
         context.popContext();
         return result->value;
       }
      catch (...)
       {
         context.popContext();
         throw;
       }
    }


   BuildFunction::BuildFunction(const Input::Token& token, const std::shared_ptr<FunctionContext>& prototype, const std::vector<std::shared_ptr<Expression> >& captures) :
      Expression(token), prototype(prototype), captures(captures)
    {
    }

   BuildFunction::BuildFunction(const Input::Token& token, const std::vector<std::shared_ptr<Expression> >& captures, const std::weak_ptr<FunctionContext>& prototype) :
      Expression(token), prototypeToo(prototype), captures(captures)
    {
    }

   std::shared_ptr<Types::ValueType> BuildFunction::evaluate (CallingContext& context) const
    {
       /* This doesn't perform an operation that can fail. */
      std::vector<std::shared_ptr<Types::ValueType> > captured;
      for (std::vector<std::shared_ptr<Expression> >::const_iterator iter = captures.begin(); captures.end() != iter; ++iter)
       {
         captured.emplace_back((*iter)->evaluate(context));
       }
      if (true == prototypeToo.expired())
       {
         return std::make_shared<Types::FunctionValue>(prototype, captured);
       }
      return std::make_shared<Types::FunctionValue>(captured, prototypeToo);
    }


   TernaryOperation::TernaryOperation(const Input::Token& token,
      const std::shared_ptr<Expression>& condition, const std::shared_ptr<Expression>& thenCase, const std::shared_ptr<Expression>& elseCase) :
      Expression(token), condition(condition), thenCase(thenCase), elseCase(elseCase)
    {
    }

   std::shared_ptr<Types::ValueType> TernaryOperation::evaluate (CallingContext& context) const
    {
      /* We don't want to catch an exception generated while evaluating the arguments, */
      /* just the one from performing this operation. */
      std::shared_ptr<Types::ValueType> COND = condition->evaluate(context);
      bool result;
      try
       {
         result = COND->logical();
       }
      catch (const Types::TypedOperationException& e)
       {
         std::string msg = constructMessage(e);
         if (nullptr != context.debugger)
          {
            context.debugger->EnterDebugger(msg, context);
          }
         throw Types::TypedOperationException(msg);
       }
      return (true == result) ?
         thenCase->evaluate(context) :
         elseCase->evaluate(context);
    }

 } // namespace Engine

 } // namespace Backwards
