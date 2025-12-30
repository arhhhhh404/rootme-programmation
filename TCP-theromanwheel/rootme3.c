#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>

void rot13(char *ch) {
    for (int i = 0; ch[i]; i++) {
        if (ch[i] >= 'a' && ch[i] <= 'z') {
            ch[i] = ((ch[i] - 'a' + 13) % 26) + 'a';
        } else if (ch[i] >= 'A' && ch[i] <= 'Z') {
            ch[i] = ((ch[i] - 'A' + 13) % 26) + 'A';
        }
    }
}

int main() {
    const char *host = "challenge01.root-me.org";
    const char *port = "52021";

    struct addrinfo hints, *res;
    int sockfd, status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(host, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "[!]> ERROR: %s\n", gai_strerror(status));
        return 1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("[!]> ERROR: socket");
        freeaddrinfo(res);
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("[!]> ERROR: connect");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    printf("[+]> Connected to %s:%s", host, port);

    while(1) {
        char buffer[2048];
        ssize_t recvcode = recv(sockfd, buffer, sizeof(buffer), 0);
        if (recvcode < 0) {
            perror("[!]> ERROR: recv");
        } else if (recvcode == 0) {
            printf("[-]> Server closed connection.\n");
        } else {
            if (!strstr(buffer, "my string is")) {
                printf("[+]> Final server answer:   %s\n", buffer);
                break;
            } else {
                buffer[recvcode] = '\0';
                printf("[+]> Server send: %s\n", buffer);
            }
        }

        const char *encoded_line = strstr(buffer, "my string is");
        if (!encoded_line) {
            fprintf(stderr, "[!]> ERROR: encoded line not found\n");
            return 1;
        }

        char encoded_decoded[512];
        if (sscanf(encoded_line, "my string is '%255[^']'. What is your answer", encoded_decoded) != 1) {
            fprintf(stderr, "[!]> ERROR: Unable to parse the encoded string\n");
            return 1;
        } else {
            printf("[+]> Encoded string: %s\n", encoded_decoded);
        }

        rot13(encoded_decoded);
        printf("[+]> Decoded string: %s\n", encoded_decoded);

        char sendbuffer[1024];

        snprintf(sendbuffer, sizeof(sendbuffer), "%s\n", encoded_decoded);
        ssize_t sendcode = send(sockfd, sendbuffer, sizeof(sendbuffer), 0);
        if (sendcode < 0) {
            perror("[!]> ERROR: send");
        } else {
            printf("[+]> Sent %zd bytes\n", sendcode);
        }
    }

    close(sockfd);
    freeaddrinfo(res);
    return 0;
}

    
