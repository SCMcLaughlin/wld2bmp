
#include "define.h"
#include "err_code.h"
#include "pfs.h"
#include "wld.h"

static void read_pfs(const char* path)
{
    Pfs pfs;
    int rc;
    uint32_t i = 0;
    int hadErr = false;
    
    rc = pfs_open(&pfs, path);
    
    if (rc)
    {
        printf("Could not open '%s': %s\n", path, err_str(rc));
        goto finish;
    }
    
    for (;;)
    {
        SimpleString* name = pfs_get_name(&pfs, i++);
        uint32_t len;
        SimpleBuffer* file;
        Wld wld;
        
        if (!name)
            break;
        
        len = sstr_length(name);
        
        if (len <= 4 || strncmp(sstr_data(name) + len - 4, ".wld", 4) != 0)
            continue;
        
        file = pfs_get(&pfs, sstr_data(name), len);
        
        if (!file)
            continue;
        
        rc = wld_open(&wld, file);
        
        if (rc)
        {
        err:
            printf("ERROR: %s\n", err_str(rc));
        #ifdef EDP_WINDOWS
            getchar();
        #endif
            hadErr = true;
        }
        else
        {
            rc = pfs_put(&pfs, sstr_data(name), sstr_length(name), buf_data(wld.data), buf_length(wld.data));
            
            if (rc)
                goto err;
        }
        
        wld_close(&wld);
    }
    
    rc = pfs_save(&pfs);
    
    if (rc)
    {
        printf("Error saving S3D: %s\n", err_str(rc));
    #ifdef EDP_WINDOWS
        getchar();
    #endif
    }
    
finish:
    pfs_close(&pfs);
    
    if (!rc && !hadErr)
    {
        printf("Success\n");
    #ifdef EDP_WINDOWS
        getchar();
    #endif
    }
}

int main(int argc, const char** argv)
{
    int i;
    
    if (argc <= 1)
    {
        printf("No input files specified, aborting\n");
    #ifdef EDP_WINDOWS
        getchar();
    #endif
    }
    
    for (i = 1; i < argc; i++)
    {
        read_pfs(argv[i]);
    }
    
    return 0;
}
