# rootme programmation

[Root-Me Programming Challenges](https://www.root-me.org/en/Challenges/Programming/)

## TCP - Back to school

[Challenge Link](https://www.root-me.org/en/Challenges/Programming/TCP-Back-to-school)

 - We have to create a TCP socket to **challenge01.root-me.org:52002**
  - after the connection we calculte the number 1 and 2 and round the result to two decimal place
  - and then send the result two second after receive the two number

##### compilation

```bash
gcc -Wall -Wextra -O2 -o rootme1 rootme1.c -lm
```

## TCP - Encoded string

[Challenge Link](https://www.root-me.org/en/Challenges/Programming/TCP-Encoded-string)

- We have to create a TCP socket to challenge01.root-me.org:52023
  - after the connection we decode the encoded string
  - adn then send the decoded string two second after receive the encoded string

##### compilation

install the library: libssl-dev

```bash
gcc -Wall -Wextra -O2 -o rootme2 rootme2.c -lssl -lcrypto
```
