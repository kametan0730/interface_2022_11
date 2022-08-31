#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

int main(){
    int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // PF_PACKETを指定してsocketをオープン
    if(sock == -1){
        perror("Could not open socket");
        return 1;
    }
    unsigned char buf[1550]; // 通信の受信用のメモリを確保
    while(true){
        ssize_t n = recv(sock, buf, sizeof(buf), 0); // socketから受信する
        if(n == -1){
            perror("Could not recv");
            return -1;
        }
        if(n != 0){
            printf("Received %lu bytes: ", n);
            for(int i = 0; i < n; ++i){
                printf("%02x", buf[i]);
            }
            printf("\n");
        }
    }
}
