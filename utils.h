#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>


#define DIFF_SECONDS 1


typedef enum
{
	SWO_HIDDEN_INTERFACES,
	SWO_ZERO_ON_START,
	SWO_END
} SettingsWindowOption;

#define settingsWindowOptionStringSize 18

const char settingsWindowOptionStrings[3][settingsWindowOptionStringSize] =
{
	"Hidden Interfaces",
	"Zero On Start",
	"END"
};


#define GRAPH_MIN_WIDTH 45
#define GRAPH_MIN_HEIGHT 20

typedef enum
{
	GM_ONE,
	GM_TWO_WIDE,
	GM_TWO_TALL,
	GM_FOUR_WIDE,
	GM_FOUR_TALL,
	GM_TWO_WIDE_TWO_TALL
} GraphMode;


typedef enum
{
	MODE_NORMAL,
	MODE_SEARCH,
	MODE_GRAPH,
	MODE_GRAPH_SELECTION,
	MODE_INTERFACE_DETAIL
} Mode;

#define modeStringSize 25

const char modeStrings[5][modeStringSize] =
{
	"Normal",
	"Sort",
	"Graph",
	"Graph Selection",
	"Mode Interface Detail"
};

typedef enum
{
	GT_BYTES_RECV,
	GT_PKTS_RECV,
	GT_BYTES_SEND,
	GT_PKTS_SEND,
	GT_END
} GraphType;

#define graphTypeStringSize 17

const char graphTypeStrings[5][graphTypeStringSize] =
{
	"Bytes received",
	"Packets received",
	"Bytes sent",
	"Packets sent",
	"END"
};

typedef enum
{
	HEADER_INTERFACE_NAME,
	HEADER_RCVD_BYTES,
	HEADER_RCVD_PKTS,
	HEADER_SENT_BYTES,
	HEADER_SENT_PKTS
} InterfaceHeaderContent;

void getModeString(Mode mode, char *modeString);
void getGraphTypeString(GraphType gt, char *graphTypeString);
bool fileExists(const char * fileName);
int modulo(int a, int b);
void getSettingsWindowOptionString(SettingsWindowOption swo, char *settingsWindowOptionString);

#endif
