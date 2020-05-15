// #include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
#include <algorithm>
// #include <stdlib.h>
#include <unistd.h>
// #include <iomanip>

#include <ncurses.h>

#include "footer.h"
#include "graph.h"
#include "interface.h"
#include "selectionWindow.h"
#include "utils.h"
#include "logger.h"
#include "settings.h"


Mode mode = MODE_NORMAL;
int longest = 14;



InterfaceHeader *interfaceHeader = NULL;
std::vector<InterfaceRow *> interfaceRows;
std::vector<Interface *> interfaces;
Footer *footer = NULL;
InterfaceDetailWindow *interfaceDetailWindow = NULL;
std::vector<Graph *> graphs;

extern Logger *logger;
extern Settings *settings;

GraphMode gm;
GraphMode gmLast;




void initializeInterfaces()
{
	FILE *fp = fopen("/proc/net/wireless", "r");
	char buf[200];
	char ifname[20];
	std::vector<char *> wirelessIfaceNames;
	
	if (fp != NULL)
	{
		// Skip first two lines
		for (int i = 0; i < 2; i++) {
			fgets(buf, 200, fp);
		}

		while (fgets(buf, 200, fp)) {
			char *wirelessInterfaceName = new char[20];
			sscanf(buf, "%s", wirelessInterfaceName);
			wirelessInterfaceName[strlen(wirelessInterfaceName) - 1] = '\0';
			wirelessIfaceNames.push_back(wirelessInterfaceName);
		}
	}
	for (size_t i = 0; i < wirelessIfaceNames.size(); ++i)
	{
		logger->Log("Wireless interface name: ");
		logger->Log(wirelessIfaceNames[i]);
		logger->Log("\n");
	}

	fp = fopen("/proc/net/dev", "r");

	// skip first two lines
	for (int i = 0; i < 2; i++) {
		fgets(buf, 200, fp);
	}

	while (fgets(buf, 200, fp)) {
		sscanf(buf, "%s", ifname);
		ifname[strlen(ifname) - 1] = '\0';
		logger->Log("Found iface name: ");
		logger->Log(ifname);
		logger->Log("\n");

		int len = strlen(ifname);
		if (len > longest)
		{
			longest = len;
		}
	}
	longest++;

	fclose(fp);


	fp = fopen("/proc/net/dev", "r");
	// skip first two lines
	for (int i = 0; i < 2; i++) {
		fgets(buf, 200, fp);
	}

	while (fgets(buf, 200, fp)) {
		sscanf(buf, "%s", ifname);
		ifname[strlen(ifname) - 1] = '\0';

		for (size_t i = 0; i < wirelessIfaceNames.size(); ++i)
		{
			if (strcmp(ifname, wirelessIfaceNames[i]) == 0)
			{
				interfaces.push_back(new Interface(ifname, longest, 1));
			}
			else
			{
				interfaces.push_back(new Interface(ifname, longest, 0));
			}
		}
	}

	wirelessIfaceNames.clear();
}

int initializeInterfaceUI()
{
	int placement = 1;

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		bool skip = false;
		if (!settings->root["hiddenInterfaces"].isNull() && settings->root["hiddenInterfaces"].isArray())
		{
			for (size_t j = 0; j < settings->root["hiddenInterfaces"].size(); ++j)
			{
				if (strcmp(settings->root["hiddenInterfaces"][(int)j].asString().c_str(), interfaces[i]->name) == 0)
				{
					logger->Log("Skipping hidden interface: ");
					logger->Log(interfaces[i]->name);
					logger->Log("\n");
					interfaces[i]->RemoveFromUI();
					skip = true;
				}
			}
		}

		if (!skip)
		{
			InterfaceRow *interfaceRow = new InterfaceRow(0, placement, COLS, 3);
			interfaceRows.push_back(interfaceRow);
			interfaces[i]->setInterfaceRow(interfaceRow);

			placement += 3;
		}
	}

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Refresh();
	}

	refresh();

	return placement;
}

void updateInterfaceUI()
{
	interfaceHeader->Print();
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Print();
	}
	footer->Print();
	refresh();
}

