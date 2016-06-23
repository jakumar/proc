#ifndef IOCTL_HANDLER
#define IOCTL_HANDLER

typedef struct kv_data_ {
    char key[MAX_KEYSIZE];
    char value[MAX_VALUESIZE];
} kv_data_t;

typedef struct radix_leaf_data_ {
    unsigned int    num_kv;
    kv_data_t       **data;
} radix_leaf_data_t;

#endif // IOCTL_HANDLER
