@echo off


rem --- using: MS VC++ 2005 ---


set COMPILER=cl
set LINKER=link
set COMPILE_OPTIONS=/c /O2 /GL /arch:SSE /fp:fast /EHsc /GR- /GS- /MT /W4 /WL /nologo /D_CRT_SECURE_NO_DEPRECATE /D_PLATFORM_WIN /Ilibrary/src /Ilibrary/src/general /Ilibrary/src/graphics /Ilibrary/src/image /Ilibrary/src/whitebalance



mkdir library\obj
del /Q library\obj\*



@echo.
@echo --- compile ---

%COMPILER% %COMPILE_OPTIONS% library/src/general/LogFast.cpp /Folibrary/obj/LogFast.obj
%COMPILER% %COMPILE_OPTIONS% library/src/general/PowFast.cpp /Folibrary/obj/PowFast.obj

%COMPILER% %COMPILE_OPTIONS% library/src/graphics/ColorConstants.cpp /Folibrary/obj/ColorConstants.obj
%COMPILER% %COMPILE_OPTIONS% library/src/graphics/ColorConversion.cpp /Folibrary/obj/ColorConversion.obj
%COMPILER% %COMPILE_OPTIONS% library/src/graphics/Matrix3f.cpp /Folibrary/obj/Matrix3f.obj
%COMPILER% %COMPILE_OPTIONS% library/src/graphics/Vector3f.cpp /Folibrary/obj/Vector3f.obj

%COMPILER% %COMPILE_OPTIONS% library/src/image/ImageWrapper.cpp /Folibrary/obj/ImageWrapper.obj
%COMPILER% %COMPILE_OPTIONS% library/src/image/ImageWrapperConst.cpp /Folibrary/obj/ImageWrapperConst.obj

%COMPILER% %COMPILE_OPTIONS% library/src/whitebalance/WhiteBalancer.cpp /Folibrary/obj/WhiteBalancer.obj

%COMPILER% %COMPILE_OPTIONS% library/src/p3wbWhiteBalancer.cpp /Folibrary/obj/p3wbWhiteBalancer.obj



@echo.
@echo --- link ---

%LINKER% /DLL /DEF:library/p3wbWhiteBalancer.def /LTCG /OPT:REF /OPT:NOWIN98 /VERSION:1.2 /NOLOGO /OUT:p3whitebalancer.dll kernel32.lib library/obj/*.obj


del /Q library\obj\*
