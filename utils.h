#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>


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

#endif
