#include "ip.h"

#include "arp.h"
#include "ethernet.h"
#include "log.h"
#include "my_buf.h"
#include "utils.h"

/**
 * サブネットにIPアドレスが含まれているか比較
 * @param subnet_prefix
 * @param subnet_mask
 * @param target_address
 * @return
 */
bool in_subnet(uint32_t subnet_prefix, uint32_t subnet_mask, uint32_t target_address){
    return ((target_address & subnet_mask) == (subnet_prefix & subnet_mask));
}

/**
 * 自分宛のIPパケットの処理
 * @param input_dev
 * @param ip_packet
 * @param len
 */
void ip_input_to_ours(net_device *input_dev, ip_header *ip_packet, size_t len){
    // 上位プロトコルの処理に移行
    switch(ip_packet->protocol){
        case IP_PROTOCOL_NUM_ICMP:
            LOG_IP("ICMP received!\n");
            return;

        case IP_PROTOCOL_NUM_UDP:
            return;
        case IP_PROTOCOL_NUM_TCP:
            // まだこのルータにはTCPを扱う機能はない
            return;
        default:

            LOG_IP("Unhandled ip protocol %04x", ip_packet->protocol);
            return;
    }
}

/**
 * IPパケットの受信処理
 * @param input_dev
 * @param buffer
 * @param len
 */
void ip_input(net_device *input_dev, uint8_t *buffer, ssize_t len){
    // IPアドレスのついていないインターフェースからの受信は無視
    if(input_dev->ip_dev == nullptr or input_dev->ip_dev->address == 0){
        return;
    }

    // IPヘッダ長より短かったらドロップ
    if(len < sizeof(ip_header)){
        LOG_IP("Received IP packet too short from %s\n", input_dev->name);
        return;
    }

    // 送られてきたバッファをキャストして扱う
    auto *ip_packet = reinterpret_cast<ip_header *>(buffer);

    LOG_IP("Received IP packet type %d from %s to %s\n", ip_packet->protocol,
           ip_ntoa(ip_packet->src_addr), ip_ntoa(ip_packet->dest_addr));

    if(ip_packet->version != 4){
        LOG_IP("Incorrect IP version\n");
        return;
    }

    // IPヘッダオプションがついていたらドロップ
    if(ip_packet->header_len != (sizeof(ip_header) >> 2)){
        LOG_IP("IP header option is not supported\n");
        return;
    }

    if(ip_packet->dest_addr == IP_ADDRESS_LIMITED_BROADCAST){ // 宛先アドレスがブロードキャストアドレスの場合
        return ip_input_to_ours(input_dev, ip_packet, len); // 自分宛の通信として処理
    }

    // 宛先IPアドレスをルータが持ってるか調べる
    for(net_device *dev = net_dev_list; dev; dev = dev->next){
        if(dev->ip_dev != nullptr and dev->ip_dev->address != IP_ADDRESS(0, 0, 0, 0)){
            // 宛先IPアドレスがルータの持っているIPアドレス or ディレクティッド・ブロードキャストアドレスの時の処理
            if(dev->ip_dev->address == ntohl(ip_packet->dest_addr) or dev->ip_dev->broadcast == ntohl(ip_packet->dest_addr)){
                return ip_input_to_ours(dev, ip_packet, len); // 自分宛の通信として処理
            }
        }
    }
}

/**
 * IPパケットにカプセル化して送信
 * @param dest_addr 送信先のIPアドレス
 * @param src_addr 送信元のIPアドレス
 * @param payload_mybuf 包んで送信するmy_buf構造体の先頭
 * @param protocol_num IPプロトコル番号
 */
void ip_encapsulate_output(uint32_t dest_addr, uint32_t src_addr, my_buf *payload_mybuf, uint8_t protocol_num){

    // 連結リストをたどってIPヘッダで必要なIPパケットの全長を算出する
    uint16_t total_len = 0;
    my_buf *current = payload_mybuf;
    while(current != nullptr){
        total_len += current->len;
        current = current->next;
    }

    // IPヘッダ用のバッファを確保する
    my_buf *ip_mybuf = my_buf::create(IP_HEADER_SIZE);
    payload_mybuf->add_header(ip_mybuf); // 包んで送るデータにヘッダとして連結する

    // IPヘッダの各項目を設定
    auto *ip_buf = reinterpret_cast<ip_header *>(ip_mybuf->buffer);
    ip_buf->version = 4;
    ip_buf->header_len = sizeof(ip_header) >> 2;
    ip_buf->tos = 0;
    ip_buf->total_len = htons(sizeof(ip_header) + total_len);
    ip_buf->protocol = protocol_num; // 8bit

    static uint16_t id = 0;
    ip_buf->identify = id++;
    ip_buf->frag_offset = 0;
    ip_buf->ttl = 0xff;
    ip_buf->header_checksum = 0;
    ip_buf->dest_addr = htonl(dest_addr);
    ip_buf->src_addr = htonl(src_addr);
    ip_buf->header_checksum = checksum_16(reinterpret_cast<uint16_t *>(ip_mybuf->buffer), ip_mybuf->len);

    for(net_device* dev = net_dev_list; dev; dev = dev->next){
        if(dev->ip_dev == nullptr or dev->ip_dev->address == IP_ADDRESS(0, 0, 0, 0)) continue;
        if(in_subnet(dev->ip_dev->address, dev->ip_dev->netmask, dest_addr)){
            arp_table_entry* entry;
            entry = search_arp_table_entry(dest_addr);
            if(entry == nullptr){
                LOG_IP("Trying ip output, but no arp record to %s\n", ip_htoa(dest_addr));
                send_arp_request(dev, dest_addr);
                my_buf::my_buf_free(payload_mybuf, true);
                return;
            }
            ethernet_encapsulate_output(dev, entry->mac_addr, ip_mybuf, ETHER_TYPE_IP);
        }
    }
}
