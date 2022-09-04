#include <net/if.h>
#include "config.h"

#include "ip.h"
#include "log.h"
#include "net.h"
#include "utils.h"



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
}
