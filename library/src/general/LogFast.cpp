/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#include <math.h>

#include "LogFast.hpp"


using namespace hxa7241_general;




/// adjustable -----------------------------------------------------------------

/**
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

/**
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 23
 */
void log2SetTable
(
   float* const pTable,
   const udword precision
)
{
   // step along table elements and x-axis positions
   // [HXA7241: add half increment, so the stepped function intersects the curve
   // at midpoints -- halving the error.]
   float oneToTwo = 1.0f + (1.0f / static_cast<float>( 1 << (precision + 1) ));
   for( int i = 0;  i < (1 << precision);  ++i )
   {
      // make y-axis value for table element
      pTable[i] = ::logf(oneToTwo) / 0.69314718055995f;

      oneToTwo += 1.0f / static_cast<float>( 1 << precision );
   }
}

}


/// wrapper --------------------------------------------------------------------
LogFast::LogFast
(
   const udword precision
)
 : precision_m( precision <= 23 ? precision : 23 )
 , pTable_m   ( new float[ 1 << precision_m ] )
{
   log2SetTable( pTable_m, precision_m );
}


LogFast::~LogFast()
{
   delete[] pTable_m;
}




// original:
//
///* Creates the ICSILog lookup table. Must be called
//   once before any call to icsi_log().
//   n is the number of bits to be taken from the mantissa
//   (0<=n<=23)
//   lookup_table is a pointer to a floating point array
//   (memory has to be allocated by the user) of 2^n positions.
//*/
//void fill_icsi_log_table(const int n, float *lookup_table)
//{
//   float numlog;
//   int *const exp_ptr = ((int*)&numlog);
//   int x = *exp_ptr; //x is the float treated as an integer
//   x = 0x3F800000; //set the exponent to 0 so numlog=1.0
//   *exp_ptr = x;
//   int incr = 1 << (23-n); //amount to increase the mantissa
//   int p=pow(2,n);
//   for(int i=0;i<p;++i)
//   {
//      lookup_table[i] = log2(numlog); //save the log value
//      x += incr;
//      *exp_ptr = x; //update the float value
//   }
//}
//
///* Computes an approximation of log(val) quickly.
//   val is a IEEE 754 float value, must be >0.
//   lookup_table and n must be the same values as
//   provided to fill_icsi_table.
//   returns: log(val). No input checking performed.
//*/
//inline float icsi_log(register float val,
//register const float *lookup_table, register const int n)
//{
//   register int *const exp_ptr = ((int*)&val);
//   register int x = *exp_ptr; //x is treated as integer
//   register const int log_2 = ((x >> 23) & 255) - 127;//exponent
//   x &= 0x7FFFFF; //mantissa
//   x = x >> (23-n); //quantize mantissa
//   val = lookup_table[x]; //lookup precomputed value
//   return ((val + log_2)* 0.69314718); //natural logarithm
//}








/// test -----------------------------------------------------------------------
#ifdef TESTING


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

bool test_LogFast
(
   std::ostream* pOut,
   const bool    isVerbose,
   const dword   seed
)
{
   bool isOk = true;

   if( pOut ) *pOut << "[ test_LogFast ]\n\n";


   RandomMwc2 rand( seed );
   const dword COUNT = 1000000;

   /// adjustable
   {
      const LogFast logFast( 11 );

      float sumDifE = 0.0f;
      float maxDifE = FLOAT_MIN_POS;
      float sumDifT = 0.0f;
      float maxDifT = FLOAT_MIN_POS;

      for( dword i = 0;  i < COUNT;  ++i )
      {
         const float number = static_cast<float>(i + 0.0001f) + rand.getFloat();

         {
            const float le    = ::logf(number);
            const float lef   = logFast.e(number);
            const float leDif = ::fabsf( lef - le ) / le;
            sumDifE += leDif;
            maxDifE = (maxDifE >= leDif) ? maxDifE : leDif;

            if( (i < 10) && pOut && isVerbose ) *pOut << number << "  E " << lef << " " << leDif << "  ";
         }
         {
            const float lt    = ::log10f(number);
            const float ltf   = logFast.ten(number);
            const float ltDif = ::fabsf( ltf - lt ) / lt;
            sumDifT += ltDif;
            maxDifT = (maxDifT >= ltDif) ? maxDifT : ltDif;

            if( (i < 10) && pOut && isVerbose ) *pOut << "T " << ltf << " " << ltDif << "\n";
         }
      }
      if( pOut && isVerbose ) *pOut << "\n";

      const float meanDifE = sumDifE / COUNT;
      const float meanDifT = sumDifT / COUNT;

      if( pOut && isVerbose ) *pOut << "precision: " << logFast.precision() <<  "\n";
      if( pOut && isVerbose ) *pOut << "mean diff,  E: " << meanDifE << "  10: " << meanDifT << "\n";
      if( pOut && isVerbose ) *pOut << "max  diff,  E: " << maxDifE  << "  10: " << maxDifT  << "\n";
      if( pOut && isVerbose ) *pOut << "\n";

      bool isOk_ = (maxDifE < 0.001f) & (maxDifT < 0.001f) &
         (meanDifE < 0.00001f) & (meanDifT < 0.00001f);

      if( pOut ) *pOut << "adjustable : " <<
         (isOk_ ? "--- succeeded" : "*** failed") << "\n\n";

      isOk &= isOk_;
   }


   /*rand = RandomMwc2( seed );

   /// temp
   {
      float maxDifEp =  FLOAT_MIN_POS;
      float maxDifEn = -FLOAT_MIN_POS;

      for( float i = 1.0f;  i < 2.0f; )
      {
         i += (1.0f / 128.0f);

         const float le    = ::logf(i);
         const float lef   = logFast(i);
         const float leDif = (lef - le) / le;

         maxDifEp = (maxDifEp >= leDif) ? maxDifEp : leDif;
         maxDifEn = (maxDifEn <= leDif) ? maxDifEn : leDif;

         if( pOut && isVerbose ) *pOut << i << "  E " << lef << " " << leDif << "\n";
      }
      if( pOut && isVerbose ) *pOut << "\n";

      if( pOut && isVerbose ) *pOut << "max  diffs,  E: " << maxDifEp  << "  " << maxDifEn  << "\n";
      if( pOut && isVerbose ) *pOut << "\n";
   }*/


   if( pOut ) *pOut << (isOk ? "--- successfully" : "*** failurefully") <<
      " completed " << "\n\n\n";

   if( pOut ) pOut->flush();


   return isOk;
}

}//namespace


#endif//TESTING
