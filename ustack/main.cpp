#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#define MAX_PKT_BURST 32
#define MEMPOOL_CACHE_SIZE 250

static volatile bool force_quit;

uint16_t g_port_id = 0;

#define MAX_RX_QUEUE_PER_LCORE 1024

void show_info(struct rte_mbuf *pkt, unsigned port_id, int q)
{
    
    // 解析数据包内容
    struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
    if (rte_be_to_cpu_16(eth_hdr->ether_type) == RTE_ETHER_TYPE_IPV4) {
        struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);

        // 判断传输层协议类型
        if (  ipv4_hdr->next_proto_id == IPPROTO_UDP) {
            // TCP 或 UDP 数据包
            void *trans_hdr = (void *)ipv4_hdr + sizeof(struct rte_ipv4_hdr);

            // 获取协议和端口信息
            const char *protocol = (ipv4_hdr->next_proto_id == IPPROTO_TCP) ? "TCP" : "UDP";
            uint16_t src_port = rte_be_to_cpu_16(*(uint16_t *)trans_hdr);
            uint16_t dst_port = rte_be_to_cpu_16(*((uint16_t *)trans_hdr + 1));

            // 计算传输层头部长度
            uint16_t trans_header_len = (ipv4_hdr->next_proto_id == IPPROTO_TCP)
                                            ? ((struct rte_tcp_hdr *)trans_hdr)->data_off * 4
                                            : sizeof(struct rte_udp_hdr);

            // 计算数据段长度
            uint16_t data_len = rte_be_to_cpu_16(ipv4_hdr->total_length) - sizeof(struct rte_ipv4_hdr) - trans_header_len;

            // 打印四元组信息
            printf("portid[%u][%d]%s [" RTE_ETHER_ADDR_PRT_FMT "][%u.%u.%u.%u]:%u -> [" RTE_ETHER_ADDR_PRT_FMT "][%u.%u.%u.%u]:%u, Data Length: %u\n",port_id,q,
                    protocol,RTE_ETHER_ADDR_BYTES(&(eth_hdr->src_addr)),
                    ipv4_hdr->src_addr & 0xFF, (ipv4_hdr->src_addr >> 8) & 0xFF,(ipv4_hdr->src_addr >> 16) & 0xFF,(ipv4_hdr->src_addr >> 24) & 0xFF,
                    src_port,RTE_ETHER_ADDR_BYTES(&(eth_hdr->dst_addr)),
                    ipv4_hdr->dst_addr & 0xFF,(ipv4_hdr->dst_addr >> 8) & 0xFF,(ipv4_hdr->dst_addr >> 16) & 0xFF,(ipv4_hdr->dst_addr >> 24) & 0xFF,
                    dst_port,
                    data_len);
        }
        else if( ipv4_hdr->next_proto_id == IPPROTO_TCP )
        {}
        else 
        {
            // 其他传输层协议，根据需要进行处理
            printf("Received unknown transport layer protocol packet\n");
        }
    }
}

static void rx_loop(unsigned portid)
{
    struct rte_mbuf *bufs[MAX_PKT_BURST];
    unsigned nb_rx;

    while (!force_quit) 
    {
		for(int q_id = 0; q_id < 1; ++q_id)
		{
			nb_rx = rte_eth_rx_burst(portid, q_id, bufs, MAX_PKT_BURST);

			if (unlikely(nb_rx == 0))
				continue;
            for (uint16_t buf = 0; buf < nb_rx; buf++)
            {	
                show_info(bufs[buf],portid,q_id);
                rte_pktmbuf_free(bufs[buf]);
            } 
		}
    }
}

static void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        printf("\n\nSignal %d received, preparing to exit...\n", signum);
        force_quit = true;
    }
}

static const struct rte_eth_conf port_conf_defaule=
{
    .rxmode ={
        .max_lro_pkt_size = RTE_ETHER_MAX_LEN
    }
};


int main(int argc, char **argv) 
{
    int ret;
  
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");

    const char *device_name = "0000:0b:00.0"; // or "fsl-gmac0" or "net_pcap0"
    force_quit = false;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    auto port_id = g_port_id;
    ret = rte_eth_dev_get_port_by_name(device_name, &port_id);
    if (ret == 0) {
        printf("Device %s port ID: %u\n", device_name, port_id);
    } else {
        printf("Failed to get port ID for device %s\n", device_name);
        return EXIT_FAILURE;
    }
    g_port_id = port_id;

    struct rte_eth_dev_info dev_info;
    ret = rte_eth_dev_info_get(port_id, &dev_info);
    if (ret != 0)
       rte_exit(EXIT_FAILURE, "Error getting device info: %s\n", strerror(-ret));

    int num_rx_queues = 1;
    int num_tx_queues = 0;
    struct rte_eth_conf local_port_conf = port_conf_defaule;

    ret = rte_eth_dev_configure(port_id, num_rx_queues, num_tx_queues, &local_port_conf);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n", ret, port_id);

    unsigned nb_mbufs = 8192;
    struct rte_mempool *q1_mbuf_pool = rte_pktmbuf_pool_create
            ("mbuf_pool1", nb_mbufs,0 /*MEMPOOL_CACHE_SIZE*/, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (q1_mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

    ret = rte_eth_rx_queue_setup(port_id, 0, MAX_RX_QUEUE_PER_LCORE, rte_eth_dev_socket_id(port_id), NULL, q1_mbuf_pool);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n", ret, port_id);

	ret = rte_eth_promiscuous_enable(port_id);
	if (ret != 0)
    	rte_exit(EXIT_FAILURE, "rte_eth_promiscuous_enable:err=%s, port=%u\n",
             rte_strerror(-ret), port_id);
    ret = rte_eth_dev_start(port_id);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n", ret, port_id);

    rx_loop(port_id);   
    


    // show_dev(&dev_info);

    // uint16_t nb_rxd = dev_info.rx_desc_lim.nb_max;
    // uint16_t nb_txd = dev_info.tx_desc_lim.nb_min;
    // ret = rte_eth_dev_adjust_nb_rx_tx_desc(port_id, &nb_rxd, &nb_txd);
	// if (ret != 0)
	// 	rte_exit(EXIT_FAILURE, "Cannot rte_eth_dev_adjust_nb_rx_tx_desc device: err=%d, port=%u\n", ret, port_id);
    // printf("nb_rxd [%u], nb_txd[%u]\n",nb_rxd, nb_txd);
	

    // struct rte_eth_rxconf rxq_conf = dev_info.default_rxconf;
	
    //rxq_conf.offloads = local_port_conf.rxmode.offloads;
    
    

    // ret = rte_eth_rx_queue_setup(port_id, 1, MAX_RX_QUEUE_PER_LCORE,
    //                              rte_eth_dev_socket_id(port_id),
    //                              &rxq_conf, q2_mbuf_pool);
    // if (ret < 0)
    //     rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n", ret, port_id);

	
    
    // struct rte_ether_addr addr;
	// ret = rte_eth_macaddr_get(port_id, &addr);
	// if (ret != 0)
	// 	return ret;

	// printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
	// 		   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
	// 		port_id, RTE_ETHER_ADDR_BYTES(&addr));

    

    printf("Closing port %d...", port_id);
    ret = rte_eth_dev_stop(port_id);
    if (ret != 0)
        printf("rte_eth_dev_stop: err=%d, port=%d\n", ret, port_id);
    rte_eth_dev_close(port_id);
    printf(" Done\n");

    rte_eal_cleanup();
    printf("Bye...\n");

    return EXIT_SUCCESS;
}
