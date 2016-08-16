/**
 *
 * Header file for Linux kernel module for maintaining proc fs entries
 *
 */

#ifndef PROC_IOCTL
#define PROC_IOCTL

#define LKM_DRIVER_TYPE               'T'

/* IOCTL command numbers */
#define LKM_PROC_DIR_CMD              _IOW(LKM_DRIVER_TYPE, 1, lkm_cmd_msg_t)
#define LKM_PROC_INFO_ADD             _IOW(LKM_DRIVER_TYPE, 2, lkm_cmd_msg_t)
#define LKM_PROC_ENTRY_ADD            _IOW(LKM_DRIVER_TYPE, 3, proc_ioctl_msg_t)
#define LKM_PROC_ENTRY_UPDATE         _IOW(LKM_DRIVER_TYPE, 4, proc_ioctl_msg_t)

#define LKMDRVNAME                    "test3"

#define LKMDRV                        "/dev/"LKMDRVNAME

#define MAX_RADIX_KEY_SIZE  (MAX_PATH_SIZE + MAX_FILE_SIZE)

typedef struct radix_leaf_data_ {
    char    rkey[MAX_RADIX_KEY_SIZE];
    int     num_kv;
    proc_schema_t   *kv;
} radix_leaf_data_t;

typedef struct lkm_cmd_msg_s {
   int   buffer_size;
   char *msg;
} lkm_cmd_msg_t;


typedef struct proc_ioctl_msg_ {
    char *msg;
} proc_ioctl_msg_t;

#endif // IOCTL_HANDLER
