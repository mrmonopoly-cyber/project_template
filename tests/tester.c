#define CC "gcc"
#include "../empty_project/BuildDependencies/defs.h"
#include "../empty_project/BuildDependencies/nob.h"
#include "../empty_project/BuildDependencies/dependency.h"
#define TEST_UTILITIES
#include "test_interface.h"

#define CLI_TYPES
typedef struct CCliUserArgs{
    bool verbose;
    bool help;
    bool clean;
    bool clean_all;
    struct
    {
        const char **names;
        size_t num;
        size_t cap;
        bool all;
    }test_to_run;
}CliArgs;
#include "../empty_project/BuildDependencies/cli.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <dlfcn.h>

#define TESTS_DIR "."

static bool _run_test(const char* name)
{
    const size_t build_dir_name_len = strlen(BUILD_DIR);
    const size_t tests_dir_name_len = strlen(TESTS_DIR);
    const char test_src_file_name[] = "nob.c";

    bool res = false;
    const size_t tests_name_len = strlen(name);
    const size_t o_file_name_len = 
        build_dir_name_len +
        strlen("/") +
        tests_name_len +
        strlen(".so")
        + 1;

    const size_t file_src_len =
        tests_dir_name_len +
        strlen("/") +
        tests_name_len +
        strlen("/") +
        strlen(test_src_file_name)
        + 1;

    char *o_file_name = malloc(o_file_name_len + 1); //"BUILD_DIR/test_name.so"
    char *file_src = malloc(file_src_len + 1); //"tests/tests_name/nob.c"

    snprintf(o_file_name, o_file_name_len, "%s/%s.so", BUILD_DIR, name);
    snprintf(file_src, file_src_len, "%s/%s/%s",
            TESTS_DIR, name, test_src_file_name);

    //compile test
    {
        res = true;
        if ( needs_rebuild1(o_file_name, file_src) )
        {
            Cmd cmd = {0};

            nob_log(INFO, "compiling test: %s", name);
            cmd_append(&cmd, CC);
            nob_cc_flags(&cmd);
            cmd_append(&cmd, "-fPIC");
            cmd_append(&cmd, "-shared");
            cmd_append(&cmd, "-I../empty_project/BuildDependencies");
            cmd_append(&cmd, "-o", o_file_name);
            cmd_append(&cmd, file_src);

            res = cmd_run(&cmd);

            cmd_free(cmd);
        }
        free(file_src);
        if ( !res )
        {
            free(o_file_name);
            nob_log(ERROR, "failed to compile test: %s. Skipping", name);
            goto end;
        }
    }

    //run test
    {
        void* handle = dlopen(o_file_name, RTLD_NOW | RTLD_GLOBAL);
        if ( !handle )
        {
            nob_log(ERROR, "error loading %s: %s", o_file_name, dlerror());
            res = false;
            goto end;
        }

        bool (*run_test)(const char* test_dir_abs_path, const char* test_root_dir) =  dlsym(handle, "run_test");
        if ( !run_test )
        {
            nob_log(ERROR, "error loading run_test: %s", dlerror());
            res = false;
            goto end;
        }

        mkdir_if_not_exists(temp_sprintf("%s/%s", BUILD_DIR, name));

        if( !(res = run_test(
                        temp_sprintf("%s/%s", BUILD_DIR, name),
                        temp_sprintf("%s/%s", TESTS_DIR, name)
                        )) )
        {
            nob_log(ERROR, "test %s, failed", name);
        }

        dlclose(handle);
    }

    free(o_file_name);
    if ( !res ) goto end;


    res = true;
end:
    return res;
}

static bool _run_all_tests(void)
{
    Dir_Entry tests = {0};
    bool res = false;

    if ( !dir_entry_open(TESTS_DIR, &tests) )
    {
        nob_log(ERROR, "failed to open tests dir");
        return false;
    }

    while ( dir_entry_next(&tests) )
    {
        File_Type file_type = get_file_type(temp_sprintf("%s/%s", TESTS_DIR, tests.name));

        if (
                tests.name[0] != '.' &&
                strcmp(tests.name, "..") &&
                strcmp(tests.name, BUILD_DIR) &&
                strcmp(tests.name, TESTS_DIR) &&
                file_type == FILE_DIRECTORY &&
                !(res = _run_test(tests.name))
           )
        {
            goto end;
        }

    }

end:
    dir_entry_close(tests);
    return res;
}

static bool _walk_delete(Walk_Entry entry)
{
    delete_file(entry.path);
    return true;
}

static bool f_clean_nob(const char* prog_name_path)
{
    bool res = false;
    const char* old = temp_sprintf("%s.old", prog_name_path);

    delete_file(prog_name_path);
    if ( file_exists(old) ) delete_file(old);

    if ( !(res = walk_dir(DEPENDENCY_LOCAL_PROGRAMS_DB, _walk_delete, .post_order = true)) )
    {
        goto end;
    }

end:
    return res;
}

