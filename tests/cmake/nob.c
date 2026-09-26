#define TEST_UTILITIES
#include "../test_interface.h"

#include <assert.h>

#define CC "clang"
#include "build_tools/cmake.h"

bool run_test(const char* test_dir_abs_path, const char* test_root_dir)
{
    bool res = false;
    Procs procs = {0};
    const GDef build_gen_defs [] =
    {
        {"WAMR_BUILD_PLATFORM"                  , "linux"},
        {"WAMR_BUILD_TARGET"                    , "X86_64"},
        {"WAMR_ROOT_DIR"                        , "./wasm-micro-runtime"},
        {"WAMR_BUILD_INTERP"                    , "0"},
        {"WAMR_BUILD_FAST_INTERP"               , "0"},
        {"WAMR_BUILD_AOT"                       , "1"},
        {"WAMR_BUILD_LIBC_BUILTIN"              , "0"},
        {"WAMR_BUILD_LIBC_WASI"                 , "1"},
        {"WAMR_BUILD_SIMD"                      , "0"},
        {"WAMR_BUILD_REF_TYPES"                 , "1"},
        {"WAMR_BUILD_THREAD_MGR"                , "0"},
        {"WAMR_BUILD_SHARED_MEMORY"             , "1"},
        {"WAMR_BUILD_LIB_PTHREAD"               , "0"},
        {"WAMR_BUILD_LIB_WASI_THREADS"          , "0"},
        {"WAMR_BUILD_LINUX_PERF"                , "0"},
        {"WAMR_BUILD_DEBUG_INTERP"              , "0"},
        {"WAMR_BUILD_LOAD_CUSTOM_SECTION"       , "1"},
        {"WAMR_BUILD_CUSTOM_NAME_SECTION"       , "1"},

        {"CMAKE_EXPORT_COMPILE_COMMANDS"        , "ON"},
        {"CMAKE_C_COMPILER"                     , CC},
        {"CMAKE_BUILD_TYPE"                     , "Release"},
    };

    for ( CmakeGenerator gen =0; gen<_CmakeGenerator__Count; gen++ )
    {
        clean_test_dir(test_dir_abs_path);

        if (
                !(res = cmake_configure(
                        temp_sprintf("%s/ThirdParty/wamr", test_root_dir),
                        test_dir_abs_path,
                        .generator = gen,
                        .global_defs = (ArrayViewGDef) FAT_ARRAY_INIT(build_gen_defs)
                        ))
           )
        {
            goto end;
        }

        if ( !(res = cmake_build(test_dir_abs_path, .async = &procs)) )
        {
            goto end;
        }

        procs_flush(&procs);

        if ( !(res=file_exists(temp_sprintf("%s/libvmlib.a", test_dir_abs_path))) )
        {
            goto end;
        }
    }


end:
    return res;
}

#define CMAKE_IMPLEMENTATION
#include "build_tools/cmake.h"

#define DEFS_IMPLEMENTATION
#include "defs.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
