#include "settings.h"

Settings *settings = new Settings();


Settings::Settings()
{
	char *homedir;
	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}
	const char *filename = "/.bwmon";
	strcat(homedir, filename);
	strcpy(this->settingsFileName, homedir);

	this->settingsFileExists = fileExists(this->settingsFileName);
}

Settings::Settings(char *filename)
{
	strncpy(this->settingsFileName, filename, settingsFileNameLength - 1);

	this->settingsFileExists = fileExists(this->settingsFileName);
}

Settings::~Settings()
{
}

void Settings::InitializeSettings()
{
	if (this->settingsFileExists)
	{
		std::ifstream settingsFile(this->settingsFileName);

		// Read the file
		settingsFile >> this->root;

		settingsFile.close();
	}
	else
	{
		// Initialize the default settings locally
		this->root["sortingColumn"] = -1;
		this->root["zeroOnStart"] = 1;
		this->root["graphs"][0] = 0;
		this->root["graphs"][1] = 1;
		this->root["graphs"][2] = 2;
		this->root["graphs"][3] = 3;

		// Call save
		this->SaveSettings();
	}
}

void Settings::SaveSettings()
{
	extern Logger *logger;

	std::ofstream settingsFile(this->settingsFileName);

	Json::StyledWriter writer;
	logger->Log(writer.write(this->root).c_str());
	settingsFile << writer.write(this->root).c_str();
	settingsFile.close();
}
