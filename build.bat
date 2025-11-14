@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

:: --- Usage Notes (2024/1/10) ------------------------------------------------
::
:: This is a central build script for the Rubik project, for use in
:: Windows development environments. It takes a list of simple alphanumeric-
:: only arguments which control (a) what is built, (b) which compiler & linker
:: are used, and (c) extra high-level build options. By default, if no options
:: are passed, then the main "rubik" graphical rubik is built.
::
:: Below is a non-exhaustive list of possible ways to use the script:
:: `build rubik`
:: `build rubik clang`
:: `build rubik release`
:: `build rubik asan telemetry`
::
:: For a full list of possible build targets and their build command lines,
:: search for @build_targets in this file.
::
:: Below is a list of all possible non-target command line options:
::
:: - `asan`: enable address sanitizer
:: - `telemetry`: enable RAD telemetry profiling support

:: --- Unpack Arguments -------------------------------------------------------
for %%a in (%*) do set "%%a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set clang=1
if not "%release%"=="1" set debug=1
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile]
if "%clang%"=="1"   set msvc=0 && echo [clang compile]
if "%~1"==""                     echo [default mode, assuming `rubik` build] && set rubik=1
if "%~1"=="release" if "%~2"=="" echo [default mode, assuming `rubik` build] && set rubik=1

:: --- Unpack Command Line Build Arguments ------------------------------------
set auto_compile_flags=
if "%profile%"=="1" set auto_compile_flags=%auto_compile_flags% -DPROFILE=1 -DPROFILE_SPALL=1 && echo [profiling enabled]
if "%asan%"=="1"    set auto_compile_flags=%auto_compile_flags% -fsanitize=address && echo [asan enabled]

:: --- Compile/Link Line Definitions ------------------------------------------
set cl_common=     /I..\src\ /I..\simp\ /I..\local\ /I"%VULKAN_SDK%\Include" /nologo /FC /Z7
set cl_debug=      call cl /Od /Ob1 /DBUILD_DEBUG=1 %cl_common% %auto_compile_flags%
set cl_release=    call cl /O2 /DBUILD_DEBUG=0 %cl_common% %auto_compile_flags%
set cl_link=       /link /MANIFEST:EMBED /INCREMENTAL:NO /pdbaltpath:%%%%_PDB%%%% /NATVIS:"%~dp0\simp\natvis\base.natvis" /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib
set cl_out=        /out:
set cl_natvis=     /NATVIS:
set clang_common=  -I..\src\ -I..\simp\ -I..\local\ -I"%VULKAN_SDK%\Include" -gcodeview -fdiagnostics-absolute-paths -Wall -Wno-unknown-warning-option -Wno-missing-braces -Wno-unused-function -Wno-writable-strings -Wno-unused-value -Wno-unused-variable -Wno-unused-local-typedef -Wno-deprecated-register -Wno-deprecated-declarations -Wno-unused-but-set-variable -Wno-single-bit-bitfield-constant-conversion -Wno-compare-distinct-pointer-types -Wno-initializer-overrides -Wno-incompatible-pointer-types-discards-qualifiers -Xclang -flto-visibility-public-std -D_USE_MATH_DEFINES -Dstrdup=_strdup -Dgnu_printf=printf -ferror-limit=10000
set clang_debug=   call clang -g -O0 -DBUILD_DEBUG=1 %clang_common% %auto_compile_flags%
set clang_release= call clang -g -O2 -DBUILD_DEBUG=0 %clang_common% %auto_compile_flags%
set clang_link=    -fuse-ld=lld -Xlinker /MANIFEST:EMBED -Xlinker /pdbaltpath:%%%%_PDB%%%% -Xlinker /NATVIS:"%~dp0\simp\natvis\base.natvis" -L "%VULKAN_SDK%\Lib" -lvulkan-1
set clang_out=     -o
set clang_natvis=  -Xlinker /NATVIS:

