// #include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
#include <algorithm>
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

void initializeNetInfo()
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
		int len = strlen(ifname);
		if (len > longest)
		{
			longest = len + 1;
		}

		interfaces.push_back(new Interface(ifname, longest));
	}

	fclose(fp);
}

int initializeInterfaceUi()
{
	int placement = 1;

	// Ignore interfaces that are hidden
	logger->Log("Thing\n");
	logger->Log(std::to_string(settings->root["hiddenInterfaces"].isNull()));
	logger->Log("\n");
	logger->Log(std::to_string(settings->root["hiddenInterfaces"].isArray()));
	logger->Log("\n");
	Json::Value other;
	other["asdf"] = "qwer";
	logger->Log(settings->root["hiddenInterfaces"][0].asString().c_str());
	//logger->Log(settings->root["hiddenInterfaces"]["vethc3890af"].asString().c_str());
	logger->Log("\n");
	logger->Log(std::to_string(settings->root["hiddenInterfaces"].size()));
	logger->Log("\n");

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		bool skip = false;
		if (!settings->root["hiddenInterfaces"].isNull() && settings->root["hiddenInterfaces"].isArray())
		{
			for (size_t j = 0; j < settings->root["hiddenInterfaces"].size(); ++j)
			{
				//logger->Log("Comparing: ");
				//logger->Log(settings->root["hiddenInterfaces"][j].asString().c_str());
				//logger->Log(" to: ");
				//logger->Log(interfaces[i]->name);
				//logger->Log("\n");
				if (strcmp(settings->root["hiddenInterfaces"][(int)j].asString().c_str(), interfaces[i]->name) == 0)
				{
					logger->Log("Skipping: ");
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

		Interface * iface = getMatchingInterface(ifname);
		if (iface != NULL)
		{
			iface->Update(r_bytes, t_bytes, r_packets, t_packets);
		}
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

	updateScreen();
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
			logger->Log("Returning index: ");
			logger->Log(std::to_string(i));
			logger->Log("\n");
			return i;
		}
	}
	logger->Log("Returning index: -1\n");
	return -1;
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

	settings->InitializeSettings();

	logger->StartLogfile();
	logger->Log("Test log message\n");

	initializeNetInfo();
	int placement = initializeInterfaceUi();

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

	interfaceFooter = new InterfaceFooter(0, LINES-1, COLS, 1);
	SelectionWindow *selectionWindow = NULL;

	graphs.push_back(new Graph(GT_PKTS_RECV, 0, (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2), &interfaces));
	graphs.push_back(new Graph(GT_PKTS_SEND, (int)(COLS / 2), (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2), &interfaces));

	for (size_t i = 0; i < graphs.size(); i++)
	{
		graphs[i]->Create();
	}

	int activeIndex = -1;
	int interfaceIndex = -1;
	int activeGraph = -1;

	time(&lastTime);
	time(&now);
	double diff;
	double lastDiff;

	parseNetInfo();
	updateScreen();

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
				// TODO
				logger->Log("RESIZE\n");
				logger->Log(std::to_string(COLS));
				logger->Log("\n");
				logger->Log(std::to_string(LINES));
				logger->Log("\n");

				clear();

				interfaceFooter->Resize(0, LINES-1, COLS, 1);

				graphs[0]->Resize(0, (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2));
				graphs[1]->Resize((int)(COLS / 2), (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2));
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
				}
				else if (mode == MODE_INTERFACE_DETAIL)
				{
					mode = MODE_NORMAL;
					interfaceFooter->UpdateMode(mode);
					interfaceFooter->Print();

					if (interfaceDetailWindow != NULL)
					{
						delete interfaceDetailWindow;
						interfaceDetailWindow = NULL;
					}
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
							interfaceIndex = translateInterfaceIndex(activeIndex);
							interfaces[interfaceIndex]->SetActive(false);
							interfaces[interfaceIndex]->Print();
						}
						activeIndex += 1;
						activeIndex = modulo(activeIndex, (int)interfaceRows.size());
						interfaceIndex = translateInterfaceIndex(activeIndex);
						interfaces[interfaceIndex]->SetActive(true);
						interfaces[interfaceIndex]->Print();
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
							interfaces[interfaceIndex]->SetActive(false);
							interfaces[interfaceIndex]->Print();
						}
						else
						{
							activeIndex = 0;
						}
						activeIndex -= 1;
						activeIndex = modulo(activeIndex, (int)interfaceRows.size());
						interfaceIndex = translateInterfaceIndex(activeIndex);
						interfaces[interfaceIndex]->SetActive(true);
						interfaces[interfaceIndex]->Print();
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
					interfaces[interfaceIndex]->SetActive(false);
					interfaces[interfaceIndex]->Print();

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

						InterfaceRow *currentInterfaceRow = interfaces[interfaceIndex]->getInterfaceRow();
						interfaceDetailWindow = new InterfaceDetailWindow(currentInterfaceRow->GetPlacementX(), currentInterfaceRow->GetPlacementY() + 2, currentInterfaceRow->GetWidth(), 8, interfaces[interfaceIndex]);
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
					else if (mode == MODE_INTERFACE_DETAIL)
					{
						InterfaceDetailOption activeIdo = (InterfaceDetailOption)interfaceDetailWindow->GetActiveItem();
						Interface *activeInterface = interfaceDetailWindow->GetInterface();

						if (activeIdo == IDO_SELECT)
						{
							for (size_t i = 0; i < graphs.size(); i++)
							{
								graphs[i]->UpdateGraphInterface(activeInterface);
							}
						}
						else if (activeIdo = IDO_HIDE)
						{
							interfaceIndex = translateInterfaceIndex(activeIndex);
							interfaces[interfaceIndex]->SetActive(false);
							interfaces[interfaceIndex]->Print();
							activeIndex = -1;

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
							interfaceFooter->UpdateMode(mode);
							interfaceFooter->Print();

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
								updateScreen();

								// Delete the unneeded row
								delete interfaceRows[interfaceRows.size()];
								interfaceRows.erase(interfaceRows.end() - 1, interfaceRows.end());
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