void sortInterfaces(InterfaceHeaderContent column)
{
	if (column == HEADER_RCVD_BYTES)
	{
		Interface *tmp = NULL;
		for (size_t i = 0; i < (interfaces.size() - 1); ++i)
		{
			for (size_t j = (i + 1); j < interfaces.size(); ++j)
			{
				if ((interfaces[i]->r_bytes - interfaces[i]->r_bytesZeroed) < (interfaces[j]->r_bytes - interfaces[j]->r_bytesZeroed))
				{
					tmp = interfaces[i];
					interfaces[i] = interfaces[j];
					interfaces[j] = tmp;
				}
			}
		}
	}
	else if (column == HEADER_RCVD_PKTS)
	{
		Interface *tmp = NULL;
		for (size_t i = 0; i < (interfaces.size() - 1); ++i)
		{
			for (size_t j = (i + 1); j < interfaces.size(); ++j)
			{
				if ((interfaces[i]->r_packets - interfaces[i]->r_packetsZeroed) < (interfaces[j]->r_packets - interfaces[j]->r_packetsZeroed))
				{
					tmp = interfaces[i];
					interfaces[i] = interfaces[j];
					interfaces[j] = tmp;
				}
			}
		}
	}
	if (column == HEADER_SENT_BYTES)
	{
		Interface *tmp = NULL;
		for (size_t i = 0; i < (interfaces.size() - 1); ++i)
		{
			for (size_t j = (i + 1); j < interfaces.size(); ++j)
			{
				if ((interfaces[i]->t_bytes - interfaces[i]->t_bytesZeroed) < (interfaces[j]->t_bytes - interfaces[j]->t_bytesZeroed))
				{
					tmp = interfaces[i];
					interfaces[i] = interfaces[j];
					interfaces[j] = tmp;
				}
			}
		}
	}
	else if (column == HEADER_SENT_PKTS)
	{
		Interface *tmp = NULL;
		for (size_t i = 0; i < (interfaces.size() - 1); ++i)
		{
			for (size_t j = (i + 1); j < interfaces.size(); ++j)
			{
				if ((interfaces[i]->t_packets - interfaces[i]->t_packetsZeroed) < (interfaces[j]->t_packets - interfaces[j]->t_packetsZeroed))
				{
					tmp = interfaces[i];
					interfaces[i] = interfaces[j];
					interfaces[j] = tmp;
				}
			}
		}
	}

	int interfaceRowIndex = 0;
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		if (!interfaces[i]->IsHidden())
		{
			interfaces[i]->setInterfaceRow(interfaceRows[interfaceRowIndex]);
			interfaceRowIndex++;
		}
	}

	updateInterfaceUI();
}

int translateInterfaceIndex(int index)
{
	int matchIndex = -1;
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		if (!interfaces[i]->IsHidden())
		{
			matchIndex++;
		}
		if (matchIndex == index)
		{
			return i;
		}
	}
	return -1;
}

GraphMode assessGraphUISize()
{
	int numWide = 0;
	int numTall = 0;
	if (COLS > (GRAPH_MIN_WIDTH * 4))
	{
		// Or if the screen is wide enough add the other graphs
		logger->Log("Wide enough for four\n");
		numWide = 4;
	}
	else if (COLS > (GRAPH_MIN_WIDTH * 2))
	{
		// Or if the screen is wide enough add the other graphs
		logger->Log("Wide enough for two ");
		logger->Log(std::to_string(COLS));
		logger->Log(":");
		logger->Log(std::to_string(GRAPH_MIN_WIDTH*2));
		logger->Log("\n");
		numWide = 2;
	}
	else
	{
		logger->Log("Wide enough for one ");
		logger->Log(std::to_string(COLS));
		logger->Log(":");
		logger->Log(std::to_string(GRAPH_MIN_WIDTH*2));
		logger->Log("\n");
		numWide = 1;
	}

	logger->Log("Available vertical space: ");
	logger->Log(std::to_string((LINES - ((interfaceRows.size() * 3) + 1) - 2)));
	logger->Log(":");
	logger->Log(std::to_string(GRAPH_MIN_HEIGHT));
	logger->Log("\n");
	// If the screen is tall enough, add in the other graphs
	if ((LINES - ((interfaceRows.size() * 3) + 1) - 2) > (GRAPH_MIN_HEIGHT * 4))
	{
		logger->Log("Tall enough for four\n");
		numTall = 32;
	}
	else if ((LINES - ((interfaceRows.size() * 3) + 1) - 2) > (GRAPH_MIN_HEIGHT * 2))
	{
		logger->Log("Tall enough for two\n");
		numTall = 16;
	}
	else
	{
		logger->Log("Tall enough for one\n");
		numTall = 8;
	}

	logger->Log("Values: ");
	logger->Log(std::to_string(numWide));
	logger->Log("x");
	logger->Log(std::to_string(numTall));
	logger->Log(" | ");
	logger->Log(std::to_string(numWide | numTall));
	logger->Log("\n");

	// Translate numWide and numTall to GM types
	switch (numWide | numTall)
	{
		case 4 | 8:
		case 4 | 16:
		case 4 | 32:
		{
			logger->Log("Returning 4 wide\n");
			return GM_FOUR_WIDE;
		}
		case 2 | 8:
		{
			logger->Log("Returning 2 wide\n");
			return GM_TWO_WIDE;
		}
		case 2 | 16:
		case 2 | 32:
		{
			logger->Log("Returning 2 wide 2 tall\n");
			return GM_TWO_WIDE_TWO_TALL;
		}
		case 1 | 32:
		{
			logger->Log("Returning 4 tall\n");
			return GM_FOUR_TALL;
		}
		case 1 | 16:
		{
			logger->Log("Returning 2 tall\n");
			return GM_TWO_TALL;
		}
		case 1 | 8:
		{
			logger->Log("Returning 1\n");
			return GM_ONE;
		}
	}
}

