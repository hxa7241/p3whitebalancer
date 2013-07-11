/*------------------------------------------------------------------------------

   Perceptuum3 rendering components
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#include <math.h>

#include "LogFast.hpp"
#include "PowFast.hpp"
#include "Vector3f.hpp"
#include "Matrix3f.hpp"
#include "ColorConstants.hpp"
#include "ColorConversion.hpp"
#include "ImageWrapperConst.hpp"
#include "ImageWrapper.hpp"

#include "p3wbWhiteBalancer-v12.h"

#include "WhiteBalancer.hpp"


using namespace p3whitebalancer;




// implementation --------------------------------------------------------------
namespace
{

using namespace hxa7241_graphics;
using namespace hxa7241_image;


// constants -------------------------------------------------------------------
const char EXCEPTION_MESSAGE[]           = "numerical failure";
const char NAN_INPUT_EXCEPTION_MESSAGE[] = "NaN in input parameter";

const float FLAT_WHITE[] = { (1.0f / 3.0f), (1.0f / 3.0f) };

const Matrix3f CONE_TO_RUDERMAN(
   // 'Statistics of Cone Responses to Natural Images'
   // Ruderman, Cronin, Chiao;
   // J. Optical Soc. of America, vol. 15, no. 8.
   // 1998
   1.0f/::sqrtf(3.0f),  1.0f/::sqrtf(3.0f),  1.0f/::sqrtf(3.0f),
   1.0f/::sqrtf(6.0f),  1.0f/::sqrtf(6.0f), -2.0f/::sqrtf(6.0f),
   1.0f/::sqrtf(2.0f), -1.0f/::sqrtf(2.0f),  0.0f
   //0.57735026918963f,  0.57735026918963f,  0.57735026918963f,
   //0.40824829046386f,  0.40824829046386f, -0.81649658092773f,
   //0.70710678118655f, -0.70710678118655f,  0.00000000000000f
);
const Matrix3f RUDERMAN_TO_CONE( CONE_TO_RUDERMAN.inverted(
   EXCEPTION_MESSAGE ) );

const Matrix3f XYZ_TO_CONE( color::getXyzToCone() );
const Matrix3f CONE_TO_XYZ( XYZ_TO_CONE.inverted( EXCEPTION_MESSAGE ) );

const hxa7241_general::LogFast LOGFAST( 12 );
const hxa7241_general::PowFast POWFAST( 12 );




// functions -------------------------------------------------------------------
/*Vector3f xyzToRuderman
(
   const Vector3f& xyz
)
{
   const Vector3f lms( (XYZ_TO_CONE * xyz).clampedMin( Vector3f::SMALL() ) );
   const Vector3f lmsLog( LOGFAST.ten(lms[0]), LOGFAST.ten(lms[1]),
      LOGFAST.ten(lms[2]) );

   return Vector3f( CONE_TO_RUDERMAN * lmsLog );
}


Vector3f rudermanToXyz
(
   const Vector3f& lab
)
{
   const Vector3f lmsLog( RUDERMAN_TO_CONE * lab );
   const Vector3f lms( POWFAST.ten(lmsLog[0]), POWFAST.ten(lmsLog[1]),
      POWFAST.ten(lmsLog[2]) );

   return Vector3f( CONE_TO_XYZ * lms );
}*/


// classes ---------------------------------------------------------------------
class Ruderman
{
/// standard object services ---------------------------------------------------
public:
            Ruderman( const Matrix3f& rgbToXyz,
                      const Matrix3f& xyzToRgb );
// use defaults
//           ~Ruderman();
//            Ruderman( const Ruderman& );
//   Ruderman& operator=( const Ruderman& );

/// queries --------------------------------------------------------------------
           Vector3f fromRgb( const Vector3f& )                            const;
           Vector3f toRgb  ( const Vector3f& )                            const;

/// fields ---------------------------------------------------------------------
private:
   Matrix3f rgbToCone_m;
   Matrix3f coneToRgb_m;
};


