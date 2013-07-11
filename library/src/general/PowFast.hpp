/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef PowFast_h
#define PowFast_h




#include "hxa7241_general.hpp"
namespace hxa7241_general
{

/// adjustable -----------------------------------------------------------------
   /**
    * Fast approximation to pow, with adjustable precision.<br/><br/>
    *
    * For precision 11: mean error < 0.01%, max error < 0.02%<br/><br/>
    *
    * (f arguments must be > -87.3ish and < 88.7ish for e, and > -37.9ish and
    * < 38.5ish for ten)
    * (r argument must be > 0)
    */
   class PowFast
   {
   /// standard object services ------------------------------------------------
   public:
      explicit PowFast( udword precision = 11 );

              ~PowFast();
   private:
               PowFast( const PowFast& );
      PowFast& operator=( const PowFast& );
   public:

   /// queries -----------------------------------------------------------------
      float  two( float )                                                 const;
      float  e  ( float )                                                 const;
      float  ten( float )                                                 const;

      udword precision()                                                  const;

   /// fields ------------------------------------------------------------------
   private:
      udword  precision_m;
      udword* pTable_m;
   };

}//namespace




/// INLINES ///

/// adjustable -----------------------------------------------------------------
namespace
{
   using namespace hxa7241;

/**
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 18
 */
inline
float powFastLookup
(
   const float   val,
   const float   ilog2,
   udword* const pTable,
   const udword  precision
)
{
   const float _2p23 = 8388608.0f;

   // build float bits
   const int i = static_cast<int>( (val * (_2p23 * ilog2)) + (127.0f * _2p23) );

   // replace mantissa with lookup
   const int it = (i & 0xFF800000) | pTable[(i & 0x7FFFFF) >> (23 - precision)];

   // convert bits to float
   return *reinterpret_cast<const float*>( &it );
}

}


namespace hxa7241_general
{

inline
float PowFast::two
(
   const float f
) const
{
   return powFastLookup( f, 1.0f, pTable_m, precision_m );
}


inline
float PowFast::e
(
   const float f
) const
{
   return powFastLookup( f, 1.44269504088896f, pTable_m, precision_m );
}


inline
float PowFast::ten
(
   const float f
) const
{
   return powFastLookup( f, 3.32192809488736f, pTable_m, precision_m );
}


inline
udword PowFast::precision() const
{
   return precision_m;
}

}//namespace


#endif//PowFast_h
