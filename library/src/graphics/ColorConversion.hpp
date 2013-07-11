/*------------------------------------------------------------------------------

   HXA7241 Graphics library.
   Copyright (c) 2004-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef ColorConversion_h
#define ColorConversion_h




#include "hxa7241_graphics.hpp"
namespace hxa7241_graphics
{


namespace color
{

/**
 * Make conversions between CIE XYZ and sRGB.
 */
void makeSrgbConversions
(
   Matrix3f* pXyzToRgb,
   Matrix3f* pRgbToXyz
);


/**
 * Make conversions between CIE XYZ and a color space.<br/><br/>
 *
 * @pColorspace6
 * which way around is the chromaticities array? -- the sRGB ones would be:
 * <pre>{
 *    // x    y
 *    0.64, 0.33,   // r
 *    0.30, 0.60,   // g
 *    0.15, 0.06    // b
 * }</pre><br/><br/>
 * each value is [0,1]
 *
 * @pWhitePoint2 each value is (0,1)
 *
 * @exceptions throws if colorspace or whitepoint invalid
 */
void makeColorSpaceConversions
(
   const float* pColorspace6,
   const float* pWhitePoint2,
   Matrix3f*    pXyzToRgb,
   Matrix3f*    pRgbToXyz
);

}//namespace


}//namespace




#endif//ColorConversion_h