Ruderman::Ruderman
(
   const Matrix3f& rgbToXyz,
   const Matrix3f& xyzToRgb
)
 : rgbToCone_m( XYZ_TO_CONE * rgbToXyz )
 , coneToRgb_m( xyzToRgb * CONE_TO_XYZ )
{
}


Vector3f Ruderman::fromRgb
(
   const Vector3f& rgb
) const
{
   // manually inlined implementation

   // (retro-style inlining)
   #define DOT(a,b) ((a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]))
   #define MUL(m,v) { DOT(m.getRow0(), v), DOT(m.getRow1(), v),\
      DOT(m.getRow2(), v) }

   // convert to cone-log space
   float lmsIn[] = MUL(rgbToCone_m, rgb);
   lmsIn[0] = lmsIn[0] >= FLOAT_SMALL_48 ? lmsIn[0] : FLOAT_SMALL_48;
   lmsIn[1] = lmsIn[1] >= FLOAT_SMALL_48 ? lmsIn[1] : FLOAT_SMALL_48;
   lmsIn[2] = lmsIn[2] >= FLOAT_SMALL_48 ? lmsIn[2] : FLOAT_SMALL_48;
   const float lmsLogIn[] = { LOGFAST.ten(lmsIn[0]), LOGFAST.ten(lmsIn[1]),
      LOGFAST.ten(lmsIn[2]) };

   // convert to ruderman space
   float rud[] = MUL(CONE_TO_RUDERMAN, lmsLogIn);

   return Vector3f( rud );

   #undef MUL
   #undef DOT
}
/*{
   // simple implementation

   const Vector3f lms( (rgbToCone_m ^ rgb).clampedMin( Vector3f::SMALL() ) );
   const Vector3f lmsLog( LOGFAST.ten(lms[0]), LOGFAST.ten(lms[1]),
      LOGFAST.ten(lms[2]) );

   return Vector3f( CONE_TO_RUDERMAN ^ lmsLog );
}*/


Vector3f Ruderman::toRgb
(
   const Vector3f& rud
) const
{
   const Vector3f lmsLog( RUDERMAN_TO_CONE ^ rud );
   const Vector3f lms( POWFAST.ten(lmsLog[0]), POWFAST.ten(lmsLog[1]),
      POWFAST.ten(lmsLog[2]) );

   return Vector3f( coneToRgb_m ^ lms );
}


class PixelMap
{
/// standard object services ---------------------------------------------------
public:
            PixelMap( const Matrix3f& rgbToXyz,
                      const Matrix3f& xyzToRgb,
                      //float           maxMagnitude,
                      const Vector3f& inIlluminantLab,
                      float           strength01 );
// use defaults
//           ~PixelMap();
//            PixelMap( const PixelMap& );
//   PixelMap& operator=( const PixelMap& );

/// queries --------------------------------------------------------------------
           Vector3f operator()( const Vector3f& rgb )                     const;

/// fields ---------------------------------------------------------------------
private:
   //Ruderman ruderman_m;
   Vector3f toY_m;
   //Vector3f translation_m;
   //float    maxMagnitude_m;
   Matrix3f rgbToCone_m;
   Matrix3f coneToRgb_m;
   Matrix3f rudermanTranslation_m;
};


PixelMap::PixelMap
(
   const Matrix3f& rgbToXyz,
   const Matrix3f& xyzToRgb,
   //const float     maxMagnitude,
   const Vector3f& inIlluminantLab,
   const float     strength01
)
// : ruderman_m    ( rgbToXyz, xyzToRgb )
 : toY_m         ( rgbToXyz.getRow1() )
