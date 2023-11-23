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

/*
   This has been modified to be a decimal fixed-point number class that follows bc's
   rules for result precision.
*/

#include <cstdlib>
#include <string>
#include "Fixed.hpp"

namespace BigInt
 {


   unsigned long Fixed::defPrec = 0;

   Fixed_Round_Mode Fixed::mode = ROUND_TIES_EVEN;


   Fixed::Fixed (const std::string & from)
    {
      fromString(from);
    }

   Fixed::Fixed (const char * from)
    {
      fromString(from);
    }


   bool operator > (const Fixed & lhs, const Fixed & rhs)
    {
      if ((true == lhs.isNaN()) || (true == rhs.isNaN()) || (true == lhs.isInf()) || (true == rhs.isInf()))
       {
         return false;
       }
      return lhs.compare(rhs) > 0;
    }

   bool operator < (const Fixed & lhs, const Fixed & rhs)
    {
      if ((true == lhs.isNaN()) || (true == rhs.isNaN()) || (true == lhs.isInf()) || (true == rhs.isInf()))
       {
         return false;
       }
      return lhs.compare(rhs) < 0;
    }

   bool operator >= (const Fixed & lhs, const Fixed & rhs)
    {
      if ((true == lhs.isNaN()) || (true == rhs.isNaN()) || (true == lhs.isInf()) || (true == rhs.isInf()))
       {
         return false;
       }
      return lhs.compare(rhs) >= 0;
    }

   bool operator <= (const Fixed & lhs, const Fixed & rhs)
    {
      if ((true == lhs.isNaN()) || (true == rhs.isNaN()) || (true == lhs.isInf()) || (true == rhs.isInf()))
       {
         return false;
       }
      return lhs.compare(rhs) <= 0;
    }

   bool operator == (const Fixed & lhs, const Fixed & rhs)
    {
      if ((true == lhs.isInf()) && (true == rhs.isInf()))
       {
         return true;
       }
      if ((true == lhs.isNaN()) || (true == rhs.isNaN()) || (true == lhs.isInf()) || (true == rhs.isInf()))
       {
         return false;
       }
      return lhs.compare(rhs) == 0;
    }

   bool operator != (const Fixed & lhs, const Fixed & rhs)
    {
      if ((true == lhs.isInf()) && (true == rhs.isInf()))
       {
         return false;
       }
      if ((true == lhs.isNaN()) || (true == rhs.isNaN()) || (true == lhs.isInf()) || (true == rhs.isInf()))
       {
         return true;
       }
      return lhs.compare(rhs) != 0;
    }


   Fixed Fixed::operator - (void) const
    {
      Fixed returnedFixed(*this);
      returnedFixed.Data.negate();
      return returnedFixed;
    }


   int Fixed::compare (const Fixed & to) const
    {
      Fixed temp;

      if (Digits > to.Digits)
       {
         temp = to;
         temp.changePrecision(Digits);
         return Data.compare(temp.Data);
       }
      else if (Digits < to.Digits)
       {
         temp = *this;
         temp.changePrecision(to.Digits);
         return temp.Data.compare(to.Data);
       }

      return Data.compare(to.Data);
    }


   Fixed operator + (const Fixed & lhs, const Fixed & rhs)
    {
      if (true == lhs.nan)
       {
         return lhs;
       }
      if (true == rhs.nan)
       {
         return rhs;
       }
      if (true == lhs.infinity)
       {
         if (true == rhs.infinity)
          {
            return Fixed(false, true);
          }
         return lhs;
       }
      if (true == rhs.infinity)
       {
         return rhs;
       }

      Fixed temp;

      if (lhs.Digits > rhs.Digits)
       {
         temp = rhs;
         temp.changePrecision(lhs.Digits);
         temp.Data = lhs.Data + temp.Data;
       }
      else if (lhs.Digits < rhs.Digits)
       {
         temp = lhs;
         temp.changePrecision(rhs.Digits);
         temp.Data = temp.Data + rhs.Data;
       }
      else
       {
         temp.Data = lhs.Data + rhs.Data;
         temp.Digits = lhs.Digits;
       }

      return temp;
    }

   Fixed operator - (const Fixed & lhs, const Fixed & rhs)
    {
      if (true == lhs.nan)
       {
         return lhs;
       }
      if (true == rhs.nan)
       {
         return rhs;
       }
      if (true == lhs.infinity)
       {
         if (true == rhs.infinity)
          {
            return Fixed(false, true);
          }
         return lhs;
       }
      if (true == rhs.infinity)
       {
         return rhs;
       }

      Fixed temp;

      if (lhs.Digits > rhs.Digits)
       {
         temp = rhs;
         temp.changePrecision(lhs.Digits);
         temp.Data = lhs.Data - temp.Data;
       }
      else if (lhs.Digits < rhs.Digits)
       {
         temp = lhs;
         temp.changePrecision(rhs.Digits);
         temp.Data = temp.Data - rhs.Data;
       }
      else
       {
         temp.Data = lhs.Data - rhs.Data;
         temp.Digits = lhs.Digits;
       }

      return temp;
    }

   Fixed operator * (const Fixed & lhs, const Fixed & rhs)
    {
      if (true == lhs.nan)
       {
         return lhs;
       }
      if (true == rhs.nan)
       {
         return rhs;
       }
      if (true == lhs.infinity)
       {
         if (true == rhs.isZero())
          {
            return Fixed(false, true);
          }
         return lhs;
       }
      if (true == rhs.infinity)
       {
         if (true == lhs.isZero())
          {
            return Fixed(false, true);
          }
         return rhs;
       }

      Fixed temp;

      temp.Data = lhs.Data * rhs.Data;
      temp.Digits = lhs.Digits + rhs.Digits;

      // This strange rule here is how the library gets its name:
      // the bc rules for result scale.
      temp.changePrecision(std::min(temp.Digits, std::max(std::max(lhs.Digits, rhs.Digits), Fixed::getDefaultPrecision())));

      return temp;
    }

   Fixed operator / (const Fixed & lhs, const Fixed & rhs)
    {
      if (true == lhs.nan)
       {
         return lhs;
       }
      if (true == rhs.nan)
       {
         return rhs;
       }
      if (true == lhs.infinity)
       {
         if (true == rhs.infinity)
          {
            return Fixed(false, true);
          }
         return lhs;
       }
      if (true == rhs.infinity)
       {
         return Fixed(0UL);
       }
      if (true == rhs.isZero())
       {
         if (true == lhs.isZero())
          {
            return Fixed(false, true);
          }
         return Fixed(true, false);
       }

      Fixed q = lhs;
      Fixed r = rhs;
      Integer d;
      bool s = lhs.Data.isSigned() ^ rhs.Data.isSigned();

         // Getting result precision from division:
         // result precision = q - r
         // desired scale = q - r + x
         // x = scale + r - q
         // x > 0, scale q by x ; x < 0, scale r by -x
      if (Fixed::getDefaultPrecision() + r.Digits >= q.Digits)
       {
         q.Data = q.Data * pow10(Fixed::getDefaultPrecision() + r.Digits - q.Digits);
       }
      else
       {
         r.Data = r.Data * pow10(q.Digits - Fixed::getDefaultPrecision() - r.Digits);
       }
      q.Digits = Fixed::getDefaultPrecision();

      d = r.Data;
      d.abs();

      quotrem(q.Data, r.Data, q.Data, r.Data);

      r.Data = r.Data * Integer(2U);

      if (Fixed::decideRound(s, q.Data.isEven(), d.compare(r.Data.abs()),
                             r.Data.isZero(), q.Data.is0mod5()))
       {
         if (s) q.Data = q.Data - Integer(1U);
         else q.Data = q.Data + Integer(1U);
       }

      return q;
    }


   void Fixed::changePrecision (unsigned long newPrec)
    {
      Integer scale, rem;
      bool s = Data.isSigned();

      if (newPrec == Digits) return;
      if (newPrec > Digits)
       {
         scale = pow10(newPrec - Digits);
         Data = Data * scale;
       }
      else
       {
         scale = pow10(Digits - newPrec);
         quotrem(Data, scale, Data, rem);

         rem = rem * Integer(2U);

         if (decideRound(s, Data.isEven(), scale.compare(rem.abs()),
                         rem.isZero(), Data.is0mod5()))
          {
            if (s) Data = Data - Integer(1U);
            else Data = Data + Integer(1U);
          }
       }
      Digits = newPrec;
    }


   std::string Fixed::toString (void) const
    {
      std::string result, sign;
      if (true == infinity)
       {
         return "Infinity";
       }
      else if (true == nan)
       {
         return "Not a Result";
       }
      result = Data.toString();
      if (Digits == 0) return result;
      if ('-' == result[0])
       {
         sign = "-";
         result = result.substr(1);
       }
      if (result.length() <= Digits)
       {
         while (result.length() < Digits)
          {
            result = '0' + result;
          }
         result = sign + '0' + '.' + result;
       }
      else
       {
         result = sign + result.substr(0, result.length() - Digits) + '.' +
            result.substr(result.length() - Digits, Digits);
       }
      return result;
    }


   void Fixed::fromString (const char* src)
    {
      const char* iter = src, *base;
      std::string conv;

      conv.reserve(2048);

      infinity = false;
      nan = false;

         // Fast path for an integer.
      if (*iter == '-') ++iter;
      if ((*iter != '.') && (*iter != ','))
       {
         while ((*iter >= '0') && (*iter <= '9')) ++iter;

         if ((*iter != '.') && (*iter != ',') && ('e' != *iter) && ('E' != *iter))
          {
            Data.fromString(src);
            Digits = 0;
            return;
          }
       }

         // Not an integer. Count the digits after the decimal place.
      Digits = 0;
      base = iter;
      iter++;
      if ((*base == '.') || (*base == ','))
       {
         while ((*iter >= '0') && (*iter <= '9')) { ++iter; ++Digits; }
       }

      Integer extraScale (1U);
         // Note: any sane value of exponent here will be well within
         // the capabilities of the underlying representation.
      long exponent = 0;
         // The exponent is either immediately after the number,
         // or after the decimal portion.
      if (('e' == *iter) || ('E' == *iter))
       {
         exponent = std::strtol(iter + 1, nullptr, 10);
       }
      else if (('e' == *base) || ('E' == *base))
       {
         exponent = std::strtol(base + 1, nullptr, 10);
       }
         // Was an exponent given?
      if (exponent < 0)
       {
         Digits -= exponent;
       }
      else if (exponent > 0)
       {
         extraScale = pow10(static_cast<unsigned long>(exponent));
       }

         // Now, recompose just the number without the separator.
      if (base != src)
       {
         conv.append(src, base - src);
       }
      ++base;
      if (base != iter)
       {
         conv.append(base, iter - base);
       }

      Data.fromString(conv);
      Data = Data * extraScale;
    }


   Integer Fixed::roundToInteger (void) const
    {
      Fixed temp (*this);
      temp.changePrecision(0);
      return temp.Data;
    }

   Fixed Fixed::roundToInteger (Fixed_Round_Mode withMode) const
    {
      Fixed temp (*this);
      if (0U != temp.Digits)
       {
         Integer scale, rem;
         bool s = temp.Data.isSigned();

         scale = pow10(temp.Digits); // All the way to zero.
         quotrem(temp.Data, scale, temp.Data, rem);

         rem = rem * Integer(2U);

         if (decideRound(s, temp.Data.isEven(), scale.compare(rem.abs()),
                         rem.isZero(), temp.Data.is0mod5(), withMode))
          {
            if (s) temp.Data = temp.Data - Integer(1U);
            else temp.Data = temp.Data + Integer(1U);
          }

         temp.Data = temp.Data * scale;
       }
      return temp;
    }


   bool Fixed::decideRound (bool sign, bool even, int comp, bool zero, bool zmf)
    {
      return decideRound(sign, even, comp, zero, zmf, mode);
    }

   bool Fixed::decideRound (bool sign, bool even, int comp, bool zero, bool zmf, Fixed_Round_Mode thisMode)
    {
      switch (thisMode)
       {
         case ROUND_TIES_EVEN:
            if ((comp < 0) || ((comp == 0) && !even)) return true;
            break;
         case ROUND_TIES_AWAY:
            if (comp <= 0) return true;
            break;
         case ROUND_POSITIVE_INFINITY:
            if (!sign && !zero) return true;
            break;
         case ROUND_NEGATIVE_INFINITY:
            if (sign && !zero) return true;
            break;
         case ROUND_ZERO:
            break;
         case ROUND_TIES_ODD:
            if ((comp < 0) || ((comp == 0) && even)) return true;
            break;
         case ROUND_TIES_ZERO:
            if (comp < 0) return true;
            break;
         case ROUND_AWAY:
            if (!zero) return true;
            break;
         case ROUND_DOUBLE:
            if (!zero && zmf) return true;
            break;
       }
      return false;
    }


 } /* namespace BigInt */
