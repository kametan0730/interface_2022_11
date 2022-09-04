#ifndef CURO_IP_H
#define CURO_IP_H

#include <iostream>
#include <queue>

#define IP_ADDRESS_LEN 4
#define IP_ADDRESS(A, B, C, D) (A * 0x1000000u + B * 0x10000 + C * 0x100 + D)
#define IP_ADDRESS_LIMITED_BROADCAST IP_ADDRESS(255, 255, 255, 255)

#define IP_HEADER_SIZE 20

#define IP_PROTOCOL_TYPE_ICMP 0x01
#define IP_PROTOCOL_TYPE_TCP 0x06
#define IP_PROTOCOL_TYPE_UDP 0x11

#define IP_FRAG_OFFSET_MASK_RESERVED_FLAG         0b1000000000000000
#define IP_FRAG_OFFSET_MASK_DF_FLAG    0b0100000000000000
#define IP_FRAG_OFFSET_MASK_MF_FLAG    0b0010000000000000
#define IP_FRAG_OFFSET_MASK_OFFSET                0b0001111111111111

struct ip_header{
    uint8_t header_len: 4;
    uint8_t version: 4;
    uint8_t tos;
    uint16_t total_len;
    uint16_t identify;
    uint16_t frag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t src_addr;
    uint32_t dest_addr;
} __attribute__((packed));

struct ip_device{
    uint32_t address = 0;
    uint32_t netmask = 0;
};


struct net_device;

bool in_subnet(uint32_t subnet_prefix, uint32_t subnet_mask, uint32_t target_address);

struct net_device;

void ip_input(net_device *input_dev, uint8_t *buffer, ssize_t len);

struct my_buf;

void ip_encapsulate_output(uint32_t dest_addr, uint32_t src_addr, my_buf *buffer, uint8_t protocol_type);

#endif //CURO_IP_H