// , translation_m ( (inIlluminantLab * Vector3f( 0.0f, 1.0f, 1.0f )) *
//      (strength01 >= 0.0f ? (strength01 <= 1.0f ? strength01 : 1.0f) : 0.0f) )
// , maxMagnitude_m    ( maxMagnitude )
 , rgbToCone_m( XYZ_TO_CONE * rgbToXyz )
 , coneToRgb_m( xyzToRgb * CONE_TO_XYZ )
 , rudermanTranslation_m( RUDERMAN_TO_CONE *
      Matrix3f( Matrix3f::TRANSLATE, -((inIlluminantLab *
         Vector3f( 0.0f, 1.0f, 1.0f )) * (strength01 >= 0.0f ?
         (strength01 <= 1.0f ? strength01 : 1.0f) : 0.0f)) ) *
         CONE_TO_RUDERMAN )
{
}


Vector3f PixelMap::operator()
(
   const Vector3f& i_inPixelRgb
) const
{
   // manually inlined implementation

   // (retro-style inlining)
   #define DOT(a,b) ((a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]))
   #define MUL(m,v) { DOT(m.getRow0(), v), DOT(m.getRow1(), v),\
      DOT(m.getRow2(), v) }

   // convert to cone-log space
   float lmsIn[] = MUL(rgbToCone_m, i_inPixelRgb);
   lmsIn[0] = lmsIn[0] >= FLOAT_SMALL_48 ? lmsIn[0] : FLOAT_SMALL_48;
   lmsIn[1] = lmsIn[1] >= FLOAT_SMALL_48 ? lmsIn[1] : FLOAT_SMALL_48;
   lmsIn[2] = lmsIn[2] >= FLOAT_SMALL_48 ? lmsIn[2] : FLOAT_SMALL_48;
   const float lmsLogIn[] = { LOGFAST.ten(lmsIn[0]), LOGFAST.ten(lmsIn[1]),
      LOGFAST.ten(lmsIn[2]) };

   // do translation, in Ruderman chromatic 2D sub-space
   float lmsLogOut[] = MUL(rudermanTranslation_m, lmsLogIn);
   lmsLogOut[0] += rudermanTranslation_m.getCol3()[0];
   lmsLogOut[1] += rudermanTranslation_m.getCol3()[1];
   lmsLogOut[2] += rudermanTranslation_m.getCol3()[2];

   // convert back from cone-log space
   const float lmsOut[] = {POWFAST.ten(lmsLogOut[0]), POWFAST.ten(lmsLogOut[1]),
      POWFAST.ten(lmsLogOut[2]) };
   float outRgb[] = MUL(coneToRgb_m, lmsOut);

   // restore original luminance
   const float outLuminance = DOT(outRgb, toY_m);
   const float inLuminance  = DOT(i_inPixelRgb, toY_m);
   const float scaling      = (0.0f != outLuminance) ?
      (inLuminance / outLuminance) : 0.0f;
   const Vector3f outPixelRgb( (outRgb[0] * scaling), (outRgb[1] * scaling),
      (outRgb[2] * scaling) );

   return outPixelRgb;

   #undef MUL
   #undef DOT
}
/*{
   // matrix concatenated implementation

   // convert to cone-log space
   const Vector3f lmsIn( (rgbToCone_m ^ i_inPixelRgb).clampedMin(
      Vector3f::SMALL() ) );
   const Vector3f lmsLogIn( LOGFAST.ten(lmsIn[0]), LOGFAST.ten(lmsIn[1]),
      LOGFAST.ten(lmsIn[2]));

   // do translation, in Ruderman chromatic 2D sub-space
   const Vector3f lmsLogOut( rudermanTranslation_m * lmsLogIn );

   // convert back from cone-log space
   const Vector3f lmsOut( POWFAST.ten(lmsLogOut[0]), POWFAST.ten(lmsLogOut[1]),
      POWFAST.ten(lmsLogOut[2]) );
   const Vector3f outRgb( coneToRgb_m ^ lmsOut );

   // restore original luminance
   const float outY         = outRgb.dot( toY_m );
   const float outLuminance = (0.0f != outY) ? outY : 1.0f;
   const float inLuminance  = i_inPixelRgb.dot( toY_m );
   const Vector3f outPixelRgb( outRgb * (inLuminance / outLuminance) );

   return outPixelRgb;
}*/
/*{
   // optimized ruderman implementation

   // convert to ruderman space
   const Vector3f inLab( ruderman_m.fromRgb( i_inPixelRgb ) );

   // do translation, in chromatic 2D sub-space
   const Vector3f outLab( inLab - translation_m );

   // convert back from ruderman space
   const Vector3f outRgb( ruderman_m.toRgb( outLab ) );

   // restore original luminance
   const float outY         = outRgb.dot( toY_m );
   const float outLuminance = (0.0f != outY) ? outY : 1.0f;
   const float inLuminance  = i_inPixelRgb.dot( toY_m );
   const Vector3f outPixelRgb( outRgb * (inLuminance / outLuminance) );

   return outPixelRgb;
}*/
/*{
   // simple implementation

   // convert to ruderman space
   const Vector3f inLab( xyzToRuderman( rgbToXyz_m * i_inPixelRgb ) );

   // do translation, in chromatic 2D sub-space
   static const Vector3f ZERO_ONE_ONE( 0.0f, 1.0f, 1.0f );
   const Vector3f outLab( inLab - ((inIlluminantLab_m * strength01_m) *
      ZERO_ONE_ONE) );

   // convert back to rgb space
   return Vector3f( xyzToRgb_m * rudermanToXyz( outLab ) );
}*/
/*{
   // simple implementation, with white conservation

   // note original luminance
   const Vector3f inXyz( rgbToXyz_m * i_inPixelRgb );
   const float    inLuminance = inXyz.getY();

   // convert to ruderman space
   const Vector3f inLab( xyzToRuderman( inXyz ) );

   // do translation, in chromatic 2D sub-space
   static const Vector3f ZERO_ONE_ONE( 0.0f, 1.0f, 1.0f );
   const Vector3f outLab( inLab - ((inIlluminantLab_m * strength01_m) *
      ZERO_ONE_ONE) );

   // convert back from ruderman space
   const Vector3f outXyz( rudermanToXyz( outLab ) );

   // restore original luminance
   const float    outLuminance = (0.0f != outXyz.getY()) ? outXyz.getY() : 1.0f;
   const Vector3f outPixelRgb( xyzToRgb_m * (outXyz * (inLuminance /
      outLuminance)) );

   // maybe pull input white back to white
   Vector3f outAdjusted( outPixelRgb );
   {
      // calc interpolation factor
      float i = 0.0f;
      {
         static const float THRESHOLD = 0.8f;
         const float inMagnitude = i_inPixelRgb.average();

         // calc input brightness, then margin factor
         const float b   = inMagnitude / maxMagnitude_m;
         float       bif = (b - THRESHOLD) / (1.0f - THRESHOLD);
         bif = (bif >= 0.0f) ? bif : 0.0f;

         // calc input whiteness (desaturation), then margin factor
         const float w   = 1.0f - (((i_inPixelRgb / inMagnitude) -
            Vector3f::ONE()).abs().sum() / 4.0f);
         float       wif = (w - THRESHOLD) / (1.0f - THRESHOLD);
         wif = (wif >= 0.0f) ? wif : 0.0f;

         i = bif * wif;
      }

      // interpolate mapped back to input
      if( i > 0.0f )
      {
         outAdjusted = outPixelRgb + ((i_inPixelRgb - outPixelRgb) * i);
      }
   }

   return outAdjusted;
}*/


