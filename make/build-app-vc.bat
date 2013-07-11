@echo off


rem --- using: MS VC++ 2005 ---


set COMPILER=cl
set LINKER=link
set COMPILE_OPTIONS=/c /O2 /GL /arch:SSE /fp:fast /EHsc /GR- /GS- /MT /W4 /WL /nologo /D_PLATFORM_WIN /Iapplication/src /Iapplication/src/general /Iapplication/src/image



mkdir application\obj
del /Q application\obj\*



@echo.
@echo --- compile ---

%COMPILER% %COMPILE_OPTIONS% application/src/general/DynamicLibraryInterface.cpp /Foapplication/obj/DynamicLibraryInterface.obj

%COMPILER% %COMPILE_OPTIONS% application/src/image/exr.cpp /Foapplication/obj/exr.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/png.cpp /Foapplication/obj/png.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/ppm.cpp /Foapplication/obj/ppm.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/rgbe.cpp /Foapplication/obj/rgbe.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/ImageAdopter.cpp /Foapplication/obj/ImageAdopter.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/ImageFormatter.cpp /Foapplication/obj/ImageFormatter.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/ImageQuantizing.cpp /Foapplication/obj/ImageQuantizing.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/PixelsPtr.cpp /Foapplication/obj/PixelsPtr.obj
%COMPILER% %COMPILE_OPTIONS% application/src/image/StreamExceptionSet.cpp /Foapplication/obj/StreamExceptionSet.obj

%COMPILER% %COMPILE_OPTIONS% application/src/p3whitebalancer.cpp /Foapplication/obj/p3whitebalancer.obj



@echo.
@echo --- link ---

%LINKER% /LTCG /OPT:REF /OPT:NOWIN98 /VERSION:1.2 /NOLOGO /OUT:p3whitebalancer.exe kernel32.lib advapi32.lib p3whitebalancer.lib application/obj/*.obj


del /Q application\obj\*
