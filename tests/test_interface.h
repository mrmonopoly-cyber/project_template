#pragma once

#include <stdbool.h>

bool run_test(const char* test_dir_abs_path, const char* test_root_dir);

//=================================test utilities===============================================

#ifdef TEST_UTILITIES
#include <assert.h>
#include <string.h>

#include "../empty_project/BuildDependencies/nob.h"

static inline bool clean(Walk_Entry entry)
{
    const char* test_dir = (const char*) entry.data;

    assert(test_dir);

    if ( strcmp(entry.path, test_dir) )
    {
        delete_file(entry.path);
    }

    return true;

}

static inline void clean_test_dir(const char* test_dir_abs_path)
{
    walk_dir(test_dir_abs_path, clean, .data = (void*) test_dir_abs_path, .post_order = true);
}
#endif // TEST_UTILITIES
