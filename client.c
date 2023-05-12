#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 88988

struct Packet {
    char data[1024];
    uint16_t checksum;
};

uint16_t calculate_checksum(const char* data, size_t len) {
    uint32_t sum = 0;
    while (len > 1) {
        sum += *(uint16_t*)data;
        data += 2;
        len -= 2;
    }
    if (len) {
        sum += *(uint8_t*)data;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return ~sum;
}

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    struct Packet packet;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("Enter message: ");
        fgets(packet.data, sizeof(packet.data), stdin);
        packet.checksum = calculate_checksum(packet.data, strlen(packet.data));
        send(sock, &packet, sizeof(packet), 0);
        valread = recv(sock, buffer, 1024, 0);
        printf("%s\n", buffer);
    }
    close(sock);
    return 0;
}