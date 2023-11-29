#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include "normal_udp.h"

#define ETHTYPE_OFFSET                  12
#define DOT1Q_ETHTYPE_OFFSET            16
#define IP_ID_OFFSET  			18
#define DST_IP_OFFSET 			30
#define PAYLOAD_OFFSET			42
#define IPV4_PACKET                     0x0008
#define DOT1Q_FRAME                     0x0081
#define DST_IP_FILTER 			0xefefef08	//239.239.239.8 is DST IP we want to read
#define BEST_QUOTE_MSG			1
#define DEPTH_UPDATE_MSG		2

int packet_handler_full(char *input_data, char *output_data);


int main(int argc, char** argv)
{
    int sockfd;
    char buf[10240];
    ssize_t n;

    uint16_t prev_ip_id = 0;
    char output_data[256];

    int size = 0;
    int result = 0;
 
    struct ifreq req;    //网络接口地址
    struct sockaddr_ll addr;  

    if (argc != 2) {
	printf("Usage: ./dce-sub-raw-socket ens1f[0:1]\n");
	return 1;
    } 

    if ((sockfd = socket(PF_PACKET,  SOCK_RAW, htons(ETH_P_ALL)))== -1)
    {    
        printf("socket error!\n");
        return 1;
    }
    strncpy(req.ifr_name, argv[1], IFNAMSIZ);            //指定网卡名称
    if(-1 == ioctl(sockfd, SIOCGIFFLAGS, &req))    //获取网络接口
    {
        perror("ioctl");
        //close(sockfd);
        exit(-1);
    }
     
    req.ifr_flags |= IFF_PROMISC;
    if(-1 == ioctl(sockfd, SIOCGIFFLAGS, &req))    //网卡设置混杂模式
    {
        perror("ioctl");
        //close(sockfd);
        exit(-1);
    }
    
    // 绑定套接字到指定网卡  
    memset(&addr, 0, sizeof(addr));  
    addr.sll_family = PF_PACKET;  
    addr.sll_protocol = htons(ETH_P_ALL);  
    addr.sll_ifindex = if_nametoindex(req.ifr_name);  
    if (addr.sll_ifindex == 0) {  
        fprintf(stderr, "Unknown interface: %s\n", req.ifr_name);  
        exit(EXIT_FAILURE);  
    }  
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {  
        perror("bind");  
        exit(EXIT_FAILURE);  
    }  


    while (1)
    {
        n = recv(sockfd, buf, sizeof(buf), 0);
        if (n == -1)
        {
            printf("recv error!\n");
            break;
        }
        else if (n==0)
            continue;
        //接收数据不包括数据链路帧头
//        printf("recv %ld\n", n);

        size = n;
	char* packet_buf = (char*)(buf);

	if(size > 64)
	{

        	uint16_t *ethtype = (uint16_t *)(packet_buf + ETHTYPE_OFFSET);
        	uint16_t *dot1q_ethtype = (uint16_t *)(packet_buf + DOT1Q_ETHTYPE_OFFSET);
		int dot1q_offset = 0;

        	if(*ethtype == IPV4_PACKET)
                	dot1q_offset = 0;
        	else if(*ethtype==DOT1Q_FRAME && *dot1q_ethtype==IPV4_PACKET)
                	dot1q_offset = 4;
        	else
			continue;

        	if(ntohl(*((uint32_t *)(packet_buf + dot1q_offset + DST_IP_OFFSET))) != DST_IP_FILTER)
			continue;
		
		uint16_t last_ip_id = ntohs(*((uint16_t *)(packet_buf + dot1q_offset + IP_ID_OFFSET)));
		if((uint16_t)(prev_ip_id+0x0001) != last_ip_id)
		{
			printf("<WARN> IP_ID sequence number gap previous/latest: %d/%d \n", prev_ip_id, last_ip_id);
		}
		prev_ip_id = last_ip_id;

		memset(output_data, 0x00, 256);
		result = packet_handler_full(packet_buf, output_data);

		if(result == BEST_QUOTE_MSG)
		{
			normal_best_quote *best = (normal_best_quote *)output_data;
			if(best->bid_price>0xffff)  best->bid_price = 0;
			if(best->ask_price>0xffff)  best->ask_price = 0;
			if(best->last_price>0xffff) best->last_price = 0;

                	printf("%5d, %s, best, %d_%.1f x %.1f_%d, %d/%d, %.1f/%d\n",
                		last_ip_id, best->contract_id,
                		best->bid_qty, best->bid_price, best->ask_price, best->ask_qty,
                		best->match_tot_qty, best->open_interest, 
				best->last_price, best->last_match_qty);
		}
		else if(result == DEPTH_UPDATE_MSG)
		{
			normal_depth_update *deep = (normal_depth_update *)output_data;
			if(deep->bid1_price>0xffff)  deep->bid1_price = 0;
			if(deep->bid2_price>0xffff)  deep->bid2_price = 0;
			if(deep->bid3_price>0xffff)  deep->bid3_price = 0;
			if(deep->bid4_price>0xffff)  deep->bid4_price = 0;
			if(deep->bid5_price>0xffff)  deep->bid5_price = 0;
			if(deep->ask1_price>0xffff)  deep->ask1_price = 0;
			if(deep->ask2_price>0xffff)  deep->ask2_price = 0;
			if(deep->ask3_price>0xffff)  deep->ask3_price = 0;
			if(deep->ask4_price>0xffff)  deep->ask4_price = 0;
			if(deep->ask5_price>0xffff)  deep->ask5_price = 0;

                	printf("%5d, %s, deep, %d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f|%d_%.1f x %.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d|%.1f_%d\n",
                                last_ip_id, deep->contract_id,
                                deep->bid5_qty, deep->bid5_price,
                                deep->bid4_qty, deep->bid4_price,
                                deep->bid3_qty, deep->bid3_price,
                                deep->bid2_qty, deep->bid2_price,
                                deep->bid1_qty, deep->bid1_price,
                                deep->ask1_price, deep->ask1_qty,
                                deep->ask2_price, deep->ask2_qty,
                                deep->ask3_price, deep->ask3_qty,
                                deep->ask4_price, deep->ask4_qty,
                                deep->ask5_price, deep->ask5_qty);
		}
		else
		{
			printf("<ERROR> packet_decode_err: %d\n", size);
			continue;
		}
	}

    }
    //close(sockfd);
    return 0;
}

