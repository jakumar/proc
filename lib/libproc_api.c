#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include "../include/libproc_api.h"
#include "../include/proc_ioctl.h"


typedef enum {
    IOCTL_ADD = 1,
    IOCTL_UPDATE,
} ioctl_cmd_type;

typedef struct handle_list_t {
    void *handle;
    struct handle_list_t *next;
    struct handle_list_t *prev;
} handle_list_t;

handle_list_t *head;
handle_list_t *tail;

int libproc_send_ioctl(void *info, ioctl_cmd_type cmd)
{
    int fd;
    int err=0;

    proc_ioctl_msg_t *ioctl_msg = malloc(sizeof(proc_ioctl_msg_t));

    if (ioctl_msg == NULL) {
        printf("\nERROR: Failed to allocate memory");
        return -1;
    }

    if ((fd = open(LKMDRV, O_RDWR)) == -1) {
        printf("\nERROR: Failed to open device:%s", LKMDRV);
        return -1; 
    }

    ioctl_msg->msg = info;
    if (cmd == IOCTL_ADD) {
        if (ioctl(fd, LKM_PROC_ENTRY_ADD, ioctl_msg) != 0) {
            printf("\nERROR: IOCTL message failed");
            err = -1;
        }
    } else if (cmd == IOCTL_UPDATE) {
        if (ioctl(fd, LKM_PROC_ENTRY_UPDATE, ioctl_msg) != 0) {
            printf("\nERROR: IOCTL message failed");
            err = -1;
        }
    }

    close(fd);
    free(ioctl_msg);
    return err;
}


void * libproc_entry_init(char *path, char *file, 
                          int num_entry, proc_schema_t *info)
{
    proc_info_t *ginfo;
    int i=0;

    ginfo = (proc_info_t *)malloc(sizeof(proc_info_t));

    strncpy(ginfo->path, path, MAX_PATH_SIZE);
    ginfo->path[MAX_PATH_SIZE-1] = '\0';
    strncpy(ginfo->file, file, MAX_FILE_SIZE);
    ginfo->file[MAX_FILE_SIZE-1] = '\0';
    ginfo->num_entry = num_entry;
    ginfo->schema = (proc_schema_t *)malloc(num_entry*sizeof(proc_schema_t));

    for (i=0;i<num_entry;i++) {
        strncpy(ginfo->schema[i].key, info[i].key, MAX_KEY_SIZE);
        ginfo->schema[i].key[MAX_KEY_SIZE-1] = '\0';
        strncpy(ginfo->schema[i].value, info[i].value, MAX_VALUE_SIZE);
        ginfo->schema[i].value[MAX_VALUE_SIZE-1] = '\0';
    }

    // update the list for booking
    handle_list_t *node = malloc(sizeof(handle_list_t));
    if (!node) {
        printf("\n Out of memory !!!");
        return NULL;
    }

    if (!head || !tail) {
        node->handle = (void *)ginfo;
        node->next =
        node->prev = NULL;
        head =
        tail = node;
    } else {
        tail->next = node;
        node->prev = tail;
        node->next = NULL;
        tail = node;
    }
    
    printf("\nPATHHH:[%s]", ginfo->path);
    printf("\nFILEEE:[%s]", ginfo->file);
    if (libproc_send_ioctl((void *)ginfo, IOCTL_ADD) == -1) {
        printf("\nERROR: Send IOCTL Failed !!!");
        //return NULL;
    }

    return ginfo;
}

void libproc_update_entry(void *handle, char *key, char *value)
{
    proc_info_t *ginfo = (proc_info_t *)handle;
    int i=0;

    printf("\nPath:[%s]", ginfo->path);
    printf("\nFile:[%s]", ginfo->file);
    for (i=0 ; i<ginfo->num_entry ; i++) {
        if (strcmp(key, ginfo->schema[i].key) == 0) {
            strncpy(ginfo->schema[i].value, value, MAX_VALUE_SIZE);
            if (libproc_send_ioctl((void *)ginfo, IOCTL_UPDATE) == -1) {
                printf("\nERROR: Send IOCTL Failed !!!");
            }

            return;
        }
    }
}
