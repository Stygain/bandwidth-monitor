#include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>

#include <ncurses.h>


#define HEADER_COLOR 1
#define ACTIVE_COLOR 2
#define OPTION_COLOR 3
#define HEADER_ACTIVE_COLOR 4
#define HEADER_SORT_COLOR 5

int longest = 14;
std::ofstream logfile;

typedef enum
{
	MODE_NORMAL,
	MODE_SEARCH,
	MODE_GRAPH
} Mode;

Mode mode = MODE_NORMAL;

const char modeStrings[3][10] =
{
	"Normal",
	"Sort",
	"Graph"
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
	GT_PKTS_SEND
} GraphType;

const char graphTypeStrings[4][20] =
{
	"Bytes received",
	"Packets received",
	"Bytes sent",
	"Packets sent"
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

class InterfaceFooter
{
	public:
		InterfaceFooter()
		{
			this->win = newwin(1, COLS, LINES-1, 0);
		}

		~InterfaceFooter()
		{
		}

		void Print()
		{
			wclear(this->win);

			char modeString[10];
			getModeString(mode, modeString);
			wprintw(this->win, " Mode: %s ",
					modeString);

			wattron(this->win, COLOR_PAIR(OPTION_COLOR));
			if (mode == MODE_SEARCH)
			{
				wprintw(this->win, " %s | %s | %s | %s ",
						"hl to Select", "q Quit", "d Deselect", "enter Select");
			}
			else if (mode == MODE_GRAPH)
			{
				wprintw(this->win, " %s | %s | %s | %s ",
						"hl to Select", "q Quit", "c clear", "C clear all");
			}
			else
			{
				wprintw(this->win, " %s | %s | %s | %s | %s | %s | %s ",
				"jk enter to Select", "q Quit", "d Deselect", "s Sort", "g Graph", "z Zero", "Z Un-Zero");
			}
			wattroff(this->win, COLOR_PAIR(OPTION_COLOR));

			wrefresh(this->win);
		}

	public:
		WINDOW *win;
};


class InterfaceHeader
{
	public:
		InterfaceHeader()
		{
			this->win = newwin(1, COLS, 0, 0);
		}

		~InterfaceHeader()
		{
		}

		void PrintTabTitle(int index, char *text)
		{
			if (this->activeTab == index)
			{
				wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
				wprintw(this->win, "%s",
						text);
				wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
				wattron(this->win, COLOR_PAIR(HEADER_COLOR));
			}
			else if (this->sortingHeader == index)
			{
				wattron(this->win, COLOR_PAIR(HEADER_SORT_COLOR));
				wprintw(this->win, "%s",
						text);
				wattroff(this->win, COLOR_PAIR(HEADER_SORT_COLOR));
				wattron(this->win, COLOR_PAIR(HEADER_COLOR));
			}
			else
			{
				wprintw(this->win, "%s",
						text);
			}
		}

		void Print()
		{
			wclear(this->win);
			wattron(this->win, COLOR_PAIR(HEADER_COLOR));
			if (this->activeTab == 0)
			{
				wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
				mvwprintw(this->win, 0, 1, "%*s",
						longest, "Interface Name");
				wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
				wattron(this->win, COLOR_PAIR(HEADER_COLOR));
			}
			else
			{
				mvwprintw(this->win, 0, 1, "%*s",
						longest, "Interface Name");
			}
			wprintw(this->win, " | ");
			this->PrintTabTitle(1, "Rcvd Bytes");
			wprintw(this->win, " | ");
			this->PrintTabTitle(2, "Rcvd Pkts");
			wprintw(this->win, " | ");
			this->PrintTabTitle(3, "Sent Bytes");
			wprintw(this->win, " | ");
			this->PrintTabTitle(4, "Sent Pkts");
			wattroff(this->win, COLOR_PAIR(HEADER_COLOR));
			wrefresh(this->win);
		}

		int GetTabCount()
		{
			return this->tabCount;
		}

	public:
		WINDOW *win;
		int sortingHeader = -1;
		int activeTab = -1;

	private:
		int tabCount = 5;
};

class InterfaceRow
{
	public:
		InterfaceRow(int placement)
		{
			this->placement = placement;
			this->win = newwin(3, COLS, this->placement, 0);
		}

		~InterfaceRow()
		{
		}

	public:
		WINDOW *win;

	private:
		int placement;
};

class Interface
{
	public:
		Interface(char *name)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);
		}

		Interface(char *name, InterfaceRow * interfaceRow)
		{
			this->interfaceRow = interfaceRow;
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);
		}

		~Interface()
		{
			delete this->name;
		}

		void Refresh()
		{
			wrefresh(this->interfaceRow->win);
		}

		void Update(unsigned long int r_bytes,
				unsigned long int t_bytes,
				unsigned long int r_packets,
				unsigned long int t_packets)
		{
			this->r_bytesLast = this->r_bytes;
			this->r_bytes = r_bytes;
			this->t_bytesLast = this->t_bytes;
			this->t_bytes = t_bytes;
			this->r_packetsLast = this->r_packets;
			//logfile << "rpkts: " << r_packets << " old rpkts: " << this->r_packets << " diff: " << this->r_packetsLast << "\n";
			this->r_packets = r_packets;
			this->t_packetsLast = this->t_packets;
			this->t_packets = t_packets;
		}

		void Print()
		{
			wclear(this->interfaceRow->win);
			wborder(this->interfaceRow->win, 0, 0, 0, 0, 0, 0, 0, 0);
			if (this->active) {
				wattron(this->interfaceRow->win, COLOR_PAIR(ACTIVE_COLOR));
			}
			mvwprintw(this->interfaceRow->win, 1, 1, "%*s | %10lu | %9lu | %10lu | %9lu",
					longest, this->name, (this->r_bytes - this->r_bytesZeroed), (this->r_packets - this->r_packetsZeroed), (this->t_bytes - this->t_bytesZeroed), (this->t_packets - this->t_packetsZeroed));
			if (this->active) {
				wattroff(this->interfaceRow->win, COLOR_PAIR(ACTIVE_COLOR));
			}
			wrefresh(this->interfaceRow->win);
		}

		void SetActive(bool active)
		{
			this->active = active;
		}

		void Zero()
		{
			r_bytesZeroed = r_bytes;
			t_bytesZeroed = t_bytes;
			r_packetsZeroed = r_packets;
			t_packetsZeroed = t_packets;
		}

		void UnZero()
		{
			r_bytesZeroed = 0;
			t_bytesZeroed = 0;
			r_packetsZeroed = 0;
			t_packetsZeroed = 0;
		}

		void setInterfaceRow(InterfaceRow *interfaceRow)
		{
			this->interfaceRow = interfaceRow;
		}

	public:
		char *name;
		unsigned long int r_bytes;
		unsigned long int r_bytesLast = 0;
		unsigned long int r_bytesZeroed = 0;
		unsigned long int t_bytes;
		unsigned long int t_bytesLast = 0;
		unsigned long int t_bytesZeroed = 0;
		unsigned long int r_packets;
		unsigned long int r_packetsLast = 0;
		unsigned long int r_packetsZeroed = 0;
		unsigned long int t_packets;
		unsigned long int t_packetsLast = 0;
		unsigned long int t_packetsZeroed = 0;

	private:
		InterfaceRow * interfaceRow = NULL;
		bool active = false;
};


