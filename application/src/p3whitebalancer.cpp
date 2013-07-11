/*------------------------------------------------------------------------------

   Perceptuum3 rendering components
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


/*
   file contents:

   * includes
   * usings
   * release (conditional compilation)
      * internal (anonymous-namespace) declarations
         * string constants
         * functions
      * main
      * internal function definitions
   * testing (conditional compilation)
      * internal (anonymous-namespace) declarations
         * string constants
         * functions
      * main
      * internal function definitions
*/


#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <exception>

#include "Primitives.hpp"

#include "ImageAdopter.hpp"
#include "ImageFormatter.hpp"

#include "p3wbWhiteBalancer-v12.h"




using namespace hxa7241;
using std::string;
using std::vector;
using hxa7241_image::ImageFormatter;
using hxa7241_image::ImageAdopter;




#define NAME "P3 WhiteBalancer v1.2"
#define TITLE "----------------------------------------------------------------------\n"\
"  "NAME"\n"\
"\n"\
"  Copyright (c) 2007, Harrison Ainsworth / HXA7241.\n"\
"  http://www.hxa7241.org/whitebalancer/\n"\
"\n"\
"  2007-12-24\n"\
"----------------------------------------------------------------------\n"




#ifndef TESTING


namespace
{

/// constants ------------------------------------------------------------------
const char HELP_MESSAGE[] =
"\n"
TITLE
"\n"
"P3 WhiteBalancer makes the colors in an image look more like they did in the\n"
"scene where the image was created.\n"
"\n"
"usage:\n"
"  p3whitebalancer {--help|-?}\n"
"  p3whitebalancer [options] [-x:optionsFilePathName] [-z] imageFilePathName\n"
"\n"
"options (with defaults shown):\n"
"  image formatting libraries:\n"
"   -lp:<string>    libpng pathname\n"
"   -le:<string>    openexr pathname\n"
"  image metadata:\n"
"   -ic:<6 floats>  colorspace: 0.64_0.33_0.30_0.60_0.15_0.06\n"
"   -iw:<2 floats>  whitepoint: 0.333_0.333\n"
"   -ig:<float>     gamma transform (encode): 0.45\n"
"   -ii:<3 floats>  original illuminant (rgb): estimated from image\n"
"  mapping options:\n"
"   -ms:<float>     strength (0-1): 0.8\n"
"  output file name:\n"
"   -on:<string>    output file path name (no ext): inputFilePathName_p3wb\n"
"\n"
"image file name must be last, and must end in '.ppm', '.png',\n"
".exr', '.hdr', '.pic', '.rad', or '.rgbe'.\n"
"\n"
"optionsFilePathName defaults to 'p3whitebalancer-opt.txt'\n"
"\n"
"-z switches on some feedback\n"
"\n"
"example:\n"
"  p3whitebalancer somerendering.exr\n"
"  p3whitebalancer -ms:0.6 -on:resultimage somerendering.png\n"
"\n";
const char BANNER_MESSAGE[] =
"  "NAME"  :  http://www.hxa7241.org/\n";

const char SWITCH_HELP1[] = "-?";
const char SWITCH_HELP2[] = "--help";

const char OPTIONS_FILE_NAME_DEFAULT[] = "p3whitebalancer-opt.txt";

const char EXCEPTION_PREFIX[]     = "*** execution failed:  ";
const char EXCEPTION_ABSTRACT[]   = "no annotation for cause";

const char LIB_VERSION_UNSUPPORTED[] =
   "p3whitebalancer library version incompatible";

const char WARNING_WRONG_SWITCH[] = "unrecognized option";

const char BAD_OPTION_NAME[]       = "bad option name";
const char BAD_OPTION_VALUE[]      = "bad option value";
const char BAD_GAMMA_OPTION[]      = "bad gamma option value";
const char BAD_STRENGTH_OPTION[]   = "bad strength option value";
const char BAD_COLORSPACE_OPTION[] = "bad colorspace option value";
const char BAD_WHITEPOINT_OPTION[] = "bad whitepoint option value";


/// support declarations -------------------------------------------------------
void whiteBalance
(
   int   argc,
   char* argv[]
);

void getInitialOptions
(
   const int                argc,
   char*const               argv[],
   bool&                    isFeedback,
   string&                  inImagePathname,
   std::map<string,string>& options
);

float getOptionF
(
   const std::map<string,string>& options,
   const char                     name[],
   float                          dfault
);

vector<float> getOptionV
(
   const std::map<string,string>& options,
   const char                     name[],
   const dword                    length
);

string getOptionS
(
   const std::map<string,string>& options,
   const char                     name[]
);

vector<float> parseFps
(
   const string&  group,
   const dword    length
);

float parseFp
(
   const string& s
);

void tokenize
(
   const string&   str,
   const char      separator,
   vector<string>& tokens
);

void makeOutPathname
(
   const string& inPathname,
   const string& optionOutPathname,
   string&       outPathname
);

void checkGamma
(
   float
);

void checkPrimaries
(
   const vector<float>& colorspace,
   const vector<float>& whitepoint
);

void checkStrength
(
   float
);

void displayImageData
(
   const ImageAdopter& image
);

}




