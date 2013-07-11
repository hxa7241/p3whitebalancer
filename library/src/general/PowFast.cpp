/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#include <math.h>

#include "LogFast.hpp"

#include "PowFast.hpp"


using namespace hxa7241_general;




/// adjustable -----------------------------------------------------------------
/**
 * Following the adjustable-lookup idea in:
 *
 * 'Revisiting a basic function on current CPUs: A fast logarithm implementation
 * with adjustable accuracy'
 * Oriol Vinyals, Gerald Friedland, Nikki Mirghafori;
 * ICSI,
 * 2007-06-21.
 *
 * [Improved (doubled accuracy) and rewritten by HXA7241, 2007.]
 */
namespace
{

const float _2p23 = 8388608.0f;


/**
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 18
 */
void powFastSetTable
(
   udword* const pTable,
   const udword  precision
)
{
   const float size = static_cast<float>( 1 << precision );

   // step along table elements and x-axis positions
   // (add half increment, so the stepped function intersects the curve at
   // midpoints -- halving the error.)
   float zeroToOne = 1.0f / (size * 2.0f);
   for( int i = 0;  i < static_cast<int>(size);  ++i )
   {
      // make y-axis value for table element
      const float f = (::powf( 2.0f, zeroToOne ) - 1.0f) * _2p23;
      pTable[i] = static_cast<udword>( f < _2p23 ? f : (_2p23 - 1.0f) );

      zeroToOne += 1.0f / size;
   }
}

}


/// wrapper --------------------------------------------------------------------
PowFast::PowFast
(
   const udword precision
)
 : precision_m( precision <= 18 ? precision : 18 )
 , pTable_m   ( new udword[ 1 << precision_m ] )
{
   powFastSetTable( pTable_m, precision_m );
}


PowFast::~PowFast()
{
   delete[] pTable_m;
}








/// test -----------------------------------------------------------------------
#ifdef TESTING


#include <math.h>
#include <istream>
#include <ostream>


namespace
{

class RandomMwc2
{
/// standard object services ---------------------------------------------------
public:
   explicit RandomMwc2( udword seed = 0 );

// use defaults
//         ~RandomMwc2();
//          RandomMwc2( const RandomMwc2& );
//   RandomMwc2& operator=( const RandomMwc2& );


/// commands -------------------------------------------------------------------


/// queries --------------------------------------------------------------------
           udword getUdword()                                             const;
           float  getFloat()                                              const;


/// fields ---------------------------------------------------------------------
private:
   mutable udword seeds_m[2];
};


/// standard object services ---------------------------------------------------
RandomMwc2::RandomMwc2
(
   const udword seed
)
{
   seeds_m[0] = seed ? seed : 521288629u;
   seeds_m[1] = seed ? seed : 362436069u;
}


/// queries --------------------------------------------------------------------
udword RandomMwc2::getUdword() const
{
   // Use any pair of non-equal numbers from this list for the two constants:
   // 18000 18030 18273 18513 18879 19074 19098 19164 19215 19584
   // 19599 19950 20088 20508 20544 20664 20814 20970 21153 21243
   // 21423 21723 21954 22125 22188 22293 22860 22938 22965 22974
   // 23109 23124 23163 23208 23508 23520 23553 23658 23865 24114
   // 24219 24660 24699 24864 24948 25023 25308 25443 26004 26088
   // 26154 26550 26679 26838 27183 27258 27753 27795 27810 27834
   // 27960 28320 28380 28689 28710 28794 28854 28959 28980 29013
   // 29379 29889 30135 30345 30459 30714 30903 30963 31059 31083

   seeds_m[0] = 18000u * (seeds_m[0] & 0xFFFFu) + (seeds_m[0] >> 16);
   seeds_m[1] = 30903u * (seeds_m[1] & 0xFFFFu) + (seeds_m[1] >> 16);

   return (seeds_m[0] << 16) + (seeds_m[1] & 0xFFFFu);
}

float RandomMwc2::getFloat() const
{
   return static_cast<float>(getUdword()) / 4294967296.0f;
}

}


namespace hxa7241_general
{

bool test_PowFast
(
   std::ostream* pOut,
   const bool    isVerbose,
   const dword   seed
)
{
   bool isOk = true;

   if( pOut ) *pOut << "[ test_PowFast ]\n\n";


   RandomMwc2 rand( seed );

   /// adjustable
   {
      const PowFast powFastAdj( 11 );

      float sumDifE = 0.0f;
      float maxDifE = FLOAT_MIN_POS;

      const dword E_COUNT = 88;
      for( dword i = 1 - E_COUNT;  i < E_COUNT;  ++i )
      {
         const float number = static_cast<float>(i + 0.0001f) + rand.getFloat();

         {
            const float pe    = ::powf(2.71828182845905f, number);
            const float pef   = powFastAdj.e(number);
            const float peDif = ::fabsf( pef - pe ) / pe;
            sumDifE += peDif;
            maxDifE = (maxDifE >= peDif) ? maxDifE : peDif;

            if( pOut && isVerbose ) *pOut << number << "  E " << pef << " " << peDif << "\n";
         }
      }
      if( pOut && isVerbose ) *pOut << "\n";

      float sumDifT = 0.0f;
      float maxDifT = FLOAT_MIN_POS;

      const dword T_COUNT = 38;
      for( dword i = 1 - T_COUNT;  i < T_COUNT;  ++i )
      {
         const float number = static_cast<float>(i + 0.0001f) + rand.getFloat();

         {
            const float pt    = ::powf(10.0f, number);
            const float ptf   = powFastAdj.ten(number);
            const float ptDif = ::fabsf( ptf - pt ) / pt;
            sumDifT += ptDif;
            maxDifT = (maxDifT >= ptDif) ? maxDifT : ptDif;

            if( pOut && isVerbose ) *pOut << number << "  T " << ptf << " " << ptDif << "\n";
         }
      }
      if( pOut && isVerbose ) *pOut << "\n";

      const float meanDifE = sumDifE / (E_COUNT - (1 - E_COUNT));
      const float meanDifT = sumDifT / (T_COUNT - (1 - T_COUNT));

      if( pOut && isVerbose ) *pOut << "precision: " << powFastAdj.precision() <<  "\n";
      if( pOut && isVerbose ) *pOut << "mean diff,  E: " << meanDifE << "  10: " << meanDifT << "\n";
      if( pOut && isVerbose ) *pOut << "max  diff,  E: " << maxDifE  << "  10: " << maxDifT  << "\n";
      if( pOut && isVerbose ) *pOut << "\n";

      bool isOk_ = (maxDifE < 0.0002f) & (maxDifT < 0.0002f) &
         (meanDifE < 0.0001f) & (meanDifT < 0.0001f);

      if( pOut ) *pOut << "adjustable : " <<
         (isOk_ ? "--- succeeded" : "*** failed") << "\n\n";

      isOk &= isOk_;
   }


   if( pOut ) *pOut << (isOk ? "--- successfully" : "*** failurefully") <<
      " completed " << "\n\n\n";

   if( pOut ) pOut->flush();


   return isOk;
}

}//namespace


#endif//TESTING
