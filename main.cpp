#include <iostream>
#include <string.h>
#include <vector>
#include <thread>
#include <chrono>

#include <ncurses.h>

class Interface {
	public:
		Interface(char *name, int placement)
		{
			this->name = new char[strlen(name) + 1];
			strcpy(this->name, name);

			this->placement = placement;
			this->win = newwin(3, COLS-3, this->placement, 0);
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
			mvwprintw(this->win, 1, 1, "%s: rb: %lu rp: %lu tb: %lu tp: %lu\n",
					this->name, this->r_bytes, this->r_packets, this->t_bytes, this->t_packets);
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
	int placement = 0;

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
	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		interfaces[i]->Print();
		interfaces[i]->Refresh();
	}
	refresh();
}

int main (int argc, char *argv[])
{
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

	initializeNetInfo();
	while (true)
	{
		parseNetInfo();
		updateScreen();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//int ch = getch();
	}

	std::this_thread::sleep_for(std::chrono::seconds(500));

	endwin();

	return 0;
}
