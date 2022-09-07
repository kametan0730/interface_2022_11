#ifndef CURO_CONFIG_H
#define CURO_CONFIG_H

#include <cstdint>
#include <cstdio>

struct net_device;

void configure_ip_address(net_device *dev, uint32_t address, uint32_t netmask);

#endif //CURO_CONFIG_H
