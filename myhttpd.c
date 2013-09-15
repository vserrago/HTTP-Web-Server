#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

#define PORTPARAM p
#define DEBUGPARAM d


int main(int argc, char *argv [])
{
    int i;  //Loop Var

    for(i=1; i<argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    exit(0);
}
