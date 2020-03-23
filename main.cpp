#include <iostream>
#include <string.h>
#include <vector>
#include <time.h>

#include <ncurses.h>


int longest = 14;

class InterfaceHeader 
{
	public:
		InterfaceHeader()
		{
			this->win = newwin(2, COLS, 0, 0);
		}

		void Refresh()
		{
			wrefresh(this->win);
		}

		void Print()
		{
			wclear(this->win);
			mvwprintw(this->win, 1, 1, "%*s | %s | %s | %s | %s",
					longest, "Interface Name", "Rcvd Bytes", "Rcvd Pkts", "Sent Bytes", "Sent Pkts");
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
			mvwprintw(this->win, 1, 1, "%*s | %10lu | %9lu | %10lu | %9lu",
					longest, this->name, this->r_bytes, this->r_packets, this->t_bytes, this->t_packets);
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

InterfaceHeader *interfaceHeader;
std::vector<Interface *> interfaces;

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
	int placement = 2;

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
	interfaceHeader->Refresh();
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Print();
		interfaces[i]->Refresh();
	}
	refresh();
}

int main (int argc, char *argv[])
{
	time_t now;
	time_t lastTime;

	initscr();
	cbreak();
	nodelay(stdscr, TRUE);

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

	initializeNetInfo();

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
			//printf("char: %d\n", ch);
		}
	}

	endwin();

	return 0;
}
