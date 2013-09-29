#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>
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


int main(int argc, char *argv [])
{
    //Server Config Vars
    char* port     = DEFAULTPORT;       //Port to listen in on 
    char* address  = DEFAULTADDRESS;    //Address of server
    char* confname = DEFAULTCONFNAME;   //Name of config file

    //Server structs
    stserver* serv;                     //Server var
    configuration* config;              //Config struct var

    //Getopt vars
    int opt;                            //Option element to be returned by getopt
    int longindex;                      //Used by getopt_long

    //Verbose options for getopt
    struct option long_options[] = 
    {
        //Name      Arg requirement     Flag  Value
        {"debug",   no_argument,        NULL, 'd'},
        {"port",    required_argument,  NULL, 'p'}
    };

    //Parse arguments
    while((opt = getopt_long(argc,argv,"dp:", long_options, &longindex)) != -1)
    {
        switch(opt)
        {
            //Debug option
            case 'd':
                debugflag = 1;
                servdeblog("Debug mode enabled\n");
                break;
            //Port option
            case 'p':
                port = optarg;
                int portnum = atoi(port);
                //Ensure that portnumber is in the range of useable ports
                if(portnum < MINPORTNUM || MAXPORTNUM < portnum)
                    exiterr("Portnumber is invalid\n");
                servdeblog("Port flag set with given port %s\n", optarg);
                break;
            //Unkown option
            case '?': 
            default:
                //Use exit function as error message is already printed by getopt
                exit(EXIT_FAILURE);
        }
    }

    //Create server struct
    serv = allocstserv();

    //Set server vars
    serv->port     = cpynewstr(port);
    serv->address  = cpynewstr(address);
    serv->confname = cpynewstr(confname);

    servdeblog("Server Address: '%s', Port: '%s', Confname: '%s'\n", 
            serv->address, serv->port, serv->confname);

    //Parse configuration file and get info from it.
    config = parseconf(confname);

    //Create, bind and listen on port
    prepserv(serv); 

    //Connection info structs
    struct sockaddr_storage socket_st;
    socklen_t addr_size = sizeof socket_st;

    servlog("Ready to recieve\n");

    for(;;) //Forever
    {
        //Accept connection
        int c = accept(serv->sock, (struct sockaddr *) &socket_st, &addr_size);

        if(c < 0)
            exiterr("Accept error\n");

        //Get server request in string format
        char* reqstr = recievereq(c); 

        //Parse server request from string
        request* req = parsereq(reqstr);

        //Generate a response based on the request and its validity
        response* resp = handlereq(req, config);

        //Send the response back to the client
        sendresp(c,resp);

        //Free connection info
        free(reqstr);
        freereq(req);
        freeresp(resp);

        //Close connection
        close(c);

        break; //Break for testings sake
    }

    //Finish Up
    close(serv->sock); //Close socket

    //Free server structs
    freeconf(config);
    freestserv(serv);

    //Exit
    exit(0);
}
