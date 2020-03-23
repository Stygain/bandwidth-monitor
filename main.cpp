#include <iostream>
#include <string.h>
#include <vector>
#include <time.h>

#include <ncurses.h>


#define HEADER_COLOR 1
#define ACTIVE_COLOR 2
#define OPTION_COLOR 3

int longest = 14;

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
			wattron(this->win, COLOR_PAIR(OPTION_COLOR));
			mvwprintw(this->win, 0, 0, "%s | %s | %s | %s",
					"jk to select", "q quit", "d deselect", "s sort");
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

		void Print()
		{
			wclear(this->win);
			wattron(this->win, COLOR_PAIR(HEADER_COLOR));
			mvwprintw(this->win, 0, 1, "%*s | %s | %s | %s | %s",
					longest, "Interface Name", "Rcvd Bytes", "Rcvd Pkts", "Sent Bytes", "Sent Pkts");
			wattroff(this->win, COLOR_PAIR(HEADER_COLOR));
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

class Interface
{
	public:
		Interface(char *name, int placement)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);

			this->placement = placement;
			this->win = newwin(3, COLS, this->placement, 0);
		}

		void Refresh()
		{
			wrefresh(this->win);
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
			wclear(this->win);
			wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);
			if (this->active) {
				wattron(this->win, COLOR_PAIR(ACTIVE_COLOR));
			}
			mvwprintw(this->win, 1, 1, "%*s | %10lu | %9lu | %10lu | %9lu",
					longest, this->name, this->r_bytes, this->r_packets, this->t_bytes, this->t_packets);
			if (this->active) {
				wattroff(this->win, COLOR_PAIR(ACTIVE_COLOR));
			}
			wrefresh(this->win);
		}

		void SetActive(bool active)
		{
			this->active = active;
		}

	public:
		char *name;
		WINDOW *win;

	private:
		int placement;
		bool active = false;
		unsigned long int r_bytes;
		unsigned long int t_bytes;
		unsigned long int r_packets;
		unsigned long int t_packets;
};

InterfaceHeader *interfaceHeader;
std::vector<Interface *> interfaces;
InterfaceFooter *interfaceFooter;

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

		interfaces.push_back(new Interface(ifname, placement));
		placement += 3;
	}

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Refresh();
	}
	refresh();

	fclose(fp);
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

int main (int argc, char *argv[])
{
	time_t now;
	time_t lastTime;

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

	cbreak();
	nodelay(stdscr, TRUE);
	noecho();

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

	initializeNetInfo();

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
			if (ch == (int)'q')
			{
				break;
			}
			switch (ch)
			{
				case (int)'j':
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
					break;
				}
				case (int)'k':
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

	return 0;
}
