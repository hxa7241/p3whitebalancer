/*------------------------------------------------------------------------------

   HXA7241 Graphics library.
   Copyright (c) 2004-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#include <math.h>

#include "Matrix3f.hpp"
#include "ColorConstants.hpp"

#include "ColorConversion.hpp"


using namespace hxa7241_graphics;




/// exception messages ---------------------------------------------------------
namespace
{

const char INVALID_CHROMATICITIES_MESSAGE[] =
   "invalid chromaticities given to makeColorSpaceConversions";
const char INVALID_WHITEPOINT_MESSAGE[] =
   "invalid whitepoint given to makeColorSpaceConversions";
const char INVALID_COLORSPACE_MESSAGE[] =
   "invalid colorspace given to makeColorSpaceConversions";

}




/// functions ------------------------------------------------------------------
namespace hxa7241_graphics
{
namespace color
{


void makeSrgbConversions
(
   Matrix3f* pXyzToRgb,
   Matrix3f* pRgbToXyz
)
{
   makeColorSpaceConversions( getSrgbChromaticities(), getSrgbWhitePoint(),
      pXyzToRgb, pRgbToXyz );
}


void makeColorSpaceConversions
(
   const float* pColorspace6,
   const float* pWhitePoint2,
   Matrix3f*    pXyzToRgb,
   Matrix3f*    pRgbToXyz
)
{
   if( !pColorspace6 )
   {
      throw INVALID_CHROMATICITIES_MESSAGE;
   }
   if( !pWhitePoint2 )
   {
      throw INVALID_WHITEPOINT_MESSAGE;
   }

   // make chromaticities matrix
   Matrix3f chrm;
   {
      Vector3f cvs[3];
      for( dword i = 3;  i-- > 0; )
      {
         const float x = pColorspace6[i * 2 + 0];
         const float y = pColorspace6[i * 2 + 1];

         if( (x < 0.0f) | (x > 1.0f) | (y < 0.0f) | (y > 1.0f) )
         {
            throw INVALID_CHROMATICITIES_MESSAGE;
         }

         cvs[i].set( x, y, 1.0f - (x + y) );
      }

      chrm.setColumns( cvs[0], cvs[1], cvs[2], Vector3f::ZERO() );
   }

   // make white color vector
   Vector3f whiteColor;
   {
      const float x = pWhitePoint2[0];
      const float y = pWhitePoint2[1];

      if( (x < FLOAT_EPSILON) | (x >= 1.0f) |
         (y < FLOAT_EPSILON) | (y >= 1.0f) )
      {
         throw INVALID_WHITEPOINT_MESSAGE;
      }

      // check special middle case -- to make identity transform exact
      if( (::fabsf(x - (1.0f / 3.0f)) < 1e-3) &&
         (::fabsf(y - (1.0f / 3.0f)) < 1e-3) )
      {
         whiteColor = Vector3f::ONE();
      }
      else
      {
         whiteColor.set( x, y, 1.0f - (x + y) );

         whiteColor /= whiteColor.getY();
      }
   }

   Matrix3f rgbToXyz( chrm );

   // inverted chromaticities * white color to calculate the unknown
   if( !chrm.invert() )
   {
      throw INVALID_CHROMATICITIES_MESSAGE;
   }
   const Vector3f c( chrm * whiteColor );

   // scaled chrms makes the conversion matrix
   rgbToXyz *= c;

   // set output
   if( pRgbToXyz )
   {
      *pRgbToXyz = rgbToXyz;
   }

   if( pXyzToRgb )
   {
      // inverse conversion is the same, but inverted
      *pXyzToRgb = rgbToXyz;
      if( !pXyzToRgb->invert() )
      {
         throw INVALID_COLORSPACE_MESSAGE;
      }
   }
}


}//namespace
}//namespace








/// test -----------------------------------------------------------------------
#ifdef TESTING


#include <math.h>
#include <istream>
#include <ostream>


namespace hxa7241_graphics
{

std::ostream& operator<<( std::ostream&, const Vector3f& );
std::istream& operator>>( std::istream&,       Vector3f& );

std::ostream& operator<<
(
   std::ostream&   out,
   const Vector3f& obj
)
{
   return out << '(' << obj[0] << ' ' << obj[1] << ' ' << obj[2] << ')';
}

std::istream& operator>>
(
   std::istream& in,
   Vector3f&     obj
)
{
   char  c;
   float xyz[3];

   in >> c >> xyz[0] >> xyz[1] >> xyz[2] >> c;
   obj.set( xyz );

   return in;
}


bool test_ColorConversion
(
   std::ostream* pOut,
   const bool    isVerbose,
   const dword   //seed
)
{
   bool isOk = true;

   if( pOut ) *pOut << "[ test_ColorConversion ]\n\n";


   // constants comparison
   {
      bool isOk_ = true;

      // from http://www.poynton.com/ color faq
      // (these appear to be a little imprecise)
      static const Vector3f RGB_OF_XYZ111(  1.204794f,  0.948292f,  0.908916f );
      static const Vector3f RGB_OF_XYZ100(  3.240479f, -0.969256f,  0.055648f );
      static const Vector3f RGB_OF_XYZ010( -1.537150f,  1.875992f, -0.204043f );
      static const Vector3f RGB_OF_XYZ001( -0.498535f,  0.041556f,  1.057331f );
      static const Vector3f TOLERANCE1( 1e-3f, 1e-3f, 1e-3f );
      static const Vector3f TOLERANCE2( 1e-6f, 1e-6f, 1e-6f );

      static const Vector3f xyzs[] = {
         Vector3f::ONE(), Vector3f::X(), Vector3f::Y(), Vector3f::Z() };
      static const Vector3f rgbs[] = {
         RGB_OF_XYZ111, RGB_OF_XYZ100, RGB_OF_XYZ010, RGB_OF_XYZ001 };

      Matrix3f xyzToRgb;
      Matrix3f rgbToXyz;
      color::makeSrgbConversions( &xyzToRgb, &rgbToXyz );

      for( udword i = 0;  i < sizeof(xyzs)/sizeof(xyzs[0]);  ++i )
      {
         // forward to RGB
         const Vector3f rgb( xyzToRgb * xyzs[i] );
         const Vector3f dif1( (rgb - rgbs[i]).absEq() );
         isOk_ &= (dif1 < TOLERANCE1) == Vector3f::ONE();

         if( pOut && isVerbose ) *pOut << rgb << "  " << rgbs[i] << "  " <<
            dif1 << "  " << isOk_ << "\n";

         // back to XYZ
         const Vector3f xyz( rgbToXyz * rgb );
         const Vector3f dif2( (xyz - xyzs[i]).absEq() );
         isOk_ &= (dif2 < TOLERANCE2) == Vector3f::ONE();

         if( pOut && isVerbose ) *pOut << xyz << "  " << xyzs[i] << "  " <<
            dif2 << "  " << isOk_ << "\n\n";
      }

      //if( pOut && isVerbose ) *pOut << "\n";

      if( pOut ) *pOut << "constants : " <<
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
