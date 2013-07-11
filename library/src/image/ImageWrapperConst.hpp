/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef ImageWrapperConst_h
#define ImageWrapperConst_h


#include "hxa7241_graphics.hpp"




#include "hxa7241_image.hpp"
namespace hxa7241_image
{
   using hxa7241_graphics::Vector3f;


/**
 * Wrapper of constant image of float triplet pixels.<br/><br/>
 *
 * Constant.
 *
 * @exceptions
 * Constructor can throw.
 */
class ImageWrapperConst
{
public:
   enum EChannelOrder
   {
      RGB_e,
      BGR_e
   };

   enum EChannelType
   {
      HALF_e,
      FLOAT_e
   };


/// standard object services ---------------------------------------------------
            ImageWrapperConst( dword         width,
                               dword         height,
                               EChannelOrder channelOrder,
                               udword        pixelStride,
                               const float*  pPixels );
//            ImageWrapperConst( dword         width,
//                               dword         height,
//                               EChannelOrder channelOrder,
//                               udword        pixelStride,
//                               const uword*  pPixels );

           ~ImageWrapperConst();
            ImageWrapperConst( const ImageWrapperConst& );
   ImageWrapperConst& operator=( const ImageWrapperConst& );


/// queries --------------------------------------------------------------------
           dword    getWidth()                                            const;
           dword    getHeight()                                           const;
           dword    getLength()                                           const;

           Vector3f get( dword x,
                         dword y )                                        const;
           Vector3f get( dword i )                                        const;


/// implementation -------------------------------------------------------------
protected:
           void     construct( dword         width,
                               dword         height,
                               EChannelOrder channelOrder,
                               EChannelType  channelType,
                               udword        pixelStride,
                               const void*   pPixels );


/// fields ---------------------------------------------------------------------
protected:
   dword         width_m;
   dword         height_m;

   EChannelOrder channelOrder_m;
   EChannelType  channelType_m;
   udword        pixelStride_m;

   const void*   pPixels_m;
};


}//namespace




#endif//ImageWrapperConst_h
