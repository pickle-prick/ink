#!/bin/bash
set -eu
cd "$(dirname "$0")"

# --- Unpack Arguments --------------------------------------------------------
for arg in "$@"; do declare $arg='1'; done
if [ ! -v clang ];     then gcc=1; fi
if [ ! -v release ];   then debug=1; fi
if [ -v debug ];       then echo "[debug mode]"; fi
if [ -v release ];     then echo "[release mode]"; fi
if [ -v clang ];       then compiler="${CC:-clang}"; echo "[clang compile]"; fi
if [ -v gcc ];         then compiler="${CC:-gcc}"; echo "[gcc compile]"; fi

# --- Unpack Command Line Build Arguments -------------------------------------
auto_compile_flags=''
if [ -v profile ];   then auto_compile_flags="-DPROFILE_SPALL=1"; echo "[profiling enabled]"; fi

# --- Compile/Link Line Definitions -------------------------------------------
clang_common='-I../src/ -I/usr/include/freetype2/ -I../simp/ -I../local/ -g -Wno-unknown-warning-option -fdiagnostics-absolute-paths -Wall -Wno-missing-braces -Wno-unused-function -Wno-writable-strings -Wno-unused-value -Wno-unused-variable -Wno-unused-local-typedef -Wno-deprecated-register -Wno-deprecated-declarations -Wno-unused-but-set-variable -Wno-single-bit-bitfield-constant-conversion -Wno-compare-distinct-pointer-types -Wno-initializer-overrides -Wno-incompatible-pointer-types-discards-qualifiers -Wno-for-loop-analysis -Xclang -flto-visibility-public-std -D_USE_MATH_DEFINES -Dstrdup=_strdup -Dgnu_printf=printf'
clang_debug="$compiler -g -O0 -DBUILD_DEBUG=1 ${clang_common} ${auto_compile_flags}"
clang_release="$compiler -g -O2 -DBUILD_DEBUG=0 ${clang_common} ${auto_compile_flags}"
clang_link="-lpthread -lm -lrt -ldl"
clang_out="-o"
gcc_common='-I../src/ -I/usr/include/freetype2/ -I../simp/ -I../local/ -g -Wno-unknown-warning-option -Wall -Wno-missing-braces -Wno-unused-function -Wno-attributes -Wno-unused-value -Wno-unused-variable -Wno-unused-local-typedef -Wno-deprecated-declarations -Wno-unused-but-set-variable -Wno-compare-distinct-pointer-types -D_USE_MATH_DEFINES -Dstrdup=_strdup -Dgnu_printf=printf -fzero-init-padding-bits=unions -std=gnu17'
gcc_debug="$compiler -g3 -O0 -DBUILD_DEBUG=1 ${gcc_common} ${auto_compile_flags}"
gcc_release="$compiler -g -O2 -DBUILD_DEBUG=0 ${gcc_common} ${auto_compile_flags}"
gcc_link="-lpthread -lm -lrt -ldl"
gcc_out="-o"

# --- Per-Build Settings ------------------------------------------------------
link_dll="-fPIC"
# link_os_gfx="-lX11 -lXext -lXxf86vm -lXrandr -lXi -lXcursor -lXfixes"
link_os_gfx="-lX11 -lXext -lXfixes"
link_render="-lvulkan"
link_os_audio="-lasound"
link_font_provider="-lfreetype"

# --- Choose Compile/Link Lines -----------------------------------------------
if [ -v gcc ];     then compile_debug="$gcc_debug"; fi
if [ -v gcc ];     then compile_release="$gcc_release"; fi
if [ -v gcc ];     then compile_link="$gcc_link"; fi
if [ -v gcc ];     then out="$gcc_out"; fi
if [ -v clang ];   then compile_debug="$clang_debug"; fi
if [ -v clang ];   then compile_release="$clang_release"; fi
if [ -v clang ];   then compile_link="$clang_link"; fi
if [ -v clang ];   then out="$clang_out"; fi
if [ -v debug ];   then compile="$compile_debug"; fi
if [ -v release ]; then compile="$compile_release"; fi

# --- Prep Directories --------------------------------------------------------
mkdir -p build
mkdir -p local

# --- Build & Run Metaprogram -------------------------------------------------
if [ -v no_meta ]; then echo "[skipping metagen]"; fi
if [ ! -v no_meta ]
then
  cd build
  $compile_debug ../simp/metagen/metagen_main.c $compile_link $out metagen
  ./metagen
  cd ..
fi

# --- Pack Fonts ---------------------------------------------------------------
if [ ! -v no_meta ]
then
  font_in_dir="./data/fonts"
  font_out_dir="./local"
  for font in "${font_in_dir}"/*.{ttf,}; do
    if [[ -f "$font" ]]; then
      filename=$(basename -- "$font")
      extension="${filename##*.}"
      name="${filename%.*}"
      font_out="${font_out_dir}/font_${name}.h"
      echo "Packing ${font} -> ${font_out}"
      xxd -i -n "ttf_${name}" "${font}" > "${font_out}"
    fi
  done
fi

# --- Compile Shaders ---------------------------------------------------------
if [ -v no_shader ]; then echo "[skipping shader]"; fi
if [ ! -v no_shader ]; then
  shader_in_dir="./simp/render/vulkan/shader"
  shader_out_dir="./simp/render/vulkan/shader"
  for shader in "${shader_in_dir}"/*.{vert,frag,comp}; do
    if [[ -f "$shader" ]]; then
      filename=$(basename -- "$shader")
      extension="${filename##*.}"
      name="${filename%.*}"

      spv_out="${shader_out_dir}/${name}_${extension}.spv"
      header_out="${shader_out_dir}/${name}_${extension}.spv.h"

      # Compile the shader to the output directory with .spv extension
      echo "Compiling ${shader} -> ${spv_out}"
      glslc "$shader" -o "$spv_out"

      echo "Embedding ${spv_out} -> ${header_out}"
      xxd -i -n "${name}_${extension}_spv" "$spv_out" > "$header_out"
    fi
  done
fi

# --- Build Everything (@build_targets) ---------------------------------------
cd build
if [ -v ink ];   then didbuild=1 && $compile ../src/ink/ink_main.c      $compile_link $link_os_gfx $link_render $link_os_audio $link_font_provider $out ink;   fi
cd ..

# --- Warn On No Builds -------------------------------------------------------
if [ ! -v didbuild ]
then
  echo "[WARNING] no valid build target specified; must use build target names as arguments to this script, like \`./build.sh ink\`."
  exit 1
fi
