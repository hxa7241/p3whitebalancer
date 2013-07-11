/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


//#include "half.h"
#include "Vector3f.hpp"

#include "ImageWrapper.hpp"


using namespace hxa7241_image;




/// standard object services ---------------------------------------------------
ImageWrapper::ImageWrapper
(
   const dword         width,
   const dword         height,
   const EChannelOrder channelOrder,
   const udword        pixelStride,
   float*const         pPixels
)
 : ImageWrapperConst( width, height, channelOrder, pixelStride, pPixels )
{
}


//ImageWrapper::ImageWrapper
//(
//   const dword         width,
//   const dword         height,
//   const EChannelOrder channelOrder,
//   const udword        pixelStride,
//   uword*const         pPixels
//)
// : ImageWrapperConst( width, height, channelOrder, pixelStride, pPixels )
//{
//}


ImageWrapper::~ImageWrapper()
{
}


ImageWrapper::ImageWrapper
(
   const ImageWrapper& that
)
 : ImageWrapperConst( that )
{
}


ImageWrapper& ImageWrapper::operator=
(
   const ImageWrapper& that
)
{
   ImageWrapperConst::operator=( that );

   return *this;
}




/// commands -------------------------------------------------------------------
void ImageWrapper::set
(
   const dword     x,
   const dword     y,
   const Vector3f& element
)
{
   set( (y * width_m) + x, element );
}


void ImageWrapper::set
(
   const dword     i,
   const Vector3f& element
)
{
   float pixel[3];
   switch( channelOrder_m )
   {
      case RGB_e : element.get( pixel );                        break;
      case BGR_e : element.get( pixel[2], pixel[1], pixel[0] ); break;
   }

   void* pPixelBytes = static_cast<ubyte*>(const_cast<void*>(pPixels_m)) +
      (i * pixelStride_m);

   switch( channelType_m )
   {
      case HALF_e :
      {
         uword* pt = static_cast<uword*>( pPixelBytes );
         pt[0] = 0;//floatToHalf( pixel[0] );
         pt[1] = 0;//floatToHalf( pixel[1] );
         pt[2] = 0;//floatToHalf( pixel[2] );
         break;
      }
      case FLOAT_e :
      {
         float* pt = static_cast<float*>( pPixelBytes );
         pt[0] = pixel[0];
         pt[1] = pixel[1];
         pt[2] = pixel[2];
         break;
      }
   }
}
