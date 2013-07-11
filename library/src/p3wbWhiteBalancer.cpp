/*------------------------------------------------------------------------------

   Perceptuum3 rendering components
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifdef _PLATFORM_LINUX
#include <fenv.h>
#endif
#include <float.h>
#include <string.h>
#include <exception>

#include "WhiteBalancer.hpp"

#include "p3wbWhiteBalancer-v12.h"




/// constants ------------------------------------------------------------------

namespace
{

const char LIBRARY_NAME[]      = "P3WhiteBalancer";
const char LIBRARY_COPYRIGHT[] =
   "Copyright (c) 2007, Harrison Ainsworth / HXA7241.";

}




/// version meta-interface =====================================================

int p3wbIsVersionSupported
(
   const int version
)
{
   return (p3wb12_VERSION == version) | (p3wb11_VERSION == version) ?
      p3wb_SUPPORTED_FULLY : p3wb_SUPPORTED_NOT;
}


const char* p3wbGetName()
{
   return LIBRARY_NAME;
}


const char* p3wbGetCopyright()
{
   return LIBRARY_COPYRIGHT;
}


int p3wbGetVersion()
{
   return p3wb12_VERSION;
}




/// functions ==================================================================

int p3wbWhiteBalance1
(
   unsigned int i_width,
   unsigned int i_height,
   unsigned int i_formatFlags,
   unsigned int i_pixelStride,
   const float* i_inPixels,
   float*       o_outPixels
)
{
   // delegate with default options
   return p3wbWhiteBalance2( 0, 0, 0, p3wb11_GW, 0.0f,
      i_width, i_height, i_formatFlags, i_pixelStride,
      i_inPixels, o_outPixels,
      0 );
}


int p3wbWhiteBalance2
(
   const float* i_colorSpace6,
   const float* i_whitePoint2,
   const float* i_inIlluminant3,
   unsigned int i_options,
   float        i_strength,
   unsigned int i_width,
   unsigned int i_height,
   unsigned int i_formatFlags,
   unsigned int i_pixelStride,
   const float* i_pInPixels,
   float*       o_pOutPixels,
   char*        o_pMessage128
)
{
   bool isOk = false;
   if( o_pMessage128 )
   {
      o_pMessage128[ 0 ] = 0;
   }

#if defined(_PLATFORM_WIN) && !defined(__STRICT_ANSI__)
   // set fp control word: rounding mode near, no exceptions
   const unsigned int fpControlWord =
      ::_controlfp( _MCW_EM | _RC_NEAR, _MCW_EM | _MCW_RC );
#endif

#ifdef _PLATFORM_LINUX
#ifdef FE_ALL_EXCEPT
   const int fpExceptions = ::fedisableexcept( FE_ALL_EXCEPT );
#endif
#ifdef FE_TONEAREST
   const int fpRounding = ::fegetround();
   if( fpRounding >= 0 )
   {
      ::fesetround( FE_TONEAREST );
   }
#endif
#endif //_PLATFORM_LINUX

   // handle exceptions
   try
   {
      // delegate the actual activity
      p3whitebalancer::whiteBalance(
         i_colorSpace6,
         i_whitePoint2,
         i_inIlluminant3,
         i_options,
         i_strength,
         i_width,
         i_height,
         i_formatFlags,
         i_pixelStride,
         i_pInPixels,
         o_pOutPixels );

      isOk = true;
   }
   catch( const std::exception& exception )
   {
      if( o_pMessage128 )
      {
         ::strncpy( o_pMessage128, exception.what(), 127 );
         o_pMessage128[ 127 ] = 0;
      }
   }
   catch( const char*const exceptionString )
   {
      if( o_pMessage128 )
      {
         ::strncpy( o_pMessage128, exceptionString, 127 );
         o_pMessage128[ 127 ] = 0;
      }
   }
   catch( ... )
   {
      if( o_pMessage128 )
      {
         ::strncpy( o_pMessage128, "unannotated exception", 127 );
         o_pMessage128[ 127 ] = 0;
      }
   }

#if defined(_PLATFORM_WIN) && !defined(__STRICT_ANSI__)
   // restore fp control word
   ::_controlfp( fpControlWord, 0xFFFFFFFFu );
#endif

#ifdef _PLATFORM_LINUX
   if( fpRounding >= 0 )
   {
      ::fesetround( fpRounding );
   }
   if( -1 != fpExceptions )
   {
      ::feenableexcept( fpExceptions );
   }
#endif //_PLATFORM_LINUX

   return isOk ? 1 : 0;
}








/// test =======================================================================

#ifndef TESTING


int p3wbTestUnits
(
   int       ,
   const int ,
   const int ,
   const int
)
{
   return -1;
}


#else


#include <iostream>


using namespace hxa7241;


/// unit test declarations
namespace hxa7241_general
{
   bool test_LogFast( std::ostream* pOut, bool isVerbose, dword seed );
   bool test_PowFast( std::ostream* pOut, bool isVerbose, dword seed );
}

namespace hxa7241_graphics
{
   bool test_ColorConversion( std::ostream* pOut, bool isVerbose, dword seed );
   bool test_Matrix3f       ( std::ostream* pOut, bool isVerbose, dword seed );
}

namespace p3whitebalancer
{
   bool test_WhiteBalancer( std::ostream* pOut, bool isVerbose, dword seed );
}



/// unit test caller
static bool (*TESTERS[])(std::ostream*, bool, dword) =
{
   &hxa7241_general::test_LogFast            //  1
,  &hxa7241_general::test_PowFast            //  2

,  &hxa7241_graphics::test_ColorConversion   //  3
,  &hxa7241_graphics::test_Matrix3f          //  4

,  &p3whitebalancer::test_WhiteBalancer      //  5
};


int p3wbTestUnits
(
   int       whichTest,
   const int isOutput,
   const int isVerbose,
   const int seed
)
{
   bool isOk = true;

   if( isOutput ) std::cout << "\n\n";

   const dword noOfTests = sizeof(TESTERS)/sizeof(TESTERS[0]);
   if( 0 >= whichTest )
   {
      for( dword i = 0;  i < noOfTests;  ++i )
      {
         isOk &= (TESTERS[i])(
            isOutput ? &std::cout : 0, 0 != isVerbose, seed );
      }
   }
   else
   {
      if( whichTest > noOfTests )
      {
         whichTest = noOfTests;
      }
      isOk &= (TESTERS[whichTest - 1])(
         isOutput ? &std::cout : 0, 0 != isVerbose, seed );
   }

   if( isOutput ) std::cout <<
      (isOk ? "--- successfully" : "*** failurefully") << " completed " <<
      ((0 >= whichTest) ? "all lib unit tests" : "one lib unit test") << "\n\n";

   return isOk ? 1 : 0;
}


#endif//TESTING
