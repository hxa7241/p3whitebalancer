/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


//#include "half.h"
#include "Vector3f.hpp"

#include "ImageWrapperConst.hpp"


using namespace hxa7241_image;




namespace
{

/// constants ------------------------------------------------------------------
const char SIZE_EXCEPTION_MESSAGE[] =
   "size out of range, in ImageWrapper construction";
const char PIXEL_STRIDE_EXCEPTION_MESSAGE[] =
   "pixel stride too small, in ImageWrapper construction";
const char NULL_PIXELS_POINTER_EXCEPTION_MESSAGE[] =
   "pixels pointer null, in ImageWrapper construction";

}




/// standard object services ---------------------------------------------------
ImageWrapperConst::ImageWrapperConst
(
   const dword         width,
   const dword         height,
   const EChannelOrder channelOrder,
   const udword        pixelStride,
   const float*const   pPixels
)
{
   ImageWrapperConst::construct( width, height, channelOrder, FLOAT_e,
      pixelStride, pPixels );
}


//ImageWrapperConst::ImageWrapperConst
//(
//   const dword         width,
//   const dword         height,
//   const EChannelOrder channelOrder,
//   const udword        pixelStride,
//   const uword*const   pPixels
//)
//{
//   ImageWrapperConst::construct( width, height, channelOrder, /*HALF_e,*/
//      pixelStride, pPixels );
//}


ImageWrapperConst::~ImageWrapperConst()
{
}


ImageWrapperConst::ImageWrapperConst
(
   const ImageWrapperConst& that
)
{
   ImageWrapperConst::operator=( that );
}


ImageWrapperConst& ImageWrapperConst::operator=
(
   const ImageWrapperConst& that
)
{
   if( &that != this )
   {
      width_m        = that.width_m;
      height_m       = that.height_m;
      channelOrder_m = that.channelOrder_m;
      channelType_m  = that.channelType_m;
      pixelStride_m  = that.pixelStride_m;
      pPixels_m      = that.pPixels_m;
   }

   return *this;
}




/// queries --------------------------------------------------------------------
dword ImageWrapperConst::getWidth() const
{
   return width_m;
}


dword ImageWrapperConst::getHeight() const
{
   return height_m;
}


dword ImageWrapperConst::getLength() const
{
   return width_m * height_m;
}


Vector3f ImageWrapperConst::get
(
   const dword x,
   const dword y
) const
{
   return get( (y * width_m) + x );
}


Vector3f ImageWrapperConst::get
(
   const dword i
) const
{
   float pixel[3];
   {
      const void* pPixelBytes = static_cast<const ubyte*>(pPixels_m) +
         (i * pixelStride_m);
      switch( channelType_m )
      {
         case HALF_e :
         {
            //const uword* pt = static_cast<const uword*>( pPixelBytes );
            pixel[0] = 0.0f;//halfToFloat( pt[0] );
            pixel[1] = 0.0f;//halfToFloat( pt[1] );
            pixel[2] = 0.0f;//halfToFloat( pt[2] );
            break;
         }
         case FLOAT_e :
         {
            const float* pt = static_cast<const float*>( pPixelBytes );
            pixel[0] = pt[0];
            pixel[1] = pt[1];
            pixel[2] = pt[2];
            break;
         }
      }
   }

   return (RGB_e == channelOrder_m) ?
      Vector3f( pixel ) : Vector3f( pixel[2], pixel[1], pixel[0] );
}




/// implementation -------------------------------------------------------------
void ImageWrapperConst::construct
(
   const dword         width,
   const dword         height,
   const EChannelOrder channelOrder,
   const EChannelType  channelType,
         udword        pixelStride,
   const void* const   pPixels
)
{
   // dimensions positive, and length <= DWORD_MAX
   if( (width < 0) || (height < 0) || (height > (DWORD_MAX / 3)) ||
      ((0 != height) && (width > (DWORD_MAX / height * 3))) )
   {
      throw SIZE_EXCEPTION_MESSAGE;
   }

   // maybe default pixel stride
   pixelStride = (0 != pixelStride) ? pixelStride :
      ( (FLOAT_e == channelType) ? (sizeof(float) * 3) : (sizeof(uword) * 3) );

   // pixel stride not smaller than RGB size
   if( ((FLOAT_e == channelType) && (pixelStride < (sizeof(float) * 3))) ||
      ((HALF_e == channelType) && (pixelStride < (sizeof(uword) * 3))) )
   {
      throw PIXEL_STRIDE_EXCEPTION_MESSAGE;
   }

   // pixels not null
   if( !pPixels )
   {
      throw NULL_PIXELS_POINTER_EXCEPTION_MESSAGE;
   }

   width_m        = width;
   height_m       = height;
   channelOrder_m = channelOrder;
   channelType_m  = channelType;
   pixelStride_m  = pixelStride;
   pPixels_m      = pPixels;
}
