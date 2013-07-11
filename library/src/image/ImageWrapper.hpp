/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef ImageWrapper_h
#define ImageWrapper_h


#include "ImageWrapperConst.hpp"




#include "hxa7241_image.hpp"
namespace hxa7241_image
{
   using namespace hxa7241;


/**
 * Wrapper of image of float triplet pixels.<br/><br/>
 *
 * @exceptions
 * Constructor can throw.
 */
class ImageWrapper
   : public ImageWrapperConst
{
/// standard object services ---------------------------------------------------
public:
            ImageWrapper( dword         width,
                          dword         height,
                          EChannelOrder channelOrder,
                          udword        pixelStride,
                          float*        pPixels );
//            ImageWrapper( dword         width,
//                          dword         height,
//                          EChannelOrder channelOrder,
//                          udword        pixelStride,
//                          uword*        pPixels );

           ~ImageWrapper();
            ImageWrapper( const ImageWrapper& );
   ImageWrapper& operator=( const ImageWrapper& );


/// commands -------------------------------------------------------------------
           void  set( dword x,
                      dword y,
                      const Vector3f& );
           void  set( dword i,
                      const Vector3f& );
};


}//namespace




#endif//ImageWrapper_h
