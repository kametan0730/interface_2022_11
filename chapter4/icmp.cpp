#include "icmp.h"

#include <cstring>
#include "ip.h"
#include "log.h"
#include "my_buf.h"
#include "net.h"
#include "utils.h"

/**
 * ICMPパケットの受信処理
 * @param source
 * @param destination
 * @param buffer
 * @param len
 */
void icmp_input(uint32_t source, uint32_t destination, void *buffer, size_t len){

    // ICMPヘッダ長より短かったら
    if(len < sizeof(icmp_header)){
        LOG_ICMP("Received ICMP packet too short\n");
        return;
    }

    // ICMPのパケットとして解釈する
    auto *header = reinterpret_cast<icmp_header *>(buffer);

    switch(header->type){
        case ICMP_TYPE_ECHO_REPLY:{
            // ICMP echoメッセージとして解釈する
            auto *reply = reinterpret_cast<icmp_echo *>(buffer);
            LOG_ICMP("Received icmp echo reply id %04x seq %d\n", ntohs(reply->identify), ntohs(reply->sequence));
        }
            break;
        case ICMP_TYPE_ECHO_REQUEST:{
            // ICMP echoメッセージとして解釈する
            auto *request = reinterpret_cast<icmp_echo *>(buffer);
            LOG_ICMP("Received icmp echo request id %04x seq %d\n", ntohs(request->identify), ntohs(request->sequence));

            my_buf *reply_my_buf = my_buf::create(len);

            auto *reply = reinterpret_cast<icmp_echo *>(reply_my_buf->buffer);
            reply->header.type = ICMP_TYPE_ECHO_REPLY;
            reply->header.code = 0;
            reply->header.checksum = 0;
            reply->identify = request->identify; // 識別番号をコピー
            reply->sequence = request->sequence; // シーケンス番号をコピー
            memcpy(&reply->data, &request->data, len - 8); //
            //reply->header.checksum = checksum_16_my_buf(reply_my_buf);

            ip_encapsulate_output(source, destination, reply_my_buf, IP_PROTOCOL_TYPE_ICMP);
        }
            break;

        default:
        LOG_ICMP("Received unhandled icmp type %d\n", header->type);
            break;
    }
}

/**
 * ICMP Time exceededメッセージを送信する
 * @param src_addr
 * @param dest_addr
 * @param code
 * @param error_ip_buffer
 * @param len
 */
void send_icmp_time_exceeded(uint32_t src_addr, uint32_t dest_addr, uint8_t code, void *error_ip_buffer, size_t len){
    if(len < sizeof(ip_header) + 8){ // パケットが小さすぎる場合
        return;
    }

    my_buf *time_exceeded_my_buf = my_buf::create(sizeof(icmp_time_exceeded) + sizeof(ip_header) + 8);
    auto *time_exceeded = reinterpret_cast<icmp_time_exceeded *>(time_exceeded_my_buf->buffer);

    time_exceeded->header.type = ICMP_TYPE_TIME_EXCEEDED;
    time_exceeded->header.code = code;
    time_exceeded->header.checksum = 0;
    time_exceeded->unused = 0;
    memcpy(time_exceeded->data, error_ip_buffer, sizeof(ip_header) + 8);
    time_exceeded->header.checksum = checksum_16(reinterpret_cast<uint16_t *>(time_exceeded_my_buf), time_exceeded_my_buf->len);

    LOG_ICMP("Sending time exceeded\n");

    ip_encapsulate_output(dest_addr, src_addr, time_exceeded_my_buf, IP_PROTOCOL_TYPE_ICMP);
}