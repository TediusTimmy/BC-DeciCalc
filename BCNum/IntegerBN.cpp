/*
Copyright (c) 2023 Thomas DiModica.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Thomas DiModica nor the names of other contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THOMAS DIMODICA AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THOMAS DIMODICA OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include "Integer.hpp"
#include <openssl/bn.h>
#include <cstring>
#include <vector>

namespace BigInt
 {

      // I'm going to handle this the same way OpenBSD does.
   static void bn_check (int x)
    {
      if (0 == x)
       {
         throw std::bad_alloc();
       }
    }

   static void bn_check (const void* p)
   { 
      if (nullptr == p)
       {
         throw std::bad_alloc();
       }
    }

   static size_t FREE_LIST_SIZE = 128; // FREE_LIST_SIZE - 1 MUST BE a power of 2.
   class StaticHolder
    {
   private:
      std::vector<BIGNUM*> freeList;
      size_t front, back;
      BN_CTX* tctx;

      StaticHolder() : freeList(FREE_LIST_SIZE), front(0U), back(0U), tctx(nullptr) { }

   public:
      static StaticHolder& getInstance()
       {
         static StaticHolder instance;
         return instance;
       }

      BIGNUM* getNew()
       {
         BIGNUM* result;
         if (front == back)
          {
            bn_check(result = BN_new());
          }
         else
          {
            result = freeList[front];
            front = (front + 1U) & (FREE_LIST_SIZE - 1U);
          }
         return result;
       }

      void dispose(BIGNUM* ptr)
       {
         freeList[back] = ptr;
         back = (back + 1U) & (FREE_LIST_SIZE - 1U);
         if (back == front)
          {
            BN_free(freeList[front]);
            front = (front + 1U) & (FREE_LIST_SIZE - 1U);
          }
       }

      BN_CTX* getCTX()
       {
         if (nullptr == tctx)
          {
            bn_check(tctx = BN_CTX_new());
          }
         return tctx;
       }

      ~StaticHolder()
       {
         while (front != back)
          {
            BN_free(freeList[front]);
            front = (front + 1U) & (FREE_LIST_SIZE - 1U);
          }
         BN_CTX_free(tctx);
       }
    };

   class DataHolder
    {
   public:
      BIGNUM* Data;

      DataHolder ()
       {
         Data = StaticHolder::getInstance().getNew();
       }

      ~DataHolder ()
       {
         StaticHolder::getInstance().dispose(Data);
       }

      explicit DataHolder (unsigned long src)
       {
         Data = StaticHolder::getInstance().getNew();
         bn_check(BN_set_word(Data, src));
       }

      explicit DataHolder (const char* src)
       {
         Data = StaticHolder::getInstance().getNew();
         int len = BN_dec2bn(&Data, src);
         if (static_cast<size_t>(len) != std::strlen(src))
          {
            BN_clear(Data);
          }
       }
    };

   Integer::Integer () : Sign (false) { }

   Integer::Integer (unsigned long input) : Sign (false)
    {
      if (input != 0)
       {
         Data = std::make_shared<DataHolder>((unsigned long)input);
       }
    }

   Integer::~Integer ()
    {
      Sign = false;
    }



   bool Integer::isEven (void) const
    {
      return isZero() ? true : !BN_is_odd(Data->Data);
    }

   bool Integer::is0mod5 (void) const
    {
      return isZero() ? true : (0 == BN_mod_word(Data->Data, 5U));
    }

   Integer& Integer::negate (void)
    {
      if (isZero()) Sign = false;
      else Sign = !Sign;
      return *this;
    }

   Integer& Integer::abs (void)
    {
      Sign = false;
      return *this;
    }

   long Integer::toInt (void) const //It works for its purpose.
    {
      if (isZero()) return 0;
      if (~0UL == BN_get_word(Data->Data)) return 0;
      return Sign ? -BN_get_word(Data->Data) : BN_get_word(Data->Data);
    }



   Integer Integer::operator - (void) const
    {
      Integer returnedInteger(*this);
      return returnedInteger.negate();
    }



   int Integer::compare (const Integer& to) const
    {
       /*
         Positive or zero is greater than negative.
         After this test, we know that both numbers have the same sign.
       */
      if (Sign != to.Sign)
       {
         if (Sign) return -1;
         return 1;
       }

       /*
         Test for zeros, which are always positive.
       */
      if (!Sign)
       {
         if (isZero())
          {
             // Both are Zero.
            if (to.isZero()) return 0;
             // I'm Zero, he isn't.
            return -1;
         }
        if (to.isZero())
         {
            // I'm not zero.
            return 1;
         }
       }

       /*
         Return the unsigned comparison, remembering that if we are
         negative, then the result is negated.
       */
      if (Sign) return -BN_cmp(Data->Data, to.Data->Data);
      return BN_cmp(Data->Data, to.Data->Data);
    }



   Integer operator + (const Integer& lhs, const Integer& rhs)
    {
      Integer result;

      if (rhs.isZero())
       {
         return lhs;
       }
      if (lhs.isZero())
       {
         return rhs;
       }

      result.Sign = lhs.Sign;
      result.Data = std::make_shared<DataHolder>();

      if (lhs.Sign == rhs.Sign) bn_check(BN_add(result.Data->Data, lhs.Data->Data, rhs.Data->Data));
      else bn_check(BN_sub(result.Data->Data, lhs.Data->Data, rhs.Data->Data));

      int sign = -BN_is_negative(result.Data->Data);
      sign |= BN_is_zero(result.Data->Data);
      switch (sign)
       {
         case -1:
            result.Sign = !result.Sign;
            BN_set_negative(result.Data->Data, 0);
            break;

         case 1:
            result.Sign = false;
            result.Data.reset();
            break;

         default:
            break;
       }

      return result;
    }

   Integer operator - (const Integer& lhs, const Integer& rhs)
    {
      Integer result;

      if (rhs.isZero())
       {
         return lhs;
       }
      if (lhs.isZero())
       {
         return -rhs;
       }

      result.Sign = lhs.Sign;
      result.Data = std::make_shared<DataHolder>();

      if (lhs.Sign == rhs.Sign) bn_check(BN_sub(result.Data->Data, lhs.Data->Data, rhs.Data->Data));
      else bn_check(BN_add(result.Data->Data, lhs.Data->Data, rhs.Data->Data));

      int sign = -BN_is_negative(result.Data->Data);
      sign |= BN_is_zero(result.Data->Data);
      switch (sign)
       {
         case -1:
            result.Sign = !result.Sign;
            BN_set_negative(result.Data->Data, 0);
            break;

         case 1:
            result.Sign = false;
            result.Data.reset();
            break;

         default:
            break;
       }

      return result;
    }

   Integer operator * (const Integer& lhs, const Integer& rhs)
    {
      Integer result;

      if (lhs.isZero() || rhs.isZero())
       {
         return result;
       }

      result.Data = std::make_shared<DataHolder>();
      bn_check(BN_mul(result.Data->Data, lhs.Data->Data, rhs.Data->Data, StaticHolder::getInstance().getCTX()));
      result.Sign = lhs.Sign ^ rhs.Sign;

      return result;
    }



   void Integer::fromString (const std::string& src)
    {
      fromString(src.c_str());
    }

   void Integer::fromString (const char* src)
    {
      Sign = false;
      Data = std::make_shared<DataHolder>(src);

      int sign = -BN_is_negative(Data->Data);
      sign |= BN_is_zero(Data->Data);
      switch (sign)
       {
         case -1:
            Sign = true;
            BN_set_negative(Data->Data, 0);
            break;

         case 1:
            Data.reset();
            break;

         default:
            break;
       }
    }



   std::string Integer::toString () const
    {
      std::string result;

      if (isZero()) return std::string("0");
      if (isSigned()) result = "-";

      char * rstring = BN_bn2dec(Data->Data);
      bn_check(rstring);
      result += rstring;
      OPENSSL_free(rstring);

      return result;
    }



   void quotrem (const Integer& lhs, const Integer& rhs,
                       Integer& q, Integer& r)
    {
      if (lhs.isZero())
       {
         q = Integer();
         r = Integer();
         return;
       }
      if (rhs.isZero())
       {
         r = lhs;
         q = Integer();
         return;
       }

      bool
         qSign = lhs.isSigned() ^ rhs.isSigned(),
         rSign = lhs.isSigned();
      std::shared_ptr<DataHolder> quot, rem;

      quot = std::make_shared<DataHolder>();
      rem = std::make_shared<DataHolder>();

      bn_check(BN_div(quot->Data, rem->Data, lhs.Data->Data, rhs.Data->Data, StaticHolder::getInstance().getCTX()));

      q = Integer();
      r = Integer();

      int sign = BN_is_zero(quot->Data);
      if (1 != sign)
       {
         q.Data = quot;
         q.Sign = qSign;
       }

      sign = BN_is_zero(rem->Data);
      if (1 != sign)
       {
         r.Data = rem;
         r.Sign = rSign;
       }
    }


   static BIGNUM *s_ten = nullptr, *s_power = nullptr;
   Integer pow10 (unsigned long power)
    {
      Integer result;

      if (0U == power) return Integer(1U);

      if (nullptr == s_ten)
       {
         bn_check(s_ten = BN_new());
         bn_check(BN_set_word(s_ten, 10U));
         bn_check(s_power = BN_new());
       }

      bn_check(BN_set_word(s_power, power));
      result.Data = std::make_shared<DataHolder>();

      bn_check(BN_exp(result.Data->Data, s_ten, s_power, StaticHolder::getInstance().getCTX()));

      return result;
    }

 } /* namespace BigInt */
