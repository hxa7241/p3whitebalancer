/*--------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

--------------------------------------------------------------------*/


#include <ctype.h>
#include <fstream>

#include "exr.hpp"
#include "rgbe.hpp"
#include "png.hpp"
#include "ppm.hpp"
#include "ImageAdopter.hpp"
#include "ImageQuantizing.hpp"

#include "ImageFormatter.hpp"


using namespace hxa7241_image;




namespace
{

/// constants
const char HXA7241_URI[] = "http://www.hxa7241.org/";

const char NO_READ_FORMATTER_EXCEPTION_MESSAGE[] =
   "could not read unrecognized image format";
const char NO_WRITE_FORMATTER_EXCEPTION_MESSAGE[] =
   "could not write unrecognized image format";
const char FILE_OPEN_EXCEPTION_MESSAGE[] =
   "could not open image file";
const char HALF_PIXELS_EXCEPTION_MESSAGE[] =
   "Half type pixels not implemented";


std::string getFileNameExtension
(
   const char i_filePathname[]
);

void deleteTriples
(
   const dword quantMax,
   void*       pTriplesInt
);

}




/// standard object services ---------------------------------------------------
ImageFormatter::ImageFormatter()
 : exrLibraryPathName_m()
 , pngLibraryPathName_m()
{
}


ImageFormatter::ImageFormatter
(
   const char exrLibraryPathName[],
   const char pngLibraryPathName[]
)
 : exrLibraryPathName_m()
 , pngLibraryPathName_m()
{
   ImageFormatter::set( exrLibraryPathName, pngLibraryPathName );
}


ImageFormatter::~ImageFormatter()
{
}


ImageFormatter::ImageFormatter
(
   const ImageFormatter& that
)
{
   ImageFormatter::operator=( that );
}


ImageFormatter& ImageFormatter::operator=
(
   const ImageFormatter& that
)
{
   if( &that != this )
   {
      exrLibraryPathName_m = that.exrLibraryPathName_m;
      pngLibraryPathName_m = that.pngLibraryPathName_m;
   }

   return *this;
}




/// commands -------------------------------------------------------------------
void ImageFormatter::set
(
   const char exrLibraryPathName[],
   const char pngLibraryPathName[]
)
{
   if( exrLibraryPathName ) exrLibraryPathName_m = exrLibraryPathName;
   if( pngLibraryPathName ) pngLibraryPathName_m = pngLibraryPathName;
}




/// queries --------------------------------------------------------------------
void ImageFormatter::readImage
(
   const char    i_filePathname[],
   const float   i_deGamma,
   ImageAdopter& o_image
) const
{
   // declare image data to be filled
   dword  width            = 0;
   dword  height           = 0;
   dword  quantMax         = 0;
   float  primaries[8]     = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
   float  scalingToGetCdm2 = 0.0f;
   float  deGamma          = 0.0f;
   float* pTriplesFp       = 0;

   // choose formatter and read image data (some may be left as zero)

   // get name ext
   const std::string nameExt( getFileNameExtension( i_filePathname ) );

   // OpenEXR (exr)
   if( nameExt == "exr" )
   {
      // read image file into data
      exr::read( exrLibraryPathName_m.c_str(), i_filePathname, 0, width, height,
         primaries, scalingToGetCdm2, pTriplesFp );
   }
   else
   {
      // make file in-stream
      std::ifstream inBytes( i_filePathname, std::ifstream::binary );
      if( !inBytes )
      {
         throw FILE_OPEN_EXCEPTION_MESSAGE;
      }

      // Radiance (rgbe pic hdr rad)
      if( (nameExt == "hdr") || (nameExt == "rad") ||
         (nameExt == "rgbe") || (nameExt == "pic") )
      {
         float exposure = 0.0f;

         // read image file into data
         rgbe::read( inBytes, false, width, height, primaries, exposure,
            pTriplesFp );

         scalingToGetCdm2 = (exposure != 0.0f) ? 1.0f / exposure : 0.0f;
      }
      // PNG or PPM (png ppm)
      else if( (nameExt == "png") || (nameExt == "ppm") )
      {
         void* pTriplesInt = 0;

         // PNG
         if( nameExt == "png" )
         {
            bool is48Bit = false;

            // read image file into data
            png::read( pngLibraryPathName_m.c_str(), inBytes, 0, primaries,
               deGamma, width, height, is48Bit, pTriplesInt );

            quantMax = !is48Bit ? 255 : 65535;
         }
         // PPM
         else
         {
            // read image file into data
            ppm::read( inBytes, 0, width, height, quantMax, pTriplesInt );
         }

         try
         {
            // convert to float pixels
            // (parameter gamma overrides file gamma)
            quantizing::makeFloatImage( (0.0f != i_deGamma) ?
               i_deGamma : deGamma, width, height, quantMax, pTriplesInt,
               0, pTriplesFp );

            deleteTriples( quantMax, pTriplesInt );
         }
         catch( ... )
         {
            deleteTriples( quantMax, pTriplesInt );
            throw;
         }
      }
   }

   if( !pTriplesFp )
   {
      throw NO_READ_FORMATTER_EXCEPTION_MESSAGE;
   }

   // set image with data, adopting pixel storage
   o_image.set( width, height, scalingToGetCdm2, primaries, deGamma,
      quantMax, pTriplesFp );
}


