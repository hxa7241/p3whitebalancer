/*------------------------------------------------------------------------------

   Perceptuum3 rendering components
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


/**
 * Options/constants for p3wbWhiteBalancer interface. These will change with
 * implementation versions.
 *
 * Include this file, and link with the library or access purely dynamically.
 */




#ifndef p3wbWhiteBalancer_v12_h
#define p3wbWhiteBalancer_v12_h


#ifdef __cplusplus
extern "C" {
#endif


#include "p3wbWhiteBalancer.h"




/* version ------------------------------------------------------------------ */
/**
 * Version, for use with p3wbIsVersionSupported().
 */
enum p3wb12EVersion
{
   /* current */
   p3wb12_VERSION = 0x00010200,

   /* previous */
   p3wb11_VERSION = 0x00010001,

   /* old and defunct */
   p3wb10_VERSION = 0x00010000
};




/* balancing option flags --------------------------------------------------- */
/**
 * Options for use with p3wbWhiteBalance_() in i_options parameter.
 */
enum p3wb11EBalancingOptions
{
   p3wb11_GW  = 0
};




/* pixel option flags ------------------------------------------------------- */
/**
 * Options for use with p3wbWhiteBalance_() in i_formatFlags parameter.
 *
 * @p3wb11_RGB  pixel parts/channels in storage order R, G, B
 * @p3wb11_BGR  pixel parts/channels in storage order B, G, R
 */
enum p3wb11EPixelOptions
{
   p3wb11_RGB = 0,
   p3wb11_BGR = 1
};




#ifdef __cplusplus
} /* extern "C" */
#endif


#endif/*p3wbWhiteBalancer_v12_h*/