class GraphDataColumn
{
	public:
		GraphDataColumn(GraphType graphType, std::vector<Interface *> *interfaces, Interface *interface)
		{
			this->graphType = graphType;
			//this->value = (rand() % 11);
			this->interfaces = interfaces;
			this->interface = interface;
		}

		void SetNext(GraphDataColumn *next)
		{
			this->next = next;
		}

		void Update()
		{
			if (next == NULL)
			{
				if (graphType == GT_PKTS_RECV)
				{
					int pktsRecvDiff = 0;
					if (this->interface != NULL)
					{
						if (this->interface->r_packetsLast != 0)
						{
							pktsRecvDiff += this->interface->r_packets - this->interface->r_packetsLast;
						}
					}
					else
					{
						for (size_t i = 0; i < this->interfaces->size(); i++)
						{
							if (this->interfaces->at(i)->r_packetsLast != 0)
							{
								pktsRecvDiff += this->interfaces->at(i)->r_packets - this->interfaces->at(i)->r_packetsLast;
							}
						}
					}
					logfile << "Pkts Recv Diff: " << pktsRecvDiff << "\n";
					this->value = pktsRecvDiff;
				}
				else if (graphType == GT_PKTS_SEND)
				{
					int pktsSendDiff = 0;
					if (this->interface != NULL)
					{
						if (this->interface->t_packetsLast != 0)
						{
							pktsSendDiff += this->interface->t_packets - this->interface->t_packetsLast;
						}
					}
					else
					{
						for (size_t i = 0; i < this->interfaces->size(); i++)
						{
							if (this->interfaces->at(i)->t_packetsLast != 0)
							{
								pktsSendDiff += this->interfaces->at(i)->t_packets - this->interfaces->at(i)->t_packetsLast;
							}
						}
					}
					logfile << "Pkts Send Diff: " << pktsSendDiff << "\n";
					this->value = pktsSendDiff;
				}
			}
			else
			{
				this->value = this->next->GetValue();
			}
		}

