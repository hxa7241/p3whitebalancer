/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef exr_h
#define exr_h


#include <iosfwd>




#include "hxa7241_image.hpp"
namespace hxa7241_image
{
   using std::istream;


/**
 * Basic IO for the OpenEXR format.<br/><br/>
 *
 * This implementation requires the OpenEXR dynamic libraries when run.
 * <br/><br/>
 *
 * <cite>http://www.openexr.com/</cite>
 */
namespace exr
{
   enum EOrderingFlags
   {
      IS_LOW_TOP   = 1,
      IS_BGR       = 2
   };


   /**
    * Report whether the stream is a EXR file.
    */
   bool isRecognised
   (
      istream& i_inBytes,
      bool     i_isRewind = false
   );


   /**
    * Read EXR image.<br/><br/>
    *
    * @i_exrLibraryPathName if path not included then a standard system search
    *                       strategy is used.
    * @o_pPrimaries8        chromaticities of RGB and white:
    *                       { rx, ry, gx, gy, bx, by, wx, wy }.
    *                       all 0 if not present.
    * @o_scalingToGetCdm2   scaling needed to make cd/m^2, 0 if not present
    *
    * @exceptions throws allocation and char[] message exceptions
    */
   void read
   (
      const char i_exrLibraryPathName[],
      const char i_filePathName[],
      dword      i_orderingFlags,
      dword&     o_width,
      dword&     o_height,
      float*     o_pPrimaries8,
      float&     o_scalingToGetCdm2,
      float*&    o_pTriples
   );


   /**
    * Write EXR image.<br/><br/>
    *
    * @i_exrLibraryPathName if path not included then a standard system search
    *                       strategy is used.
    * @i_pPrimaries8        chromaticities of RGB and white:
    *                       { rx, ry, gx, gy, bx, by, wx, wy }.
    *                       if zero, they are not written to image metadata.
    * @i_scalingToGetCdm2   scaling needed to make cd/m^2.
    *                       if zero, it is not written to image metadata
    * @i_orderingFlags      bit combination of EOrderingFlags values
    *
    * @exceptions throws char[] message exceptions
    */
   void write
   (
      const char   i_exrLibraryPathName[],
      dword        i_width,
      dword        i_height,
      const float* i_pPrimaries8,
      float        i_scalingToGetCdm2,
      dword        i_orderingFlags,
      const float* i_pTriples,
      const char   i_filePathName[]
   );
}


}//namespace




#endif//exr_h
