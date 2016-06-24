/**
 *
 * Header file for Linux kernel module for maintaining proc fs entries
 *
 */

#define LKM_CMD_MSG_LEN               128
#define LKM_DATA_MSG_LEN              4096
#define LKM_KEY_VALUE_PER_MSG         10

#define INNER_KEY_LEN                 30
#define INNER_VALUE_LEN               20

/* Key:value metadata */
typedef struct key_value_md_s {
    int start_key;
    int start_data;
    int total_data_len;
    int num_data;
} key_value_md_t;

/* proc metadata */
typedef struct proc_metadata_s {
    int total;
    key_value_md_t kv_md[LKM_KEY_VALUE_PER_MSG];
    char proc_info[LKM_DATA_MSG_LEN];
} proc_data_t;

typedef struct lkm_cmd_msg_s {
   int   buffer_size;
   //char         msg[LKM_CMD_MSG_LEN];
   char *msg;
   char *msg_2;
} lkm_cmd_msg_t;

typedef struct lkm_list_data_s {
    char inner_key[INNER_KEY_LEN];
    char inner_value[INNER_VALUE_LEN];
} lkm_list_data_t;

#define LKM_DRIVER_TYPE               'T'

/* IOCTL command numbers */
#define LKM_PROC_DIR_CMD   _IOW(LKM_DRIVER_TYPE, 1, lkm_cmd_msg_t)
#define LKM_PROC_INFO_ADD  _IOW(LKM_DRIVER_TYPE, 2, lkm_cmd_msg_t)

#define LKMDRVNAME  "test3"

#define LKMDRV   "/dev/"LKMDRVNAME
