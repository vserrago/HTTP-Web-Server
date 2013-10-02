#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
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
#include "queue.h"
#include "util.h"

//TODO pass thread dependencies in argument

//Global config vars
configuration* config;      //Config struct var
queue* q;                   //Connection queue

//Semaphores
sem_t semconfig;              //Limits access to config var
sem_t semq;                   //Limits access to connection queue
sem_t semtasks;               //Keeps count on number of unhandled tasks


//Thread function
void* handlecon(void* args)
{
    //threadargs* a = args;
    int connectfd;  //Connection file descriptor

    for(;;)
    {
        //Wait for a task to come in
        sem_wait(&semtasks);

        servdeblog("Task handled by thread\n");

        //Lock queue
        sem_wait(&semq);
        {
            servdeblog("Peek value: %d\n",qpeek(q));
            //Get connection file descriptor
            connectfd = qrem(q);
        }
        //Unlock queue
        sem_post(&semq);

        //Get server request in string format
        char* reqstr = recievereq(connectfd);

        //Parse server request from string
        request* req = parsereq(reqstr);

        response* resp;

        //Lock config
        sem_wait(&semconfig);
        {
            //Generate a response based on the request and its validity
            resp = handlereq(req, config);
        }
        //Unlock config
        sem_post(&semconfig);

        //Send the response back to the client
        sendresp(connectfd,resp);

        //Free connection info
        free(reqstr);
        freereq(req);
        freeresp(resp);

        //Close connection
        close(connectfd);
    }
    pthread_exit(0);
}

int main(int argc, char *argv [])
{
    //Server Config Vars
    char* port     = DEFAULTPORT;       //Port to listen in on 
    char* address  = DEFAULTADDRESS;    //Address of server
    char* confname = DEFAULTCONFNAME;   //Name of config file

    //Server structs
    stserver* serv;                     //Server var

    //Thread vars
    pthread_t* pthrarr;                 //pthread array

    //Getopt vars
    int opt;                            //Option element to be returned by getopt
    int longindex;                      //Used by getopt_long
    int portnum;                        //Used to check numeric value of port

    //Verbose options for getopt
    struct option long_options[] = 
    {
        //Name      Arg requirement     Flag  Value
        {"address", required_argument,  NULL, 'a'},
        {"config",  required_argument,  NULL, 'c'},
        {"debug",   no_argument,        NULL, 'd'},
        {"port",    required_argument,  NULL, 'p'}
    };

    //Parse arguments
    while((opt = getopt_long(argc,argv,"a:c:dp:", long_options, &longindex)) != -1)
    {
        switch(opt)
        {
            //Address option
            case 'a':
                address = optarg;
                servdeblog("Address set with value '%s'\n", address);
                break;
            //Config file option
            case 'c':
                confname = optarg;
                servdeblog("Configuration file set with value '%s'\n", confname);
                break;
            //Debug option
            case 'd':
                debugflag = 1;
                servdeblog("Debug mode enabled\n");
                break;
            //Port option
            case 'p':
                port = optarg;
                portnum = atoi(port); //Convert port to numeric value
                //Ensure that portnumber is in the range of useable ports
                if(portnum < MINPORTNUM || MAXPORTNUM < portnum)
                    exiterr("Portnumber is invalid\n");
                servdeblog("Port set with value '%s'\n", port);
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

    //Create queue
    q = createqueue(config->queuesize);

    //Create, bind and listen on port
    prepserv(serv); 

    //Create semaphores
    sem_init(&semconfig, 0, 1); //Binary semaphore
    sem_init(&semq, 0, 1);      //Also binary
    sem_init(&semtasks, 0, 0);  //Represents the amount of tasks in queue

    //Create threadpool
    pthrarr = malloc(config->poolsize * sizeof(pthread_t));

    int i;
    pthread_t pthr;
    for(i=0; i < config->poolsize; i++)
    {
        //Create thread and add it to pool
        pthread_create(&pthr,NULL, handlecon, 0);
        pthrarr[i] = pthr;

        servdeblog("Thread %d created\n", i);
        //TODO free args
    }

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

        //Lock queue
        sem_wait(&semq);
        {
            //Add connection to queue
            qadd(q,c);

            servdeblog("Connection '%d' added to queue\n",c);
            servdeblog("Nextout: %d\n",qpeek(q));
            //servdeblog("Removed value: %d\n",qrem(q));

            //Increment task count
            sem_post(&semtasks);
        }
        //Unlock queue
        sem_post(&semq);

        //break; //Break for testings sake
    }

    //Finish Up
    close(serv->sock); //Close socket

    //Free server structs
    freeconf(config);
    freestserv(serv);

    //Exit
    exit(0);
}
