#include <iostream>
#include <fstream>
#include "pro.h"

void proc_net_data(char *data,unsigned int len);

int main(int argc, char * argv[])
{
	std::cout << argv[0] <<"\n";
	
	const char * file_name = "ttt.pcap";
	
	std::ifstream r_file(file_name, std::ios::binary);
	if (!r_file)
	{
		std::cout << "file["<< file_name << "] open failed\n";
		return 0;
	}
	size_t f_size = 0;
	r_file.seekg(0, std::ios::end);
	f_size = r_file.tellg();
	r_file.seekg(0, std::ios::beg);

	std::cout << "file[" << file_name << "] size[" << f_size <<"]\n";

	std::cout << "******************************\n";
	sh::pcap_head head = {};
	r_file.read((char *)(&head), sizeof(head));
	size_t f_pos = sizeof(head);

	int cut = 0;
	while (f_pos < f_size)
	{
		if (f_pos + sizeof(sh::pcap_data) > f_size)
		{
			break;
		}
		sh::pcap_data data_head = {};
		r_file.read((char *)(&data_head), sizeof(sh::pcap_data));
		f_pos += sizeof(sh::pcap_data);
		cut++;
		//std::cout << "cut[" << cut << "]";
		char arry[4096 * 2] = {};
		char * buff = arry;
		if (data_head.caplen > 8192)
		{
			buff = new char[data_head.caplen]();
		}
		r_file.read(buff, data_head.caplen);

		proc_net_data(buff, data_head.caplen);
		f_pos += data_head.caplen;
		if (data_head.caplen > 8192)
		{
			delete [] buff;
		}
	}
	return 0;
}
int ip_str(unsigned int ip, char * str, int len);
void proc_net_data(char *data, unsigned int len)
{
	sh::ether_hdr *eth_hdr = (sh::ether_hdr *)data; 
	sh::ipv4_hdr  *ip_hdr;
	sh::tcp_hdr   *tcp_hdr;
	 
	if (sh::swap16(eth_hdr->ether_type) == 0x0800)
	{
		ip_hdr = (sh::ipv4_hdr *)(data + sizeof(sh::ether_hdr)); 
		if (ip_hdr->protocol == 6)
		{
			tcp_hdr = (sh::tcp_hdr *)(data + sizeof(sh::ether_hdr) + (((ip_hdr->vh_len)&0x0f) * 4));
			char src_ip[32] = {};
			char des_ip[32] = {};
			ip_str(ip_hdr->src_ip, src_ip, 32);
			ip_str(ip_hdr->des_ip, des_ip, 32); 
			char buf[128] = {};
			std::snprintf(buf, sizeof(buf), "[%s:%u]->[%s:%u] [%u|%u]\n",
                src_ip, sh::swap16(tcp_hdr->s_port), des_ip, sh::swap16(tcp_hdr->d_port),
                sh::swap32(tcp_hdr->seq) ,sh::swap32(tcp_hdr->ack));
			std::cout << buf;
		}
	}
}

int ip_str(unsigned int ip, char * str, int len)
{
	if (len < 16)
	{
		return 0;
	}
	unsigned char * addr = (unsigned char *)(&ip);
	int w_len = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (addr[i] > 99)
		{
			str[w_len++] = addr[i] / 100 + '0';
			str[w_len++] = addr[i] % 100 / 10 + '0';
			str[w_len++] = addr[i] % 100 % 10 + '0';
		}
		else if (addr[i] > 9)
		{
			str[w_len++] = addr[i] / 10 + '0';
			str[w_len++] = addr[i] % 10 + '0';
		}
		else
		{
			str[w_len++] = addr[i] % 10 + '0';
		}
		str[w_len++] = '.';
	}
	str[--w_len] = 0;
	return w_len;
}