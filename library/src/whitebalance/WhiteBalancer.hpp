/*------------------------------------------------------------------------------

   Perceptuum3 rendering components
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef WhiteBalancer_h
#define WhiteBalancer_h


#include "Primitives.hpp"




namespace p3whitebalancer
{
   using namespace hxa7241;


/**
 * White balance an image.<br/><br/>
 *
 * Pre-conditions:
 * * Input pixels may have any value, however:
 *    * Very small values and negatives will be clamped to a sub-perceptual
 *      small value.
 *    * Very large values will be clamped a super-perceptual large value.
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
 *
 * @throws exceptions
 */
void whiteBalance
(
   const float* i_colorSpace6,
   const float* i_whitePoint2,
   const float* i_pInIlluminant3,
   unsigned int i_options,
   float        i_strength,
   udword       i_width,
   udword       i_height,
   udword       i_formatFlags,
   udword       i_pixelStride,
   const float* i_pInPixels,
   float*       o_pOutPixels
);


}




#endif/*WhiteBalancer_h*/