inline
bool isNan
(
   const float f
)
{
   // is NaN if (IEEE-754): exponent is all ones and mantissa is not all zeros
   return (*reinterpret_cast<const udword*>(&f) & 0x7FFFFFFF) > 0x7F800000;
}


inline
bool isNan
(
   const Vector3f& v
)
{
   return isNan(v[0]) | isNan(v[1]) | isNan(v[2]);
}


inline
Vector3f preconditionPixel
(
   const Vector3f& i_pixel
)
{
   // clamp between zero and FLOAT_LARGE_48
   return Vector3f(
      (i_pixel[0] > 0.0f) ? ((i_pixel[0] < FLOAT_LARGE_48) ?
         i_pixel[0] : FLOAT_LARGE_48) : 0.0f,
      (i_pixel[1] > 0.0f) ? ((i_pixel[1] < FLOAT_LARGE_48) ?
         i_pixel[1] : FLOAT_LARGE_48) : 0.0f,
      (i_pixel[2] > 0.0f) ? ((i_pixel[2] < FLOAT_LARGE_48) ?
         i_pixel[2] : FLOAT_LARGE_48) : 0.0f );
}


inline
Vector3f postconditionPixel
(
   const Vector3f& i_pixel
)
{
   // clamp min to zero
   return Vector3f(
      (i_pixel[0] > 0.0f) ? i_pixel[0] : 0.0f,
      (i_pixel[1] > 0.0f) ? i_pixel[1] : 0.0f,
      (i_pixel[2] > 0.0f) ? i_pixel[2] : 0.0f );
}


