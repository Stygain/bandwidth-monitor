#include "utils.h"


void getModeString(Mode mode, char *modeString)
{
	strcpy(modeString, modeStrings[mode]);
}

void getGraphTypeString(GraphType gt, char *graphTypeString)
{
	strcpy(graphTypeString, graphTypeStrings[gt]);
}

void getSettingsWindowOptionString(SettingsWindowOption swo, char *settingsWindowOptionString)
{
	strcpy(settingsWindowOptionString, settingsWindowOptionStrings[swo]);
}

bool fileExists(const char *fileName)
{
	if (FILE *file = fopen(fileName, "r"))
	{
		fclose(file);
		return true;
	} else {
		return false;
	}   
}

int modulo(int a, int b)
{
	return ((b + (a % b)) % b);
}