GraphMode resizeUI()
{
	logger->Log("New width: ");
	logger->Log(std::to_string(COLS));
	logger->Log("\n");

	logger->Log("New height: ");
	logger->Log(std::to_string(LINES));
	logger->Log("\n");

	gmLast = gm;
	gm = assessGraphUISize();
	logger->Log("GRAPH MODE: ");
	logger->Log(std::to_string((int)gm));
	logger->Log("\n");

	clear();

	footer->Resize(0, LINES-1, COLS, 1);

	switch (gm)
	{
		case GM_ONE:
		{
			int startXPos = 0;
			int startYPos = (interfaceRows.size() * 3) + 1;
			int width = COLS;
			int height = (LINES - startYPos - 2);
			graphs[0]->Resize(startXPos, startYPos, width, height);
			if (gmLast != gm)
			{
				if (gmLast == GM_TWO_WIDE || gmLast == GM_TWO_TALL)
				{
					delete graphs[1];
					graphs[1] = NULL;
				}
				if (gmLast == GM_FOUR_WIDE || gmLast == GM_FOUR_TALL || gmLast == GM_TWO_WIDE_TWO_TALL)
				{
					delete graphs[1];
					delete graphs[2];
					delete graphs[3];
					graphs[1] = NULL;
					graphs[2] = NULL;
					graphs[3] = NULL;
				}
			}
			break;
		}
		case GM_TWO_WIDE:
		{
			int startXPos = 0;
			int startYPos = (interfaceRows.size() * 3) + 1;
			int width = (int)(COLS / 2) - 1;
			int height = (LINES - startYPos - 2);
			graphs[0]->Resize(startXPos, startYPos, width, height);

			if (graphs[1] == NULL)
			{
				GraphType gt1 = (GraphType)settings->root["graphs"][1].asInt();
				graphs[1] = new Graph(gt1, &interfaces);
				graphs[1]->Create(startXPos + width, startYPos, width, height);
			}
			else
			{
				graphs[1]->Resize(startXPos + width, startYPos, width, height);
			}

			if (gmLast != gm)
			{
				if (gmLast == GM_FOUR_WIDE || gmLast == GM_FOUR_TALL || gmLast == GM_TWO_WIDE_TWO_TALL)
				{
					delete graphs[2];
					delete graphs[3];
					graphs[2] = NULL;
					graphs[3] = NULL;
				}
			}
			break;
		}
		case GM_TWO_TALL:
		{
			int startYPos = (interfaceRows.size() * 3) + 1;
			int startXPos = 0;
			int height = (int)((LINES - startYPos) / 2);
			int width = COLS;
			graphs[0]->Resize(startXPos, startYPos, width, height);

			if (graphs[1] == NULL)
			{
				GraphType gt1 = (GraphType)settings->root["graphs"][1].asInt();
				graphs[1] = new Graph(gt1, &interfaces);
				graphs[1]->Create(startXPos, startYPos + height, width, height);
			}
			else
			{
				graphs[1]->Resize(startXPos, startYPos + height, width, height);
			}

			if (gmLast != gm)
			{
				if (gmLast == GM_FOUR_WIDE || gmLast == GM_FOUR_TALL || gmLast == GM_TWO_WIDE_TWO_TALL)
				{
					delete graphs[2];
					delete graphs[3];
					graphs[2] = NULL;
					graphs[3] = NULL;
				}
			}
			break;
		}
		case GM_FOUR_WIDE:
		{
			int startXPos = 0;
			int startYPos = (interfaceRows.size() * 3) + 1;
			int width = (int)(COLS / 4) - 1;
			int height = LINES - startYPos - 2;
			graphs[0]->Resize(startXPos, startYPos, width, height);

			if (graphs[1] == NULL)
			{
				GraphType gt1 = (GraphType)settings->root["graphs"][1].asInt();
				graphs[1] = new Graph(gt1, &interfaces);
				graphs[1]->Create(startXPos + width + 1, startYPos, width, height);
			}
			else
			{
				graphs[1]->Resize(startXPos + width + 1, startYPos, width, height);
			}

			if (graphs[2] == NULL)
			{
				GraphType gt2 = (GraphType)settings->root["graphs"][2].asInt();
				graphs[2] = new Graph(gt2, &interfaces);
				graphs[2]->Create(startXPos + (width * 2) + 1, startYPos, width, height);
			}
			else
			{
				graphs[2]->Resize(startXPos + (width * 2) + 1, startYPos, width, height);
			}

			if (graphs[3] == NULL)
			{
				GraphType gt3 = (GraphType)settings->root["graphs"][3].asInt();
				graphs[3] = new Graph(gt3, &interfaces);
				graphs[3]->Create(startXPos + (width * 3) + 1, startYPos, width, height);
			}
			else
			{
				graphs[3]->Resize(startXPos + (width * 3) + 1, startYPos, width, height);
			}
			break;
		}
		case GM_FOUR_TALL:
		{
			int startYPos = (interfaceRows.size() * 3) + 1;
			int startXPos = 0;
			int height = (int)((LINES - startYPos) / 4) - 1;
			int width = COLS;
			graphs[0]->Resize(startXPos, startYPos, width, height);

			if (graphs[1] == NULL)
			{
				GraphType gt1 = (GraphType)settings->root["graphs"][1].asInt();
				graphs[1] = new Graph(gt1, &interfaces);
				graphs[1]->Create(startXPos, startYPos + height + 1, width, height);
			}
			else
			{
				graphs[1]->Resize(startXPos, startYPos + height + 1, width, height);
			}

			if (graphs[2] == NULL)
			{
				GraphType gt2 = (GraphType)settings->root["graphs"][2].asInt();
				graphs[2] = new Graph(gt2, &interfaces);
				graphs[2]->Create(startXPos, startYPos + (height * 2) + 1, width, height);
			}
			else
			{
				graphs[2]->Resize(startXPos, startYPos + (height * 2) + 1, width, height);
			}

			if (graphs[3] == NULL)
			{
				GraphType gt3 = (GraphType)settings->root["graphs"][3].asInt();
				graphs[3] = new Graph(gt3, &interfaces);
				graphs[3]->Create(startXPos, startYPos + (height * 3) + 1, width, height);
			}
			else
			{
				graphs[3]->Resize(startXPos, startYPos + (height * 3) + 1, width, height);
			}
			break;
		}
		case GM_TWO_WIDE_TWO_TALL:
		{
			int startYPos = (interfaceRows.size() * 3) + 1;
			int startXPos = 0;
			int height = (int)((LINES - startYPos) / 2) - 2;
			int width = (int)(COLS / 2) - 1;
			graphs[0]->Resize(startXPos, startYPos, width, height);

			if (graphs[1] == NULL)
			{
				GraphType gt1 = (GraphType)settings->root["graphs"][1].asInt();
				graphs[1] = new Graph(gt1, &interfaces);
				graphs[1]->Create(startXPos + width + 1, startYPos, width, height);
			}
			else
			{
				graphs[1]->Resize(startXPos + width + 1, startYPos, width, height);
			}

			if (graphs[2] == NULL)
			{
				GraphType gt2 = (GraphType)settings->root["graphs"][2].asInt();
				graphs[2] = new Graph(gt2, &interfaces);
				graphs[2]->Create(startXPos, startYPos + height + 1, width, height);
			}
			else
			{
				graphs[2]->Resize(startXPos, startYPos + height + 1, width, height);
			}

			if (graphs[3] == NULL)
			{
				GraphType gt3 = (GraphType)settings->root["graphs"][3].asInt();
				graphs[3] = new Graph(gt3, &interfaces);
				graphs[3]->Create(startXPos + width + 1, startYPos + height + 1, width, height);
			}
			else
			{
				graphs[3]->Resize(startXPos + width + 1, startYPos + height + 1, width, height);
			}
			break;
		}
	}

	return gm;
}