#define FAIL(...) do{res=1; nob_log(ERROR, __VA_ARGS__); goto end;}while(0);
int main(int argc, char **argv)
{
    int res = 0;
    CliArgs args = {0};

    GO_REBUILD_URSELF_PLUS(argc, argv,
            PROJECT_ROOT"/../empty_project/BuildDependencies/c_cli.h",
            PROJECT_ROOT"/../empty_project/BuildDependencies/cli.h",
            PROJECT_ROOT"/../empty_project/BuildDependencies/dependency.h",
            PROJECT_ROOT"/../empty_project/BuildDependencies/build_tools/cmake.h",
            PROJECT_ROOT"/../empty_project/BuildDependencies/build_tools/makefile.h",
            PROJECT_ROOT"/../empty_project/BuildDependencies/build_tools/template.h",

            PROJECT_ROOT"/../empty_project/BuildDependencies/defs.h"
            );

    set_log_handler(cancer_log_handler);

    if ( !cli_parse(&args, argc, argv) )
    {
        return 1;
    }

    if ( !file_exists(BUILD_DIR) ) mkdir_if_not_exists(BUILD_DIR);

    nob_log(INFO, "build dir: %s", BUILD_DIR);
    nob_log(INFO, "tests dir: %s", TESTS_DIR);

    if ( args.clean )
    {
        clean_test_dir(BUILD_DIR);
        delete_file(BUILD_DIR);
    }

    if ( args.clean_all && !f_clean_nob(argv[0]))
    {
        nob_log(ERROR, "failed cleaning nob");
        return 1;
    }

    if ( args.test_to_run.all )
    {
        if ( !_run_all_tests() )
        {
            nob_log(ERROR, "tests failed");
            res = 1;
        }
    }
    else
    {
        ArrayViewString str_view = {.data = args.test_to_run.names, .len =args.test_to_run.num};
        FOR_EACH_FAT_ARRAY_STR(str_view, test)
        {
            if ( !_run_test(test) )
            {
                nob_log(ERROR, "tests failed: %s", test);
                res = 1;
            }
        }
    }

end:
    return res;
}

#define CCLI_IMPLEMENTATION
#include "../empty_project/BuildDependencies/c_cli.h"

CCLI_PARSER_DECLARE(clean);
CCLI_PARSER_DECLARE(call);
CCLI_PARSER_DECLARE(tests);

static CCliArgDef defs [] = 
{
    //--clean, -c
    {
        .f_long = CCLI_LONG_FLAG(clean),
        .f_short = CCLI_SHORT_FLAG(c),
        .f_args = CCLI_NO_ARG,
        .f_description = "clean",
        .f_parser = CCLI_PARSER_NAME(clean),
    },
    //--clean-all, -call
    {
        .f_long = CCLI_LONG_FLAG(clean_all),
        .f_short = CCLI_SHORT_FLAG(call),
        .f_args = CCLI_NO_ARG,
        .f_description = "clean all",
        .f_parser = CCLI_PARSER_NAME(call),
    },

    //--tests, -t
    {
        .f_long = CCLI_LONG_FLAG(tests),
        .f_short = CCLI_SHORT_FLAG(t),
        .f_args = 
        {
            CCLI_NEW_ARG(names, CCliArgStr),
        },
        .f_description = "list of tests to run. If None is passed all the tests are executed",
        .f_parser = CCLI_PARSER_NAME(tests),
        .f_attributes = CCliFlagAttribute_ArgsList,
    },
};
static void cli_default(CliArgs* args)
{
    args->test_to_run.all = true;
}

CLI_PREFIX bool cli_parse(CliArgs* args, const int argc, char** argv)
{
    return c_cli_parse(defs, CCLI_ARRAYSIZE(defs), args, argc, argv, cli_default);
}

CCLI_PARSER_DECLARE_FULL(clean, args, ctx)
{
    args->clean = true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(call, args, ctx)
{
    args->clean = true;
    args->clean_all= true;
    return CCliActionOK;
}

CCLI_PARSER_DECLARE_FULL(tests, args, ctx)
{
    CCliActionReturn res = CCliActionMissingInput;
    const char* next;

    if ( (res = c_cli_parse_next_arg_str(ctx, &next)) != CCliActionOK ) goto end;

    if ( !file_exists(next) )
    {
        return CCliActionInvalidInput;
    }

    if ( args->test_to_run.num >= args->test_to_run.cap )
    {
        const size_t new_cap = args->test_to_run.cap ? args->test_to_run.cap << 1 : 1;
        const char** new_names = realloc(args->test_to_run.names, new_cap);

        assert( new_names );

        args->test_to_run.cap = new_cap;
        args->test_to_run.names = new_names;
    }

    args->test_to_run.names[args->test_to_run.num++] = next;

    res = CCliActionOK;
end:
    return res;
}

#define DEFS_IMPLEMENTATION
#include "../empty_project/BuildDependencies/defs.h"

#define NOB_IMPLEMENTATION
#include "../empty_project/BuildDependencies/nob.h"
