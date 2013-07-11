/*------------------------------------------------------------------------------

   HXA7241 Image library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#include <vector>
#include <string>
#include <utility>

#include "ImfCRgbaFile.h"

#include "DynamicLibraryInterface.hpp"
#include "StreamExceptionSet.hpp"

#include "exr.hpp"


using namespace hxa7241_image;
using namespace hxa7241_general;




namespace
{

/// constants ------------------------------------------------------------------
const char IN_STREAM_EXCEPTION_MESSAGE[]  =
   "stream read failure, in EXR read";
const char OUT_STREAM_EXCEPTION_MESSAGE[] =
   "stream write failure, in EXR write";
const char OPEN_FILE_EXCEPTION_MESSAGE[]  =
   "EXR open file failed";
const char NEW_HEADER_EXCEPTION_MESSAGE[] =
   "make new header failed, in EXR write";

const char HXA7241_URI[] = "http://www.hxa7241.org/";

const char LIB_PATHNAME_DEFAULT[] =
#ifdef _PLATFORM_WIN
   "IlmImf_dll.dll";
#elif _PLATFORM_LINUX
   "libIlmImf.so.2";
#else
   "";
#endif


/// globals
void* libraries_g[] = { 0, 0, 0, 0 };


/// ----------------------------------------------------------------------------
void loadLibraries
(
   const char exrLibraryPathName[]
);

void freeLibraries();


/// read sub-procedure declarations --------------------------------------------
void readHeader
(
   const ImfInputFile* pExrInFile,
   dword&              width,
   dword&              height,
   bool&               isLowTop,
   float*              pPrimaries8,
   float&              scalingToGetCdm2
);

void readPixels
(
   ImfInputFile* pExrInFile,
   dword         width,
   dword         height,
   bool          isLowTop,
   dword         orderingFlags,
   float*        pTriples
);

}




bool hxa7241_image::exr::isRecognised
(
   istream&   i_in,
   const bool i_isRewind
)
{
   // enable stream exceptions
   StreamExceptionSet streamExceptionSet( i_in,
      istream::badbit | istream::failbit | istream::eofbit );

   try
   {
      // save stream start position
      const istream::pos_type streamStartPos = i_in.tellg();

      // read id
      ubyte id[ 4 ];
      i_in.read( reinterpret_cast<char*>(id), sizeof(id) );

      // rewind stream
      if( i_isRewind )
      {
         i_in.seekg( streamStartPos );
      }

      // compare id
      return ((id[3] << 24) | (id[2] << 16) | (id[1] << 8) | (id[0])) == IMF_MAGIC;
   }
   // translate exceptions
   catch( const std::ios_base::failure& )
   {
      throw IN_STREAM_EXCEPTION_MESSAGE;
   }
}




/// read -----------------------------------------------------------------------
void hxa7241_image::exr::read
(
   const char  i_exrLibraryPathName[],
   const char  i_filePathName[],
   const dword i_orderingFlags,
   dword&      o_width,
   dword&      o_height,
   float*      o_pPrimaries8,
   float&      o_scalingToGetCdm2,
   float*&     o_pTriples
)
{
   loadLibraries( i_exrLibraryPathName );

   ImfInputFile* pExrInFile = 0;

   try
   {
      // open file
      pExrInFile = ::ImfOpenInputFile( i_filePathName );
      if( !pExrInFile )
      {
         //const char* ::ImfErrorMessage();
         throw OPEN_FILE_EXCEPTION_MESSAGE;
      }

      // read header
      dword width          = 0;
      dword height         = 0;
      float pPrimaries8[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
      float scalingToGetCdm2 = 0.0f;
      bool  isLowTop       = true;
      readHeader( pExrInFile, width, height, isLowTop,
         pPrimaries8, scalingToGetCdm2 );

      // allocate storage
      std::auto_ptr<float> pTriples( new float[ width * height * 3 ] );

      // read pixels
      readPixels( pExrInFile, width, height, isLowTop, i_orderingFlags,
         pTriples.get() );

      // close file
      ::ImfCloseInputFile( pExrInFile );

      freeLibraries();

      // set outputs (now that no exceptions can happen)
      o_width  = width;
      o_height = height;
      for( int i = 8;  i-- > 0;  o_pPrimaries8[i] = pPrimaries8[i] );
      o_scalingToGetCdm2 = scalingToGetCdm2;
      o_pTriples         = pTriples.release();
   }
   catch( ... )
   {
      ::ImfCloseInputFile( pExrInFile );

      freeLibraries();

      throw;
   }
}


namespace
{

void readHeader
(
   const ImfInputFile* pExrInFile,
   dword&              width,
   dword&              height,
   bool&               isLowTop,
   float*              pPrimaries8,
   float&              scalingToGetCdm2
)
{
   // get header
   const ImfHeader* pExrHeader = ::ImfInputHeader( pExrInFile );
   if( !pExrHeader )
   {
      //const char* ::ImfErrorMessage();
      throw OPEN_FILE_EXCEPTION_MESSAGE;
   }

   // get dimensions
   {
      int xMin;
      int xMax;
      int yMin;
      int yMax;
      ::ImfHeaderDataWindow( pExrHeader, &xMin, &yMin, &xMax, &yMax );
      width  = xMax - xMin + 1;
      height = yMax - yMin + 1;

      width  = (width  >= 0) ? width  : 0;
      height = (height >= 0) ? height : 0;
   }

   // get some attributes
   {
      // row order
      isLowTop = ::ImfHeaderLineOrder( pExrHeader ) == IMF_INCREASING_Y;

      // pixel value scaling
      // whiteLuminance
      // cd/m^2 of white
      // and 'white' is rgb(1,1,1)
      // so, equals scalingToGetCdm2
      scalingToGetCdm2 = 0.0f;
      if( 0 == ::ImfHeaderFloatAttribute(
         pExrHeader, "whiteLuminance", &scalingToGetCdm2 ) )
      {
         scalingToGetCdm2 = 0.0f;
      }

      // color space
      // (not possible)
      {
//          if( 0 == ImfHeader__Attribute(
//             pExrHeader, "chromaticities",  );
         {
            for( dword i = 8;  i-- > 0; )
            {
               pPrimaries8[i] = 0.0f;
            }
         }
      }
   }
}


void readPixels
(
   ImfInputFile* pExrInFile,
   const dword   width,
   const dword   height,
   const bool    isLowTop,
   const dword   orderingFlags,
   float*        pTriples
)
{
   // allocate buffer
   std::vector<ImfRgba> exrLine( width );

   // step thru rows
   for( dword y = 0;  y < height;  ++y )
   {
      // read into line buffer (subtracting from pointer? wtf???)
      ::ImfInputSetFrameBuffer(
         pExrInFile, &(exrLine[0]) - (y * width), 1, width );
      ::ImfInputReadPixels( pExrInFile, y, y );

      const dword row = ((orderingFlags & exr::IS_LOW_TOP) != 0) ^ isLowTop ?
         height - y - 1 : y;
      float* pPixel = pTriples + (row * width * 3);

      // step thru pixels
      for( dword x = 0;  x < width;  ++x )
      {
         ImfHalf rgb[3] = { exrLine[x].r, exrLine[x].g, exrLine[x].b };

         // convert channels
         for( dword c = 0;  c < 3;  ++c )
         {
            const dword i = (orderingFlags & exr::IS_BGR) ? 2 - c : c;
            pPixel[x * 3 + c] = ::ImfHalfToFloat( rgb[ i ] );
            //*(pPixel++) = ::ImfHalfToFloat( rgb[ i ] );
         }
      }
   }
}

}




/// write ----------------------------------------------------------------------
void hxa7241_image::exr::write
(
   const char         i_exrLibraryPathName[],
   const dword        i_width,
   const dword        i_height,
   const float* const ,//i_pPrimaries8,
   const float        i_scalingToGetCdm2,
   const dword        i_orderingFlags,
   const float* const i_pTriples,
   const char         i_filePathName[]
)
{
   loadLibraries( i_exrLibraryPathName );

   ImfHeader*     pExrHeader  = 0;
   ImfOutputFile* pExrOutFile = 0;

   try
   {
      pExrHeader = ::ImfNewHeader();
      if( !pExrHeader )
      {
         //const char* ::ImfErrorMessage();
         throw NEW_HEADER_EXCEPTION_MESSAGE;
      }

      // copy header
      {
         ::ImfHeaderSetDisplayWindow( pExrHeader, 0, 0, i_width - 1,
            i_height - 1 );
         ::ImfHeaderSetDataWindow( pExrHeader, 0, 0, i_width - 1,
            i_height - 1 );
         ::ImfHeaderSetScreenWindowWidth( pExrHeader,
            static_cast<float>(i_width) );
         ::ImfHeaderSetLineOrder( pExrHeader, IMF_INCREASING_Y );
         ::ImfHeaderSetCompression( pExrHeader, IMF_PIZ_COMPRESSION );
            //IMF_ZIP_COMPRESSION

         // pixel value scaling
         if( 0.0f != i_scalingToGetCdm2 )
         {
            ::ImfHeaderSetFloatAttribute( pExrHeader, "whiteLuminance",
               i_scalingToGetCdm2 );
         }

         // primaries (not possible)
         //if( i_pPrimaries8 )
         //{
         //   ::ImfHeaderSet__Attribute( pExrHeader, "chromaticities",
         //      i_pPrimaries8 );
         //}

         ::ImfHeaderSetStringAttribute( pExrHeader, "software", HXA7241_URI );
      }

      // open file
      pExrOutFile = ::ImfOpenOutputFile( i_filePathName, pExrHeader,
         IMF_WRITE_RGB );
      if( !pExrOutFile )
      {
         //const char* ::ImfErrorMessage();
         throw OPEN_FILE_EXCEPTION_MESSAGE;
      }

      // write pixels
      {
         // allocate line buffer
         std::vector<ImfRgba> exrLine( i_width );

         // step thru rows
         for( dword y = 0;  y < i_height;  ++y )
         {
            // set row pointer
            const dword row = ((i_orderingFlags & exr::IS_LOW_TOP) == 0) ?
               i_height - y - 1 : y;
            const float* pPixel = i_pTriples + (row * i_width * 3);

            // step thru pixels
            for( dword x = 0;  x < i_width;  ++x )
            {
               ImfHalf rgb[3];

               // convert channels
               for( dword c = 0;  c < 3;  ++c )
               {
                  const dword i = (i_orderingFlags & exr::IS_BGR) ? 2 - c : c;
                  ::ImfFloatToHalf( pPixel[x * 3 + c], &(rgb[i]) );
               }

               // copy to line buffer
               exrLine[x].r = rgb[0];
               exrLine[x].g = rgb[1];
               exrLine[x].b = rgb[2];
               ::ImfFloatToHalf( 1.0f, &(exrLine[x].a) );
            }

            // write line buffer (subtracting from pointer? wtf???)
            ::ImfOutputSetFrameBuffer( pExrOutFile, &(exrLine[0]) -
               (y * i_width), 1, i_width );
            ::ImfOutputWritePixels( pExrOutFile, 1 );
         }
      }

      ::ImfCloseOutputFile( pExrOutFile );
      ::ImfDeleteHeader( pExrHeader );

      freeLibraries();
   }
   catch( ... )
   {
      ::ImfCloseOutputFile( pExrOutFile );
      ::ImfDeleteHeader( pExrHeader );

      freeLibraries();

      throw;
   }
}




/// ----------------------------------------------------------------------------
namespace
{

void loadLibraries
(
   const char exrLibraryPathName[]
)
{
   // use default name if nothing given
   const char* pExrLibraryPathName = exrLibraryPathName;
   if( (0 == exrLibraryPathName) || (0 == exrLibraryPathName[0]) )
   {
      pExrLibraryPathName = LIB_PATHNAME_DEFAULT;
   }

#ifdef _PLATFORM_LINUX
   // some auxiliary ILM libs must be explicitly loaded too...
   try
   {
      // analyse lib pathname
      std::string path;
      std::string version;
      {
         // assume after-last-path-sep is name
         // extract: path, libname, libname version
         const std::string pathname( pExrLibraryPathName );

         // extract path and name
         // (path contains end separator, or is empty)
         size_t namePos = pathname.rfind( '/' );
         namePos = (std::string::npos != namePos) ? namePos + 1 : 0;
         path = pathname.substr( 0, namePos );
         const std::string name( pathname.substr( namePos ) );

         // extract version
         size_t versionPos = name.rfind( ".so" );
         versionPos += (std::string::npos != versionPos) ? 3 : 0;
         version = name.substr( versionPos );
      }

      // load auxiliary library set
      // (load in this order, store handles in reverse order)
      static const char* SUPPORT_LIBS[] = {
         "libIex.so", "libHalf.so" };//, "libImath.so" };
      for( dword i = 0;  i < 2;  ++i )
      {
         const std::string libPathName( path + SUPPORT_LIBS[i] + version );
         dynamiclink::loadLibrary( libPathName.c_str(), libraries_g[3 - i] );
      }
   }
   catch( ... )
   {
      freeLibraries();
      throw;
   }
#endif

   dynamiclink::loadLibrary( pExrLibraryPathName, libraries_g[0] );
}


void freeLibraries()
{
   // free in array forwards order
   for( udword i = 0;  i < sizeof(libraries_g)/sizeof(libraries_g[0]);  ++i )
   {
      dynamiclink::freeLibrary( libraries_g[i] );
   }
}

}




/// exr dynamic library forwarders ---------------------------------------------
ImfInputFile* ImfOpenInputFile
(
   const char name[]
)
{
   typedef ImfInputFile* (*PFunction)(
      const char[]
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfOpenInputFile" ) );

   return (function)(
      name
   );
}


int ImfCloseInputFile
(
   ImfInputFile* pIn
)
{
   typedef int (*PFunction)(
      ImfInputFile*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfCloseInputFile" ) );

   return (function)(
      pIn
   );
}


const ImfHeader* ImfInputHeader
(
   const ImfInputFile* pIn
)
{
   typedef const ImfHeader* (*PFunction)(
      const ImfInputFile*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfInputHeader" ) );

   return (function)(
      pIn
   );
}


int ImfHeaderLineOrder
(
   const ImfHeader* pHdr
)
{
   typedef int (*PFunction)(
      const ImfHeader*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderLineOrder" ) );

   return (function)(
      pHdr
   );
}


int ImfHeaderFloatAttribute
(
   const ImfHeader* pHdr,
   const char       name[],
   float*           pValue
)
{
   typedef int (*PFunction)(
      const ImfHeader*,
      const char[],
      float*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderFloatAttribute" ) );

   return (function)(
      pHdr,
      name,
      pValue
   );
}


void ImfHeaderDataWindow
(
   const ImfHeader* pHdr,
   int*             xMin,
   int*             yMin,
   int*             xMax,
   int*             yMax
)
{
   typedef void (*PFunction)(
      const ImfHeader*,
      int*,
      int*,
      int*,
      int*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderDataWindow" ) );

   (function)(
      pHdr,
      xMin,
      yMin,
      xMax,
      yMax
   );
}


int ImfInputSetFrameBuffer
(
   ImfInputFile* pIn,
   ImfRgba*      pBase,
   size_t        xStride,
   size_t        yStride
)
{
   typedef int (*PFunction)(
      ImfInputFile*,
      ImfRgba*,
      size_t,
      size_t
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfInputSetFrameBuffer" ) );

   return (function)(
      pIn,
      pBase,
      xStride,
      yStride
   );
}


int ImfInputReadPixels
(
   ImfInputFile* pIn,
   int           scanLine1,
   int           scanLine2
)
{
   typedef int (*PFunction)(
      ImfInputFile*,
      int,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfInputReadPixels" ) );

   return (function)(
      pIn,
      scanLine1,
      scanLine2
   );
}


float ImfHalfToFloat
(
   ImfHalf h
)
{
   typedef float (*PFunction)(
      ImfHalf
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHalfToFloat" ) );

   return (function)(
      h
   );
}


const char* ImfErrorMessage()
{
   typedef const char* (*PFunction)();

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfErrorMessage" ) );

   return (function)();
}


ImfHeader* ImfNewHeader()
{
   typedef ImfHeader* (*PFunction)();

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfNewHeader" ) );

   return (function)();
}


void ImfHeaderSetDisplayWindow
(
   ImfHeader* hdr,
   int        xMin,
   int        yMin,
   int        xMax,
   int        yMax
)
{
   typedef void (*PFunction)(
      ImfHeader*,
      int,
      int,
      int,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderSetDisplayWindow" ) );

   (function)(
      hdr,
      xMin,
      yMin,
      xMax,
      yMax
   );
}


void ImfHeaderSetDataWindow
(
   ImfHeader* hdr,
   int        xMin,
   int        yMin,
   int        xMax,
   int        yMax
)
{
   typedef void (*PFunction)(
      ImfHeader*,
      int,
      int,
      int,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderSetDataWindow" ) );

   (function)(
      hdr,
      xMin,
      yMin,
      xMax,
      yMax
   );
}


void ImfHeaderSetScreenWindowWidth
(
   ImfHeader* hdr,
   float      width
)
{
   typedef void (*PFunction)(
      ImfHeader*,
      float
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0],
      "ImfHeaderSetScreenWindowWidth" ) );

   (function)(
      hdr,
      width
   );
}


void ImfHeaderSetLineOrder
(
   ImfHeader* hdr,
   int        lineOrder
)
{
   typedef void (*PFunction)(
      ImfHeader*,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderSetLineOrder" ) );

   (function)(
      hdr,
      lineOrder
   );
}


void ImfHeaderSetCompression
(
   ImfHeader* hdr,
   int        compression
)
{
   typedef void (*PFunction)(
      ImfHeader*,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfHeaderSetCompression" ) );

   (function)(
      hdr,
      compression
   );
}


int ImfHeaderSetFloatAttribute
(
   ImfHeader* hdr,
   const char name[],
   float      value
)
{
   typedef int (*PFunction)(
      ImfHeader*,
      const char[],
      float
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0],
      "ImfHeaderSetFloatAttribute" ) );

   return (function)(
      hdr,
      name,
      value
   );
}


int ImfHeaderSetStringAttribute
(
   ImfHeader* hdr,
   const char name[],
   const char value[]
)
{
   typedef int (*PFunction)(
      ImfHeader*,
      const char[],
      const char[]
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0],
      "ImfHeaderSetStringAttribute" ) );

   return (function)(
      hdr,
      name,
      value
   );
}


ImfOutputFile* ImfOpenOutputFile
(
   const char       name[],
   const ImfHeader* hdr,
   int              channels
)
{
   typedef ImfOutputFile* (*PFunction)(
      const char[],
      const ImfHeader*,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfOpenOutputFile" ) );

   return (function)(
      name,
      hdr,
      channels
   );
}


void ImfFloatToHalf
(
   float    f,
   ImfHalf* h
)
{
   typedef void (*PFunction)(
      float,
      ImfHalf*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfFloatToHalf" ) );

   (function)(
      f,
      h
   );
}


int ImfOutputSetFrameBuffer
(
   ImfOutputFile* out,
   const ImfRgba* base,
   size_t         xStride,
   size_t         yStride
)
{
   typedef int (*PFunction)(
      ImfOutputFile*,
      const ImfRgba*,
      size_t,
      size_t
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfOutputSetFrameBuffer" ) );

   return (function)(
      out,
      base,
      xStride,
      yStride
   );
}


int ImfOutputWritePixels
(
   ImfOutputFile* out,
   int            numScanLines
)
{
   typedef int (*PFunction)(
      ImfOutputFile*,
      int
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfOutputWritePixels" ) );

   return (function)(
      out,
      numScanLines
   );
}


int ImfCloseOutputFile
(
   ImfOutputFile* out
)
{
   typedef int (*PFunction)(
      ImfOutputFile*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfCloseOutputFile" ) );

   return (function)(
      out
   );
}


void ImfDeleteHeader
(
   ImfHeader* hdr
)
{
   typedef void (*PFunction)(
      ImfHeader*
   );

   PFunction function = reinterpret_cast<PFunction>(
      dynamiclink::getFunction( libraries_g[0], "ImfDeleteHeader" ) );

   (function)(
      hdr
   );
}








/// test -----------------------------------------------------------------------
#ifdef TESTING


#include <fstream>
#include <sstream>

#include "png.hpp"


namespace hxa7241_image
{
namespace exr
{
   using namespace hxa7241;


bool test_exr
(
   std::ostream* pOut,
   const bool    isVerbose,
   const dword   //seed
)
{
   bool isOk = true;

   if( pOut ) *pOut << "[ test_exr ]\n\n";


   static const char LIB_FILE[] = "C:\\h\\dev\\projects\\p3tonemapper\\temp\\exe\\IlmImf_dll.dll";//"temp\\IlmImf_dll.dll";


   // manual inspection test
   {
      dword  width;
      dword  height;
      float  pPrimaries8[8];
      float  scalingToGetCdm2;
      float* pTriples = 0;
      try
      {
         hxa7241_image::exr::read(
            LIB_FILE,
            "zzztest.exr",
            0,
            width,
            height,
            pPrimaries8,
            scalingToGetCdm2,
            pTriples
         );

         if( pOut && isVerbose ) *pOut << "width " << width << "\n";
         if( pOut && isVerbose ) *pOut << "height " << height << "\n";

         if( pOut && isVerbose ) *pOut << "primaries ";
         for( dword i = 0;  i < 8;  ++i )
         {
            if( pOut && isVerbose ) *pOut << pPrimaries8[i] << " ";
         }
         if( pOut && isVerbose ) *pOut << "\n";

         if( pOut && isVerbose ) *pOut << "scalingToGetCdm2 " <<
            scalingToGetCdm2 << "\n";
         if( pOut && isVerbose ) *pOut << "pTriples " << pTriples << "\n";

         float min[3] = { FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
         float max[3] = { FLOAT_MIN_NEG, FLOAT_MIN_NEG, FLOAT_MIN_NEG };
         for( dword i = 0;  i < (width * height);  ++i )
         {
            for( dword c = 0;  c < 3;  ++c )
            {
               const float channel = pTriples[ i * 3 + c ];
               min[c] = (channel < min[c]) ? channel : min[c];
               max[c] = (channel > max[c]) ? channel : max[c];
            }
         }
         if( pOut && isVerbose ) *pOut << "min " << min[0] << " " << min[1] <<
            " " << min[2] << "\n";
         if( pOut && isVerbose ) *pOut << "max " << max[0] << " " << max[1] <<
            " " << max[2] << "\n";

         if( pOut && isVerbose ) *pOut << "\n";


         {
            if( pOut ) pOut->flush();

            std::vector<ubyte> pixels( width * height * 3 );
            const float scale  = 1.0f;
            const float offset = 0.0f;
            for( dword i = 0;  i < (width * height);  ++i )
            {
               for( dword c = 3;  c-- > 0; )
               {
                  const float ch   = (pTriples[(i * 3) + c] * scale) + offset;
                  const float ch01 = (ch < 0.0f) ? 0.0f : (
                     (ch >= 1.0f) ? FLOAT_ALMOST_ONE : ch);

                  pixels[(i * 3) + c] = ubyte( dword(ch01 * 256.0f) );
               }
            }

            // write image
            std::ofstream outf( "zzztest.png", std::ofstream::binary );

            hxa7241_image::png::write(
               "",
               width, height,
               pPrimaries8, 0.0f, false, 0,
               &(pixels[0]), outf );

            if( pOut && isVerbose ) *pOut << "wrote image to zzztest.png" <<
               "\n";
         }


      }
      catch( const std::exception& e )
      {
         if( pOut && isVerbose ) *pOut << "exception: " << e.what() << "\n";
      }
      catch( const char pMessage[] )
      {
         if( pOut && isVerbose ) *pOut << "exception: " << pMessage << "\n";
      }
      catch( ... )
      {
         if( pOut && isVerbose ) *pOut << "unannotated exception\n";
      }

      delete[] pTriples;

      if( pOut && isVerbose ) *pOut << "\n";
   }

   if( pOut && isVerbose ) *pOut << "needs manual inspection\n";
   if( pOut && isVerbose ) *pOut << "\n";


   if( pOut ) *pOut << (isOk ? "--- successfully" : "*** failurefully") <<
      " completed " << "\n\n\n";

   if( pOut ) pOut->flush();


   return isOk;
}


}//namespace
}//namespace


#endif//TESTING
