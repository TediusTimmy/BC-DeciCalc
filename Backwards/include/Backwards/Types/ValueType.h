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
#ifndef BACKWARDS_TYPES_VALUETYPE_H
#define BACKWARDS_TYPES_VALUETYPE_H

#include <string>
#include <exception>
#include <memory>

namespace Backwards
 {

namespace Types
 {

   class FloatValue;
   class StringValue;
   class ArrayValue;
   class DictionaryValue;
   class FunctionValue;
   class NilValue;
   class CellRefValue;
   class CellRangeValue;

   inline void boost_hash_combine(size_t& seed, size_t value)
    {
      seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

#define DECLARE(x) \
      virtual std::shared_ptr<ValueType> x (const ValueType& rhs) const = 0; \
      virtual std::shared_ptr<ValueType> x (const FloatValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const StringValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const ArrayValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const DictionaryValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const FunctionValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const NilValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const CellRefValue& lhs) const; \
      virtual std::shared_ptr<ValueType> x (const CellRangeValue& lhs) const;

#define DECLAREBOOL(x) \
      virtual bool x (const ValueType& rhs) const = 0; \
      virtual bool x (const FloatValue& lhs) const; \
      virtual bool x (const StringValue& lhs) const; \
      virtual bool x (const ArrayValue& lhs) const; \
      virtual bool x (const DictionaryValue& lhs) const; \
      virtual bool x (const FunctionValue& lhs) const; \
      virtual bool x (const NilValue& lhs) const; \
      virtual bool x (const CellRefValue& lhs) const; \
      virtual bool x (const CellRangeValue& lhs) const;

#define DECLAREVISITOR \
   virtual std::shared_ptr<ValueType> add (const ValueType& rhs) const override; \
   virtual std::shared_ptr<ValueType> sub (const ValueType& rhs) const override; \
   virtual std::shared_ptr<ValueType> mul (const ValueType& rhs) const override; \
   virtual std::shared_ptr<ValueType> div (const ValueType& rhs) const override; \
   virtual bool greater (const ValueType& rhs) const override; \
   virtual bool less (const ValueType& rhs) const override; \
   virtual bool geq (const ValueType& rhs) const override; \
   virtual bool leq (const ValueType& rhs) const override; \
   virtual bool equal (const ValueType& rhs) const override; \
   virtual bool notEqual (const ValueType& rhs) const override; \
   virtual bool sort (const ValueType& rhs) const override; \
   virtual bool sort (const FloatValue& lhs) const override; \
   virtual bool sort (const StringValue& lhs) const override; \
   virtual bool sort (const ArrayValue& lhs) const override; \
   virtual bool sort (const DictionaryValue& lhs) const override; \
   virtual bool sort (const FunctionValue& lhs) const override; \
   virtual bool sort (const NilValue& lhs) const override; \
   virtual bool sort (const CellRefValue& lhs) const override; \
   virtual bool sort (const CellRangeValue& lhs) const override; \
   virtual size_t hash() const;

#define IMPLEMENT(x,y) \
   std::shared_ptr<ValueType> x::y (const ValueType& rhs) const \
      { return rhs.y(*this); }

#define IMPLEMENTBOOL(x,y) \
   bool x::y (const ValueType& rhs) const \
      { return rhs.y(*this); }

#define IMPLEMENTVISITOR(x) \
   IMPLEMENT(x,add) \
   IMPLEMENT(x,sub) \
   IMPLEMENT(x,mul) \
   IMPLEMENT(x,div) \
   IMPLEMENTBOOL(x,greater) \
   IMPLEMENTBOOL(x,less) \
   IMPLEMENTBOOL(x,geq) \
   IMPLEMENTBOOL(x,leq) \
   IMPLEMENTBOOL(x,equal) \
   IMPLEMENTBOOL(x,notEqual) \
   IMPLEMENTBOOL(x,sort)

   class ValueType
    {

   public:
      virtual ~ValueType() = default;

      virtual const std::string& getTypeName() const = 0;

      virtual std::shared_ptr<ValueType> neg() const;
      virtual bool logical() const;

      DECLARE(add)
      DECLARE(sub)
      DECLARE(mul)
      DECLARE(div)

      DECLAREBOOL(greater)
      DECLAREBOOL(less)
      DECLAREBOOL(geq)
      DECLAREBOOL(leq)
      DECLAREBOOL(equal)
      DECLAREBOOL(notEqual)

      bool compare (const ValueType& rhs) const;

      virtual bool sort (const ValueType& rhs) const = 0;
      virtual bool sort (const FloatValue& lhs) const = 0;
      virtual bool sort (const StringValue& lhs) const = 0;
      virtual bool sort (const ArrayValue& lhs) const = 0;
      virtual bool sort (const DictionaryValue& lhs) const = 0;
      virtual bool sort (const FunctionValue& lhs) const = 0;
      virtual bool sort (const NilValue& lhs) const = 0;
      virtual bool sort (const CellRefValue& lhs) const = 0;
      virtual bool sort (const CellRangeValue& lhs) const = 0;

      virtual size_t hash () const = 0;

    };

   class TypedOperationException final : public std::exception
    {

   private:
      std::string message;

   public:
      TypedOperationException(const std::string& message) : message(message) { }

      ~TypedOperationException() throw() { }

      const char * what() const throw() { return message.c_str(); }

    };

 } // namespace Types

 } // namespace Backwards

#endif /* BACKWARDS_TYPES_VALUETYPE_H */
