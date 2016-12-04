
#include "wld.h"

#define WLD_SIGNATURE   0x54503d02
#define WLD_VERSION1    0x00015500
#define WLD_VERSION2    0x1000C800

typedef struct WldHeader {
    uint32_t signature;
    uint32_t version;
    uint32_t fragCount;
    uint32_t unknownA[2];
    uint32_t stringsLength;
    uint32_t unknownB;
} WldHeader;

static void wld_init(Wld* wld, SimpleBuffer* file)
{
    array_init(&wld->fragsByIndex, Frag*);
    tbl_init(&wld->fragsByNameRef, Frag*);
    
    wld->strings        = NULL;
    wld->stringsLength  = 0;
    wld->data           = file;
}

static void wld_process_string(void* str, uint32_t len)
{
    static const byte hash[] = {0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A};
    byte* data = (byte*)str;
    uint32_t i;
    
    for (i = 0; i < len; i++)
    {
        data[i] = data[i] ^ hash[i & 7];
    }
}

static int iscap(int c)
{
    return (c >= 'A' && c <= 'Z');
}

static void wld_bmpify(Frag* frag)
{
    Frag03* f03     = (Frag03*)frag;
    uint32_t len    = f03->stringLength;
    
    wld_process_string(f03->string, len);
    
    printf("%s -> ", f03->string);
    
    f03->string[len - 4] = iscap(f03->string[len - 4]) ? 'B' : 'b';
    f03->string[len - 3] = iscap(f03->string[len - 3]) ? 'M' : 'm';
    f03->string[len - 2] = iscap(f03->string[len - 2]) ? 'P' : 'p';
    
    printf("%s\n", f03->string);
    
    wld_process_string(f03->string, len);
}

int wld_open(Wld* wld, SimpleBuffer* file)
{
    byte* data      = (byte*)buf_data(file);
    uint32_t len    = buf_length(file);
    WldHeader* h    = (WldHeader*)data;
    uint32_t p      = sizeof(WldHeader);
    uint32_t ver;
    int stringsLength;
    int nameRef;
    Frag* frag;
    uint32_t n;
    uint32_t i;
    
    wld_init(wld, file);
    
    if (p > len)
        goto oob;
    
    if (h->signature != WLD_SIGNATURE)
        return ERR_Invalid;
    
    ver = h->version & 0xfffffffe;
    
    if (ver != WLD_VERSION1 && ver != WLD_VERSION2)
        return ERR_Invalid;
    
    stringsLength       = -((int)h->stringsLength);
    wld->strings        = (char*)&data[p];
    wld->stringsLength  = stringsLength;
    
    p += h->stringsLength;
    
    if (p > len)
        goto oob;
    
    wld_process_string(wld->strings, -stringsLength);
    
    frag = NULL;
    if (!array_push_back(&wld->fragsByIndex, (void*)&frag))
        return ERR_OutOfMemory;
    
    n = h->fragCount;
    
    for (i = 0; i < n; i++)
    {
        frag = (Frag*)&data[p];
        
        p += sizeof(Frag);
        
        if (p > len)
            goto oob;
        
        p += frag->length - sizeof(uint32_t);
        
        if (p > len)
            goto oob;
        
        if (!array_push_back(&wld->fragsByIndex, (void*)&frag))
            return ERR_OutOfMemory;
        
        nameRef = frag->nameRef;
        
        if (nameRef < 0 && nameRef > stringsLength)
        {
            int rc = tbl_set_int(&wld->fragsByNameRef, nameRef, (void*)&frag);
            
            if (rc != ERR_Again)
            {
                if (rc)
                    return rc;
                
                if (frag->type == 0x03)
                    wld_bmpify(frag);
            }
        }
    }
    
    wld_process_string(wld->strings, -stringsLength);
    
    return ERR_None;
    
oob:
    return ERR_OutOfBounds;
}

void wld_close(Wld* wld)
{
    array_deinit(&wld->fragsByIndex, NULL);
    tbl_deinit(&wld->fragsByNameRef, NULL);
    
    if (wld->data)
    {
        buf_destroy(wld->data);
        wld->data = NULL;
    }
}
