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

//Function prototypes
configuration* genconf(void);
configuration* parseconf(char * confname);
void exiterr(const char* format, ...);
void exitperr(const char* format, ...);
void servlog(const char* format, ...);
void servdeblog(const char* format, ...);
