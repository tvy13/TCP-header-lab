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
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 12345
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 12345
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct Packet packet;
        valread = recv(new_socket, &packet, sizeof(packet), 0);
        if (valread == 0) {
            break;
        }
        uint16_t received_checksum = packet.checksum;
        packet.checksum = 0;
        uint16_t calculated_checksum = calculate_checksum((const char*)&packet, sizeof(packet));
        if (received_checksum == calculated_checksum) {
            printf("Received Message: %s\n", packet.data);
            send(new_socket, "ACK", 3, 0);
        } else {
            printf("Checksum Error: Received %u, calculated %u\n", received_checksum, calculated_checksum);
            send(new_socket, "NACK", 4, 0);
        }
    }
    close(new_socket);
    close(server_fd);
    return 0;
}
