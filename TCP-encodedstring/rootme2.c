#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <netdb.h>
#include <errno.h>

int main() {
  const char *host = "challenge01.root-me.org";
  const char *port = "52023";

  const BIO_METHOD *BIO_f_base64(void);

  struct addrinfo hints, *res;
  int status, sockfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(host, port, &hints, &res);
  if (status != 0) {
    fprintf(stderr, "[!]> ERROR: %s\n", gai_strerror(status));
    return -1;
  }

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("[!]> ERROR: socket");
    return -1;
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
    perror("[!]> ERROR: connect");
    return -1;
  }

  printf("[+]> Connected to %s:%s\n", host, port);

  char recvbuffer[2048];
  ssize_t recvcode = recv(sockfd, recvbuffer, sizeof(recvbuffer) - 1, 0);
  if (recvcode < 0) {
    perror("[!]> ERROR: recv");
  } else if (recvcode == 0) {
    printf("[-]> Server closed connection\n");
  } else {
    recvbuffer[recvcode] = '\0';
    printf("[+]> Server send: %s\n", recvbuffer);
  }
  
  const char *encoded_line = strstr(recvbuffer, "my string");
  if (!encoded_line) {
    fprintf(stderr, "[!]> ERROR: encoded line not found\n");
    return -1;
  }

  char encoded[512];
  if (sscanf(encoded_line, "my string is '%511[^']'. What is your answer ?", encoded) != 1) {
    fprintf(stderr, "[!]> ERROR: unable to parse encoded string\n");
    return -1;
  } else {
    printf("[+]> Encoded string: %s\n", encoded);
  }

  char decoded[512];
  BIO *b64, *bio;
  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  bio = BIO_new_mem_buf(encoded, -1);
  bio = BIO_push(b64, bio);

  int decodedlen = BIO_read(bio, decoded, sizeof(decoded) -2);
  if (decodedlen > 0) {
    decoded[decodedlen++] = '\n',
    decoded[decodedlen] = '\0';
    printf("[+]> Decoded string: %s\n", decoded);
  } else {
    fprintf(stderr, "[!]> ERROR: base64 decode failed\n");
  }
  BIO_free_all(bio);

  ssize_t sendcode = send(sockfd, decoded, decodedlen, 0);
  if (sendcode < 0) {
    perror("[!]> ERROR: send");
  } else {
    printf("[+]> Qent %zd bytes\n", sendcode);
  }

  char finalbuffer[2048];
  ssize_t finalcode = recv(sockfd, finalbuffer, sizeof(finalbuffer) - 1, 0);
  if (finalcode > 0) {
    finalbuffer[finalcode] = '\0';
    printf("[+]> Final server answer:\n    %s\n", finalbuffer);
  } else if (finalcode == 0) {
    printf("[-]> Server closed connection\n");
  } else {
    perror("[!]> ERROR: recv final");
  }
  
  close(sockfd);
  freeaddrinfo(res);
  return 0;
}
    
