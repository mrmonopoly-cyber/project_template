#define TEST_UTILITIES
#include "../test_interface.h"

#include "build_tools/python.h"

bool run_test(const char* test_dir_abs_path, const char* test_root_dir)
{
    (void) test_dir_abs_path;
    return python_run(temp_sprintf("%s/build.py", test_root_dir));
}


#define PYTHON_IMPLEMENTATION
#include "build_tools/python.h"

#define MAKEFILE_IMPLEMENTATION
#include "build_tools/makefile.h"

#define DEPENDENCY_IMPLEMENTATION
#include "dependency.h"

#define DEFS_IMPLEMENTATION
#include "defs.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
