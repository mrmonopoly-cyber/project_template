#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"

#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))

#define CXX "c++"

#define BUILD_DIR "build"
#define O_FILE "main"

typedef struct GDef{
    const char* def;
    const char* val;
}GDef;

static const char* src_dirs[] = 
{
    "src",
    //add here your sources directory like ThirdParty dependencies sources
};

static const char* compiler_opts[] = 
{
    "-Wall",
    "-Wextra",
    "-std=c++23",
    //add here your compiler options: -c, -ggdb, -O2, ...
};

static const char* linker_opts[] = 
{

    //add here your compiler options: -lm, -lgdb, ...
};

static const char* include_path[] =
{
    //add here your include path: -I...
    //consider the root of the project the starting source path
};

static const GDef global_defs[] =
{
    //add here your global definitions: -DVAR=VALUE == (GDef) {.def="VAR", .val="VALUE"}
};

static bool f_compile(Walk_Entry entry)
{
    bool res=true;

    if(entry.type == FILE_REGULAR)
    {
        Cmd cmd = {0};
        const char* file_name = nob_temp_file_name(entry.path);

        cmd_append(&cmd, CXX);

        //compiler options
        for(size_t i=0; i < ArraySize(compiler_opts); i++)
        {
            if(compiler_opts[i]) cmd_append(&cmd, compiler_opts[i]);
        }

        //include path
        for(size_t i=0; i < ArraySize(include_path); i++)
        {
            if(include_path[i]) cmd_append(&cmd, temp_sprintf("-I%s", include_path[i]));
        }

        //global definitions
        for(size_t i=0; i < ArraySize(global_defs); i++)
        {
            const GDef* def = &global_defs[i];
            if(def && def->def)
            {
                if(def->val)
                {
                    cmd_append(&cmd, temp_sprintf("-D%s=%s", def->def, def->val));
                }
                else
                {
                    cmd_append(&cmd, temp_sprintf("-D%s", def->def));
                }
            }
        }

        cmd_append(&cmd, "-c");
        cmd_append(&cmd, "-o", temp_sprintf("%s/%.*s.o", BUILD_DIR, (int) strlen(file_name)-4, file_name));

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

    cmd_append(&cmd, CXX);

    //linker options
    for(size_t i=0; i < ArraySize(linker_opts); i++)
    {
        if(linker_opts[i]) cmd_append(&cmd, linker_opts[i]);
    }

    cmd_append(&cmd, "-o", O_FILE);

    while(dir_entry_next(&dir))
    {
        const char* file_path = temp_sprintf("%s/%s", BUILD_DIR, dir.name);
        if (FILE_REGULAR == get_file_type(file_path))
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
    GO_REBUILD_URSELF(argc, argv);

    printf("build directory: %s\n", BUILD_DIR);
    printf("output file: %s\n", O_FILE);

    mkdir_if_not_exists(BUILD_DIR);

    //source directories
    for(size_t i=0; i < ArraySize(src_dirs); i++)
    {
        if(src_dirs[i])
        {
            printf("compiling sources in src: %s\n", src_dirs[i]);
            if(!walk_dir(src_dirs[i], f_compile))
            {
                fprintf(stderr, "failed compiling sources in %s\n", src_dirs[i]);
                return 1;
            }
        }
    }

    if(!f_link())
    {
        fprintf(stderr, "failed liking\n");
        return 1;
    }

  return 0;
}
