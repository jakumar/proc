/**
 *
 * Header file for Linux kernel module for maintaining proc fs entries
 *
 */

#ifndef PROC_IOCTL
#define PROC_IOCTL

#define LKM_KEY_VALUE_PER_MSG         10
#define LKM_MAX_OUTER_KEY_PER_MSG     5
#define LKM_MAX_INNER_KV_PER_MSG      250
#define LKM_OUTER_KEY_LEN             256
#define LKM_INNER_KEY_LEN             30
#define LKM_INNER_VALUE_LEN           20

#define LKM_MAX_PROC_DIR_PER_MSG      10
#define LKM_MAX_PROC_FILE_PER_MSG     LKM_MAX_PROC_DIR_PER_MSG * 10
#define LKM_FILE_NAME_LEN             64
#define LKM_DIR_NAME_LEN              LKM_OUTER_KEY_LEN - LKM_FILE_NAME_LEN

#define LKM_DRIVER_TYPE               'T'

/* IOCTL command numbers */
#define LKM_PROC_DIR_CMD              _IOW(LKM_DRIVER_TYPE, 1, lkm_cmd_msg_t)
#define LKM_PROC_INFO_ADD             _IOW(LKM_DRIVER_TYPE, 2, lkm_cmd_msg_t)

#define LKMDRVNAME                    "test3"

#define LKMDRV                        "/dev/"LKMDRVNAME

typedef struct kv_data_ {
    char key[LKM_INNER_KEY_LEN];
    char value[LKM_INNER_VALUE_LEN];
} kv_data_t;

typedef struct radix_leaf_data_ {
    unsigned int    num_kv;
    kv_data_t      *kv;
} radix_leaf_data_t;

#define LKM_DATA_MSG_LEN               \
    (LKM_MAX_INNER_KV_PER_MSG * sizeof(kv_data_t))

/* proc data */
typedef struct proc_data_s {
    int num_key;
    char key_info[LKM_MAX_OUTER_KEY_PER_MSG][LKM_OUTER_KEY_LEN];
    int num_data_per_key[LKM_MAX_OUTER_KEY_PER_MSG];
    char kv_data[LKM_DATA_MSG_LEN];
} proc_data_t;

/* proc directory and file data */
typedef struct proc_dir_file_s {
    int num_dir;
    char dir_names[LKM_MAX_PROC_DIR_PER_MSG][LKM_DIR_NAME_LEN];
    int num_files_per_dir[LKM_MAX_PROC_DIR_PER_MSG];
    char file_names[LKM_MAX_PROC_FILE_PER_MSG][LKM_FILE_NAME_LEN];
} proc_dir_file_t;

typedef struct lkm_cmd_msg_s {
   int   buffer_size;
   char *msg;
} lkm_cmd_msg_t;

void
procfs_create(char *file_name, struct proc_dir_entry *parent_dir);

int
proc_process_and_add_node_to_radix_tree(void *data_object);

#endif // IOCTL_HANDLER