/// entry point ////////////////////////////////////////////////////////////////
int main
(
   int   argc,
   char* argv[]
)
{
   int returnValue = EXIT_FAILURE;

   // catch everything
   try
   {
      // check for help request
      if( (argc <= 1) || (std::string(argv[1]) == SWITCH_HELP1) ||
         (std::string(argv[1]) == SWITCH_HELP2) )
      {
         // output help
         std::cout << HELP_MESSAGE;

         returnValue = EXIT_SUCCESS;
      }
      // execute
      else
      {
         std::cout << BANNER_MESSAGE;

         whiteBalance( argc, argv );

         returnValue = EXIT_SUCCESS;
      }
   }
   // print exception message
   catch( const std::exception& e )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << e.what() << '\n';
   }
   catch( const char*const pExceptionString )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << pExceptionString << '\n';
   }
   catch( const std::string exceptionString )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << exceptionString << '\n';
   }
   catch( ... )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << EXCEPTION_ABSTRACT << '\n';
   }

   try
   {
      std::cout.flush();
   }
   catch( ... )
   {
      // suppress exceptions
   }

   return returnValue;
}




/// other functions ------------------------------------------------------------
namespace
{

void whiteBalance
(
   int   argc,
   char* argv[]
)
{
   // get input image pathname, and basic options
   bool                    isFeedback;
   string                  inImagePathname;
   std::map<string,string> options;
   getInitialOptions( argc, argv, isFeedback, inImagePathname, options );

   // make formatter
   const ImageFormatter formatter( getOptionS( options, "le" ).c_str(),
      getOptionS( options, "lp" ).c_str() );

   // get gamma option
   const float enGamma = getOptionF( options, "ig", 0.0f );
   checkGamma( enGamma );
   const float deGamma = (0.0f != enGamma) ? (1.0f / enGamma) : 0.0f;

   // read image
   ImageAdopter image;
   clock_t      timeRead = 0;
   {
      const clock_t t0 = ::clock();
      formatter.readImage( inImagePathname.c_str(), deGamma, image );
      timeRead = ::clock() - t0;
      if( isFeedback )
      {
         displayImageData( image );
      }
   }

   // call balancer
   clock_t timeBalance = 0;
   {
      // get relevant options
      const vector<float> colorspace( getOptionV( options, "ic", 6 ) );
      const vector<float> whitepoint( getOptionV( options, "iw", 2 ) );
      const vector<float> illuminant( getOptionV( options, "ii", 3 ) );
      const float         strength = getOptionF( options, "ms", -1.0f );
      checkPrimaries( colorspace, whitepoint );
      checkStrength( strength );

      if( isFeedback )
      {
         std::cout << "\n" << "library: " << ::p3wbGetName() << " version " <<
            ::p3wbGetVersion() << "\n" << ::p3wbGetCopyright() << "\n";
      }
      if( !::p3wbIsVersionSupported( p3wb12_VERSION ) )
      {
         throw LIB_VERSION_UNSUPPORTED;
      }

      char pMessage128[128] = "\0";

      const clock_t t0 = ::clock();
      const bool isOk = 0 != ::p3wbWhiteBalance2(
         (!colorspace.empty() ? &(colorspace[0]) : image.getColorspace()),
         (!whitepoint.empty() ? &(whitepoint[0]) : image.getWhitepoint()),
         (!illuminant.empty() ? &(illuminant[0]) : 0),
         p3wb11_GW,
         strength,
         image.getWidth(),
         image.getHeight(),
         0,
         sizeof(*image.getPixels()) * 3,
         image.getPixels(),
         image.getPixels(),
         pMessage128 );
      timeBalance = ::clock() - t0;
      if( !isOk )
      {
         throw string( pMessage128 );
      }
   }

   // write image
   clock_t timeWrite = 0;
   {
      // get relevant option
      string outImagePathname( getOptionS( options, "on" ) );
      makeOutPathname( inImagePathname, outImagePathname, outImagePathname );

      const clock_t t0 = ::clock();
      formatter.writeImage( outImagePathname.c_str(), enGamma, image );
      timeWrite = ::clock() - t0;
   }

   // display timing
   if( isFeedback )
   {
      const float freqency = static_cast<float>(CLOCKS_PER_SEC);
      std::cout << "\nread time:    " << (static_cast<float>(timeRead) /
         freqency) << "\n";
      std::cout << "balance time: " << (static_cast<float>(timeBalance) /
         freqency) << "\n";
      std::cout << "write time:   " << (static_cast<float>(timeWrite) /
         freqency) << "\n";
   }
}


void getInitialOptions
(
   const int                argc,
   char*const               argv[],
   bool&                    isFeedback,
   string&                  inImagePathname,
   std::map<string,string>& options
)
{
   // read input file pathname from command line last arg
   inImagePathname = argv[argc - 1];

   // read options file pathname from command line
   string optionsFilePathname( OPTIONS_FILE_NAME_DEFAULT );
   for( int i = argc - 1;  i-- > 1; )
   {
      if( (('-' == argv[i][0]) || ('/' == argv[i][0])) &&
         ('x' == argv[i][1]) &&
         ((':' == argv[i][2]) || ('=' == argv[i][2])) )
      {
         optionsFilePathname = argv[i] + 3;
         if( optionsFilePathname.empty() )
         {
            throw BAD_OPTION_VALUE;
         }
      }
   }

   // read feedback option
   isFeedback = false;
   for( int i = argc - 1;  i-- > 1; )
   {
      if( (('-' == argv[i][0]) || ('/' == argv[i][0])) && ('z' == argv[i][1]) )
      {
         isFeedback = true;
      }
   }

   // set options map
   {
      // extract options tokens
      vector<string> tokenSets[2];
      {
         // from options file
         std::ifstream optionsFile( optionsFilePathname.c_str() );
         for( string token;  optionsFile >> token; )
         {
            tokenSets[0].push_back( token );
         }

         // from command line (except first and last)
         for( dword i = 1;  i < (argc - 1);  ++i )
         {
            tokenSets[1].push_back( string(argv[i]) );
         }
      }

      // add first token set, then overwrite with second
      for( dword i = 0;  i < 2;  ++i )
      {
         for( udword j = 0;  j < tokenSets[i].size();  ++j )
         {
            const string& token = tokenSets[i][j];

            if( token.length() >= 5 )
            {
               if( (('-' != token[0]) && ('/' != token[0])) ||
                   ((':' != token[3]) && ('=' != token[3])) )
               {
                  throw BAD_OPTION_NAME;
               }

               // overwriting insert (breaking token into key and value)
               options[ token.substr( 1, 2 ) ] = token.substr( 4 );
            }
         }
      }
   }

//   // print things
//   std::cout << "\ninImagePathname     = " << inImagePathname;
//   std::cout << "\noptionsFilePathname = " << optionsFilePathname;
//   std::cout << "\n";
//
//   // print all tokens
//   std::cout << "\ntokens:\n";
//   std::map<string,string>::const_iterator i( options.begin() );
//   for( ;  i != options.end();  ++i )
//   {
//      std::cout << i->first << " " << i->second << "\n";
//   }
}


float getOptionF
(
   const std::map<string,string>& options,
   const char                     name[],
   const float                    dfault
)
{
   const string valueString( getOptionS( options, name ) );

   return !valueString.empty() ? parseFp( valueString ) : dfault;
}


vector<float> getOptionV
(
   const std::map<string,string>& options,
   const char                     name[],
   const dword                    length
)
{
   const string valueString( getOptionS( options, name ) );

   // default to empty vector if option not found
   return !valueString.empty() ? parseFps( valueString, length ) :
      vector<float>();
}


string getOptionS
(
   const std::map<string,string>& options,
   const char                     name[]
)
{
   // default to empty string if option not found
   string value;

   const std::map<string,string>::const_iterator i( options.find(string(name)));
   if( options.end() != i )
   {
      if( i->second.empty() )
      {
         throw BAD_OPTION_VALUE;
      }

      value = i->second;
   }

   return value;
}


vector<float> parseFps
(
   const string& group,
   const dword   length
)
{
   // separate values from group
   vector<string> fpStrings;
   tokenize( group, '_', fpStrings );
   if( static_cast<udword>(length) != fpStrings.size() )
   {
      throw BAD_OPTION_VALUE;
   }

   // convert values
   vector<float> fps( length );
   for( dword i = length;  i-- > 0; )
   {
      fps[i] = parseFp( fpStrings[i] );
   }

   return fps;
}


float parseFp
(
   const string& s
)
{
   const char* pNum = s.c_str();
   char*       pEnd = 0;
   const float fp   = static_cast<float>( ::strtod( pNum, &pEnd ) );

   // if stopped converting before end, or zero-length string
   if( (0 != *pEnd) | (pNum == pEnd) )
   {
      throw BAD_OPTION_VALUE;
   }

   return fp;
}


void tokenize
(
   const string&   str,
   const char      separator,
   vector<string>& tokens
)
{
   size_t posToken   = 0;
   bool   isSepFound = false;
   do
   {
      const size_t posSep = str.find( separator, posToken );
      isSepFound          = string::npos != posSep;

      tokens.push_back( str.substr( posToken,
         isSepFound ? posSep - posToken : string::npos ) );
      posToken = posSep + 1;
   }
   while( isSepFound );
}


//void tokenize
//(
//   const string&   str,
//   vector<string>& tokens
//)
//{
//   std::stringstream ss( str );
//
//   for( string token;  ss >> token; )
//   {
//      tokens.push_back( token );
//   }
//}


void makeOutPathname
(
   const string& inPathname,
   const string& optionOutPathname,
   string&       outPathname
)
{
   const size_t extPos = inPathname.rfind( '.' );

   // use option out pathname
   if( !optionOutPathname.empty() )
   {
      outPathname = optionOutPathname;
   }
   // use in pathname with replaced ext
   else
   {
      outPathname = inPathname.substr( 0, extPos ) + "_p3wb";
   }

   // append in pathname ext
   outPathname += inPathname.substr( extPos );
}


void checkGamma
(
   const float gamma
)
{
   if( (0.0f > gamma) || (1.0f < gamma) )
   {
      throw BAD_GAMMA_OPTION;
   }
}


void checkPrimaries
(
   const vector<float>& colorspace,
   const vector<float>& whitepoint
)
{
   const vector<float>* pps[] = { &colorspace, &whitepoint };
   const char* ems[] = { BAD_COLORSPACE_OPTION, BAD_WHITEPOINT_OPTION };

   for( int p = 2;  p-- > 0; )
   {
      for( int i = pps[p]->size();  i-- > 0; )
      {
         if( (0.0f >= (*pps[p])[i]) || (1.0f <= (*pps[p])[i]) )
         {
            throw ems[p];
         }
      }
   }
}


void checkStrength
(
   const float strength
)
{
   if( ((0.0f > strength) || (1.0f < strength)) && (-1.0f != strength) )
   {
      throw BAD_STRENGTH_OPTION;
   }
}


void displayImageData
(
   const ImageAdopter& image
)
{
   std::cout << "\n" << "image data:" << "\n";

   std::cout << "  width:      " << image.getWidth() << "\n";
   std::cout << "  height:     " << image.getHeight() << "\n";

   if( 0.0f != image.getScaling() )
   {
      std::cout << "  scaling:    " << image.getScaling() << "\n";
   }

   if( image.getColorspace() )
   {
      std::cout << "  colorspace: ";
      for( int i = 0;  i < 6;  ++i )
      {
         std::cout << image.getColorspace()[i] << " ";
      }
      std::cout << "\n";
   }

   if( image.getWhitepoint() )
   {
      std::cout << "  whitepoint: " << image.getWhitepoint()[0] << " " <<
         image.getWhitepoint()[1] << "\n";
   }

   if( 0.0f != image.getGamma() )
   {
      std::cout << "  gamma:      " << image.getGamma() << "\n";
   }

   if( 0 != image.getQuantMax() )
   {
      std::cout << "  quantMax:   " << image.getQuantMax() << "\n";
   }
}

}








