// #include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
// #include <stdlib.h>
#include <unistd.h>
// #include <iomanip>

#include <ncurses.h>

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
InterfaceFooter *interfaceFooter = NULL;
InterfaceDetailWindow *interfaceDetailWindow = NULL;

extern Logger *logger;
extern Settings *settings;



std::vector<Graph *> graphs;

Interface *getMatchingInterface(char *ifname)
{
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		if (strcmp(interfaces[i]->name, ifname) == 0)
		{
			return interfaces[i];
		}
	}
	return NULL;
}

bool initializeNetInfo()
{
	int placement = 1;

	FILE *fp = fopen("/proc/net/dev", "r");
	char buf[200];
	char ifname[20];
	unsigned long int r_bytes;
	unsigned long int t_bytes;
	unsigned long int r_packets;
	unsigned long int t_packets;

	// skip first two lines
	for (int i = 0; i < 2; i++) {
		fgets(buf, 200, fp);
	}

	while (fgets(buf, 200, fp)) {
		sscanf(buf, "%[^:]: %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
				ifname, &r_bytes, &r_packets, &t_bytes, &t_packets);
		int len = strlen(ifname);
		if (len > longest)
		{
			longest = len + 1;
		}

		InterfaceRow *interfaceRow = new InterfaceRow(0, placement, COLS, 3);
		interfaceRows.push_back(interfaceRow);
		interfaces.push_back(new Interface(ifname, longest, interfaceRow));
		placement += 3;
	}

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Refresh();
	}
	refresh();

	fclose(fp);

	return placement;
}

bool parseNetInfo()
{
	FILE *fp = fopen("/proc/net/dev", "r");
	char buf[200];
	char ifname[20];
	unsigned long int r_bytes;
	unsigned long int t_bytes;
	unsigned long int r_packets;
	unsigned long int t_packets;

	// skip first two lines
	for (int i = 0; i < 2; i++) {
		fgets(buf, 200, fp);
	}

	while (fgets(buf, 200, fp)) {
		sscanf(buf, "%[^:]: %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
				ifname, &r_bytes, &r_packets, &t_bytes, &t_packets);

		getMatchingInterface(ifname)->Update(r_bytes, t_bytes, r_packets, t_packets);
	}

	fclose(fp);
}

void updateScreen()
{
	interfaceHeader->Print();
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Print();
	}
	interfaceFooter->Print();
	refresh();
}