const float* checkForNans
(
   const float* pFps,
   const udword length
)
{
   for( udword i = length;  i-- > 0; )
   {
      if( isNan( pFps[i] ) )
      {
         throw NAN_INPUT_EXCEPTION_MESSAGE;
      }
   }

   return pFps;
}


void preconditionInputs
(
   const float*& i_pColorSpace6,
   const float*& i_pWhitePoint2,
   const float*  i_pInIlluminant3,
         float&  i_strength01
)
{
   // check for NaNs, or default to sRGB
   i_pColorSpace6 = i_pColorSpace6 ?
      checkForNans( i_pColorSpace6, 6 ) : color::getSrgbChromaticities();

   // check for NaNs, or default to flat white
   i_pWhitePoint2 = i_pWhitePoint2 ?
      checkForNans( i_pWhitePoint2, 2 ) : FLAT_WHITE;

   // check for NaNs
   if( i_pInIlluminant3 )
   {
      checkForNans( i_pInIlluminant3, 3 );
   }

   // check for NaNs, or default to quite-strong
   i_strength01 = (-1.0f != i_strength01) ?
      *checkForNans( &i_strength01, 1 ) : 0.8f;
}


Vector3f makeIlluminant
(
   const float*             i_pInIlluminant3,
   const ImageWrapperConst& i_image,
   const Matrix3f&          i_rgbToXyz,
   const Matrix3f&          i_xyzToRgb
)
{
   Vector3f inIlluminant;

   const Ruderman ruderman( i_rgbToXyz, i_xyzToRgb );

   // use supplied
   if( i_pInIlluminant3 )
   {
      // get image mean energy
      float mean = 0.0f;
      {
         // sum energy
         float  sum   = 0.0f;
         udword count = 0;
         for( dword i = 0, end = i_image.getLength();  i < end;  ++i )
         {
            const Vector3f p( i_image.get( i ) );

            // disclude NaNs
            if( !isNan( p ) )
            {
               sum += preconditionPixel( p ).average();
               ++count;
            }
         }

         // mean energy
         mean = sum / (count > 0 ? static_cast<float>(count) : 1.0f);
      }

      // normalise illuminant energy to image mean
      const Vector3f a( Vector3f(i_pInIlluminant3).clampMin(Vector3f::ZERO()) );
      const Vector3f b( a * (mean / (a.average() > 0.0f ? a.average() : 1.0f)));

      // convert to Ruderman space
      inIlluminant = ruderman.fromRgb( b );
   }
   // estimate
   else
   {
      // use 'gray-world' method in Ruderman space

      // sum pixels
      Vector3f sum;
      udword   count = 0;
      for( dword i = 0, end = i_image.getLength();  i < end;  ++i )
      {
         const Vector3f p( i_image.get( i ) );

         // disclude NaNs
         if( !isNan( p ) )
         {
            sum += ruderman.fromRgb( preconditionPixel( p ) );
            ++count;
         }
      }

      // mean pixel
      inIlluminant = sum / (count > 0 ? static_cast<float>(count) : 1.0f);
   }

   return inIlluminant;
}