#else//not TESTING


namespace
{

/// constants ------------------------------------------------------------------
const char HELP_MESSAGE[] =
"\n"
TITLE
"\n"
"*** TESTING BUILD ***\n"
"\n"
"usage:\n"
"   p3whitebalancer [-t...] [-o...] [-s...]\n"
"\n"
"switches:\n"
"   -t<int>         which test: 1 to 5 for lib, -1 to -5 for app, 0 for all\n"
"   -o<0 | 1 | 2>   set output level: 0 = none, 1 = summaries, 2 = verbose\n"
"   -s<32bit int>   set random seed\n"
"\n";

const char SWITCH_HELP1[] = "-?";
const char SWITCH_HELP2[] = "--help";

const char EXCEPTION_PREFIX[]     = "*** execution failed:  ";
const char EXCEPTION_ABSTRACT[]   = "no annotation for cause";


/// support declarations -------------------------------------------------------

bool test
(
   int   argc,
   char* argv[]
);

void readParameters
(
   const int  argc,
   char*const argv[],
   dword&     whichTest,
   dword&     outputLevel,
   dword&     seed
);

bool testUnits
(
   int  whichTest,
   bool isOutput,
   bool isVerbose,
   int  seed
);

}




/// entry point ////////////////////////////////////////////////////////////////
int main
(
   int   argc,
   char* argv[]
)
{
   int returnValue = EXIT_FAILURE;

   // catch everything
   try
   {
      // check for help request
      if( (argc <= 1) || (std::string(argv[1]) == SWITCH_HELP1) ||
         (std::string(argv[1]) == SWITCH_HELP2) )
      {
         // output help
         std::cout << HELP_MESSAGE;

         returnValue = EXIT_SUCCESS;
      }
      // execute
      else
      {
         returnValue = test( argc, argv ) ? EXIT_SUCCESS : EXIT_FAILURE;
      }
   }
   // print exception message
   catch( const std::exception& e )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << e.what() << '\n';
   }
   catch( const char*const pExceptionString )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << pExceptionString << '\n';
   }
   catch( const std::string exceptionString )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << exceptionString << '\n';
   }
   catch( ... )
   {
      std::cout << '\n' << EXCEPTION_PREFIX << EXCEPTION_ABSTRACT << '\n';
   }

   try
   {
      std::cout.flush();
   }
   catch( ... )
   {
      // suppress exceptions
   }

   return returnValue;
}




