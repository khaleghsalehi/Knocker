//
// Created by tomcat on 5/31/22.
//

#ifndef KNOCKER_KNOCKING_H
#define KNOCKER_KNOCKING_H


//todo make r_dst_port configurable
static int port_seq_one = 1919;
static int port_seq_tow = 2020;
static int port_seq_three = 3030;

struct request_chain {
    char *src_ip;
    int knocked_port;
    struct request_chain *next;
};

struct request_chain *queue_head = NULL;
struct request_chain *queue_ptr = NULL;

void add_req_queue(char *src_ip, int port) {
    struct request_chain *node = (struct request_chain *) malloc(sizeof(struct request_chain));
    node->src_ip = src_ip;
    node->knocked_port = port;
    node->next = queue_head;
    queue_head = node;
}

void echo_req_queue() {
    int count = 0;
    queue_ptr = queue_head;
    while (queue_ptr != NULL) {
        count++;
        syslog(LOG_NOTICE, "%s -> %d", queue_ptr->src_ip, queue_ptr->knocked_port);
        queue_ptr = queue_ptr->next;
    }
    syslog(LOG_NOTICE, "total req ip -> %d", count);
}

int knock_registered(char *ip, int port) {
    queue_ptr = queue_head;
    while (queue_ptr != NULL) {
        int res = strcmp(queue_ptr->src_ip, ip);
        if (res == 0 && queue_ptr->knocked_port == port) {
            return 0;
        }
        queue_ptr = queue_ptr->next;
    }
    return 1;
}


char *dump_ip(unsigned int ip, char *resp) {
    //todo validate ip value
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    sprintf(resp, "%d.%d.%d.%d ", bytes[3], bytes[2], bytes[1], bytes[0]);
    return resp;
}

void remove_spaces(char *s) {
    char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

#endif //KNOCKER_KNOCKING_H
