#include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
#include <fstream>
#include <stdlib.h>

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
	MODE_SEARCH
} Mode;

Mode mode = MODE_NORMAL;

const char modeStrings[2][10] =
{
	"Normal",
	"Sort"
};

void getModeString(Mode mode, char *modeString)
{
	strcpy(modeString, modeStrings[mode]);
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
			else
			{
				wprintw(this->win, " %s | %s | %s | %s | %s | %s ",
				"jk to Select", "q Quit", "d Deselect", "s Sort", "n Zero", "N Un-Zero");
			}
			wattroff(this->win, COLOR_PAIR(OPTION_COLOR));
			wrefresh(this->win);
		}

	public:
		char *name;
		WINDOW *win;

	private:
		int placement;
		unsigned long int r_bytes;
		unsigned long int t_bytes;
		unsigned long int r_packets;
		unsigned long int t_packets;
};


class InterfaceHeader
{
	public:
		InterfaceHeader()
		{
			this->win = newwin(1, COLS, 0, 0);
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
		char *name;
		WINDOW *win;
		int sortingHeader = -1;
		int activeTab = -1;

	private:
		int tabCount = 5;
		unsigned long int r_bytes;
		unsigned long int t_bytes;
		unsigned long int r_packets;
		unsigned long int t_packets;
};

class InterfaceRow
{
	public:
		InterfaceRow(int placement)
		{
			this->placement = placement;
			this->win = newwin(3, COLS, this->placement, 0);
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

		void Refresh()
		{
			wrefresh(this->interfaceRow->win);
		}

		void Update(unsigned long int r_bytes,
				unsigned long int t_bytes,
				unsigned long int r_packets,
				unsigned long int t_packets)
		{
			this->r_bytes = r_bytes;
			this->t_bytes = r_bytes;
			this->r_packets = r_packets;
			this->t_packets = r_packets;
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
		unsigned long int r_bytesZeroed = 0;
		unsigned long int t_bytes;
		unsigned long int t_bytesZeroed = 0;
		unsigned long int r_packets;
		unsigned long int r_packetsZeroed = 0;
		unsigned long int t_packets;
		unsigned long int t_packetsZeroed = 0;

	private:
		InterfaceRow * interfaceRow = NULL;
		bool active = false;
};


class GraphDataColumn
{
	public:
		GraphDataColumn()
		{
			this->value = (rand() % 11);
		}

		int GetValue()
		{
			return this->value;
		}


	private:
		int value = 4;
};


class GraphRow
{
	public:
		GraphRow(int height, int width, int placementX, int placementY, int value, int max)
		{
			//this->positionIndex = positionIndex;
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

		void Update(std::vector<GraphDataColumn *> *gDataCols, int max)
		{
			float currPos = (float)this->value / (float)this->max;
			for (size_t i = 0; i < gDataCols->size(); i++)
			{
				float valuePercent = (float)gDataCols->at(i)->GetValue() / (float)max;
				float split = (float)1 / (float)this->max;
				logfile << "My value: " << this->value << " my max: " << this->max << " my perc: " << currPos << " its value: " << gDataCols->at(i)->GetValue() << " its max: " << max << " its perc: " << valuePercent << " split is: " << split << "\n";
				if (valuePercent >= currPos && valuePercent < (currPos + split))
				{
					//wprintw(this->win, "_");
					mvwprintw(this->win, 0, i, "_");
				}
				else if (valuePercent > currPos)
				{
					//wprintw(this->win, "|");
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
		//int positionIndex;
		int value;
		int max;
		int placementY;
		int placementX;
		int height;
		int width;
};


//class GraphFooter {
//	public:
//		GraphFooter(int placement)
//		{
//			this->placement = placement;
//			this->win = newwin(1, COLS, this->placement, 0);
//			wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);
//
//			wrefresh(this->win);
//		}
//
//
//	public:
//		WINDOW *win;
//
//
//	private:
//		int placement;
//};


class Graph
{
	public:
		Graph(int placement)
		{
			this->placement = placement;
			this->win = newwin((LINES - this->placement - 1), COLS, this->placement, 0);
			wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

			wrefresh(this->win);
		}

		void Create()
		{
			//this->numCols = (int)((COLS - 2) / 2);
			this->numCols = (COLS - 3);
			for (int i = 0; i < this->numCols; i++)
			{
				this->gDataCols.push_back(new GraphDataColumn());
			}

			this->numRows = (LINES - this->placement - 3);
			for (int i = 0; i < this->numRows; i++)
			{
				this->gRows.push_back(new GraphRow(1, (COLS - 3), 1, (this->placement + ((i * 1) + 1)), (this->numRows - i - 1), (this->numRows - 1)));
			}

			wrefresh(this->win);
		}

		void Update()
		{
			// Update the data
			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				//gDataCols[i]->Update();
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

			for (size_t i = 0; i < this->gRows.size(); i++)
			{
				gRows[i]->Update(&(this->gDataCols), max);
			}

			wrefresh(this->win);
		}


	public:
		WINDOW *win;


	private:
		int placement;

		int numRows;
		int numCols;
		std::vector<GraphRow *> gRows;
		std::vector<GraphDataColumn *> gDataCols;
		//GraphFooter *gFooter;
};


InterfaceHeader *interfaceHeader;
std::vector<InterfaceRow *> interfaceRows;
std::vector<Interface *> interfaces;
InterfaceFooter *interfaceFooter;
Graph *graph;

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
// HEADER_RCVD_BYTES,
// HEADER_RCVD_PKTS,
// HEADER_SENT_BYTES,
// HEADER_SENT_PKTS
	if (column == HEADER_RCVD_BYTES)
	{
		logfile << "Header rcvd bytes\n";
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
		logfile << "Header rcvd pkts\n";
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
		logfile << "Header sent bytes\n";
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
		logfile << "Header sent pkts\n";
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
	// Interface *tmp = NULL;
	// for (size_t i = 0; i < (interfaces.size() - 1); ++i)
	// {
	// 	for (size_t j = (i + 1); j < interfaces.size(); ++j)
	// 	{
	// 		if (strcmp(interfaces[i]->name, interfaces[j]->name) < 0)
	// 		{
	// 			logfile << interfaces[i]->name << " not greater than " << interfaces[j]->name << "\n";
	// 			tmp = interfaces[i];
	// 			interfaces[i] = interfaces[j];
	// 			interfaces[j] = tmp;
	// 		}
	// 		else
	// 		{
	// 			logfile << interfaces[i]->name << " greater than " << interfaces[j]->name << "\n";
	// 		}
	// 	}
	// }
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

	graph = new Graph((interfaceRows.size() * 3) + 1);
	graph->Create();
	graph->Update();

	int activeIndex = -1;

	time(&lastTime);
	time(&now);
	double diff;
	double lastDiff;
	while (true)
	{
		time(&now);
		diff = difftime(now, lastTime);
		if (diff > lastDiff)
		{
			lastDiff = diff;
			parseNetInfo();
			updateScreen();
			time(&lastTime);
			time(&now);
		}

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
					interfaces[activeIndex]->SetActive(false);
					interfaces[activeIndex]->Print();

					activeIndex = -1;
					break;
				}
				case (int)'n':
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
				case (int)'N':
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
					if (mode != MODE_SEARCH)
					{
						break;
					}

					mode = MODE_NORMAL;
					interfaceFooter->Print();
					interfaceHeader->sortingHeader = interfaceHeader->activeTab;
					interfaceHeader->activeTab = -1;
					interfaceHeader->Print();
					sortInterfaces((InterfaceHeaderContent)interfaceHeader->sortingHeader);
					break;
				}
				default:
				{
					break;
				}
			}
		}
	}

	endwin();

	logfile.close();

	return 0;
}