/// other functions ------------------------------------------------------------
namespace
{

bool test
(
   int   argc,
   char* argv[]
)
{
   // default options
   dword whichTest = 0;
   bool  isVerbose = false;
   bool  isQuiet   = true;
   dword seed      = 0;

   // read options
   {
      dword outputLevel = 0;
      readParameters( argc, argv, whichTest, outputLevel, seed );
      isVerbose = (2 == outputLevel);
      isQuiet   = (0 == outputLevel);
   }

   // run tests
   bool isOk = true;
   if( whichTest <= 0 )
   {
      isOk &= testUnits( -whichTest, !isQuiet, isVerbose, seed );
   }
   std::cout.flush();
   if( whichTest >= 0 )
   {
      isOk &= (0 != ::p3wbTestUnits( whichTest, !isQuiet, isVerbose, seed ));
   }

   // print result
   if( !isQuiet )
   {
      std::cout <<
         (isOk ? "--- successfully" : "*** failurefully") << " completed " <<
         ((0 == whichTest) ? "all unit tests" : "one unit test") << "\n\n";
   }
   else
   {
      std::cout << isOk << "\n";
   }

   std::cout.flush();

   return isOk;
}


void readParameters
(
   const int  argc,
   char*const argv[],
   dword&     whichTest,
   dword&     outputLevel,
   dword&     seed
)
{
   // read switches from args
   for( int i = argc;  i-- > 1; )
   {
      // only read if first char identifies a switch
      const char first = argv[i][0];
      if( '-' == first )
      {
         const char       key    = argv[i][1];
         const char*const pValue = argv[i] + 2;
         switch( key )
         {
            // which test
            case 't' :
            {
               // read int, defaults to 0
               whichTest = std::atoi( pValue );

               break;
            }
            // output level
            case 'o' :
            {
               // read int, defaults to 0
               outputLevel = std::atoi( pValue );

               // clamp to range
               if( outputLevel < 0 )
               {
                  outputLevel = 0;
               }
               else if( outputLevel > 2 )
               {
                  outputLevel = 2;
               }

               break;
            }
            // seed
            case 's' :
            {
               // read int, defaults to 0
               seed = std::atoi( pValue );

               break;
            }
            default  :
               ;//break;
         }
      }
   }

   // print all args
   std::cout << '\n';
   std::cout << "   argc    = " << argc << '\n';
   for( int i = 0;  i < argc;  ++i )
   {
      std::cout << "   argv[" << i << "] = " << argv[i] << '\n';
   }

// // print parameters
// std::cout << '\n';
// std::cout << "   which test   = " << whichTest << '\n';
// std::cout << "   output level = " << outputLevel << '\n';
// std::cout << "   seed         = " << seed << '\n';
}

}


