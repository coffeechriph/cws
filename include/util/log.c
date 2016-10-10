#include "log.h"
FILE *logFile;

void cws_log_print(const char *fmt, i32 LINE, const char *FUNC, ...)
{
	va_list args;
	va_start(args, FUNC);
	
	if(logFile)
	{
		fprintf(logFile, "%d [%s]: ", LINE, FUNC);
		vfprintf(logFile, fmt, args);
		fprintf(logFile, "\n");
		fflush(logFile);
	}

	va_end(args);
}

bool cws_log_open(const char *file)
{
	//Make sure to clear up any old files
	remove(file);

	logFile = fopen(file, "a");
	if(logFile == NULL)
	{
		cws_log("Can't open file %s", file);
		return false;
	}

	return true;
}

bool cws_log_close()
{
	fflush(logFile);
	int ret = fclose(logFile);
	if(ret < 0)
	{
		cws_log("Can't close log file!");
		return false;
	}

	return true;
}