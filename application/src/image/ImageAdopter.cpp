/*--------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

--------------------------------------------------------------------*/


#include "ImageAdopter.hpp"


using namespace hxa7241_image;




namespace
{

const char SIZE_INVALID_MESSAGE[] = "size out of range, in ImageAdopter";

}




/// standard object services ---------------------------------------------------
ImageAdopter::ImageAdopter()
 : pPixels_m( 0 )
{
   ImageAdopter::set( 0, 0, 0.0f, 0, 0.0f, 255, /*PIXELS_FLOAT,*/ 0 );
}


ImageAdopter::ImageAdopter
(
   const dword      width,
   const dword      height,
   const float      scaling,
   const float*     pPrimaries8,
   const float      gamma,
   const dword      quantMax,
   float*const      pPixels
//   const EPixelType pixelType,
//   void*const       pPixels
)
 : pPixels_m( 0 )
{
   ImageAdopter::set( width, height, scaling, pPrimaries8, gamma, quantMax,
      /*pixelType,*/ pPixels );
}


ImageAdopter::~ImageAdopter()
{
   destruct( /*pixelType_m,*/ pPixels_m );
}


/*ImageAdopter::ImageAdopter
(
   const ImageAdopter& that,
)
 : pPixels_m( 0 )
{
   ImageAdopter::operator=( that );
}


ImageAdopter& ImageAdopter::operator=
(
   const ImageAdopter& that
)
{
   if( &that != this )
   {
      // allocate new pixel storage
      void*       pPixels = 0;
      const dword length  = that.width_m * that.height_m;
      switch( that.pixelType_m )
      {
         case PIXELS_HALF  :  pPixels = new uword[ length * 3 ];  break;
         case PIXELS_FLOAT :  pPixels = new float[ length * 3 ];  break;
      }

      // copy pixel values
      ::memcpy( pPixels, that.pPixels_m, length * 3 *
         (PIXELS_FLOAT == that.pixelType_m ? sizeof(float) : sizeof(uword)) );

      try
      {
         set( that.width_m, that.height_m, that.scaling_m, that.getPrimaries(),
            that.gamma_m, that.quantMax_m, that.pixelType_m, pPixels );
      }
      catch( ... )
      {
         destruct( that.pixelType_m, pPixels );
         throw;
      }
   }

   return *this;
}*/




/// commands -------------------------------------------------------------------
void ImageAdopter::set
(
   const dword      width,
   const dword      height,
   const float      scaling,
   const float*     pPrimaries8,
   const float      gamma,
   const dword      quantMax,
   float*const      pPixels
//   const EPixelType pixelType,
//   void*const       pPixels
)
{
   // check dimensions positive, and length <= DWORD_MAX
   if( (width < 0) || (height < 0) || (height > (DWORD_MAX / 3)) ||
      ((0 != height) && (width > (DWORD_MAX / height * 3))) )
   {
      throw SIZE_INVALID_MESSAGE;
   }

   width_m  = width;
   height_m = height;

   scaling_m = scaling;

   isPrimariesSet_m = false;
   if( pPrimaries8 )
   {
      // primaries are 'set' if they are not all zero
      for( dword i = 8;  i-- > 0; )
      {
         primaries8_m[i]   = pPrimaries8[i];
         isPrimariesSet_m |= (0.0f != pPrimaries8[i]);
      }
   }

   gamma_m    = gamma;
   quantMax_m = quantMax;

   destruct( /*pixelType,*/ pPixels_m );
   pPixels_m   = pPixels;
//   pixelType_m = pixelType;
//   pPixels_m   = pPixels;
}




/// queries --------------------------------------------------------------------
dword ImageAdopter::getWidth() const
{
   return width_m;
}


dword ImageAdopter::getHeight() const
{
   return height_m;
}


float ImageAdopter::getScaling() const
{
   return scaling_m;
}


const float* ImageAdopter::getPrimaries() const
{
   return isPrimariesSet_m ? primaries8_m : 0;
}


const float* ImageAdopter::getColorspace() const
{
   return getPrimaries();
}


const float* ImageAdopter::getWhitepoint() const
{
   return isPrimariesSet_m ? primaries8_m + 6 : 0;
}


float ImageAdopter::getGamma() const
{
   return gamma_m;
}


dword ImageAdopter::getQuantMax() const
{
   return quantMax_m;
}


float* ImageAdopter::getPixels() const
{
   return pPixels_m;
}


//ImageAdopter::EPixelType ImageAdopter::getPixelType() const
//{
//   return pixelType_m;
//}
//
//
//void* ImageAdopter::getPixels() const
//{
//   return pPixels_m;
//}




/// implementation -------------------------------------------------------------
void ImageAdopter::destruct
(
   float*     pPixels
//   EPixelType pixelType,
//   void*      pPixels
)
{
   delete[] pPixels;

//   switch( pixelType )
//   {
//      case PIXELS_HALF  :  delete[] static_cast<uword*>( pPixels );  break;
//      case PIXELS_FLOAT :  delete[] static_cast<float*>( pPixels );  break;
//   }
}
