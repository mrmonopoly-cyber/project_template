#include "../defs.h"
#include "../nob.h"

#ifndef CMAKE_PREFIX
#define CMAKE_PREFIX
#endif // !CMAKE_PREFIX

//==========================================types================================================
#ifndef CMAKE_TYPES
typedef enum
{
   CmakeGenerator_Ninja             = 0, //default
   CmakeGenerator_UnixMakefile,
   CmakeGenerator_NinjaMultiConfig,
   CmakeGenerator_FASTBuild,
   CmakeGenerator_WatcomWMake,

   CmakeGenerator__Count
}CmakeGenerator;

typedef struct
{
    CmakeGenerator generator;
    ArrayViewGDef cfg_opts;
}CmakeConfigureOpt;

typedef struct
{
    size_t jobs;
    Procs* async;
}CmakeBuildOpt;

#define CMAKE_TYPES
#endif // !CMAKE_TYPES

//=======================================declarations============================================
/**
* By default Ninja, or the generator specified by the user, will be used if possible,
* otherwise, as a fallback, UnixMakefile will be used
* IF that is not possible it will return an error.
*/
CMAKE_PREFIX bool _cmake_configure(
        const char* CmakeListsPath,
        const char* build_dir,
        const CmakeConfigureOpt opt);
#define cmake_configure(CMAKE, CFG_OPTS, PATH, ...) \
    _cmake_configure( (CMAKE), (CFG_OPTS), (PATH), (CmakeBuildOpt) {__VA_ARGS__} );

CMAKE_PREFIX bool _cmake_build(
        const char* build_dir,
        const CmakeBuildOpt opt);
#define cmake_build(CMAKE, CFG_OPTS, PATH, ...) \
    _cmake_build( (CMAKE), (CFG_OPTS), (PATH), (CmakeBuildOpt) {__VA_ARGS__} );


//======================================implementation===========================================
#ifdef CMAKE_IMPLEMENTATION
//====================================internal declarations======================================
CMAKE_PREFIX const char* _cmake_generator_to_str(const CmakeGenerator gen);
CMAKE_PREFIX bool _cmake_generator_is_usable(const CmakeGenerator gen);

CMAKE_PREFIX bool _cmake_configure(
        const char* CmakeListsPath,
        const char* build_dir,
        const CmakeConfigureOpt opt)
{
    bool res =false;
    Cmd cmd = {0};

    if ( !CmakeListsPath || !build_dir ) goto end;

    cmd_append(&cmd, "cmake");

    //source
    {
        cmd_append(&cmd, "-S");
        cmd_append(&cmd, CmakeListsPath);
    }

    //build dir
    {
        cmd_append(&cmd, "-B");
        cmd_append(&cmd, build_dir);
    }

    //generator
    {
        cmd_append(&cmd, "-G");
        if ( _cmake_generator_is_usable(opt.generator) )
        {
            cmd_append(&cmd, _cmake_generator_to_str(opt.generator));
        }
        else if ( _cmake_generator_is_usable(CmakeGenerator_UnixMakefile) )
        {
            cmd_append(&cmd, _cmake_generator_to_str(CmakeGenerator_UnixMakefile));
        }
        else
        {
            nob_log( ERROR, "Both Cmake generator %s and %s are not available. Aborting",
                    _cmake_generator_to_str(opt.generator), _cmake_generator_to_str(opt.generator));
            goto end;
        }
    }
    //global variables
    apply_global_definitions(&cmd, opt.cfg_opts);
    res = cmd_run(&cmd);

end:
    cmd_free(cmd);
    return res;
}

CMAKE_PREFIX bool _cmake_build(
        const char* build_dir,
        const CmakeBuildOpt opt)
{
    bool res =false;
    Cmd cmd = {0};
    size_t jobs = nprocs();

    if ( opt.jobs && opt.jobs < (size_t) nprocs() )
    {
        jobs = opt.jobs;
    }

    if ( !build_dir ) goto end;

    cmd_append(&cmd, "cmake");
    cmd_append(&cmd, "--build", build_dir);
    cmd_append(&cmd, "-j", temp_sprintf("%zu", jobs));
    if ( !(res = cmd_run(&cmd, .async = opt.async)) ) goto end;

end:
    cmd_free(cmd);
    return res;
}

//====================================internal implementation=====================================

CMAKE_PREFIX const char* _cmake_generator_to_str(const CmakeGenerator gen)
{
    switch (gen)
    {
        case CmakeGenerator_Ninja:              return "Ninja";
        case CmakeGenerator_UnixMakefile:       return "Unix Makefiles";
        case CmakeGenerator_NinjaMultiConfig:   return "Ninja Multi-Config";
        case CmakeGenerator_FASTBuild:          return "FASTBuild";
        case CmakeGenerator_WatcomWMake:        return "Watcom WMake";
        case CmakeGenerator__Count:             assert( 0 && "unreachable" );
    }

    assert( 0 && "unreachable" );
}

CMAKE_PREFIX bool _cmake_generator_is_usable(const CmakeGenerator gen)
{
    switch (gen)
    {
        case CmakeGenerator_Ninja:              return program_exists_on_path("ninja");
        case CmakeGenerator_UnixMakefile:       return program_exists_on_path("make");
        case CmakeGenerator_NinjaMultiConfig:   assert( 0 && "TODO: not yet implemented" );
        case CmakeGenerator_FASTBuild:          assert( 0 && "TODO: not yet implemented" );
        case CmakeGenerator_WatcomWMake:        assert( 0 && "TODO: not yet implemented" );
        case CmakeGenerator__Count:             assert( 0 && "unreachable" );
    }

    assert( 0 && "unreachable" );
}

#endif // CMAKE_IMPLEMENTATION
