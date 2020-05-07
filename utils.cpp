#include "utils.h"


void getModeString(Mode mode, char *modeString)
{
	strcpy(modeString, modeStrings[mode]);
};

void getGraphTypeString(GraphType gt, char *graphTypeString)
{
	strcpy(graphTypeString, graphTypeStrings[gt]);
};
