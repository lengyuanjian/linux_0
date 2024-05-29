#pragma once

namespace sh
{
	using i8  = char;
	using i16 = short;
	using i32 = int;
	using i64 = long long;

	using u8  = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned int;
	using u64 = unsigned long long;
};
namespace sh
{
	constexpr u16 swap16(u16 x)
	{
		return ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8));
	}

	constexpr u32 swap32(u32 x)
	{
		return ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | \
			(((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24));
	}

	constexpr u64 swap64(u64 x)
	{
		return ((((x) & 0xFF00000000000000ull) >> 56) | \
			(((x) & 0x00FF000000000000ull) >> 40) | \
			(((x) & 0x0000FF0000000000ull) >> 24) | \
			(((x) & 0x000000FF00000000ull) >> 8) | \
			(((x) & 0x00000000FF000000ull) << 8) | \
			(((x) & 0x0000000000FF0000ull) << 24) | \
			(((x) & 0x000000000000FF00ull) << 40) | \
			(((x) & 0x00000000000000FFull) << 56));
	}
};
namespace sh
{
#pragma pack(push) //保存对齐状态
#pragma pack(1)
	struct pcap_head
	{
		u32  magic;
		u32  jor;
		u32  this_zone;
		u32  sigfigs;
		u32  snaplen;
		u32  linktype;

	};
	struct pcap_data
	{
		u32  timestamp_h;
		u32  timestamp_l;
		u32  caplen;
		u32  len;
	};
	struct mac
	{
		u8   addr[6];
	};

	struct ether_hdr
	{
		mac  des_mac;
		mac  src_mac;
		u16  ether_type;
	};

	struct ipv4_hdr
	{
		u8   vh_len;
		u8   svr_type;
		u16  total_len;
		u16  index_id;
		u16  flag_offset;
		u8   ttl;
		u8   protocol;
		u16  cksum;
		u32  src_ip;
		u32  des_ip;
	};

	struct udp_hdr
	{
		u16  s_port;
		u16  d_port;
		u16  total_len;
		u16  cksum;
	};

	struct tcp_hdr
	{
		u16  s_port;
		u16  d_port;
		u32  seq;
		u32  ack;
		u8   data_off_h4;
		u8   flag_l6;
		u16  win_size;
		u16  cksum;
		u16  ptr;
	};

#pragma pack(pop)
};
 