#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../include/libproc_api.h"

#define MAX_TEST_ENTRIES    4

int main()
{
    int i=0;
    proc_info_t *ginfo;
    proc_schema_t  info[4] =    {
        {.key = "key1", .value = "value1"},
        {.key = "key2", .value = "value2"},
        {.key = "key3", .value = "value3"},
        {.key = "keydasd4", .value = "value4"},
    };

    ginfo = libproc_entry_init("/dummy/test/", "testfile", 
            MAX_TEST_ENTRIES, (proc_schema_t *)&info);
    printf("\nPath:[%s]", ginfo->path);
    printf("\nFile:[%s]", ginfo->file);
    for (i=0;i<ginfo->num_entry;i++)
        printf("\n%s\t\t%s", ginfo->schema[i].key, ginfo->schema[i].value);

    libproc_update_entry((void *)ginfo, "key1", "newvalue1");
    printf("\n");
    for (i=0;i<ginfo->num_entry;i++)
        printf("\n%s\t\t%s", ginfo->schema[i].key, ginfo->schema[i].value);

    return 1;
}
