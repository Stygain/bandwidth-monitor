#include "logger.h"

Logger *logger = new Logger();


Logger::Logger()
{
	strcpy(this->logfileName, "logs");
}

Logger::Logger(char *filename)
{
	strncpy(this->logfileName, filename, logfileNameLength - 1);
}

Logger::~Logger()
{
	this->EndLogfile();
}

void Logger::StartLogfile()
{
	this->logfile.open(this->logfileName);
}

void Logger::EndLogfile()
{
	this->logfile.close();
}

void Logger::Log(char *msg)
{
	logfile << msg;
}
