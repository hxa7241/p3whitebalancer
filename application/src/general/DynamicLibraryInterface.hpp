/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/


#ifndef DynamicLibraryInterface_h
#define DynamicLibraryInterface_h




#include "hxa7241_general.hpp"
namespace hxa7241_general
{

namespace dynamiclink
{

   typedef int (*FunctionPtr)();


   void loadLibrary
   (
      const char libraryPathName[],
      void*&     libraryHandle
   );


   bool freeLibrary
   (
      void*& libraryHandle
   );


   FunctionPtr getFunction
   (
      void*      libraryHandle,
      const char functionName[]
   );


   class Library
   {
   public:
      Library( const char pathName[],
               void*&     handle )
       : pHandle_m( &handle )
      {
         loadLibrary( pathName, *pHandle_m );
      }

      ~Library()
      {
         freeLibrary( *pHandle_m );
      }

   private:
      Library( const Library& );
      Library& operator=( const Library& );
   public:

      FunctionPtr getFunction( const char name[] ) const
      {
         return dynamiclink::getFunction( *pHandle_m, name );
      }

   private:
      void** pHandle_m;
   };

}//namespace

}//namespace




#endif//DynamicLibraryInterface_h