/// unit test declarations
namespace hxa7241_image
{
   namespace quantizing
   {
      bool test_quantizing( std::ostream* pOut, bool isVerbose, dword seed );
   }

   namespace exr
   {
      bool test_exr ( std::ostream* pOut, bool isVerbose, dword seed );
   }

   namespace png
   {
      bool test_png ( std::ostream* pOut, bool isVerbose, dword seed );
   }

   namespace ppm
   {
      bool test_ppm ( std::ostream* pOut, bool isVerbose, dword seed );
   }

   namespace rgbe
   {
      bool test_rgbe( std::ostream* pOut, bool isVerbose, dword seed );
   }
}


namespace
{

/// unit test caller
bool (*TESTERS[])(std::ostream*, bool, dword) =
{
   &hxa7241_image::quantizing::test_quantizing   // 1
,  &hxa7241_image::exr::test_exr                 // 2
,  &hxa7241_image::png::test_png                 // 3
,  &hxa7241_image::ppm::test_ppm                 // 4
,  &hxa7241_image::rgbe::test_rgbe               // 5
};


bool testUnits
(
   int        whichTest,
   const bool isOutput,
   const bool isVerbose,
   const int  seed
)
{
   bool isOk = true;

   if( isOutput ) std::cout << "\n\n";

   const dword noOfTests = sizeof(TESTERS)/sizeof(TESTERS[0]);
   if( 0 >= whichTest )
   {
      for( dword i = 0;  i < noOfTests;  ++i )
      {
         isOk &= (TESTERS[i])(
            isOutput ? &std::cout : 0, isVerbose, seed );
      }
   }
   else
   {
      if( whichTest > noOfTests )
      {
         whichTest = noOfTests;
      }
      isOk &= (TESTERS[whichTest - 1])(
         isOutput ? &std::cout : 0, isVerbose, seed );
   }

   if( isOutput ) std::cout <<
      (isOk ? "--- successfully" : "*** failurefully") << " completed " <<
      ((0 >= whichTest) ? "all app unit tests" : "one app unit test") << "\n\n";

   return isOk;
}

}


#endif//TESTING