		void UpdateGraphInterface(Interface *interface)
		{
			this->interface = interface;
		}

		void Clear()
		{
			this->value = 0;
		}

		int GetValue()
		{
			return this->value;
		}


	private:
		int value = 0;
		GraphType graphType;
		std::vector<Interface *> *interfaces;
		Interface *interface = NULL;

		GraphDataColumn *next = NULL;
};


class GraphRow
{
	public:
		GraphRow(int height, int width, int placementX, int placementY, int value, int max)
		{
			this->max = max;
			this->value = value;
			this->placementY = placementY;
			this->placementX = placementX;
			this->height = height;
			this->width = width;
			this->win = newwin(this->height, this->width, this->placementY, this->placementX);
			// 113 wide == this->width
			//wprintw(this->win, "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfas123");

			wrefresh(this->win);
		}

		~GraphRow()
		{
		}

		void Update(std::vector<GraphDataColumn *> *gDataCols, int max)
		{
			wclear(this->win);
			float currPos = (float)this->value / (float)this->max;
			for (size_t i = 0; i < gDataCols->size(); i++)
			{
				float valuePercent = (float)gDataCols->at(i)->GetValue() / (float)max;
				float split = (float)1 / (float)this->max;
				//logfile << "My value: " << this->value << " my max: " << this->max << " my perc: " << currPos << " its value: " << gDataCols->at(i)->GetValue() << " its max: " << max << " its perc: " << valuePercent << " split is: " << split << "\n";
				if (valuePercent >= currPos && valuePercent < (currPos + split))
				{
					mvwprintw(this->win, 0, i, "_");
				}
				else if (valuePercent > currPos)
				{
					mvwprintw(this->win, 0, i, "|");
				}
				else if (valuePercent == 0 && currPos == 0)
				{
					mvwprintw(this->win, 0, i, "_");
				}
			}

			wrefresh(this->win);
		}


	public:
		WINDOW *win;


	private:
		int value;
		int max;
		int placementY;
		int placementX;
		int height;
		int width;
};


InterfaceHeader *interfaceHeader;
std::vector<InterfaceRow *> interfaceRows;
std::vector<Interface *> interfaces;
InterfaceFooter *interfaceFooter;


class GraphTitle
{
	public:
		GraphTitle(GraphType graphType, int placementX, int placementY, int width, int height, Interface *interface)
		{
			this->placementX = placementX;
			this->placementY = placementY;
			this->width = width;
			this->height = height;

			this->interface = interface;

			this->win = newwin(this->height, this->width, this->placementY, this->placementX);

			this->UpdateGraphType(graphType);
		}

		void Update()
		{
			wclear(this->win);

			char graphTypeString[20];
			getGraphTypeString(this->graphType, graphTypeString);

			if (this->interface == NULL)
			{
				wprintw(this->win, "%s", graphTypeString);
			}
			else
			{
				wprintw(this->win, "%s through %s", graphTypeString, interface->name);
			}

			wrefresh(this->win);
		}

