#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#define IPTYPE 8
#define TCPTYPE 6

#define VERSION "0.0.1"


typedef u_int tcp_seq;


u_int32_t id;
u_int8_t flag;


static u_int32_t knocking_process(struct nfq_data *tb) {
    struct nfqnl_msg_packet_hdr *ph;
    struct nfqnl_msg_packet_hw *hwph;
    u_int32_t mark, ifi;
    int ret;
    unsigned char *data;

    struct ip *ip_header;
    struct tcphdr *tcp_header;
    char *tcp_data_area;
    int ip_header_size;
    int tcp_header_size;
    int tcp_data_len;

    flag = 1;

    ph = nfq_get_msg_packet_hdr(tb);
    if (ph) id = ntohl(ph->packet_id); // id

    hwph = nfq_get_packet_hw(tb);
    if (hwph) {
        int i, hlen = ntohs(hwph->hw_addrlen);
    }

    mark = nfq_get_nfmark(tb);
    ifi = nfq_get_indev(tb);
    ifi = nfq_get_outdev(tb);
    ifi = nfq_get_physindev(tb);
    ifi = nfq_get_physoutdev(tb);
    ret = nfq_get_payload(tb, &data);
    int i = 0;
    int size = ret;

    ip_header = (struct ip *) data;
    ip_header_size = ip_header->ip_hl * 4;


    tcp_header = (struct tcphdr *) (data + ip_header_size);
    tcp_header_size = (tcp_header->th_off * 4);
    tcp_data_area = data + ip_header_size + tcp_header_size;
    tcp_data_len = ret - ip_header_size - tcp_header_size;

    const int r_dst_port = htons(tcp_header->th_dport);


    // here where we go to check the knocking criteria
    if (ip_header->ip_p == TCPTYPE && r_dst_port == 2020 && tcp_header->syn) {
        syslog(LOG_NOTICE, "knocking triggered.");
        flag = 1;
        //todo action e.g open ssh server
    }

    return id;
}


static int hooking_(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                    struct nfq_data *nfa, void *data) {
    u_int32_t id = knocking_process(nfa);
    return nfq_set_verdict(qh, id, flag, 0, NULL);
}

int main(int argc, char *argv[]) {
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    struct nfnl_handle *nh;
    int fd;
    int rv;
    char buf[4096] __attribute__ ((aligned));

    syslog(LOG_NOTICE, "start knocking ver[%s]", VERSION);

    // reset linux iptables
    system("iptables -F");
    system("iptables -X");
    system("iptables -t nat -F");

    // check req form local machine (lo)
    system("sudo iptables -A INPUT -j NFQUEUE --queue-num 0");
    system("sudo iptables -A OUTPUT -j NFQUEUE --queue-num 0");

    syslog(LOG_NOTICE, "opening library handle");
    h = nfq_open();

    if (!h) {
        syslog(LOG_NOTICE, "error during nfq_open()");
        exit(1);
    }

    syslog(LOG_NOTICE, "unbinding existing nf_queue handler for AF_INET (if any)");
    if (nfq_unbind_pf(h, AF_INET) < 0) {
        syslog(LOG_NOTICE, "error during nfq_unbind_pf()");
        exit(1);
    }

    syslog(LOG_NOTICE, "binding nfnetlink_queue as nf_queue handler for AF_INET");
    if (nfq_bind_pf(h, AF_INET) < 0) {
        syslog(LOG_NOTICE, "error during nfq_bind_pf()");
        exit(1);
    }

    syslog(LOG_NOTICE, "binding this socket to queue '0'");
    qh = nfq_create_queue(h, 0, &hooking_, NULL);
    if (!qh) {
        syslog(LOG_NOTICE, "error during nfq_create_queue()");
        exit(1);
    }

    syslog(LOG_NOTICE, "setting copy_packet mode");
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        syslog(LOG_NOTICE, "can't set packet_copy mode");
        exit(1);
    }

    fd = nfq_fd(h);

    for (;;) {
        if ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {

            nfq_handle_packet(h, buf, rv);
            continue;
        }
        if (rv < 0 && errno == ENOBUFS) {
            syslog(LOG_NOTICE, "losing packets!");
            continue;
        }
        break;
    }

    syslog(LOG_NOTICE, "unbinding from queue 0");
    nfq_destroy_queue(qh);

#ifdef INSANE
    /* normally, applications SHOULD NOT issue this command, since
     * it detaches other programs/sockets from AF_INET, too ! */
    syslog(LOG_NOTICE,"unbinding from AF_INET");
    nfq_unbind_pf(h, AF_INET);
#endif

    syslog(LOG_NOTICE, "closing library handle");
    nfq_close(h);
    exit(0);

}
