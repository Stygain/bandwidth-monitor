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
		extern InterfaceHeader *interfaceHeader;
		extern std::vector<Interface *> interfaces;
		extern Logger *logger;

		// Read the file
		settingsFile >> this->root;

		logger->Log(this->root["sortingColumn"].asString().c_str());
		logger->Log("\n");
		int sortByIndex = this->root["sortingColumn"].asInt();
		if (interfaceHeader != NULL)
		{
			interfaceHeader->sortingHeader = sortByIndex;
			interfaceHeader->Print();
		}

		logger->Log(this->root["zeroOnStart"].asString().c_str());
		logger->Log("\n");
		int zeroOnStart = this->root["zeroOnStart"].asInt();
		if (zeroOnStart)
		{
			for (size_t i = 0; i < interfaces.size(); ++i)
			{
				interfaces[i]->Zero();
				interfaces[i]->Print();
			}
		}
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

	std::ofstream settingsFile("test2.json");

	Json::StyledWriter writer;
	logger->Log(writer.write(this->root).c_str());
	settingsFile << writer.write(this->root).c_str();
	settingsFile.close();
}
