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

//Constants
#define MINPORTNUM 60000
#define DEFAULTPORTNUM 61000

//Struct definitions
typedef struct
{
    char* httpver;
    char* rootdir;
    char** extentions;
} configuration;


//Global Vars
int sock;                           //Socket file descriptor
char* port = "61000";               //TODO un-hardcode this!
char* address = "localhost";        //TODO unhardcode this too!
unsigned char debugflag =0;         //Whether debug logging is enabled or not
char* confname = "myhttpd.conf";    //Name of config file

configuration* config;              //Config struct var


//Function prototypes
configuration* genconf();
configuration* parseconf();
void exiterr(const char* format, ...);
void exitperr(const char* format, ...);
void servlog(const char* format, ...);
void servdeblog(const char* format, ...);
