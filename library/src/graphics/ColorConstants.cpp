/*------------------------------------------------------------------------------

   HXA7241 Graphics library.
   Copyright (c) 2004-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#include <math.h>

#include "ColorConstants.hpp"


using namespace hxa7241_graphics;




namespace
{
   // ITU-R BT.709 / sRGB standard color space
   const float SRGB_COLORSPACE[] =
   {
      // x      y

      // chromaticities
      0.64f, 0.33f,      // r
      0.30f, 0.60f,      // g
      0.15f, 0.06f,      // b

      // whitepoint (D65)
      0.3127f, 0.3290f   // w
   };

   const float* SRGB_CHROMATICITIES = SRGB_COLORSPACE;
   const float* SRGB_WHITEPOINT     = SRGB_COLORSPACE + 6;
}




namespace hxa7241_graphics
{
namespace color
{


/// sRGB and ITU-R BT.709 ------------------------------------------------------

const float* getSrgbChromaticities()
{
   // ITU-R BT.709 reference primaries (sRGB)
   return SRGB_CHROMATICITIES;
}


const float* getSrgbWhitePoint()
{
   // CIE standard illuminant D65 (ITU-R BT.709) (sRGB)
   return SRGB_WHITEPOINT;
}


const float* getSrgbColorSpace()
{
   return SRGB_COLORSPACE;
}


float getSrgbGamma()
{
   // sRGB standard gamma
   static const float SRGB_GAMMA = 1.0f / 2.2f;

   return SRGB_GAMMA;
}


float get709Gamma()
{
   // ITU-R BT.709 standard gamma
   static const float _709_GAMMA = 0.45f;

   return _709_GAMMA;
}


float gammaEncodeSrgb
(
   const float fp01
)
{
   float fp01prime = 0.0f;

   // sRGB standard transfer function
   if( fp01 <= 0.00304f )
   {
      fp01prime = 12.92f * fp01;
   }
   else
   {
      fp01prime = 1.055f * ::powf( fp01, 1.0f / 2.4f ) - 0.055f;
   }

   return fp01prime;
}


float gammaEncode709
(
   const float fp01
)
{
   float fp01prime = 0.0f;

   // ITU-R BT.709 standard transfer function
   if( fp01 <= 0.018f )
   {
      fp01prime = 4.5f * fp01;
   }
   else
   {
      fp01prime = 1.099f * ::powf( fp01, 0.45f ) - 0.099f;
   }

   return fp01prime;
}


float gammaDecodeSrgb
(
   const float fp01prime
)
{
   float fp01 = 0.0f;

   // sRGB standard inverse transfer function
   if( fp01prime <= 0.03928f )
   {
      fp01 = fp01prime * (1.0f / 12.92f);
   }
   else
   {
      fp01 = ::powf( (fp01prime + 0.055f) * (1.0f / 1.055f), 2.4f );
   }

   return fp01;
}


float gammaDecode709
(
   const float fp01prime
)
{
   float fp01 = 0.0f;

   // ITU-R BT.709 standard inverse transfer function
   if( fp01prime <= 0.081f )
   {
      fp01 = fp01prime * (1.0f / 4.5f);
   }
   else
   {
      fp01 = ::powf( (fp01prime + 0.099f) * (1.0f / 1.099f), (1.0f / 0.45f) );
   }

   return fp01;
}




/// monitor --------------------------------------------------------------------

const float* getCrtLuminanceRange()
{
   // approximate CRT luminance limits
   static const float CRT_BLACK =   2.0f;
   static const float CRT_WHITE = 100.0f;

   static const float CRT_RANGE[2] = { CRT_BLACK, CRT_WHITE };

   return CRT_RANGE;
}


const float* getTftLuminanceRange()
{
   // approximate TFT luminance limits
   static const float TFT_BLACK =   2.0f;
   static const float TFT_WHITE = 150.0f;

   static const float TFT_RANGE[2] = { TFT_BLACK, TFT_WHITE };

   return TFT_RANGE;
}


const float* getCrtBestLuminanceRange()
{
   // approximate CRT luminance limits
   static const float CRT_BLACK =   0.01f;
   static const float CRT_WHITE = 175.0f;

   static const float CRT_RANGE[2] = { CRT_BLACK, CRT_WHITE };

   return CRT_RANGE;
}


const float* getTftBestLuminanceRange()
{
   // approximate TFT luminance limits
   // (backlight at medium level)
   static const float TFT_BLACK =   0.5f;
   static const float TFT_WHITE = 300.0f;

   static const float TFT_RANGE[2] = { TFT_BLACK, TFT_WHITE };

   return TFT_RANGE;
}




/// perceptual -----------------------------------------------------------------

float getLuminanceMin()
{
   static const float LUMINANCE_MIN = 1e-4f;

   return LUMINANCE_MIN;
}


float getLuminanceMax()
{
   static const float LUMINANCE_MAX = 1e+6f;

   return LUMINANCE_MAX;
}


const float* getXyzToCone()
{
   static const float XYZ_TO_CONE[] =
   {
      // from Wyszecki and Stiles
      // Hunter-Point-Estevez cone responses
       0.38971f, 0.68898f, -0.07868f,
      -0.22981f, 1.18340f,  0.04641f,
       0.00000f, 0.00000f,  1.00000f

      // Wandell ?
//       0.243f,  0.856f, -0.044f,
//      -0.391f,  1.165f,  0.087f,
//       0.010f, -0.008f,  0.563f
   };

   return XYZ_TO_CONE;
}


/*const float* getConeToRuderman()
{
   static const float CONE_TO_RUDERMAN[] =
   {
      // 'Statistics of Cone Responses to Natural Images'
      // Ruderman, Cronin, Chiao;
      // J. Optical Soc. of America, vol. 15, no. 8.
      // 1998
      //0.57735026918963f,  0.57735026918963f,  0.57735026918963f,
      //0.40824829046386f,  0.40824829046386f, -0.81649658092773f,
      //0.70710678118655f, -0.70710678118655f,  0.00000000000000f
      1.0f/::sqrtf(3.0f),  1.0f/::sqrtf(3.0f),  1.0f/::sqrtf(3.0f),
      1.0f/::sqrtf(6.0f),  1.0f/::sqrtf(6.0f), -2.0f/::sqrtf(6.0f),
      1.0f/::sqrtf(2.0f), -1.0f/::sqrtf(2.0f),  0.0f
   };

   return CONE_TO_RUDERMAN;
}*/


/*const float* getConeToOpponent()
{
   static const float CONE_TO_OPPONENT1[] =
   {
      // Wandell ?
       1.00f,  0.00f,  0.00f,
      -0.59f,  0.80f, -0.12f,
      -0.34f, -0.11f,  0.93f
   };
   static const float CONE_TO_OPPONENT2[] =
   {
      // ?
       0.4523f,  0.8724f,  0.1853f,
       0.7976f, -0.5499f, -0.2477f,
      -0.2946f, -0.5132f,  0.8062f
   };

   return CONE_TO_OPPONENT;
}*/




/// other ----------------------------------------------------------------------

float getLuminanceOfSrgb( float r, float g, float b )
{
   return (r * 0.2126f) + (g * 0.7152f) + (b * 0.0722f);
}


float getZfromXy
(
   const float x,
   const float y
)
{
   return 1.0f - x - y;
}


void getXyzfromXyy
(
   const float x,
   const float y,
   const float Y,
   float*      pXYZ
)
{
   pXYZ[0] = (x / y) * Y;
   pXYZ[1] = Y;
   pXYZ[2] = ((1.0f - x - y) / y) * Y;
}


}//namespace
}//namespace
