#ifndef CURO_CONFIG_H
#define CURO_CONFIG_H

#include <cstdint>
#include <cstdio>

struct net_device;

void configure_ip_net_route(uint32_t prefix, uint32_t prefix_len, uint32_t next_hop);

void configure_ip_address(net_device *dev, uint32_t address, uint32_t netmask);

#endif //CURO_CONFIG_H
