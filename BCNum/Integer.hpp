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
   An arbitrary precision integer class that is just a wrapper and holder for GMP.
   That the sign isn't the GMP sign is probably a hold-over from this code's pedigree.
*/

#ifndef INTEGER_HPP
#define INTEGER_HPP

#include <string>
#include <memory>

namespace BigInt
 {
   class DataHolder;

   class Integer final
    {

      private:
         std::shared_ptr<DataHolder> Data;
         bool Sign;

      public:
         Integer ();
         explicit Integer (unsigned long);
         Integer (const Integer&) = default;
         ~Integer (); // Not default due to pimpl

         bool isSigned (void) const { return Sign; }
         bool isZero (void) const { return nullptr == Data.get(); }
         bool isEven (void) const;
         bool is0mod5 (void) const;

         std::string toString () const;

         void fromString (const std::string&);
         void fromString (const char *);

         Integer& negate (void);
         Integer& abs (void);

         Integer& operator = (const Integer&) = default;

         Integer operator - (void) const;
         bool operator ! (void) const { return isZero(); }

         long toInt (void) const; //Not perfect, but not terrible.

         int compare (const Integer &) const;

          /*
            Note: Aliased Behavior: DON'T ALIAS quotient and remainder
          */
         friend void quotrem (const Integer& dividend,
                              const Integer& divisor,
                                    Integer& quotient,
                                    Integer& remainder);

         friend Integer pow10 (unsigned long);

         friend Integer operator + (const Integer&, const Integer&);
         friend Integer operator - (const Integer&, const Integer&);
         friend Integer operator * (const Integer&, const Integer&);

    }; /* class Integer */

   Integer operator + (const Integer&, const Integer&);
   Integer operator - (const Integer&, const Integer&);
   Integer operator * (const Integer&, const Integer&);

   Integer pow10 (unsigned long);

   void quotrem (const Integer&, const Integer&, Integer&, Integer&);

 } /* namespace BigInt */

#endif /* INTEGER_HPP */
