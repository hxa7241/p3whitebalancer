/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef LogFast_h
#define LogFast_h




#include "hxa7241_general.hpp"
namespace hxa7241_general
{

/// adjustable -----------------------------------------------------------------
   /**
    * Fast approximation to log, with adjustable precision.<br/><br/>
    *
    * For precision 11: mean error < 0.001%, max error < 0.01% (except between
    * 1 and 2: < 0.1%).<br/><br/>
    *
    * (arguments must be > 0).
    */
   class LogFast
   {
   /// standard object services ------------------------------------------------
   public:
      explicit LogFast( udword precision = 11 );

              ~LogFast();
   private:
               LogFast( const LogFast& );
      LogFast& operator=( const LogFast& );
   public:

   /// queries -----------------------------------------------------------------
      float  two( float )                                                 const;
      float  e  ( float )                                                 const;
      float  ten( float )                                                 const;

      udword precision()                                                  const;

   /// fields ------------------------------------------------------------------
   private:
      udword precision_m;
      float* pTable_m;
   };

}//namespace




/// INLINES ///

/// adjustable -----------------------------------------------------------------
namespace
{
   using namespace hxa7241;

/**
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 23
 */
inline
float log2Lookup
(
   const float  val,
   float* const pTable,
   const udword precision
)
{
   // get access to float bits
   const int* const pVal = reinterpret_cast<const int*>(&val);

   // extract exponent and mantissa (quantized)
   const int exp = ((*pVal >> 23) & 0xFF) - 127;
   const int man = (*pVal & 0x7FFFFF) >> (23 - precision);

   // exponent plus lookup refinement
   return static_cast<float>(exp) + pTable[ man ];
}

}


namespace hxa7241_general
{

inline
float LogFast::two
(
   const float f
) const
{
   return log2Lookup( f, pTable_m, precision_m );
}


inline
float LogFast::e
(
   const float f
) const
{
   return two( f ) * 0.69314718055995f;
}


inline
float LogFast::ten
(
   const float f
) const
{
   return two( f ) * 0.30102999566398f;
}


inline
udword LogFast::precision() const
{
   return precision_m;
}

}//namespace


#endif//LogFast_h
