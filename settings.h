#ifndef SETTINGS_H
#define SETTINGS_H

#include <string.h>
#include <fstream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "interface.h"
#include "logger.h"
#include "utils.h"

#define settingsFileNameLength 20

class Settings
{
	public:
		Settings();

		Settings(char *filename);

		~Settings();

		void Initialize();

		void SaveSettings();

	public:
		Json::Value root;

	private:
		bool settingsFileExists = false;

		char settingsFileName[settingsFileNameLength];
};

#endif
