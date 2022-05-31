//
// Created by tomcat on 5/31/22.
//

#ifndef KNOCKER_KNOCKING_H
#define KNOCKER_KNOCKING_H

#define  DROP_REQUEST           0
#define  ALLOW_REQUEST          1

//todo make r_dst_port configurable
static int port_seq_one = 1919;
static int port_seq_tow = 2020;
static int port_seq_three = 3030;

struct request_chain {
    char *src_ip;
    int knocked_port;
    struct request_chain *next;
};
struct authenticated_client {
    char *src_ip;
    char *mac_address; // in the case of LAN environment
    struct authenticated_client *next;
};


struct request_chain *queue_head = NULL;
struct request_chain *queue_ptr = NULL;

struct authenticated_client *client_list_head = NULL;
struct authenticated_client *client_list_ptr = NULL;


void add_req_queue(char *src_ip, int port) {
    struct request_chain *node = (struct request_chain *) malloc(sizeof(struct request_chain));
    node->src_ip = src_ip;
    node->knocked_port = port;
    node->next = queue_head;
    queue_head = node;
}

void add_client_allow_list(char *src_ip, char *mac_address) {
    struct authenticated_client *node = (struct authenticated_client *) malloc(sizeof(struct authenticated_client));
    node->src_ip = src_ip;
    node->mac_address = mac_address;
    node->next = client_list_head;
    client_list_head = node;
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

int is_authenticated_client(char *ip) {
    client_list_ptr = client_list_head;
    while (client_list_ptr != NULL) {
        if (strcmp(client_list_ptr->src_ip, ip) == 0) {
            return 0;
        }
        client_list_ptr = client_list_ptr->next;
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
