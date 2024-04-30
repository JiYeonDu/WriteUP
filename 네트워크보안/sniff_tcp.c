#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>
#include <arpa/inet.h>


/* Ethernet header */
struct ethheader {
  u_char  ether_dhost[6]; /* destination host address */
  u_char  ether_shost[6]; /* source host address */
  u_short ether_type;     /* protocol type (IP, ARP, RARP, etc) */
};

/* IP Header */
struct ipheader {
  unsigned char      iph_ihl:4, //IP header length
                     iph_ver:4; //IP version
  unsigned char      iph_tos; //Type of service
  unsigned short int iph_len; //IP Packet length (data + header)
  unsigned short int iph_ident; //Identification
  unsigned short int iph_flag:3, //Fragmentation flags
                     iph_offset:13; //Flags offset
  unsigned char      iph_ttl; //Time to Live
  unsigned char      iph_protocol; //Protocol type
  unsigned short int iph_chksum; //IP datagram checksum
  struct  in_addr    iph_sourceip; //Source IP address
  struct  in_addr    iph_destip;   //Destination IP address
};

/* TCP Header */
struct tcpheader{
    u_short tcp_sport;               /* source port */
    u_short tcp_dport;               /* destination port */
    u_int   tcp_seq;                 /* sequence number */
    u_int   tcp_ack;                 /* acknowledgement number */
    u_char  tcp_offx2;               /* data offset, rsvd */
#define TH_OFF(th)      (((th)->tcp_offx2 & 0xf0) >> 4)
    u_char  tcp_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short tcp_win;                 /* window */
    u_short tcp_sum;                 /* checksum */
    u_short tcp_urp;                 /* urgent pointer */
}


void got_packet(u_char *args, const struct pcap_pkthdr *header,
                              const u_char *packet)
{
    //이더넷 헤더
  struct ethheader *eth = (struct ethheader *)packet;
    printf("destination host address : %hhu\n", eth->ether_dhost);
    printf("source host address : %hhu\n", eth->ether_shost);

  if (ntohs(eth->ether_type) == 0x0800) { // 0x0800 is IP type
      //ip헤더//이더넷 헤더를 건너뛰고 ip헤더의 시작 지점을 ip 포인터로 가리키게 한다.
    struct ipheader * ip = (struct ipheader *)
                           (packet + sizeof(struct ethheader));

    printf("source ip address: %s\n", inet_ntoa(ip->iph_sourceip));
    printf("destination ip address: %s\n", inet_ntoa(ip->iph_destip));
      
      //tcp 헤더
      if(TH_OFF(tcp) * 4 >= 20){
          struct tcpheader *tcp = (struct tcpheader *)(packet+sizeof(struct ethheader)+sizeof(ipheader));
          //packet 포인터에서 이더넷헤더, ip헤더 크기만큼 이동한 위치
          
          printf("source port: %s\n", inet_ntohs(tcp->tcp_sport));
          printf("destination port: %s\n", inet_ntohs(tcp->tcp_dport));
          
          int tcp_header_len = TH_OFF(tcp) *4;
          //tcp offset은 tcp 헤더의 길이를 나타내는 필드이다. 길이는 4비트이다
          //offset과 reserved 필드를 u_char 자료형에 같이 저장했기에 분리해준다.
          
          
          const u_char *data = (packet+ sizeof(struct ethheader)+sizeof(ipheader)+tcp_header_len);
          int data_len = 100
          for (int i = 0; i <data_len; i++){
              printf("%02x", data[i]);
          }
    
      }
    
  }
}

int main()
{
  pcap_t *handle;
  char errbuf[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  char filter_exp[] = "tcp";    //tcp프로토콜
  bpf_u_int32 net;

  // Step 1: Open live pcap session on NIC with name enp0s3
  handle = pcap_open_live("enp0s3", BUFSIZ, 1, 1000, errbuf);

  // Step 2: Compile filter_exp into BPF psuedo-code
  pcap_compile(handle, &fp, filter_exp, 0, net);
  if (pcap_setfilter(handle, &fp) !=0) {
      pcap_perror(handle, "Error:");
      exit(EXIT_FAILURE);
  }

  // Step 3: Capture packets
  pcap_loop(handle, -1, got_packet, NULL);

  pcap_close(handle);   //Close the handle
  return 0;
}


