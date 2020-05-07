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
	this->EndSettingsFile();
	delete settingsFile;
}

void Settings::StartSettingsFile()
{
	Json::Value root;
	std::ifstream file("test.json");
	file >> root;
	std::cout << root["key"].asString() << "\n";
}

void Settings::InitializeSettings()
{
	if (!this->settingsFileExists)
	{
		// Read the file
	}
	else
	{
		// Initialize the default settings locally
		// Call save
	}
}

void Settings::EndSettingsFile()
{
	this->settingsFile->close();
}

void Settings::SaveSettings()
{
}
