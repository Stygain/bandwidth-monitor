#include "settings.h"

Settings *settings = new Settings();


Settings::Settings()
{
	strcpy(this->settingsFileName, "~/.bwmon");

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
	if (!this->settingsFileExists)
	{
		std::ifstream settingsFile("test.json");

		// Read the file
		settingsFile >> this->root;

		settingsFile.close();
	}
	else
	{
		// Initialize the default settings locally
		this->root["sortingColumn"] = -1;
		this->root["zeroOnStart"] = 0;

		// Call save
	}
}

void Settings::SaveSettings()
{
	extern Logger *logger;

	std::ofstream settingsFile("test.json");

	Json::StyledWriter writer;
	logger->Log(writer.write(this->root).c_str());
	settingsFile << writer.write(this->root).c_str();
	settingsFile.close();
}
