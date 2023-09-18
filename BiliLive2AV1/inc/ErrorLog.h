#pragma once
#include <string>
#include <stdio.h>

extern  std::string ERRORLOGPATH;



void SetErrorLogPath(std::string &&path);

class ErrorLog
{
private:
    /* data */
public:
    FILE * Handle;
    const std::string LogPath;
    ErrorLog()
    {
        FILE * Handle =fopen(ERRORLOGPATH.c_str(),"w+");
    }
    ~ErrorLog()
    {
        fclose(Handle);
    }
};

#define ERRLOGHandle  ERRLOG.Handle;
extern ErrorLog ERRLOG;
