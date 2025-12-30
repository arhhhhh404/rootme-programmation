#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <zlib.h>
#include <netdb.h>

int main() {
    const char *host = "challenge01.root-me.org";
    const char *port = "52022";

    const BIO_METHOD *BIO_f_base64(void);

    struct addrinfo hints, *res;
    int status, sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(host, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "[!]> ERROR : %s", gai_strerror(status));
        return -1;
    }
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("[!]> ERROR: socket");
        freeaddrinfo(res);
        return -1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("[!]> ERROR: connect");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    printf("[+]> Connected to %s:%s\n", host, port);

    while(1) {
        char buffer[2048];
        ssize_t recvcode = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (recvcode < 0) {
            perror("[!]> ERROR: recv");
        } else if (recvcode == 0) {
            printf("[-]> Server closed connection.\n");
        } else {
            buffer[recvcode] = '\0';
            printf("[+]> Server send: %s\n", buffer);
        }
        if (!strstr(buffer, "my string is")) {
            printf("[+]> Final server answer:   %s\n", buffer);
            break;
        }

        const char *compressed_encoded_line = strstr(buffer, "my string");
        if (!compressed_encoded_line) {
            fprintf(stderr, "[!]> ERROR: compressed_encoded line not found\n");
            return -1;
        }

        char compressed_encoded[512];
        if (sscanf(compressed_encoded_line, "my string is '%255[^']'. What is your answer ?", compressed_encoded) != 1) {
            fprintf(stderr, "[!]> ERROR: unable to parse compressed_encoded string\n");
            return -1;
        } else {
            printf("[+]> Encoded string: %s\n", compressed_encoded);
        }

        char compressed_decoded[512];
        BIO *bio, *b64;
        b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_new_mem_buf(compressed_encoded, -1);
        bio = BIO_push(b64, bio);

        int compressed_decoded_len = BIO_read(bio, compressed_decoded, sizeof(compressed_decoded) - 1);
        if (compressed_decoded_len > 0) {
            compressed_decoded[compressed_decoded_len] = '\0';
            printf("[+]> Decoded string: %s\n", compressed_decoded);
        } else {
            perror("[!]> ERROR: Base64 decoding failed");
            return -1;
        }
        BIO_free_all(bio);
    
        char decompressed[2048];
        uLongf decompressed_len = sizeof(decompressed);

        if (uncompress(decompressed, &decompressed_len, compressed_decoded, compressed_decoded_len) == Z_OK) {
        decompressed[decompressed_len] = '\0';
        printf("[+]> Decompressed string: %s\n", decompressed);
        } else {
            perror("[!]> ERROR: zlib error");
            return -1;
        }

        char sendbuffer[256];
        snprintf(sendbuffer, sizeof(sendbuffer), "%s\n", decompressed);
        ssize_t sendcode = send(sockfd, sendbuffer, strlen(sendbuffer), 0);
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
