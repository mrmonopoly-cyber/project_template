#include "BuildDependencies/defs.h"
#include "BuildDependencies/cli.h"
#include "BuildDependencies/nob.h"

static CliArgs args;


static bool f_compile(Walk_Entry entry)
{
    bool res=true;

    if( entry.type == FILE_REGULAR && file_has_suffix_with_null(entry.path, ".c") )
    {
        Cmd cmd = {0};
        const char* file_name = nob_temp_file_name(entry.path);

        cmd_append(&cmd, CC);

        apply_all_defualt_compile_opts(&cmd);

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o",
                temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-2, file_name));

        cmd_append(&cmd, entry.path);

        res = cmd_run(&cmd);

        cmd_free(cmd);
    }

    return res;
}

static bool f_link(void)
{
    Dir_Entry dir = {0};
    Cmd cmd = {0};
    bool res = true;

    if( !dir_entry_open(BUILD_DIR, &dir) ) return false;

    cmd_append(&cmd, CC);

    apply_all_defualt_linker_opts(&cmd);

    cmd_append(&cmd, "-o", O_FILE);

    while( dir_entry_next(&dir) )
    {
        const char* file_path = temp_sprintf("%s/%s", BUILD_DIR, dir.name);
        if (
                get_file_type(file_path) ==  FILE_REGULAR &&
                file_has_suffix_with_null(file_path, ".o")
           )
        {
            nob_log(INFO, "found %s", file_path);
            cmd_append(&cmd, file_path);
        }
    }

    res = cmd_run(&cmd);

    dir_entry_close(dir);
    cmd_free(cmd);
    return res;
}

static bool f_run()
{
    bool res= false;
    Cmd cmd = {0};

    cmd_append(&cmd, "./"O_FILE);

    res = cmd_run(&cmd);


    cmd_free(cmd);
    return res;
}

static bool _walk_delete(Walk_Entry entry)
{
    if ( entry.type != FILE_DIRECTORY )
    {
        delete_file(entry.path);
    }

    return true;
}

static bool f_clean()
{
    bool res= false;
    Cmd cmd = {0};

    if ( file_exists(O_FILE) ) delete_file(O_FILE);

    if ( !(res = walk_dir(BUILD_DIR, _walk_delete)) ) goto end;
    delete_file(BUILD_DIR);

end:
    cmd_free(cmd);
    return res;
}

static bool f_clean_nob(const char* prog_name_path)
{
    const char* old = temp_sprintf("%s.old", prog_name_path);

    delete_file(prog_name_path);
    if ( file_exists(old) ) delete_file(old);

    return true;
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF_PLUS(argc, argv,
            PROJECT_ROOT"/BuildDependencies/c_cli.h",
            PROJECT_ROOT"/BuildDependencies/cli.h",
            PROJECT_ROOT"/BuildDependencies/build_tools/cmake.h",

            PROJECT_ROOT"/BuildDependencies/defs.h"
            );


    if ( !cli_parse(&args, argc, argv) )
    {
        return 1;
    }

    nob_log(INFO, "build directory: %s", BUILD_DIR);
    nob_log(INFO, "output file: %s", O_FILE);

    mkdir_if_not_exists(BUILD_DIR);

    if (
            args.build ||
            ( args.run && !file_exists(O_FILE) )
       )
    {
        //source directories
        FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
        {
            if( dir )
            {
                nob_log(INFO, "compiling sources in src: %s", dir);
                if( !walk_dir(dir, f_compile) )
                {
                    nob_log(ERROR, "failed compiling sources in %s", dir);
                    return 1;
                }
            }
        }

        if( !f_link() )
        {
            nob_log(ERROR, "failed liking");
            return 1;
        }
    }


    if ( args.run && !f_run() )
    {
        nob_log(ERROR, "failed running");
        return 1;
    }

    if ( args.clean && !f_clean() )
    {
        nob_log(ERROR, "failed cleaning");
        return 1;
    }
    
    if ( args.clean_all && !f_clean_nob(argv[0]) )
    {
        nob_log(ERROR, "failed cleaning nob");
        return 1;
    }

  return 0;
}

#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define CLI_IMPLEMENTATION
#include "BuildDependencies/cli.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"
