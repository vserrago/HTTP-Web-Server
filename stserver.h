#include <stdarg.h>

//Constants
#define MINPORTNUM 60000
#define MAXPORTNUM 65535
#define DEFAULTPORTNUM 61000
#define DEFAULTADDRESS "localhost"
#define DEFAULTPORT "61000"
#define DEFAULTCONFNAME "myhttpd.conf"

#define ST200 "HTTP/1.0 200 OK\r\n"
#define ST201 "HTTP/1.0 201 File created successfully\r\n"
#define ST400 "HTTP/1.0 400 Bad request\r\n"
#define ST403 "HTTP/1.0 403 No read permissions\r\n"
#define ST404 "HTTP/1.0 404 Not found\r\n"
#define ST501 "HTTP/1.0 501 Not implemented\r\n"

#define CONTYPETEXT "Content-Type: text/html\r\n"

#define HTML201 "statuspages/st201.htm"
#define HTML400 "statuspages/st400.htm"
#define HTML403 "statuspages/st403.htm"
#define HTML404 "statuspages/st404.htm"
#define HTML501 "statuspages/st501.htm"

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
    unsigned char badreq; //Flag for if request is not valid
    char* reqtype;  //Requested type, Ex GET
    char* reqfile;  //Requested file
    char* httpver;  //HTTP version
    char* content;  //Content from post
    int   contlen;  //Length of content
}request;

typedef struct response
{
    char* status;       //Status line, ex HTTP/1.0 200 OK
    char* date;         //Date
    char* contype;      //Content-Type
    char* contlenstr;   //Content-length string
    char* content;      //Content to return as a string
    int   contlen;      //Content-length
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
void prepserv(stserver* serv);
char* recievereq(int sockfd);
request* parsereq(char* reqstr);
response* handlereq(request* req, configuration* config);
void sendresp(int sockfd, response* resp);
char* getdate(void);

//Exit functions
void exiterr(const char* format, ...);  //Exit with a given error message
void exitperr(const char* format, ...); //Call perror and then exit

//Log functions
void servlog(const char* format, ...);  //Log status of server
void servdeblog(const char* format, ...); //Log debug info
