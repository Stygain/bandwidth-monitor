#include "utils.h"


void getModeString(Mode mode, char *modeString)
{
	strcpy(modeString, modeStrings[mode]);
};

void getGraphTypeString(GraphType gt, char *graphTypeString)
{
	strcpy(graphTypeString, graphTypeStrings[gt]);
};

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
