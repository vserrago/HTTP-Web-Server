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
char* port     = DEFAULTPORT;       //Port to listen in on 
char* address  = DEFAULTADDRESS;    //Address of server
char* confname = DEFAULTCONFNAME;   //Name of config file


int main(int argc, char *argv [])
{
    int i, j;  //Loop Variables
    unsigned char portflag; //Command line param flags
    int opt; //Option element to be returned by getopt

    //Server structs
    stserver* serv;                     //Server var
    configuration* config;              //Config struct var

    //Create server struct
    serv = allocstserv();

    //Parse arguments
    while((opt = getopt(argc,argv,"dp:")) != -1)
    {
        switch(opt)
        {
            case 'd':
                debugflag = 1;
                servdeblog("Debug mode enabled\n");
                break;
            case 'p':
                port = optarg;
                int portnum = atoi(port);
                //Ensure that portnumber is in the range of useable ports
                if(portnum < MINPORTNUM || MAXPORTNUM < portnum)
                    exiterr("Portnumber is invalid\n");
                servdeblog("Port flag set with given port %s\n", optarg);
                break;
            case '?': //Character for unknown arg
            default:
                //Use exit function as error message is already printed by getopt
                exit(EXIT_FAILURE);
        }
    }

    //Set server vars
    serv->port     = cpynewstr(port);
    serv->address  = cpynewstr(address);
    serv->confname = cpynewstr(confname);

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

        char* reqstr = recievereq(c); //Get resquest in a string format

        request* req = parsereq(reqstr);

        response* resp = handlereq(req, config);

        sendresp(c,resp);

        //TODO free everything
        free(reqstr);
        freereq(req);
        freeresp(resp);

        //Close connection
        close(c);
        break;
    }

    //Finish Up
    close(serv->sock); //Close socket

    //Free server structs
    freeconf(config);
    freestserv(serv);
    exit(0);
}
