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
#include "Backwards/Engine/DebuggerHook.h"

#include <sstream>

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"

#include "Backwards/Engine/CallingContext.h"
#include "Backwards/Engine/Expression.h"
#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/FunctionContext.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/StackFrame.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/ArrayValue.h"
#include "Backwards/Types/DictionaryValue.h"
#include "Backwards/Types/FunctionValue.h"

namespace Backwards
 {

namespace Engine
 {

   void DefaultDebugger::printValue(std::ostream& stream, const std::shared_ptr<Types::ValueType>& val)
    {
      if (nullptr != val.get())
       {
         if (typeid(Types::FloatValue) == typeid(*val))
          {
            stream << std::dynamic_pointer_cast<const Types::FloatValue>(val)->value.toString();
          }
         else if (typeid(Types::StringValue) == typeid(*val))
          {
            stream << "\"" << std::dynamic_pointer_cast<const Types::StringValue>(val)->value << "\"";
          }
         else if (typeid(Types::ArrayValue) == typeid(*val))
          {
            const std::vector<std::shared_ptr<Types::ValueType> >& array = std::dynamic_pointer_cast<const Types::ArrayValue>(val)->value;
            stream << "{ ";
            for (std::vector<std::shared_ptr<Types::ValueType> >::const_iterator iter = array.begin();
               array.end() != iter; ++iter)
             {
               if (array.begin() != iter)
                {
                  stream << "; ";
                }
               printValue(stream, *iter);
             }
            stream << " }";
          }
         else if (typeid(Types::DictionaryValue) == typeid(*val))
          {
            const std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>& dict =
               std::dynamic_pointer_cast<const Types::DictionaryValue>(val)->value;
            stream << "{ ";
            for (std::map<std::shared_ptr<Types::ValueType>, std::shared_ptr<Types::ValueType>, Types::ChristHowHorrifying>::const_iterator
               iter = dict.begin(); dict.end() != iter; ++iter)
             {
               if (dict.begin() != iter)
                {
                  stream << "; ";
                }
               printValue(stream, iter->first);
               stream << ":";
               printValue(stream, iter->second);
             }
            stream << " }";
          }
         else if (typeid(Types::FunctionValue) == typeid(*val))
          {
            stream << "Function : " << std::dynamic_pointer_cast<const FunctionContext>(std::dynamic_pointer_cast<const Types::FunctionValue>(val)->value)->name;
            const std::vector<std::shared_ptr<Types::ValueType> >& array = std::dynamic_pointer_cast<const Types::FunctionValue>(val)->captures;
            if (false == array.empty())
             {
               stream << " [ ";
               for (std::vector<std::shared_ptr<Types::ValueType> >::const_iterator iter = array.begin();
                  array.end() != iter; ++iter)
                {
                  if (array.begin() != iter)
                   {
                     stream << "; ";
                   }
                  printValue(stream, *iter);
                }
               stream << " ]";
             }
          }
         else
          {
            stream << "Type not understood.";
          }
       }
      else
       {
         stream << "A collection contains a NULL.";
       }
    }

   static void outputFrame(std::ostream& out, StackFrame* frame)
   {
      out << "#" << frame->depth << ": >" << frame->function->name <<
         "< from line " << frame->callingToken.lineNumber << " in " << frame->callingToken.sourceFile;
   }

   void DefaultDebugger::EnterDebugger(const std::string& exceptionMessage, CallingContext& context)
    {
      if (false == exceptionMessage.empty())
       {
         context.logger->log("Entered debugger with message: " + exceptionMessage);
       }

      std::string line, prevLine;
      StackFrame* frame = context.currentFrame;
      std::stringstream str;
      str << "In function ";
      outputFrame(str, frame);
      context.logger->log(str.str());
      line = context.logger->get();
      while ("quit" != line)
      {
         // Empty lines repeat the previous command.
         if ("" == line)
          {
            line = prevLine;
          }

         if ("down" == line)
          {
            if (nullptr == frame->prev)
             {
               context.logger->log("Already in bottom-most frame.");
             }
            else
             {
               frame = frame->prev;
               std::stringstream str;
               str << "In function ";
               outputFrame(str, frame);
               context.logger->log(str.str());
             }
          }
         else if ("up" == line)
          {
            if (nullptr == frame->next)
             {
               context.logger->log("Already in top-most frame.");
             }
            else
             {
               frame = frame->next;
               std::stringstream str;
               str << "In function ";
               outputFrame(str, frame);
               context.logger->log(str.str());
             }
          }
         else if ("bt" == line)
          {
            StackFrame* tframe = frame;
            std::stringstream str;
            while (nullptr != tframe)
             {
               if (tframe != frame)
                {
                  str << std::endl;
                }
               outputFrame(str, tframe);
               tframe = tframe->prev;
             }
            context.logger->log(str.str());
          }
         else if ("show" == line)
          {
            bool printComma = false;
            std::stringstream str;
            str << "These variables are in the current stack frame: ";
            for (std::vector<std::string>::const_iterator iter = frame->function->argNames.begin();
               frame->function->argNames.end() != iter; ++iter)
             {
               if (frame->function->argNames.begin() != iter)
                {
                  str << ", ";
                }
               str << *iter;
               printComma = true;
             }
            for (std::vector<std::string>::const_iterator iter = frame->function->localNames.begin();
               frame->function->localNames.end() != iter; ++iter)
             {
               if ((frame->function->localNames.begin() != iter) || (true == printComma))
                {
                  str << ", ";
                }
               str << *iter;
               printComma = true;
             }
            for (std::vector<std::string>::const_iterator iter = frame->function->captureNames.begin();
               frame->function->captureNames.end() != iter; ++iter)
             {
               if ((frame->function->captureNames.begin() != iter) || (true == printComma))
                {
                  str << ", ";
                }
               str << *iter;
               printComma = true;
             }
            if (nullptr != context.topScope())
             {
               str << std::endl << "These variables are in the current scope: ";
               for (std::vector<std::string>::const_iterator iter = context.topScope()->names.begin();
                  context.topScope()->names.end() != iter; ++iter)
                {
                  if (context.topScope()->names.begin() != iter)
                   {
                     str << ", ";
                   }
                  str << *iter;
                }
             }
            str << std::endl << "These variables are in the global scope: ";
            for (std::vector<std::string>::const_iterator iter = context.globalScope->names.begin();
               context.globalScope->names.end() != iter; ++iter)
             {
               if (context.globalScope->names.begin() != iter)
                {
                  str << ", ";
                }
               str << *iter;
             }
            context.logger->log(str.str());
          }
         else if ("print" == line.substr(0,5))
          {
            Input::StringInput string (line.substr(5, line.npos));
            Input::Lexer lexer (string, "Print Argument");

            Parser::GetterSetter gs;
            Parser::SymbolTable table (gs, *context.globalScope);
            if (nullptr != context.topScope())
             {
               table.pushScope(context.topScope());
             }
            table.injectContext(frame->function);

            std::shared_ptr<Expression> res = Parser::Parser::ParseFullExpression(lexer, table, *context.logger);
            
            if (nullptr != res.get())
             {
               try
                {
                  std::shared_ptr<CallingContext> newContext = context.duplicate();

                  newContext->currentFrame = frame;

                  std::stringstream str;
                  printValue(str, res->evaluate(*newContext));
                  context.logger->log(str.str());
                }
               catch (Types::TypedOperationException& e)
                {
                  context.logger->log(std::string("Error: ") + e.what());
                }
               catch (FatalException& e)
                {
                  context.logger->log(std::string("Error: ") + e.what());
                }
             }
            else
             {
               context.logger->log("Didn't understand that.");
             }
          }
         else
          {
            std::stringstream str;
            str << "Did not understand >" << line << "<." << std::endl
               << "Known commands are: " << std::endl
               << "\tquit - exit the debugger and continue running" << std::endl
               << "\tbt - give a back trace to the current stack frame" << std::endl
               << "\tup - go up one calling stack frame" << std::endl
               << "\tdown - go down one callee stack frame" << std::endl
               << "\tshow - show the variables in this stack frame" << std::endl
               << "\tprint variable_name - show the value in the given variable";
            context.logger->log(str.str());
          }

         prevLine = line;
         line = context.logger->get();
       }
    }

 } // namespace Engine

 } // namespace Backwards
