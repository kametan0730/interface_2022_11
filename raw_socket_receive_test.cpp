#include <iostream>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>

int main(){

    struct ifreq ifr{};

    strcpy(ifr.ifr_name, "eth0");

    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // PF_PACKETを指定してsocketをオープン
    if(sock == -1){
        perror("Open failed: ");
        return 1;
    }

    if(ioctl(sock, SIOCGIFINDEX, &ifr) == -1){ // インターフェースのindexを取得し、ifrに格納する
        perror("recv");
        close(sock);
        return 1;
    }

    struct sockaddr_ll addr{};
    memset(&addr, 0x00, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifr.ifr_ifindex;
    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1){
        perror("Bind failed: ");
        close(sock);
    }

    int val = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, val | O_NONBLOCK); // ノンブロック

    printf("Created dev\n");

    unsigned char buf[1550];
    while(true){
            /* ソケットからデータ受信 */
            int n = recv(sock, buf, sizeof(buf), 0);
            if(n == -1){
                if(errno == EAGAIN){
                    continue;
                }else{
                    perror("Recv failed: ");
                    //close(a->fd);
                    return -1;
                }
            }
            if(n != 0){

                printf("Received %d bytes: ", n);

                for(int i = 0; i < n; ++i){
                    printf("%02x", buf[i]);
                }
                printf("\n");
            }
    }
}