		void UpdateGraphType(GraphType graphType)
		{
			this->graphType = graphType;

			this->Update();
		}

		void UpdateGraphInterface(Interface *interface)
		{
			this->interface = interface;

			this->Update();
		}

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		GraphType graphType;
		Interface *interface;
};


class Graph
{
	public:
		Graph(GraphType graphType, int placementX, int placementY, int width, int height)
		{
			this->graphType = graphType;
			this->placementX = placementX;
			this->placementY = placementY;
			this->width = width;
			this->height = height;

			graphTitle = new GraphTitle(this->graphType, this->placementX + 1, this->placementY, this->width, 1, NULL);

			this->win = newwin(this->height, this->width, this->placementY + 1, this->placementX);
			wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

			wrefresh(this->win);
		}

		~Graph()
		{
			gDataCols.clear();
			gRows.clear();
		}

		void Create()
		{
			this->numCols = (this->width - 3);
			//logfile << "This width: " << this->width << " number of columns: " << this->numCols << "\n";
			for (int i = 0; i < this->numCols; i++)
			{
				this->gDataCols.push_back(new GraphDataColumn(this->graphType, &interfaces, this->interface));
			}

			for (size_t i = 0; i < (this->gDataCols.size() - 1); i++)
			{
				gDataCols[i]->SetNext(gDataCols[i+1]);
			}

			this->numRows = (this->height - 2);
			//logfile << "This height: " << this->height << " number of rows: " << this->numRows << "\n";
			for (int i = 0; i < this->numRows; i++)
			{
				this->gRows.push_back(
						new GraphRow(
								1,
								(this->width - 3),
								(this->placementX + 1),
								(this->placementY + 1 + ((i * 1) + 1)),
								(this->numRows - i - 1),
								(this->numRows - 1)
							)
						);
			}

			wrefresh(this->win);
		}

		void Update()
		{
			// Update the data
			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				gDataCols[i]->Update();
			}

