/*------------------------------------------------------------------------------

   Perceptuum3 rendering components
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


/**
 * Interface for the P3 WhiteBalancer component. Makes the colors in an image
 * look more like they did in the scene where the image was created.
 *
 * http://www.hxa7241.org/whitebalancer/
 *
 *
 * The library has a C interface, defined in p3wbWhiteBalancer.h (this file) and
 * p3wbWhiteBalancer-v__.h . The former, main, part should be stable across
 * versions. The latter specifies constants that will change with versions.
 * There are no other dependencies.
 *
 * To build a client: include p3wbWhiteBalancer-v__.h and link with the
 * p3whitebalancer.lib|.a import library (Windows) or the libp3whitebalancer.so
 * library (Linux), or access purely dynamically.
 *
 *
 * There are two interface sections: meta-versioning, functions.
 *
 * Versioning meta interface:
 * For checking a dynamically linked library supports the interfaces here.
 *
 * Function interface:
 * Call the function with an image and parameters, and receive a result image.
 * There are two alternatives: all parameters, and simple (uses defaults).
 */




#ifndef p3wbWhiteBalancer_h
#define p3wbWhiteBalancer_h


#ifdef __cplusplus
extern "C" {
#endif




/*= version meta-interface ===================================================*/

/**
 * Return values for p3wbIsVersionSupported().
 */
enum p3wbESupported
{
   p3wb_SUPPORTED_NOT   = 0,
   p3wb_SUPPORTED_FULLY = 2
};


/**
 * Get whether this interface is supported by the implementation.
 *
 * (For checking a dynamically linked library implementation.)
 *
 * @version  version value from the options/constants header.
 * @return   a p3wbESupported value
 */
int p3wbIsVersionSupported
(
   int version
);


/**
 * Get the name of the library.
 */
const char* p3wbGetName();


/**
 * Get the copyright of the library.
 */
const char* p3wbGetCopyright();


/**
 * Get the version of the library.
 */
int p3wbGetVersion();







/*= functions ================================================================*/

/**
 * White balance an image, with simple parameters (uses defaults).
 *
 * @i_width        width of input and output images, in pixels
 * @i_height       height of input and output images, in pixels
 * @i_formatFlags  pixel channel order, from the options/constants header
 * @i_pixelStride  number of bytes to add to a pixel pointer to get next,
 *                 will be >= 3 * sizeof(*i_inPixels)
 *                 (give 0 for default: 3 * sizeof(*i_inPixels))
 * @i_inPixels     array of input RGB pixels, channel order as i_formatFlags,
 *                 padding as i_pixelStride,
 * @o_outPixels    array of output RGB pixels, channel order as i_formatFlags,
 *                 padding as i_pixelStride
 *                 (may point to same array as input pixels)
 *
 * @return  1 means succeeded, 0 means failed
 */
int p3wbWhiteBalance1
(
   unsigned int i_width,
   unsigned int i_height,
   unsigned int i_formatFlags,
   unsigned int i_pixelStride,
   const float* i_inPixels,
   float*       o_outPixels
);


/**
 * White balance an image, with full parameters.
 *
 * Pre-conditions:
 * * Input pixels may have any value, however:
 *    * Very small values and negatives will be clamped to a sub-perceptual
 *      small value.
 *    * Very large values will be clamped to a super-perceptual large value.
 * * Non-pixel data must be valid (or it may cause failureful return).
 *
 * Post-conditions:
 * * Output pixel channels will be positive.
 * * Input pixels that are black result in black output pixels
 * * Input pixels containing NaNs pass through unused and unchanged.
 *
 * @i_colorSpace6    chromaticities of image,
 *                   array of six { rx, ry, gx, gy, bx, by },
 *                   each value is > 0 and < 1
 *                   (give 0 for default: ITU-R BT.709 / sRGB)
 * @i_whitePoint2    whitepoint of image,
 *                   array of two { x, y },
 *                   each value is > 0 and < 1
 *                   (give 0 for default: flat 1/3, 1/3)
 * @i_inIlluminant3  illuminant color of image's original context,
 *                   array of three { r, g, b },
 *                   only relative proportions are needed, not absolute
 *                   (give 0 for automatic estimation)
 * @i_options        balancing options, from the options/constants header
 * @i_strength       strength of color-shift, >= 0 and <= 1
 *                   (give -1 for default: 0.8)
 * @i_width          width of input and output images, in pixels
 * @i_height         height of input and output images, in pixels
 * @i_formatFlags    pixel channel order, from the options/constants header
 * @i_pixelStride    number of bytes to add to a pixel pointer to get next,
 *                   will be >= 3 * sizeof(*i_inPixels)
 *                   (give 0 for default: 3 * sizeof(*i_inPixels))
 * @i_inPixels       array of input RGB pixels, channel order as i_formatFlags,
 *                   padding as i_pixelStride,
 * @o_outPixels      array of output RGB pixels, channel order as i_formatFlags,
 *                   padding as i_pixelStride
 *                   (may point to same array as input pixels)
 * @o_message128     string for exception message 128 chars long (or 0),
 *                   will be zero-terminated
 *
 * @return  1 means succeeded, 0 means failed
 */
int p3wbWhiteBalance2
(
   const float* i_colorSpace6,
   const float* i_whitePoint2,
   const float* i_inIlluminant3,
   unsigned int i_options,
   float        i_strength,
   unsigned int i_width,
   unsigned int i_height,
   unsigned int i_formatFlags,
   unsigned int i_pixelStride,
   const float* i_inPixels,
   float*       o_outPixels,
   char*        o_message128
);








/*= test =====================================================================*/

/**
 * Run unit tests.
 *
 * Only works for test builds.
 *
 * @return  1 means succeeded, 0 means failed, -1 means not a test build
 */
int p3wbTestUnits
(
   int whichTest,
   int isOutput,
   int isVerbose,
   int seed
);




#ifdef __cplusplus
} /* extern "C" */
#endif


#endif/*p3wbWhiteBalancer_h*/
