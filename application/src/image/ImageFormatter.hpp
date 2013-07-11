/*--------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

--------------------------------------------------------------------*/


#ifndef ImageFormatter_h
#define ImageFormatter_h


#include <string>




#include "hxa7241_image.hpp"
namespace hxa7241_image
{


/**
 * A general interface for image file IO.<br/><br/>
 *
 * Supports OpenEXR, Radiance-RGBE, PNG, and PPM to read, and PNG and PPM to
 * write.<br/><br/>
 *
 * @exceptions queries throw char[] messages, all throw allocation exceptions
 */
class ImageFormatter
{
/// standard object services ---------------------------------------------------
public:
            ImageFormatter();
            ImageFormatter( const char exrLibraryPathName[],
                            const char pngLibraryPathName[] );

           ~ImageFormatter();
            ImageFormatter( const ImageFormatter& );
   ImageFormatter& operator=( const ImageFormatter& );


/// commands -------------------------------------------------------------------
           void  set( const char exrLibraryPathName[],
                      const char pngLibraryPathName[] );


/// queries --------------------------------------------------------------------
   /**
    * @filePathname extension must be one of:
    *               .exr .rgbe. .pic .hdr .rad .png .ppm
    * @deGamma      gamma to decode with, or 0 for default
    */
           void  readImage ( const char    filePathname[],
                             float         deGamma,
                             ImageAdopter& image )                        const;
   /**
    * @filePathname extension must be one of: .png .ppm
    * @enGamma      gamma to encode with, or 0 for image value
    */
           void  writeImage( const char          filePathname[],
                             float               enGamma,
                             const ImageAdopter& image )                  const;


/// fields ---------------------------------------------------------------------
private:
   std::string exrLibraryPathName_m;
   std::string pngLibraryPathName_m;
};


}//namespace




#endif//ImageFormatter_h