			// Scan the data columns for the maximum
			int max = 0;
			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				int tempValue = gDataCols[i]->GetValue();
				if (tempValue > max)
				{
					max = tempValue;
				}
			}

			// Print the graph contents
			for (size_t i = 0; i < this->gRows.size(); i++)
			{
				gRows[i]->Update(&(this->gDataCols), max);
			}

			wrefresh(this->win);
		}

		void Clear()
		{
			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				gDataCols[i]->Clear();
			}

			for (size_t i = 0; i < this->gRows.size(); i++)
			{
				gRows[i]->Update(&(this->gDataCols), 2);
			}

			wrefresh(this->win);
		}

		void UpdateGraphInterface(Interface *interface)
		{
			this->interface = interface;

			graphTitle->UpdateGraphInterface(interface);

			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				gDataCols[i]->UpdateGraphInterface(interface);
			}

			this->Clear();
			this->Update();
		}


	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		int numRows;
		int numCols;

		Interface *interface = NULL;

		GraphType graphType;
		GraphTitle *graphTitle;
		std::vector<GraphRow *> gRows;
		std::vector<GraphDataColumn *> gDataCols;
};


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

		InterfaceRow *interfaceRow = new InterfaceRow(placement);
		interfaceRows.push_back(interfaceRow);
		interfaces.push_back(new Interface(ifname, interfaceRow));
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

	logfile << "Test print\n";

	while (fgets(buf, 200, fp)) {
		sscanf(buf, "%[^:]: %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
				ifname, &r_bytes, &r_packets, &t_bytes, &t_packets);
		logfile << "rbytes: " << r_bytes << " rpkts: " << r_packets << " tbytes: " << t_bytes << " tpkts: " << t_packets << "\n";

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
		//logfile << "Header rcvd bytes\n";
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
		//logfile << "Header rcvd pkts\n";
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
		//logfile << "Header sent bytes\n";
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
		//logfile << "Header sent pkts\n";
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

	logfile.open("logs");

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

	//WINDOW *win1 = newwin(3, COLS, 0, 0);
	//WINDOW *win2 = newwin(3, COLS, 3, 0);
	//WINDOW *win3 = newwin(3, COLS, 6, 0);
	//wborder(win1, 0, 0, 0, 0, 0, 0, 0, 0);
	//wborder(win2, 0, 0, 0, 0, 0, 0, 0, 0);
	//wborder(win3, 0, 0, 0, 0, 0, 0, 0, 0);
	//wrefresh(win1);
	//wrefresh(win2);
	//wrefresh(win3);
	//mvwprintw(win1, 1, 1, "asdf");
	//mvwprintw(win2, 1, 1, "qwer");
	//mvwprintw(win3, 1, 1, "uiop");
	//wrefresh(win1);
	//wrefresh(win2);
	//wrefresh(win3);

	interfaceHeader = new InterfaceHeader();
	interfaceFooter = new InterfaceFooter();

	int placement = initializeNetInfo();

	graphs.push_back(new Graph(GT_PKTS_RECV, 0, (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2)));
	graphs.push_back(new Graph(GT_PKTS_SEND, (int)(COLS / 2), (interfaceRows.size() * 3) + 1, (int)(COLS / 2) - 1, (LINES - ((interfaceRows.size() * 3) + 1) - 2)));

	for (size_t i = 0; i < graphs.size(); i++)
	{
		graphs[i]->Create();
	}

	int activeIndex = -1;

	time(&lastTime);
	time(&now);
	double diff;
	double lastDiff;

	updateScreen();

	while (true)
	{
		time(&now);
		diff = difftime(now, lastTime);
		//logfile << "Diff: ";
		//logfile << std::fixed << std::setprecision(8) << diff;
		//logfile << "\n";
		if (diff > 1)
		{
			//lastDiff = diff;
			parseNetInfo();
			updateScreen();

			for (size_t i = 0; i < graphs.size(); i++)
			{
				graphs[i]->Update();
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
				if (mode == MODE_SEARCH || mode == MODE_GRAPH)
				{
					mode = MODE_NORMAL;
					interfaceHeader->activeTab = -1;
					interfaceHeader->Print();
					interfaceFooter->Print();
				}
				else
				{
					break;
				}
			}
			switch (ch)
			{
				case KEY_DOWN:
				case (int)'j':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

					if (activeIndex != -1)
					{
						interfaces[activeIndex]->SetActive(false);
						interfaces[activeIndex]->Print();
					}
					activeIndex += 1;
					activeIndex = modulo(activeIndex, (int)interfaces.size());
					interfaces[activeIndex]->SetActive(true);
					interfaces[activeIndex]->Print();
					break;
				}

				case KEY_UP:
				case (int)'k':
				{
					if (mode != MODE_NORMAL)
					{
						break;
					}

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
					interfaceFooter->Print();
					break;
				}

				case KEY_LEFT:
				case (int)'h':
				{
					if (mode != MODE_SEARCH)
					{
						break;
					}

					if (interfaceHeader->activeTab == -1)
					{
						interfaceHeader->activeTab = 0;
					}
					interfaceHeader->activeTab -= 1;
					interfaceHeader->activeTab = modulo(interfaceHeader->activeTab, interfaceHeader->GetTabCount());
					interfaceHeader->Print();
					break;
				}

				case KEY_RIGHT:
				case (int)'l':
				{
					if (mode != MODE_SEARCH)
					{
						break;
					}

					interfaceHeader->activeTab += 1;
					interfaceHeader->activeTab = modulo(interfaceHeader->activeTab, interfaceHeader->GetTabCount());
					interfaceHeader->Print();
					break;
				}

				case KEY_ENTER:
				case 10:
				{
					if (mode == MODE_SEARCH)
					{
						mode = MODE_NORMAL;
						interfaceFooter->Print();
						interfaceHeader->sortingHeader = interfaceHeader->activeTab;
						interfaceHeader->activeTab = -1;
						interfaceHeader->Print();
						sortInterfaces((InterfaceHeaderContent)interfaceHeader->sortingHeader);
					}
					else if (mode == MODE_NORMAL)
					{
						for (size_t i = 0; i < graphs.size(); i++)
						{
							graphs[i]->UpdateGraphInterface(interfaces[activeIndex]);
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

	endwin();

	logfile.close();

	return 0;
}
