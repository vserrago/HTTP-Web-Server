#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MINPORTNUM 60000
#define DEFAULTPORTNUM 61000

char* confname = "myhttpd.conf";

//Global Vars
int sock;  //Socket file descriptor
char * port = "61000"; //TODO un-hardcode this!
char * address = "localhost"; //TODO unhardcode this too!
char debugflag;

typedef struct
{
    int httpver;
    char * rootdir;
    char ** extentions;
} configuration;

configuration* config; //Config struct var

configuration* parseconf()
{
    configuration* c = malloc(sizeof(configuration));   //Create struct
    
    //Set each value to a defualt value
    c->httpver = 0;
    c->rootdir = NULL;
    c->extentions = NULL;

    
    FILE* f = fopen(confname, "r"); //Open file for reading

    if(f == NULL)
    {
        //printf("fopen error\n");
        perror("fopen");
        exit(0);
    }

    char s [100];

    fgets(s,100,f);
    printf("DirPath Line: %s", s);

    fgets(s,100,f);
    printf("File Line: %s", s);

    if(fclose(f) !=0) //Close file and make sure it closes properly
    {
        printf("File close error\n");
        exit(0);
    }

    return c;
}

int main(int argc, char *argv [])
{
    int i, j;  //Loop Variables
    unsigned char portflag; //Command line param flags
    struct sockaddr_storage socket_st;
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;

    /* Arg Parsing */
    for(i=1; i<argc; i++)
    {
        printf("Param: %s\n", argv[i]);
        if(argv[i][0] == '-')
        {
            int len = strlen(argv[i]);
            for(j=1; j<len; j++)
            {
                switch(argv[i][j])
                {
                    case 'p': 
                        printf("p param given\n");
                        portflag = 1;
                        break;
                    case 'd':
                        printf("d param given\n");
                        debugflag = 1;
                        break;
                    default:
                        printf("Unkown param given\n");
                        exit(1);
                        break;
                }
            }
        }
        else if(portflag)
        {
            port = argv[i];
            int portnum = atoi(port);
            //Ensure that portnumber is in the range of useable ports
            if(portnum < MINPORTNUM || USHRT_MAX < portnum)
            {
                printf("Portnumber is invalid\n");
                exit(3);
            }
            printf("Portnum is %s\n", port);
        }
        else
        {
            printf("Unkown arg given\n");
            exit(2);
        }
    }

    config = parseconf();


    /* Socket Sutff */
    memset(&hints, 0, sizeof hints);    //Wipe space for hints
    hints.ai_family = AF_UNSPEC;        //Don't specify IP type
    hints.ai_socktype = SOCK_STREAM;    //Use TCP socket
    hints.ai_flags = AI_PASSIVE;        //Figure out ip


    if ((status = getaddrinfo(address, port, &hints, &servinfo)) != 0)
    {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sock == -1)
    {
        printf("Socket error\n");
        exit(0);
    }

    if(bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0)
    {
        printf("binderror\n");
        exit(0);
    }

    int backlog = 10;

    if(listen(sock, backlog) != 0)
    {
        printf("Listen error\n");
        exit(0);
    }

    socklen_t addr_size = sizeof socket_st;

    printf("Ready to recieve\n");
    for(;;) //Forever
    {
        int c = accept(sock, (struct sockaddr *) &socket_st, &addr_size);

        if(c < 0)
        {
            printf("Accept error\n");
            exit(0);
        }

        for(;;)
        {
            char  buff [1000];
            int bytesrecieved = recv(c,buff, 1000,0);

            if(bytesrecieved < 0)
            {
                printf("Recieve Error\n");
                exit (0);
            }

            if(bytesrecieved == 0)
            {
                printf("Connection Closed\n");
                exit(0);
            }

            printf("%d bytes recieved\n", bytesrecieved);

            printf("Message Recieved: %s", buff);
            if (send(c, "Hello, world!", 13, 0) == -1)
                perror("send");
            exit(0);
        }
    }


    freeaddrinfo(servinfo); //Free address info
    exit(0);
}
