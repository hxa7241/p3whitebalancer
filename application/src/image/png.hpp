/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef png_h
#define png_h


#include <iosfwd>




#include "hxa7241_image.hpp"
namespace hxa7241_image
{
   using std::istream;
   using std::ostream;


/**
 * Basic IO for the PNG format.<br/><br/>
 *
 * Supports these features:
 * * RGB
 * * 24 or 48 bit pixels
 * * can include primaries and gamma
 * * allows byte, channel, and row re-ordering
 * <br/>
 *
 * This implementation requires the libpng and zlib dynamic libraries when run.
 * (from, for example, libpng-1.2.8-bin and libpng-1.2.8-dep archives)<br/><br/>
 *
 * <cite>http://www.libpng.org/</cite>
 */
namespace png
{
   enum EOrderingFlags
   {
      IS_TOP_FIRST = 1,
      IS_BGR       = 2,
      IS_LO_ENDIAN = 4
   };


   /**
    * Report whether the stream is a PNG file.
    */
   bool isRecognised
   (
      istream& i_inBytes,
      bool     i_isRewind = false
   );


   /**
    * Read PNG image.<br/><br/>
    *
    * @i_pngLibraryPathName if path not included then a standard system search
    *                       strategy is used.
    * @i_orderingFlags      bit combination of EOrderingFlags values
    * @o_pPrimaries8        chromaticities of RGB and white:
    *                       { rx, ry, gx, gy, bx, by, wx, wy }
    * @o_is48Bit            true for 48 bit triples, false for 24
    * @o_pTriples           array of byte triples, or word triples if is48Bit is
    *                       true. orphaned storage
    *
    * @exceptions throws allocation and char[] message exceptions
    */
   void read
   (
      const char i_pngLibraryPathName[],
      istream&   i_inBytes,
      dword      i_orderingFlags,
      float*     o_pPrimaries8,
      float&     o_gamma,
      dword&     o_width,
      dword&     o_height,
      bool&      o_is48Bit,
      void*&     o_pTriples
   );


   /**
    * Write PNG image.<br/><br/>
    *
    * Triples are bottom row first, R then G then B.
    *
    * @i_pngLibraryPathName if path not included then a standard system search
    *                       strategy is used.
    * @i_pPrimaries8        chromaticities of RGB and white:
    *                       { rx, ry, gx, gy, bx, by, wx, wy }
    *                       if zero, they are not written to image metadata
    * @i_gamma              if zero, it is not written to image metadata
    * @i_is48Bit            true for 48 bit triples, false for 24
    * @i_orderingFlags      bit combination of EOrderingFlags values
    * @i_pTriples           array of byte triples, or word triples if is48Bit is
    *                       true
    *
    * @exceptions throws char[] message exceptions
    */
   void write
   (
      const char   i_pngLibraryPathName[],
      dword        i_width,
      dword        i_height,
      const float* i_pPrimaries8,
      float        i_gamma,
      bool         i_is48Bit,
      dword        i_orderingFlags,
      const void*  i_pTriples,
      ostream&     o_outBytes
   );
}


}//namespace




#endif//png_h
