/*--------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

--------------------------------------------------------------------*/


#ifndef ImageAdopter_h
#define ImageAdopter_h




#include "hxa7241_image.hpp"
namespace hxa7241_image
{


/**
 * A simple pointer-adopting image wrapper.<br/><br/>
 *
 * A record of image file data. Absence is represented by zeros, for scaling,
 * primaries, gamma, and quantMax.<br/><br/>
 *
 * @exceptions
 * constructors and set can throw
 */
class ImageAdopter
{
public:
//   enum EPixelType
//   {
//      PIXELS_HALF,
//      PIXELS_FLOAT
//   };


/// standard object services ---------------------------------------------------
public:
            ImageAdopter();
            ImageAdopter( dword        width,
                          dword        height,
                          float        scaling,
                          const float* pPrimaries8,
                          float        gamma,
                          dword        quantMax,
                          float*       pPixels );
//                          EPixelType   pixelType,
//                          void*        pPixels );
           ~ImageAdopter();
private:
            ImageAdopter( const ImageAdopter& );
   ImageAdopter& operator=( const ImageAdopter& );
public:


/// commands -------------------------------------------------------------------
           void  set( dword        width,
                      dword        height,
                      float        scaling,
                      const float* pPrimaries8,
                      float        gamma,
                      dword        quantMax,
                      float*       pPixels );
//                      EPixelType   pixelType,
//                      void*        pPixels );


/// queries --------------------------------------------------------------------
           dword        getWidth()                                        const;
           dword        getHeight()                                       const;
           float        getScaling()                                      const;
           const float* getPrimaries()                                    const;
           const float* getColorspace()                                   const;
           const float* getWhitepoint()                                   const;
           float        getGamma()                                        const;
           dword        getQuantMax()                                     const;
           float*       getPixels()                                       const;
//         EPixelType   getPixelType()                                    const;
//         void*        getPixels()                                       const;


/// implementation -------------------------------------------------------------
protected:
   static  void  destruct( float*     pPixels );
//                           EPixelType pixelType,
//                           void*      pPixels );


/// fields ---------------------------------------------------------------------
private:
   dword      width_m;
   dword      height_m;

   float      scaling_m;

   bool       isPrimariesSet_m;
   float      primaries8_m[8];

   float      gamma_m;
   dword      quantMax_m;

   float*     pPixels_m;
//   EPixelType pixelType_m;
//   void*      pPixels_m;
};


}//namespace




#endif//ImageAdopter_h