int modulo(int a, int b)
{
	return ((b + (a % b)) % b);
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

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->setInterfaceRow(interfaceRows[i]);
	}

	updateScreen();
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

	logger->StartLogfile();
	logger->Log("Test log message\n");

	int placement = initializeNetInfo();

	interfaceHeader = new InterfaceHeader(longest);
	interfaceFooter = new InterfaceFooter();
	SelectionWindow *selectionWindow = NULL;

	graphs.push_back(new Graph(GT_PKTS_RECV, 0, (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2), &interfaces));
	graphs.push_back(new Graph(GT_PKTS_SEND, (int)(COLS / 2), (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2), &interfaces));

	for (size_t i = 0; i < graphs.size(); i++)
	{
		graphs[i]->Create();
	}

	int activeIndex = -1;
	int activeGraph = -1;

	time(&lastTime);
	time(&now);
	double diff;
	double lastDiff;

	parseNetInfo();
	updateScreen();

	settings->InitializeSettings();

	while (true)
	{
		time(&now);
		diff = difftime(now, lastTime);
		if (diff > 1)
		{
			parseNetInfo();
			updateScreen();

			for (size_t i = 0; i < graphs.size(); i++)
			{
				graphs[i]->Update();
			}
			sortInterfaces((InterfaceHeaderContent)interfaceHeader->sortingHeader);

			if (interfaceDetailWindow != NULL)
			{
				interfaceDetailWindow->Update();
			}

			time(&lastTime);
			time(&now);
		}
		usleep(500);

		int ch = getch();
		if (ch != -1)
		{
			if (ch == KEY_RESIZE) {
				// Do something
			}
			if (ch == (int)'q')
			{
				if (mode == MODE_SEARCH)
				{
					mode = MODE_NORMAL;
					interfaceFooter->UpdateMode(mode);
					interfaceFooter->Print();

					interfaceHeader->activeTab = -1;
					interfaceHeader->Print();
				}
				else if (mode == MODE_GRAPH)
				{
					activeGraph = -1;
					mode = MODE_NORMAL;
					interfaceFooter->UpdateMode(mode);
					interfaceFooter->Print();

					for (size_t i = 0; i < graphs.size(); i++)
					{
						graphs[i]->setActive(false);
					}
				}
				else if (mode == MODE_GRAPH_SELECTION)
				{
					mode = MODE_GRAPH;
					interfaceFooter->UpdateMode(mode);
					interfaceFooter->Print();

					int graphIndex = modulo(activeGraph, (int)graphs.size());
					if (selectionWindow != NULL)
					{
						delete selectionWindow;
						selectionWindow = NULL;
					}
					graphs[graphIndex]->SetUpdate(true);
				}
				else
				{
					break;
				}
				interfaceFooter->Print();
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
							interfaces[activeIndex]->SetActive(false);
							interfaces[activeIndex]->Print();
						}
						activeIndex += 1;
						activeIndex = modulo(activeIndex, (int)interfaces.size());
						interfaces[activeIndex]->SetActive(true);
						interfaces[activeIndex]->Print();
					}
					else if (mode == MODE_GRAPH_SELECTION)
					{
						if (selectionWindow != NULL)
						{
							selectionWindow->activeItem++;
							selectionWindow->activeItem = modulo(selectionWindow->activeItem, selectionWindow->max);
							selectionWindow->Update();
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
							interfaces[activeIndex]->SetActive(false);
							interfaces[activeIndex]->Print();
						}
						else
						{
							activeIndex = 0;
						}
						activeIndex -= 1;
						activeIndex = modulo(activeIndex, (int)interfaces.size());
						interfaces[activeIndex]->SetActive(true);
						interfaces[activeIndex]->Print();
					}
					else if (mode == MODE_GRAPH_SELECTION)
					{
						if (selectionWindow != NULL)
						{
							if (selectionWindow->activeItem != -1)
							{
								selectionWindow->activeItem--;
							}
							selectionWindow->activeItem = modulo(selectionWindow->activeItem, selectionWindow->max);
							selectionWindow->Update();
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

					interfaces[activeIndex]->SetActive(false);
					interfaces[activeIndex]->Print();

					for (size_t i = 0; i < graphs.size(); i++)
					{
						graphs[i]->UpdateGraphInterface(NULL);
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
					interfaceFooter->UpdateMode(mode);
					interfaceFooter->Print();

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
						int graphIndex = modulo(activeGraph, (int)graphs.size());
						activeGraph = graphIndex;
						for (size_t i = 0; i < graphs.size(); i++)
						{
							graphs[i]->setActive(false);
						}
						graphs[graphIndex]->setActive(true);
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
						int graphIndex = modulo(activeGraph, (int)graphs.size());
						activeGraph = graphIndex;
						for (size_t i = 0; i < graphs.size(); i++)
						{
							graphs[i]->setActive(false);
						}
						graphs[graphIndex]->setActive(true);
					}
					break;
				}

				case KEY_ENTER:
				case 10:
				{
					if (mode == MODE_SEARCH)
					{
						mode = MODE_NORMAL;
						interfaceFooter->UpdateMode(mode);
						interfaceFooter->Print();

						interfaceHeader->sortingHeader = interfaceHeader->activeTab;
						interfaceHeader->activeTab = -1;
						interfaceHeader->Print();

						settings->root["sortingColumn"] = interfaceHeader->sortingHeader;
						settings->SaveSettings();

						sortInterfaces((InterfaceHeaderContent)interfaceHeader->sortingHeader);
					}
					else if (mode == MODE_NORMAL)
					{
						mode = MODE_INTERFACE_DETAIL;
						interfaceFooter->UpdateMode(mode);
						interfaceFooter->Print();

						InterfaceRow *currentInterfaceRow = interfaces[activeIndex]->getInterfaceRow();
						interfaceDetailWindow = new InterfaceDetailWindow(currentInterfaceRow->GetPlacementX(), currentInterfaceRow->GetPlacementY() + 2, currentInterfaceRow->GetWidth(), 8, interfaces[activeIndex]);

						//for (size_t i = 0; i < graphs.size(); i++)
						//{
						//	graphs[i]->UpdateGraphInterface(interfaces[activeIndex]);
						//}
					}
					else if (mode == MODE_GRAPH)
					{
						if (activeGraph != -1)
						{
							mode = MODE_GRAPH_SELECTION;
							interfaceFooter->UpdateMode(mode);
							interfaceFooter->Print();

							int graphIndex = modulo(activeGraph, (int)graphs.size());
							selectionWindow = new SelectionWindow(graphs[graphIndex]->GetGraphType(), graphs[graphIndex]->GetPlacementX(), graphs[graphIndex]->GetPlacementY() + 1, graphTypeStringSize + 1, (int)GT_END + 2);
							graphs[graphIndex]->SetUpdate(false);
						}
					}
					else if (mode == MODE_GRAPH_SELECTION)
					{
						mode = MODE_GRAPH;
						interfaceFooter->UpdateMode(mode);
						interfaceFooter->Print();

						int graphIndex = modulo(activeGraph, (int)graphs.size());

						// Get the new graph type
						GraphType newType = selectionWindow->GetActiveItemGraphType();

						if (selectionWindow != NULL)
						{
							delete selectionWindow;
							selectionWindow = NULL;
						}
						graphs[graphIndex]->SetUpdate(true);

						// Get the old graph placement and sizing
						int oldPlacementX = graphs[graphIndex]->GetPlacementX();
						int oldPlacementY = graphs[graphIndex]->GetPlacementY();
						int oldWidth = graphs[graphIndex]->GetWidth();
						int oldHeight = graphs[graphIndex]->GetHeight();
						Interface *oldInterface = graphs[graphIndex]->GetInterface();

						// Delete the graph
						delete graphs[graphIndex];

						// Replace the graph
						graphs[graphIndex] = new Graph(newType, oldPlacementX, oldPlacementY, oldWidth, oldHeight, &interfaces);
						graphs[graphIndex]->Create();
						graphs[graphIndex]->UpdateGraphInterface(oldInterface);
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

					for (size_t i = 0; i < graphs.size(); i++)
					{
						graphs[i]->Clear();
					}

					break;
				}

				case (int)'g':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					mode = MODE_GRAPH;
					interfaceFooter->UpdateMode(mode);
					interfaceFooter->Print();

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
	delete interfaceFooter;
	graphs.clear();
	delete logger;

	endwin();

	return 0;
}
