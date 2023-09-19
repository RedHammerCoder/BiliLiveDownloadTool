#include "ErrorLog.h"

void SetErrorLogPath(const std::string &path)
{
    ERRORLOGPATH =path;
    ERRORLOGPATH+="/Errlog.txt";
    fprintf(stderr , "ERROR log addr : %s \n",ERRORLOGPATH.c_str());
    ERRLOG.SetErrorLog();
}


ErrorLog ERRLOG;
std::string ERRORLOGPATH;