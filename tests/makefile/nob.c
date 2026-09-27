#define TEST_UTILITIES
#include "../test_interface.h"

#include <assert.h>

#include "build_tools/makefile.h"
#include "nob.h"

bool run_test(const char* test_dir_abs_path, const char* test_root_dir)
{
    bool res = false;
    static char in_path[128] = {0};
    static char out_path[128] = {0};

    if (
            !(res = makefile_run_command(
                    .makefile_path = temp_sprintf("%s/ThirdParty/raylib/src", test_root_dir),
                    .jobs = 1))
       )
    {
        goto end;
    }

    snprintf(in_path, sizeof(in_path), "%s/ThirdParty/raylib/src/libraylib.a", test_root_dir);
    snprintf(out_path, sizeof(out_path), "%s/libraylib.a", test_dir_abs_path);
    if ( !(res = copy_file(in_path, out_path)) )
    {
        goto end;
    }

    if (
            !(res = makefile_run_command(
                    .command = "clean",
                    .makefile_path = temp_sprintf("%s/ThirdParty/raylib/src", test_root_dir)))
       )
    {
        goto end;
    }


end:
    return res;
}

#define MAKEFILE_IMPLEMENTATION
#include "build_tools/makefile.h"

#define DEPENDENCY_IMPLEMENTATION
#include "dependency.h"

#define DEFS_IMPLEMENTATION
#include "defs.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
