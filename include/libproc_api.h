#define MAX_KEY_SIZE        256
#define MAX_VALUE_SIZE      256

#define MAX_PATH_SIZE   256
#define MAX_FILE_SIZE   256

typedef struct proc_schema_ {
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
} proc_schema_t;

typedef struct proc_info_ {
    char path[MAX_PATH_SIZE];
    char file[MAX_FILE_SIZE];
    int  num_entry;
    proc_schema_t *schema;
} proc_info_t;

extern void * libproc_entry_init(char *path, char *file, int num_entry, proc_schema_t *info);
extern void libproc_update_entry(void *handle, char *key, char *value);
