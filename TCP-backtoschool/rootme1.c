#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <math.h>

int main() {
  const char *host = "challenge01.root-me.org";
  const char *port = "52002";

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

  char *calc_line = strstr(recvbuffer, "Calculate");
  if (!calc_line) {
    fprintf(stderr, "[!]> ERROR: calculation line not found\n");
    return -1;
}

  int num1, num2;
  if (sscanf(calc_line, "Calculate the square root of %d and multiply by %d", &num1, &num2) != 2) {
    fprintf(stderr, "[!]> ERROR: unable to parse numbers\n");
    return -1;
  }

  double result = sqrt(num1) * num2;
  result = round(result * 100.0) / 100.0;
  printf("[+]> Calculus: %.2f\n", result);

  char sendbuffer[64];
  int len = snprintf(sendbuffer, sizeof(sendbuffer), "%.2f\n", result);
  ssize_t sendcode = send(sockfd, sendbuffer, len, 0);
  if (sendcode < 0) {
      perror("[!]> ERROR: send");
  } else {
      printf("[+]> Sent %zd bytes\n", sendcode);
  }

  char finalbuf[2048];
  ssize_t finalcode = recv(sockfd, finalbuf, sizeof(finalbuf) - 1, 0);
  if (finalcode > 0) {
    finalbuf[finalcode] = '\0';
    printf("[+]> Final server answer:\n    %s\n", finalbuf);
  } else if (finalcode == 0) {
    printf("[-]> Server closed connection.\n");
  } else {
    perror("[!]> ERROR: recv final");
  }

  close(sockfd);
  freeaddrinfo(res);
  return 0;
}