/*float getMaxMagnitude
(
   const ImageWrapperConst& i_image
)
{
   float max = FLOAT_MIN_NEG;

   // step through all pixels
   for( dword i = i_image.getLength();  i-- > 0; )
   {
      const Vector3f p( i_image.get( i ) );

      // disclude NaNs
      if( !isNan( p ) )
      {
         const float magnitude = preconditionPixel( p ).average();
         max = (max >= magnitude) ? max : magnitude;
      }
   }

   return max;
}*/

}




// exported function -----------------------------------------------------------
void p3whitebalancer::whiteBalance
(
   const float* i_pColorSpace6,
   const float* i_pWhitePoint2,
   const float* i_pInIlluminant3,
   const udword ,//i_options,
         float  i_strength01,
   const udword i_width,
   const udword i_height,
   const udword i_formatFlags,
   const udword i_pixelStride,
   const float* i_pInPixels,
   float*       o_pOutPixels
)
{
   // precondition
   preconditionInputs( i_pColorSpace6, i_pWhitePoint2, i_pInIlluminant3,
      i_strength01 );

   // wrap (and check) images
   const ImageWrapperConst::EChannelOrder channelOrder =
      (p3wb11_BGR == i_formatFlags) ?
      ImageWrapperConst::BGR_e : ImageWrapperConst::RGB_e;
   const ImageWrapperConst inImage( i_width, i_height, channelOrder,
      i_pixelStride, i_pInPixels );
   ImageWrapper outImage( i_width, i_height, channelOrder, i_pixelStride,
      o_pOutPixels );

   // make rgb <-> xyz color conversion (and check primaries)
   Matrix3f rgbToXyz;
   Matrix3f xyzToRgb;
   color::makeColorSpaceConversions( i_pColorSpace6, i_pWhitePoint2,
      &xyzToRgb, &rgbToXyz );

   // make illuminant (and check in-illuminant)
   const Vector3f inIlluminantLab( makeIlluminant( i_pInIlluminant3, inImage,
      rgbToXyz, xyzToRgb ) );

   //const float maxMagnitude = getMaxMagnitude( inImage );

   // map image
   {
      // make mapping
      const PixelMap pixelMap( rgbToXyz, xyzToRgb, inIlluminantLab,
         i_strength01 );

      // step through pixels
      for( dword i = 0, end = outImage.getLength();  i < end;  ++i )
      {
         const Vector3f p( inImage.get( i ) );

         // disclude NaNs
         if( !isNan( p ) )
         {
            // map pixel
            outImage.set( i, postconditionPixel( pixelMap(
               preconditionPixel( p ) ) ) );
         }
         else
         {
            // pass through unchanged
            outImage.set( i, p );
         }
      }
   }
}








/// test -----------------------------------------------------------------------
#ifdef TESTING


#include <ostream>


namespace p3whitebalancer
{
   using namespace hxa7241;


bool test_WhiteBalancer
(
   std::ostream* pOut,
   const bool    ,//verbose,
   const dword   //seed
)
{
   bool isOk = true;

   if( pOut ) *pOut << "[ test_WhiteBalancer ]\n\n";


   if( pOut ) *pOut << "!!! not implemented yet\n\n";
   //isOk = false;


   if( pOut ) *pOut << (isOk ? "--- successfully" : "*** failurefully") <<
      " completed " << "\n\n\n";

   if( pOut ) pOut->flush();


   return isOk;
}


}//namespace


#endif//TESTING
