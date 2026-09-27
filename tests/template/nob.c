#include <stdio.h>
#define TEST_UTILITIES
#include "../test_interface.h"

#include <assert.h>

#define CC "clang"
#include "build_tools/template.h"

bool run_test(const char* test_dir_abs_path, const char* test_root_dir)
{
    bool res = false;

    const char* argv[] =
    {
        "-b",
    };
    const size_t len = strlen(test_dir_abs_path);
    const char suffix[] = "/project";
    const size_t suffix_len = sizeof(suffix) - 1;
    const size_t path_len = len + suffix_len;
    char* path = malloc(path_len + 1);
    assert( path );

    snprintf(path, path_len + 1, "%s%s", test_root_dir, suffix);

    clean_test_dir(test_dir_abs_path);

    res = external_project_template(
            path,
            test_dir_abs_path,
            .cc = "clang",
            .o_file = temp_sprintf("%s/main", test_dir_abs_path),
            .cli_args.argc = sizeof(argv)/sizeof(argv[0]),
            .cli_args.argv = argv,
            );

    if ( !res ) goto end;

    res = file_exists(temp_sprintf("%s/main", test_dir_abs_path));

end:
    if ( path ) free(path);
    return res;
}

#define DEPENDENCY_IMPLEMENTATION
#include "dependency.h"

#define TEMPLATE_IMPLEMENTATION
#include "build_tools/template.h"

#define DEFS_IMPLEMENTATION
#include "defs.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
