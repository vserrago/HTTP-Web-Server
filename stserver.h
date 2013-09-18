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



//Function prototypes
request* genreq(void);

configuration* genconf(void);
configuration* parseconf(char * confname);
void exiterr(const char* format, ...);
void exitperr(const char* format, ...);
void servlog(const char* format, ...);
void servdeblog(const char* format, ...);
