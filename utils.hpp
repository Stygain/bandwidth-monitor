#pragma once

#include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>

#include <ncurses.h>


typedef enum
{
	MODE_NORMAL,
	MODE_SEARCH,
	MODE_GRAPH,
	MODE_GRAPH_SELECTION
} Mode;

Mode mode = MODE_NORMAL;

#define modeStringSize 20

const char modeStrings[4][modeStringSize] =
{
	"Normal",
	"Sort",
	"Graph",
	"Graph Selection"
};

void getModeString(Mode mode, char *modeString)
{
	strcpy(modeString, modeStrings[mode]);
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

void getGraphTypeString(GraphType gt, char *graphTypeString)
{
	strcpy(graphTypeString, graphTypeStrings[gt]);
};

typedef enum
{
	HEADER_INTERFACE_NAME,
	HEADER_RCVD_BYTES,
	HEADER_RCVD_PKTS,
	HEADER_SENT_BYTES,
	HEADER_SENT_PKTS
} InterfaceHeaderContent;


