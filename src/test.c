#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "proc_common.h"
#include "proc_radix_tree.h"
#include "proc_linked_list.h"
#include "proc_ioctl.h"
#include "proc_ioctl_buf.h"

radix_node_t *global_root = NULL;

int main(int argc, char **argv)
{
    char *key1 = "Romane";
    char *key2 = "Romanus";
    char *key3 = "Romanfu";
    char *key4 = "Roming";
    char *key5 = "Romanea";
    char *key6 = "Romings";
    kv_data_t   data1[] = {
                            {{"Romane-Tx"}, {"20"}},
                            {{"Romane-Rx"}, {"10"}},
                            {{"Romane-Erro"}, {"30"}},
                          };
    kv_data_t   data2[] = {
                            {{"Romanus-Tx"}, {"20"}},
                            {{"Romanus-Rx"}, {"10"}},
                            {{"Romanus-Erro"}, {"30"}},
                          };
    kv_data_t   data3[] = {
                            {{"Romanfu-Tx"}, {"20"}},
                            {{"Romanfu-Rx"}, {"10"}},
                            {{"Romanfu-Erro"}, {"30"}},
                          };
    kv_data_t   data4[] = {
                            {{"Roming-Tx"}, {"20"}},
                            {{"Roming-Rx"}, {"10"}},
                            {{"Roming-Erro"}, {"30"}},
                          };
    kv_data_t   data5[] = {
                            {{"Romanea-Tx"}, {"20"}},
                            {{"Romanea-Rx"}, {"10"}},
                            {{"Romanea-Erro"}, {"30"}},
                          };
    kv_data_t   data6[] = {
                            {{"Romings-Tx"}, {"20"}},
                            {{"Romings-Rx"}, {"10"}},
                            {{"Romings-Erro"}, {"30"}},
                          };

    ioctl_buf_xpath_t *xbuf1 = malloc(3*sizeof(ioctl_buf_xpath_t));
    ioctl_buf_xpath_t *xbuf2 = malloc(2*sizeof(ioctl_buf_xpath_t));
    ioctl_buf_xpath_t *xbuf3 = malloc(1*sizeof(ioctl_buf_xpath_t));

    ioctl_buf_t buf1;
    ioctl_buf_t buf2;
    ioctl_buf_t buf3;

    // init radix tree root
    global_root = proc_radix_tree_init();

    // Buffer 1, 3 records
    strncpy(xbuf1[0].xpath, key1, MAX_KEYSIZE);
    xbuf1[0].data.num_kv = 3;
    xbuf1[0].data.kv = data1;

    strncpy(xbuf1[1].xpath, key2, MAX_KEYSIZE);
    xbuf1[1].data.num_kv = 3;
    xbuf1[1].data.kv = data2;

    strncpy(xbuf1[2].xpath, key3, MAX_KEYSIZE);
    xbuf1[2].data.num_kv = 3;
    xbuf1[2].data.kv = data3;

    buf1.num_recs = 3;
    buf1.buf = xbuf1;
    proc_enqueue(&buf1);

    // Buffer 2, 2 records
    strncpy(xbuf2[0].xpath, key4, MAX_KEYSIZE);
    xbuf2[0].data.num_kv = 3;
    xbuf2[0].data.kv = data4;

    strncpy(xbuf2[1].xpath, key5, MAX_KEYSIZE);
    xbuf2[1].data.num_kv = 3;
    xbuf2[1].data.kv = data5;


    buf2.num_recs = 2;
    buf2.buf = xbuf2;
    proc_enqueue(&buf2);

    // Buffer 3, 1 record
    strncpy(xbuf3[0].xpath, key6, MAX_KEYSIZE);
    xbuf3[0].data.num_kv = 3;
    xbuf3[0].data.kv = data6;

    buf3.num_recs = 1;
    buf3.buf = xbuf3;
    proc_enqueue(&buf3);

    // trigger timer expiry
    timer_callback(0);

    // retrieval tests
    printf("\n\nRetrieval for Key:%s", key1);
    proc_ioctl_buf_print(proc_radix_retrieve(key1, global_root));

    printf("\n\nRetrieval for Key:%s", key3);
    proc_ioctl_buf_print(proc_radix_retrieve(key3, global_root));

    printf("\n\nRetrieval for Key:%s", key5);
    proc_ioctl_buf_print(proc_radix_retrieve(key5, global_root));

    printf("\n\nRetrieval for Key:%s", key2);
    proc_ioctl_buf_print(proc_radix_retrieve(key2, global_root));

    printf("\n\nRetrieval for Key:%s", key4);
    proc_ioctl_buf_print(proc_radix_retrieve(key4, global_root));

    printf("\n\nRetrieval for Key:%s", key6);
    proc_ioctl_buf_print(proc_radix_retrieve(key6, global_root));

    return 0;
}