void updateGraphs()
{
	switch (gm)
	{
		case GM_ONE:
		{
			graphs[0]->Update();
			break;
		}
		case GM_TWO_WIDE:
		case GM_TWO_TALL:
		{
			graphs[0]->Update();
			graphs[1]->Update();
			break;
		}
		case GM_FOUR_WIDE:
		case GM_FOUR_TALL:
		case GM_TWO_WIDE_TWO_TALL:
		{
			graphs[0]->Update();
			graphs[1]->Update();
			graphs[2]->Update();
			graphs[3]->Update();
			break;
		}
	}
}

void deactivateGraphs()
{
	switch (gm)
	{
		case GM_ONE:
		{
			graphs[0]->setActive(false);
			break;
		}
		case GM_TWO_WIDE:
		case GM_TWO_TALL:
		{
			graphs[0]->setActive(false);
			graphs[1]->setActive(false);
			break;
		}
		case GM_FOUR_WIDE:
		case GM_FOUR_TALL:
		case GM_TWO_WIDE_TWO_TALL:
		{
			graphs[0]->setActive(false);
			graphs[1]->setActive(false);
			graphs[2]->setActive(false);
			graphs[3]->setActive(false);
			break;
		}
	}
}

void printGraphs()
{
	switch (gm)
	{
		case GM_ONE:
		{
			graphs[0]->Print();
			break;
		}
		case GM_TWO_WIDE:
		case GM_TWO_TALL:
		{
			graphs[0]->Print();
			graphs[1]->Print();
			break;
		}
		case GM_FOUR_WIDE:
		case GM_FOUR_TALL:
		case GM_TWO_WIDE_TWO_TALL:
		{
			graphs[0]->Print();
			graphs[1]->Print();
			graphs[2]->Print();
			graphs[3]->Print();
			break;
		}
	}
}

int getGraphIndex(int activeIndex)
{
	switch (gm)
	{
		case GM_ONE:
		{
			return modulo(activeIndex, 1);
		}
		case GM_TWO_WIDE:
		case GM_TWO_TALL:
		{
			return modulo(activeIndex, 2);
		}
		case GM_FOUR_WIDE:
		case GM_FOUR_TALL:
		case GM_TWO_WIDE_TWO_TALL:
		{
			return modulo(activeIndex, 4);
		}
	}
}

