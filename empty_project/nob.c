#include <string.h>
#define DEFS_IMPLEMENTATION
#include "BuildDependencies/defs.h"

#define NOB_IMPLEMENTATION
#include "BuildDependencies/nob.h"


static bool f_compile(Walk_Entry entry)
{
    bool res=true;

    if(entry.type == FILE_REGULAR && file_has_suffix_with_null(entry.path, ".c"))
    {
        Cmd cmd = {0};
        const char* file_name = nob_temp_file_name(entry.path);
        const size_t len_file_name = strlen(file_name);
        const char* suffix = file_name + len_file_name - 2;

        cmd_append(&cmd, CC);

        apply_all_defualt_compile_opts(&cmd);

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o", temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-2, file_name));

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

    if(!dir_entry_open(BUILD_DIR, &dir)) return false;

    cmd_append(&cmd, CC);

    apply_all_defualt_linker_opts(&cmd);

    cmd_append(&cmd, "-o", O_FILE);

    while(dir_entry_next(&dir))
    {
        const char* file_path = temp_sprintf("%s/%s", BUILD_DIR, dir.name);
        if (
                get_file_type(file_path) ==  FILE_REGULAR &&
                file_has_suffix_with_null(file_path, ".o")
           )
        {
            printf("found %s\n", file_path);
            cmd_append(&cmd, file_path);
        }
    }

    res = cmd_run(&cmd);

    dir_entry_close(dir);
    cmd_free(cmd);
    return res;
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF_PLUS(argc, argv,
            "./BuildDependencies/defs.h"
            );

    nob_log(INFO, "build directory: %s\n", BUILD_DIR);
    nob_log(INFO, "output file: %s\n", O_FILE);

    mkdir_if_not_exists(BUILD_DIR);

    //source directories
    FOR_EACH_FAT_ARRAY_STR(default_src_dir_opts(), dir)
    {
        if(dir)
        {
            printf("compiling sources in src: %s\n", dir);
            if(!walk_dir(dir, f_compile))
            {
                nob_log(ERROR, "failed compiling sources in %s", dir);
                return 1;
            }
        }
    }

    if(!f_link())
    {
        nob_log(ERROR, "failed liking");
        return 1;
    }

  return 0;
}
