#include <stdarg.h>

//Constants
#define MINPORTNUM 60000
#define DEFAULTPORTNUM 61000
#define DEFAULTPORT "61000"
#define DEFAULTCONFNAME "myhttpd.conf"

//Global Vars
unsigned char debugflag;    //Whether debug logging is enabled or not

//Struct definitions
typedef struct //stserver
{
    int   sock;                 //Socket file descriptor
    char* port;                 //Port to listen on
    char* address;              //Ip address of the server
    char* confname;             //Name of config file
}stserver;

typedef struct //configuration
{
    char* httpver;
    char* rootdir;
    char** extentions;
    int nextentions;
} configuration;

typedef struct //request
{
    char* reqtype;  //Requested type, Ex GET
    char* reqfile;  //Requested file
    char* httpver;  //HTTP version
}request;

typedef struct response
{
    char* status;   //Status line, ex HTTP/1.0 200 OK
    char* date;
    char* contype;  //Content-Type
    char* contlenstr;  //Content-length string
    int  contlen;  //Content-length
}response;


//Struct Allocation and Free functions
stserver* allocstserv(void);
void freestserv(stserver* s);

request* allocreq(void);
void freereq(request* r);

response* allocresp(void);
void freeresp(response* r);

configuration* allocconf(void);
void freeconf(configuration* c);

//Server functions
configuration* parseconf(char * confname);
void exiterr(const char* format, ...);  //Exit with a given error message
void exitperr(const char* format, ...); //Call perror and then exit
void servlog(const char* format, ...);  //Log status of server
void servdeblog(const char* format, ...); //Log debug info
