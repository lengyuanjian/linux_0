# // gdb中使用“x”命令来打印内存的值，格式为“x/nfu addr”。含义为以f格式打印从addr开始的n个长度单元为u的内存值。参数具体含义如下：
# // *）n：输出单元的个数。
# // *）f：是输出格式。比如x是以16进制形式输出，
# //         o: 是以8进制形式输出,
# //         d：以十进制形式输出
# //         u：以十进制形式输出，但是不带符号
# //         t：以二进制形式输出
# //         a：以地址的形式输出
# //         c：以字符形式输出
# //         f：以浮点数形式输出
# //         s：以字符串形式输出,
# // *）u：标明一个单元的长度。b是一个byte，h是两个byte（halfword），w是四个byte（word），g是八个byte（giant word）。 
set env PAGET cat 
set pagination off

define p_mac
    if $argc < 1
        printf "Usage: p_mac <eth_hdr>\n"
        return
    end
    set $dst_offset = 0
    set $src_offset = $dst_offset + 6
    set $type_offset = $src_offset + 6

    printf "MAC:[%02X:%02X:%02X:%02X:%02X:%02X]->[%02X:%02X:%02X:%02X:%02X:%02X][0x%04x] \n", \
        *((unsigned char *)$arg0 + $src_offset), \
        *((unsigned char *)$arg0 + $src_offset + 1), \
        *((unsigned char *)$arg0 + $src_offset + 2), \
        *((unsigned char *)$arg0 + $src_offset + 3), \
        *((unsigned char *)$arg0 + $src_offset + 4), \
        *((unsigned char *)$arg0 + $src_offset + 5), \
        *((unsigned char *)$arg0 + $dst_offset), \
        *((unsigned char *)$arg0 + $dst_offset + 1), \
        *((unsigned char *)$arg0 + $dst_offset + 2), \
        *((unsigned char *)$arg0 + $dst_offset + 3), \
        *((unsigned char *)$arg0 + $dst_offset + 4), \
        *((unsigned char *)$arg0 + $dst_offset + 5), \
        *((unsigned short *)((unsigned char *)$arg0 + $type_offset))
end

define p_ipv4
    if $argc < 1
        printf "Usage: p_ipv4 <ipv4_hdr>\n"
        return
    end
    set $ver_ihl = (*((unsigned char *)$arg0) & 0xF)
    set $hlen = $ver_ihl * 4
    set $_total_length = *((unsigned short *)($arg0) + 1)
    set $total_length = ((((unsigned short)($_total_length))&0xff) << 8 ) | ((((unsigned short)($_total_length))&0xff00) >> 8 )
    set $src_addr0 = *(((unsigned char *)$arg0) + 3 * 4 + 0)
    set $src_addr1 = *(((unsigned char *)$arg0) + 3 * 4 + 1)
    set $src_addr2 = *(((unsigned char *)$arg0) + 3 * 4 + 2)
    set $src_addr3 = *(((unsigned char *)$arg0) + 3 * 4 + 3)
    set $dst_addr0 = *(((unsigned char *)$arg0) + 4 * 4 + 0)
    set $dst_addr1 = *(((unsigned char *)$arg0) + 4 * 4 + 1)
    set $dst_addr2 = *(((unsigned char *)$arg0) + 4 * 4 + 2)
    set $dst_addr3 = *(((unsigned char *)$arg0) + 4 * 4 + 3)
    printf "TPv4:[%u.%u.%u.%u]->[%u.%u.%u.%u] t_len[%d] h_len[%d]\n", \
        $src_addr0, $src_addr1, $src_addr2, $src_addr3, \
        $dst_addr0, $dst_addr1, $dst_addr2, $dst_addr3, \
        $hlen,$total_length
end

define p_tcp
    if $argc < 1
        printf "Usage: p_ipv4 <ipv4_hdr>\n"
        return
    end
     
    set $src_port_h = *((unsigned char *)($arg0))
    set $src_port_l = *((unsigned char *)($arg0) + 1)
    set $dst_port_h = *((unsigned char *)($arg0) + 2)
    set $dst_port_l = *((unsigned char *)($arg0) + 3)
    set $src_port = (((*((unsigned short *)($arg0))) & 0xff00) >> 8)| \
                    (((*((unsigned short *)($arg0))) & 0x00ff) << 8)
    set $dst_port = (((*((unsigned short *)($arg0) + 1)) & 0xff00) >> 8)| \
                    (((*((unsigned short *)($arg0) + 1)) & 0x00ff) << 8)

    set $seq =  ((*((unsigned int *)($arg0) + 1) & 0xFF000000) >> 24) | \
                ((*((unsigned int *)($arg0) + 1) & 0x00FF0000) >> 8)  | \
                ((*((unsigned int *)($arg0) + 1) & 0x0000FF00) << 8)  | \
                ((*((unsigned int *)($arg0) + 1) & 0x000000FF) << 24)
    set $ask =  ((*((unsigned int *)($arg0) + 2) & 0xFF000000) >> 24) | \
                ((*((unsigned int *)($arg0) + 2) & 0x00FF0000) >> 8)  | \
                ((*((unsigned int *)($arg0) + 2) & 0x0000FF00) << 8)  | \
                ((*((unsigned int *)($arg0) + 2) & 0x000000FF) << 24)

    printf "TCP:[%u]->[%u] [%u|%u]\n", \
        $src_port, $dst_port, $seq, $ask
end

b main.cpp:73
commands
    silent
    p_mac eth_hdr
    p_ipv4 ip_hdr
    p_tcp tcp_hdr
    c
end 
r