void updateAllGraphInterfaces(Interface * interface)
{
	switch (gm)
	{
		case GM_ONE:
		{
			graphs[0]->UpdateGraphInterface(interface);
			break;
		}
		case GM_TWO_WIDE:
		case GM_TWO_TALL:
		{
			graphs[0]->UpdateGraphInterface(interface);
			graphs[1]->UpdateGraphInterface(interface);
			break;
		}
		case GM_FOUR_WIDE:
		case GM_FOUR_TALL:
		case GM_TWO_WIDE_TWO_TALL:
		{
			graphs[0]->UpdateGraphInterface(interface);
			graphs[1]->UpdateGraphInterface(interface);
			graphs[2]->UpdateGraphInterface(interface);
			graphs[3]->UpdateGraphInterface(interface);
			break;
		}
	}
}

void clearGraphs()
{
	switch (gm)
	{
		case GM_ONE:
		{
			graphs[0]->Clear();
			break;
		}
		case GM_TWO_WIDE:
		case GM_TWO_TALL:
		{
			graphs[0]->Clear();
			graphs[1]->Clear();
			break;
		}
		case GM_FOUR_WIDE:
		case GM_FOUR_TALL:
		case GM_TWO_WIDE_TWO_TALL:
		{
			graphs[0]->Clear();
			graphs[1]->Clear();
			graphs[2]->Clear();
			graphs[3]->Clear();
			break;
		}
	}
}





