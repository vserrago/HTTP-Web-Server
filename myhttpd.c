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

//Local includes
#include "stserver.h"
#include "util.h"


//Server Config Vars
char* port = DEFAULTPORT;           //Port to listen in on 
char* address = "localhost";        //Address of server
char* confname = DEFAULTCONFNAME;   //Name of config file

stserver* serv;                     //Server var
configuration* config;              //Config struct var

int main(int argc, char *argv [])
{
    int i, j;  //Loop Variables
    unsigned char portflag; //Command line param flags

    //Create server struct
    serv = allocstserv();

    /* Arg Parsing */
    for(i=1; i<argc; i++)
    {
        servdeblog("Param: %s\n", argv[i]);
        if(argv[i][0] == '-')
        {
            int len = strlen(argv[i]);
            for(j=1; j<len; j++)
            {
                switch(argv[i][j])
                {
                    case 'p': 
                        servdeblog("p param given\n");
                        portflag = 1;
                        break;
                    case 'd':
                        servdeblog("d param given\n");
                        debugflag = 1;
                        break;
                    default:
                        exiterr("Unkown param given\n");
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
                exiterr("Portnumber is invalid\n");
            }
            servdeblog("Portnum is %s\n", port);
        }
        else
        {
            exiterr("Unkown arg given\n");
        }
    }

    //Set server vars
    serv->port = port;
    serv->address = address;
    serv->confname = confname;

    servdeblog("Server Address: '%s', Port: '%s', Confname: '%s'\n", serv->address, 
            serv->port, serv->confname);

    config = parseconf(confname);   //Parse configuration file and get info from it.

    prepserv(serv); //Create, bind and listen on port

    struct sockaddr_storage socket_st;
    socklen_t addr_size = sizeof socket_st;

    servlog("Ready to recieve\n");
    for(;;) //Forever
    {
        int c = accept(serv->sock, (struct sockaddr *) &socket_st, &addr_size);

        if(c < 0)
        {
            exiterr("Accept error\n");
        }

        for(;;)
        {
            int mbs = 2048; //Maximum buffer size, size of 2KiB
            char  buff [mbs];
            int bytesrecieved = recv(c,buff, mbs,0);

            if(bytesrecieved < 0)
            {
                exiterr("Recieve Error\n");
            }

            if(bytesrecieved == 0)
            {
                exiterr("Connection Closed\n");
            }

            servdeblog("%d bytes recieved, Request:\n", bytesrecieved);
            servdeblog("%s\n",buff);

            request* r = allocreq();
            char* token;

            int n;

            //Parse header request
            if((token = strtok(buff," ")) == NULL)
            {
                exiterr("Buff Token error\n");
            }
            r->reqtype = cpynewstr(token); 

            if((token = strtok(NULL," ")) == NULL)
            {
                exiterr("Buff Token error\n");
            }
            r->reqfile = cpynewstr(token); 

            if((token = strtok(NULL," \r\n")) == NULL)
            {
                exiterr("Buff Token error\n");
            }
            r->httpver = cpynewstr(token); 

            servdeblog("Reqtype: '%s', Reqfile: '%s', httpver: '%s'\n",  r->reqtype, r->reqfile, r->httpver);


            //Look for header ending 
            for(i=1;i<mbs;i++)
            {
                if(buff[i] == '\r' && buff[i-1] == '\n')
                {
                    servdeblog("CLRF WAS found\n");
                    break;
                }
            }
            //Check to see if loop parsed through entire buffer
            if(i == mbs-1)
            {
                exiterr("CLRF not found\n");
            }

            //n = (strlen(config->rootdir) + strlen(r->reqfile) + 1)
            n = strlen(config->rootdir);
            n += strlen(r->reqfile) + 2;
            //Combine paths
            char* abspath = malloc(
                    n*sizeof(char));
            strcpy(abspath, config->rootdir);
            strcat(abspath, r->reqfile);

            servdeblog("Abspath: '%s'\n", abspath);

            FILE* f = fopen(abspath, "r"); //Open file for reading
            if(f == NULL)
            {
                exitperr("fopen");
            }

            response* resp = allocresp();
            resp->status = cpynewstr("HTTP/1.0 200 OK\r\n");
            resp->date = cpynewstr("Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
            resp->contype= cpynewstr("Content-Type: text/html\r\n");
            resp->contlenstr= malloc(50*sizeof(char));

            //Get length of file
            resp->contlen = filesize(f);

            sprintf(resp->contlenstr, "Content-Length %d", resp->contlen);

            servdeblog("Content Length: %d\n", resp->contlen);
            servdeblog("Content Length String: %s\n", resp->contlenstr);

            //Get length of response total
            n = strlen(resp->status) + strlen(resp->date) + strlen(resp->contype) + strlen(resp->contlenstr) + 1 + resp->contlen;

            //Make response buffer
            char* respbuff = malloc(n*sizeof(char));
            int buffsize = n;

            strcpy(respbuff, resp->status);
            strcat(respbuff, resp->date);
            strcat(respbuff, resp->contype);
            strcat(respbuff, resp->contlenstr);
            strcat(respbuff, "\r\n");
            strcat(respbuff, "\r\n");


            n = strrchr(respbuff, '\0') - respbuff;

            servdeblog("Header: '%s'\n", respbuff);
            servdeblog("Last Occurrance of null char: %d\n", n);

            //Copy file into buffer
            for(i=n;(respbuff[i] = getc(f))!=EOF; i++);


            servdeblog("Buffer: '%s\n'", respbuff);

            int a = 0;
            for(n=0;(n += send(c, respbuff+a*sizeof(char) ,buffsize,0)) != 0;a+=n)
                ; 

            servlog("Bytes sent: %d\n", n);

            break;
        }
        close(c);
        break;
    }

    //Finish Up
    //close(serv->sock);
    //freeaddrinfo(servinfo); //Free address info
    exit(0);
}