:: --- Per-Build Settings -----------------------------------------------------
set link_dll=-DLL
:: set link_icon=logo.res
if "%msvc%"=="1"    set link_natvis=%cl_natvis%
if "%clang%"=="1"   set link_natvis=%clang_natvis%
if "%msvc%"=="1"    set only_compile=/c
if "%clang%"=="1"   set only_compile=-c
if "%msvc%"=="1"    set EHsc=/EHsc
if "%clang%"=="1"   set EHsc=
if "%msvc%"=="1"    set no_aslr=/DYNAMICBASE:NO
if "%clang%"=="1"   set no_aslr=-Wl,/DYNAMICBASE:NO
if "%msvc%"=="1"    set rc=call rc
if "%clang%"=="1"   set rc=call llvm-rc

:: --- Choose Compile/Link Lines ----------------------------------------------
if "%msvc%"=="1"      set compile_debug=%cl_debug%
if "%msvc%"=="1"      set compile_release=%cl_release%
if "%msvc%"=="1"      set compile_link=%cl_link%
if "%msvc%"=="1"      set out=%cl_out%
if "%clang%"=="1"     set compile_debug=%clang_debug%
if "%clang%"=="1"     set compile_release=%clang_release%
if "%clang%"=="1"     set compile_link=%clang_link%
if "%clang%"=="1"     set out=%clang_out%
if "%debug%"=="1"     set compile=%compile_debug%
if "%release%"=="1"   set compile=%compile_release%

:: --- Prep Directories -------------------------------------------------------
if not exist build mkdir build
if not exist local mkdir local

:: --- Produce Logo Icon File -------------------------------------------------
:: TODO
:: pushd build
:: %rc% /nologo /fo logo.res ..\data\logo.rc || exit /b 1
:: popd

:: --- Get Current Git Commit Id ----------------------------------------------
for /f %%i in ('call git describe --always --dirty')   do set compile=%compile% -DBUILD_GIT_HASH=\"%%i\"
for /f %%i in ('call git rev-parse HEAD')              do set compile=%compile% -DBUILD_GIT_HASH_FULL=\"%%i\"

:: --- Compile Shader ---------------------------------------------------------
set shader_in_dir=%~dp0\simp\render\vulkan\shader
set shader_out_dir=%~dp0\simp\render\vulkan\shader
if "%no_shader%"=="1" echo [skipping shader compiling]
if not "%no_shader%"=="1" (
  for %%G in ("%shader_in_dir%\*.vert" "%shader_in_dir%\*.frag" "%shader_in_dir%\*.comp") do (
    if exist "%%G" (
      set "shader=%%G"
      for %%A in ("%%~nxG") do (
        set "filename=%%~nA"
        set "extension=%%~xA"
        set "name=%%~nA"

        :: Remove the leading dot from the extension
        set "extension=!extension:~1!"
        set "spv_out=!shader_out_dir!\!name!_!extension!.spv"
        set "header_out=!shader_out_dir!\!name!_!extension!.spv.h"

        echo Compiling !shader! to !spv_out!
        glslc "!shader!" -o "!spv_out!"

        echo Embedding !spv_out! to !header_out!
        xxd -i -n "!name!_!extension!_spv" "!spv_out!" > "!header_out!"
      )
    )
  )
)

:: --- Build & Run Metaprogram ------------------------------------------------
if "%no_meta%"=="1" echo [skipping metagen]
if not "%no_meta%"=="1" (
  pushd build
  %compile_debug% ..\simp\metagen\metagen_main.c %compile_link% %out%metagen.exe || exit /b 1
  metagen.exe src || exit /b 1
  metagen.exe simp || exit /b 1
  popd
)

:: --- Pack Fonts -------------------------------------------------------------
if not "%no_meta%"=="1" (
  set font_in_dir=%~dp0\data\fonts
  set font_out_dir=%~dp0\local
  for %%G in (!font_in_dir!\*.ttf) do (
    set "name=%%~nG"
    echo Packing %%G to !font_out_dir!\!name!.h
    xxd -i -n "ttf_!name!" "%%G" > "!font_out_dir!\font_!name!.h"
  )
)

:: --- Build Everything (@build_targets) --------------------------------------
pushd build
if "%ink%"=="1"    set didbuild=1 && %compile% ..\src\ink\ink_main.c      %compile_link% %link_icon% %out%ink.exe   || exit /b 1
popd

:: --- Warn On No Builds ------------------------------------------------------
if "%didbuild%"=="" (
  echo [WARNING] no valid build target specified; must use build target names as arguments to this script, like `build rubik`.
  exit /b 1
)
