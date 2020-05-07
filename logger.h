#ifndef LOGS_H
#define LOGS_H

#include <string.h>
#include <fstream>

#define logfileNameLength 20

class Logger
{
	public:
		Logger();

		Logger(char *filename);

		~Logger();

		void StartLogfile();
		void EndLogfile();
		void Log(char *msg);
		void Log(const char *msg);

	private:
		std::ofstream logfile;

		char logfileName[logfileNameLength];
};

#endif
