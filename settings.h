#ifndef SETTINGS_H
#define SETTINGS_H

#include <string.h>
#include <fstream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <iostream>

#include "utils.h"

#define settingsFileNameLength 20

class Settings
{
	public:
		Settings();

		Settings(char *filename);

		~Settings();

		void StartSettingsFile();
		void InitializeSettings();
		void EndSettingsFile();

		void SaveSettings();

	private:
		bool settingsFileExists = false;
		std::ifstream *settingsFile;

		char settingsFileName[settingsFileNameLength];
};

#endif