void ImageFormatter::writeImage
(
   const char          i_filePathname[],
   const float         i_enGamma,
   const ImageAdopter& i_image
) const
{
   // extract image data
   const dword  width       = i_image.getWidth();
   const dword  height      = i_image.getHeight();
   const float* pPrimaries8 = i_image.getPrimaries();

   // get name ext
   const std::string nameExt( getFileNameExtension( i_filePathname ) );

   // make file out-stream
   std::ofstream outBytes( i_filePathname, std::ofstream::binary );
   if( !outBytes )
   {
      throw FILE_OPEN_EXCEPTION_MESSAGE;
   }

   // OpenEXR (exr)
   if( nameExt == "exr" )
   {
      exr::write( exrLibraryPathName_m.c_str(), width, height, pPrimaries8,
         i_image.getScaling(), 0, i_image.getPixels(), i_filePathname );
   }
   // Radiance (rgbe pic hdr rad)
   else if( (nameExt == "hdr") || (nameExt == "rad") ||
      (nameExt == "rgbe") || (nameExt == "pic") )
   {
      const float exposure = (i_image.getScaling() != 0.0f) ?
         1.0f / i_image.getScaling() : 0.0f;

      rgbe::write( HXA7241_URI, width, height, pPrimaries8, exposure, 0,
         i_image.getPixels(), outBytes );
   }
   // PNG or PPM (png ppm)
   else if( (nameExt == "png") || (nameExt == "ppm") )
   {
      const float enGamma  = i_image.getGamma();
      const dword quantMax = i_image.getQuantMax();

      void* pTriplesInt = 0;
      try
      {
         // convert to integer pixels
         quantizing::makeIntegerImage( (0.0f != i_enGamma) ? i_enGamma : enGamma,
            width, height, quantMax, i_image.getPixels(), 0, pTriplesInt );

         // choose formatter
         // PPM
         if( nameExt == "ppm" )
         {
            // write image data to stream
            ppm::write( HXA7241_URI, width, height, quantMax, 0, pTriplesInt,
               outBytes );
         }
         // PNG
         else
         {
            // write image data to stream
            png::write( pngLibraryPathName_m.c_str(), width, height, pPrimaries8,
               enGamma, (quantMax >= 256), 0, pTriplesInt, outBytes );
         }

         deleteTriples( quantMax, pTriplesInt );
      }
      catch( ... )
      {
         deleteTriples( quantMax, pTriplesInt );
         throw;
      }
   }
   else
   {
      throw NO_WRITE_FORMATTER_EXCEPTION_MESSAGE;
   }
}




/// implementation -------------------------------------------------------------
namespace
{

std::string getFileNameExtension
(
   const char i_filePathname[]
)
{
   std::string nameExt;

   std::string fpn( i_filePathname );
   const size_t extPos = fpn.rfind( '.' );
   if( std::string::npos != extPos )
   {
      nameExt = fpn.substr( extPos + 1 );

      // lower case-ify
      // (you would think there is a function for this, but, unbelievably, no.)
      for( dword i = nameExt.length();  i-- > 0; )
      {
         nameExt[i] = static_cast<char>(::tolower( nameExt[i] ));
      }
   }

   return nameExt;
}


void deleteTriples
(
   const dword quantMax,
   void*       pTriplesInt
)
{
   if( quantMax <= 255 )
   {
      delete[] static_cast<ubyte*>(pTriplesInt);
   }
   else
   {
      delete[] static_cast<uword*>(pTriplesInt);
   }
}

}