int main (int argc, char *argv[])
{
	time_t now;
	time_t lastTime;
	srand(time(NULL));

	initscr();
	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	start_color();
	init_pair(HEADER_COLOR, COLOR_GREEN, COLOR_BLACK);
	init_pair(ACTIVE_COLOR, COLOR_WHITE, COLOR_BLUE);
	init_pair(OPTION_COLOR, COLOR_BLACK, COLOR_GREEN);
	init_pair(HEADER_ACTIVE_COLOR, COLOR_BLACK, COLOR_GREEN);
	init_pair(HEADER_SORT_COLOR, COLOR_WHITE, COLOR_GREEN);

	cbreak();
	nodelay(stdscr, TRUE);
	noecho();
	keypad(stdscr, TRUE);

	settings->Initialize();

	logger->StartLogfile();

	initializeInterfaces();
	int placement = initializeInterfaceUI();

	interfaceHeader = new InterfaceHeader(longest);
	// Change the sorting header based on the settings
	logger->Log(settings->root["sortingColumn"].asString().c_str());
	logger->Log("\n");
	int sortByIndex = settings->root["sortingColumn"].asInt();
	if (interfaceHeader != NULL)
	{
		interfaceHeader->sortingHeader = sortByIndex;
		interfaceHeader->Print();
	}

	footer = new Footer(0, LINES-1, COLS, 1);
	SelectionWindow *selectionWindow = NULL;

	GraphType gt0 = (GraphType)settings->root["graphs"][0].asInt();
	GraphType gt1 = (GraphType)settings->root["graphs"][1].asInt();
	GraphType gt2 = (GraphType)settings->root["graphs"][2].asInt();
	GraphType gt3 = (GraphType)settings->root["graphs"][3].asInt();
	gmLast = gm;
	gm = assessGraphUISize();
	logger->Log("GRAPH MODE: ");
	logger->Log(std::to_string((int)gm));
	logger->Log("\n");
	graphs.push_back(new Graph(gt0, &interfaces));
	graphs.push_back(new Graph(gt1, &interfaces));
	graphs.push_back(new Graph(gt2, &interfaces));
	graphs.push_back(new Graph(gt3, &interfaces));
	switch (gm)
	{
		case GM_ONE:
		{
			int startXPos = 0;
			int startYPos = (interfaceRows.size() * 3) + 1;
			int width = COLS;
			int height = (LINES - startYPos - 2);
			graphs[0]->Create(startXPos, startYPos, width, height);
			break;
		}
		case GM_TWO_WIDE:
		{
			int startXPos = 0;
			int startYPos = (interfaceRows.size() * 3) + 1;
			int width = (int)(COLS / 2) - 1;
			int height = LINES - startYPos - 2;
			graphs[0]->Create(startXPos, startYPos, width, height);
			graphs[1]->Create(startXPos + width, startYPos, width, height);
			break;
		}
		case GM_TWO_TALL:
		{
			int startYPos = (interfaceRows.size() * 3) + 1;
			int startXPos = 0;
			int height = (int)((LINES - startYPos) / 2) - 1;
			int width = COLS;
			graphs[0]->Create(startXPos, startYPos, width, height);
			graphs[1]->Create(startXPos, startYPos + height + 1, width, height);
			break;
		}
		case GM_FOUR_WIDE:
		{
			int startXPos = 0;
			int startYPos = (interfaceRows.size() * 3) + 1;
			int width = (int)(COLS / 4) - 1;
			int height = LINES - startYPos - 2;
			graphs[0]->Create(startXPos, startYPos, width, height);
			graphs[1]->Create(startXPos + width + 1, startYPos, width, height);
			graphs[2]->Create(startXPos + (width * 2) + 1, startYPos, width, height);
			graphs[3]->Create(startXPos + (width * 3) + 1, startYPos, width, height);
			break;
		}
		case GM_FOUR_TALL:
		{
			int startYPos = (interfaceRows.size() * 3) + 1;
			int startXPos = 0;
			int height = (int)((LINES - startYPos) / 4) - 1;
			int width = COLS;
			graphs[0]->Create(startXPos, startYPos, width, height);
			graphs[1]->Create(startXPos, startYPos + height + 1, width, height);
			graphs[2]->Create(startXPos, startYPos + (height * 2) + 1, width, height);
			graphs[3]->Create(startXPos, startYPos + (height * 3) + 1, width, height);
			break;
		}
		case GM_TWO_WIDE_TWO_TALL:
		{
			int startYPos = (interfaceRows.size() * 3) + 1;
			int startXPos = 0;
			int height = (int)((LINES - startYPos) / 2) - 2;
			int width = (int)(COLS / 2) - 1;
			graphs[0]->Create(startXPos, startYPos, width, height);
			graphs[1]->Create(startXPos + width + 1, startYPos, width, height);
			graphs[2]->Create(startXPos, startYPos + height + 1, width, height);
			graphs[3]->Create(startXPos + width + 1, startYPos + height + 1, width, height);
			break;
		}
	}

	int activeIndex = -1;
	int interfaceIndex = -1;
	int activeGraph = -1;

	time(&lastTime);
	time(&now);
	double diff;
	double lastDiff;

	for (size_t i = 0; i < interfaces.size(); i++)
	{
		interfaces[i]->Update();
	}
	updateInterfaceUI();

	// Zero on start if settings say so
	logger->Log(settings->root["zeroOnStart"].asString().c_str());
	logger->Log("\n");
	int zeroOnStart = settings->root["zeroOnStart"].asInt();
	if (zeroOnStart)
	{
		for (size_t i = 0; i < interfaces.size(); ++i)
		{
			interfaces[i]->Zero();
			interfaces[i]->Print();
		}
	}

	logger->Log(std::to_string(COLS));
	logger->Log("\n");
	logger->Log(std::to_string(LINES));
	logger->Log("\n");

	while (true)
	{
		time(&now);
		diff = difftime(now, lastTime);
		if (diff > 1)
		{
			for (size_t i = 0; i < interfaces.size(); i++)
			{
				interfaces[i]->Update();
			}
			updateInterfaceUI();

			updateGraphs();
			sortInterfaces((InterfaceHeaderContent)interfaceHeader->sortingHeader);

			if (interfaceDetailWindow != NULL)
			{
				interfaceDetailWindow->Update();
			}
			if (selectionWindow != NULL)
			{
				selectionWindow->Update();
			}

			time(&lastTime);
			time(&now);
		}
		usleep(500);

		int ch = getch();
		if (ch != -1)
		{
			if (ch == KEY_RESIZE) {
				resizeUI();
			}
			if (ch == (int)'q')
			{
				if (mode == MODE_SEARCH)
				{
					mode = MODE_NORMAL;
					footer->UpdateMode(mode);
					footer->Print();

					interfaceHeader->activeTab = -1;
					interfaceHeader->Print();
				}
				else if (mode == MODE_GRAPH)
				{
					activeGraph = -1;
					mode = MODE_NORMAL;
					footer->UpdateMode(mode);
					footer->Print();

					deactivateGraphs();
				}
				else if (mode == MODE_GRAPH_SELECTION)
				{
					mode = MODE_GRAPH;
					footer->UpdateMode(mode);
					footer->Print();

					if (selectionWindow != NULL)
					{
						delete selectionWindow;
						selectionWindow = NULL;
					}

					printGraphs();
				}
				else if (mode == MODE_INTERFACE_DETAIL)
				{
					mode = MODE_NORMAL;
					footer->UpdateMode(mode);
					footer->Print();

					if (interfaceDetailWindow != NULL)
					{
						delete interfaceDetailWindow;
						interfaceDetailWindow = NULL;
					}

					updateInterfaceUI();
				}
				else
				{
					break;
				}
				footer->Print();
			}
			switch (ch)
			{
				case KEY_DOWN:
				case (int)'j':
				{
					if (mode == MODE_NORMAL)
					{
						if (activeIndex != -1)
						{
							interfaceIndex = translateInterfaceIndex(activeIndex);
							if (interfaceIndex != -1)
							{
								interfaces[interfaceIndex]->SetActive(false);
								interfaces[interfaceIndex]->Print();
							}
						}
						activeIndex += 1;
						activeIndex = modulo(activeIndex, (int)interfaceRows.size());
						interfaceIndex = translateInterfaceIndex(activeIndex);
						if (interfaceIndex != -1)
						{
							interfaces[interfaceIndex]->SetActive(true);
							interfaces[interfaceIndex]->Print();
						}
					}
					else if (mode == MODE_GRAPH_SELECTION)
					{
						if (selectionWindow != NULL)
						{
							selectionWindow->IncrementActiveItem();
						}
					}
					else if (mode == MODE_INTERFACE_DETAIL)
					{
						if (interfaceDetailWindow != NULL)
						{
							interfaceDetailWindow->IncrementActiveItem();
						}
					}

					break;
				}

				case KEY_UP:
				case (int)'k':
				{
					if (mode == MODE_NORMAL)
					{
						if (activeIndex != -1)
						{
							interfaceIndex = translateInterfaceIndex(activeIndex);
							if (interfaceIndex != -1)
							{
								interfaces[interfaceIndex]->SetActive(false);
								interfaces[interfaceIndex]->Print();
							}
						}
						else
						{
							activeIndex = 0;
						}
						activeIndex -= 1;
						activeIndex = modulo(activeIndex, (int)interfaceRows.size());
						interfaceIndex = translateInterfaceIndex(activeIndex);
						if (interfaceIndex != -1)
						{
							interfaces[interfaceIndex]->SetActive(true);
							interfaces[interfaceIndex]->Print();
						}
					}
					else if (mode == MODE_GRAPH_SELECTION)
					{
						if (selectionWindow != NULL)
						{
							if (selectionWindow->GetActiveItem() != -1)
							{
								selectionWindow->DecrementActiveItem();
							}
							else
							{
								selectionWindow->SetActiveItem(selectionWindow->GetActiveItem());
							}
						}
					}
					else if (mode == MODE_INTERFACE_DETAIL)
					{
						if (interfaceDetailWindow != NULL)
						{
							if (interfaceDetailWindow->GetActiveItem() != -1)
							{
								interfaceDetailWindow->DecrementActiveItem();
							}
							else
							{
								interfaceDetailWindow->SetActiveItem(interfaceDetailWindow->GetActiveItem());
							}
						}
					}

					break;
				}

				case (int)'d':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					interfaceIndex = translateInterfaceIndex(activeIndex);
					if (interfaceIndex != -1)
					{
						interfaces[interfaceIndex]->SetActive(false);
						interfaces[interfaceIndex]->Print();

						updateAllGraphInterfaces(NULL);
					}

					activeIndex = -1;
					break;
				}

				case (int)'z':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					for (size_t i = 0; i < interfaces.size(); ++i)
					{
						interfaces[i]->Zero();
						interfaces[i]->Print();
					}
					break;
				}

				case (int)'Z':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					for (size_t i = 0; i < interfaces.size(); ++i)
					{
						interfaces[i]->UnZero();
						interfaces[i]->Print();
					}
					break;
				}

				case (int)'s':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					mode = MODE_SEARCH;
					footer->UpdateMode(mode);
					footer->Print();

					break;
				}

				case KEY_LEFT:
				case (int)'h':
				{
					if (mode == MODE_SEARCH)
					{
						if (interfaceHeader->activeTab == -1)
						{
							interfaceHeader->activeTab = 0;
						}
						interfaceHeader->activeTab -= 1;
						interfaceHeader->activeTab = modulo(interfaceHeader->activeTab, interfaceHeader->GetTabCount());
						interfaceHeader->Print();
					}
					else if (mode == MODE_GRAPH)
					{
						if (activeGraph != -1)
						{
							activeGraph -= 1;
						}
						activeGraph = getGraphIndex(activeGraph);
						deactivateGraphs();
						graphs[activeGraph]->setActive(true);
					}
					break;
				}

				case KEY_RIGHT:
				case (int)'l':
				{
					if (mode == MODE_SEARCH)
					{
						interfaceHeader->activeTab += 1;
						interfaceHeader->activeTab = modulo(interfaceHeader->activeTab, interfaceHeader->GetTabCount());
						interfaceHeader->Print();
					}
					else if (mode == MODE_GRAPH)
					{
						activeGraph += 1;
						activeGraph = getGraphIndex(activeGraph);
						deactivateGraphs();
						graphs[activeGraph]->setActive(true);
					}
					break;
				}

				case KEY_ENTER:
				case 10:
				{
					if (mode == MODE_SEARCH)
					{
						mode = MODE_NORMAL;
						footer->UpdateMode(mode);
						footer->Print();

						interfaceHeader->sortingHeader = interfaceHeader->activeTab;
						interfaceHeader->activeTab = -1;
						interfaceHeader->Print();

						settings->root["sortingColumn"] = interfaceHeader->sortingHeader;
						settings->SaveSettings();

						sortInterfaces((InterfaceHeaderContent)interfaceHeader->sortingHeader);
					}
					else if (mode == MODE_NORMAL)
					{
						if (interfaceIndex != -1)
						{
							mode = MODE_INTERFACE_DETAIL;
							footer->UpdateMode(mode);
							footer->Print();

							InterfaceRow *currentInterfaceRow = interfaces[interfaceIndex]->getInterfaceRow();
							interfaceDetailWindow = new InterfaceDetailWindow(currentInterfaceRow->GetPlacementX(), currentInterfaceRow->GetPlacementY() + 2, currentInterfaceRow->GetWidth(), 8, interfaces[interfaceIndex]);
						}
					}
					else if (mode == MODE_GRAPH)
					{
						if (activeGraph != -1)
						{
							mode = MODE_GRAPH_SELECTION;
							footer->UpdateMode(mode);
							footer->Print();

							int graphIndex = getGraphIndex(activeGraph);
							selectionWindow = new SelectionWindow(graphs[graphIndex]->GetGraphType(), graphs[graphIndex]->GetPlacementX(), graphs[graphIndex]->GetPlacementY() + 1, graphTypeStringSize + 1, (int)GT_END + 2);
						}
					}
					else if (mode == MODE_GRAPH_SELECTION)
					{
						mode = MODE_GRAPH;
						footer->UpdateMode(mode);
						footer->Print();

						int graphIndex = getGraphIndex(activeGraph);

						// Get the new graph type
						GraphType newType = selectionWindow->GetActiveItemGraphType();

						if (selectionWindow != NULL)
						{
							delete selectionWindow;
							selectionWindow = NULL;
						}

						// Get the old graph placement and sizing
						int oldPlacementX = graphs[graphIndex]->GetPlacementX();
						int oldPlacementY = graphs[graphIndex]->GetPlacementY();
						int oldWidth = graphs[graphIndex]->GetWidth();
						int oldHeight = graphs[graphIndex]->GetHeight();
						Interface *oldInterface = graphs[graphIndex]->GetInterface();

						// Delete the graph
						delete graphs[graphIndex];

						// Replace the graph
						graphs[graphIndex] = new Graph(newType, &interfaces);
						graphs[graphIndex]->Create(oldPlacementX, oldPlacementY, oldWidth, oldHeight);
						graphs[graphIndex]->UpdateGraphInterface(oldInterface);

						settings->root["graphs"][0] = (int)graphs[0]->GetGraphType();
						settings->root["graphs"][1] = (int)graphs[1]->GetGraphType();
						settings->root["graphs"][2] = (int)graphs[2]->GetGraphType();
						settings->root["graphs"][3] = (int)graphs[3]->GetGraphType();
						settings->SaveSettings();
					}
					else if (mode == MODE_INTERFACE_DETAIL)
					{
						InterfaceDetailOption activeIdo = (InterfaceDetailOption)interfaceDetailWindow->GetActiveItem();
						Interface *activeInterface = interfaceDetailWindow->GetInterface();

						if (activeIdo == IDO_SELECT)
						{
							updateAllGraphInterfaces(activeInterface);
						}
						else if (activeIdo = IDO_HIDE)
						{
							interfaceIndex = translateInterfaceIndex(activeIndex);
							if (interfaceIndex != -1)
							{
								interfaces[interfaceIndex]->SetActive(false);
								interfaces[interfaceIndex]->Print();
								activeIndex = -1;
								interfaceIndex = -1;

								if (settings->root["hiddenInterfaces"][0].isNull())
								{
									settings->root["hiddenInterfaces"][0] = activeInterface->name;
								}
								else
								{
									settings->root["hiddenInterfaces"].append(activeInterface->name);
								}
								settings->SaveSettings();

								mode = MODE_NORMAL;
								footer->UpdateMode(mode);
								footer->Print();

								if (interfaceDetailWindow != NULL)
								{
									delete interfaceDetailWindow;
									interfaceDetailWindow = NULL;
								}

								activeInterface->RemoveFromUI();

								std::vector<Interface *>::iterator if_it = std::find(interfaces.begin(), interfaces.end(), activeInterface);
								std::vector<InterfaceRow *>::iterator ifr_it = std::find(interfaceRows.begin(), interfaceRows.end(), activeInterface->getInterfaceRow());
								if (if_it == interfaces.end() || ifr_it == interfaceRows.end())
								{
									logger->Log("Element Not Found\n");
								}
								else
								{
									logger->Log("Element Found\n");
									int index = std::distance(interfaces.begin(), if_it);
									int ifr_index = std::distance(interfaceRows.begin(), ifr_it);
									logger->Log(std::to_string(index) + "\n");
									
									// Move each interface up one row
									int interfaceRowIndex = ifr_index;
									if (index != interfaces.size())
									{
										for (size_t i = (size_t)(index + 1); i < interfaces.size(); ++i)
										{
											if (!interfaces[i]->IsHidden())
											{
												interfaces[i]->setInterfaceRow(interfaceRows[interfaceRowIndex]);
												interfaceRowIndex++;
											}
										}
									}
									updateInterfaceUI();

									// Delete the unneeded row
									delete interfaceRows[interfaceRows.size()];
									interfaceRows.erase(interfaceRows.end() - 1, interfaceRows.end());

									resizeUI();

									updateAllGraphInterfaces(NULL);
								}
							}
						}
					}

					break;
				}

				case (int)'c':
				{
					// Only clear the selected graph
					if (mode != MODE_GRAPH)
					{
						break;
					}

					break;
				}

				case (int)'C':
				{
					if (mode != MODE_GRAPH)
					{
						break;
					}

					clearGraphs();

					break;
				}

				case (int)'g':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					mode = MODE_GRAPH;
					footer->UpdateMode(mode);
					footer->Print();

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}

	interfaceRows.clear();
	interfaces.clear();
	delete interfaceHeader;
	delete footer;
	graphs.clear();
	delete logger;
	delete settings;

	endwin();

	return 0;
}
