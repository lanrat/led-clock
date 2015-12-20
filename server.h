#ifndef SERVER_H
#define SERVER_H

#define CONNMAX 100
#define SERVER_BUFFER_SIZE 32768
#define PORT "80"

void urldecode2(char *dst, const char *src);
void setupServer(void (*fn)(char*));

#endif