#include "config.h"

#include "binary_trie.h"
#include "ip.h"
#include "log.h"
#include "net.h"
#include "utils.h"


/**
 * デバイスに経路を設定
 * @param prefix
 * @param prefix_len
 * @param next_hop
 */
void configure_net_route(uint32_t prefix, uint32_t prefix_len, uint32_t next_hop){
    uint32_t mask = 0xffffffff;
    mask <<= (32 - prefix_len);

    ip_route_entry *ire;
    ire = (ip_route_entry *) (calloc(1, sizeof(ip_route_entry)));
    ire->type = network;
    ire->next_hop = next_hop;

    binary_trie_add(ip_fib, prefix & mask, prefix_len, ire);
}

/**

/**
 * デバイスにIPアドレスを設定
 * @param dev
 * @param address
 * @param netmask
 */
void configure_ip(net_device *dev, uint32_t address, uint32_t netmask){
    if(dev == nullptr){
        LOG_ERROR("Configure net device not found\n");
        exit(1);
    }

    printf("Set ip address to %s\n", dev->ifname);
    dev->ip_dev = (ip_device *) calloc(1, sizeof(ip_device));
    dev->ip_dev->address = address;
    dev->ip_dev->netmask = netmask;

    // IPアドレスを設定すると同時に直接接続ルートを設定する
    ip_route_entry *ire;
    ire = (ip_route_entry *) calloc(1, sizeof(ip_route_entry));
    ire->type = connected;
    ire->device = dev;

    int len = 0; // サブネットマスクとプレフィックス長の変換
    for(; len < 32; ++len){
        if(!(netmask >> (31 - len) & 0b01)){
            break;
        }
    }

    // 直接接続ネットワークの経路を設定
    binary_trie_add(ip_fib, address & netmask, len, ire);

    printf("Set directly connected route %s/%d via %s\n",
           ip_htoa(address & netmask), len, dev->ifname);
}
