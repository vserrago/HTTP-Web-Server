#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define PORTPARAM p
#define DEBUGPARAM d


int main(int argc, char *argv [])
{
    int i, j;  //Loop Variables

    for(i=1; i<argc; i++)
    {
        printf("%s\n", argv[i]);
        if(argv[i][0] == '-')
        {
            int len = strlen(argv[i]);
            for(j=1; j<len; j++)
            {
                switch(argv[i][j])
                {
                    case 'p': 
                        printf("p param given\n");
                        break;
                    case 'd':
                        printf("d param given\n");
                        break;
                    default:
                        printf("Unkown param given\n");
                        break;
                }
            }
        }
    }
    exit(0);
}
