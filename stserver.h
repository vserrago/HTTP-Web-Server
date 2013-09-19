#include <stdarg.h>

//Constants
#define MINPORTNUM 60000
#define DEFAULTPORTNUM 61000

//Struct definitions
typedef struct
{
    char* httpver;
    char* rootdir;
    char** extentions;
    int nextentions;
} configuration;

typedef struct
{
    char* reqtype;  //Requested type, Ex GET
    char* reqfile;  //Requested file
    char* httpver;  //HTTP version
}request;

typedef struct
{
    char* status;   //Status line, ex HTTP/1.0 200 OK
    char* date;
    char* contype;  //Content-Type
    int  contlen;  //Content-length
}response;


//Function prototypes
request* genreq(void);
response* genresp(void);

configuration* genconf(void);
configuration* parseconf(char * confname);
void exiterr(const char* format, ...);
void exitperr(const char* format, ...);
void servlog(const char* format, ...);
void servdeblog(const char* format, ...);
