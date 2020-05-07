#include "interface.h"


InterfaceFooter::InterfaceFooter()
{
	this->win = newwin(1, COLS, LINES-1, 0);
}

InterfaceFooter::~InterfaceFooter()
{
}

void InterfaceFooter::UpdateMode(Mode mode)
{
	this->mode = mode;
}

void InterfaceFooter::Print()
{
	werase(this->win);

	char modeString[modeStringSize];
	getModeString(this->mode, modeString);
	wprintw(this->win, " Mode: %s ",
			modeString);

	wattron(this->win, COLOR_PAIR(OPTION_COLOR));
	if (this->mode == MODE_SEARCH)
	{
		wprintw(this->win, " %s | %s | %s ",
				"hl enter to Select", "q Quit", "d Deselect");
	}
	else if (this->mode == MODE_GRAPH)
	{
		wprintw(this->win, " %s | %s | %s | %s ",
				"hl enter to Select", "q Quit", "c clear", "C clear all");
	}
	else if (this->mode == MODE_GRAPH_SELECTION)
	{
		wprintw(this->win, " %s | %s ",
				"jk enter to Select", "q Quit");
	}
	else
	{
		wprintw(this->win, " %s | %s | %s | %s | %s | %s | %s ",
		"jk enter to Select", "q Quit", "d Deselect", "s Sort", "g Graph", "z Zero", "Z Un-Zero");
	}
	wattroff(this->win, COLOR_PAIR(OPTION_COLOR));

	wrefresh(this->win);
}


InterfaceHeader::InterfaceHeader(int longest)
{
	this->longest = longest;
	this->win = newwin(1, COLS, 0, 0);
}

InterfaceHeader::~InterfaceHeader()
{
}

void InterfaceHeader::PrintTabTitle(int index, char *text)
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

void InterfaceHeader::Print()
{
	werase(this->win);
	wattron(this->win, COLOR_PAIR(HEADER_COLOR));
	if (this->activeTab == 0)
	{
		wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
		mvwprintw(this->win, 0, 1, "%*s",
				this->longest, "Interface Name");
		wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
		wattron(this->win, COLOR_PAIR(HEADER_COLOR));
	}
	else
	{
		mvwprintw(this->win, 0, 1, "%*s",
				this->longest, "Interface Name");
	}
	wprintw(this->win, " | ");
	this->PrintTabTitle(1, " Rcvd Bytes  ");
	wprintw(this->win, " | ");
	this->PrintTabTitle(2, "Rcvd Pkts");
	wprintw(this->win, " | ");
	this->PrintTabTitle(3, " Sent Bytes  ");
	wprintw(this->win, " | ");
	this->PrintTabTitle(4, "Sent Pkts");
	wattroff(this->win, COLOR_PAIR(HEADER_COLOR));
	wrefresh(this->win);
}

int InterfaceHeader::GetTabCount()
{
	return this->tabCount;
}


InterfaceRow::InterfaceRow(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(3, this->width, this->placementY, this->placementX);
}

InterfaceRow::~InterfaceRow()
{
}


Interface::Interface(char *name, int longest)
{
	this->longest = longest;
	this->name = new char[strlen(name) + 1];
	strcpy(this->name, name);
}

Interface::Interface(char *name, int longest, InterfaceRow * interfaceRow)
{
	this->longest = longest;
	this->interfaceRow = interfaceRow;
	this->name = new char[strlen(name) + 1];
	strcpy(this->name, name);
}

Interface::~Interface()
{
	delete this->name;
}

void Interface::Refresh()
{
	wrefresh(this->interfaceRow->win);
}

void Interface::Update(unsigned long int r_bytes,
		unsigned long int t_bytes,
		unsigned long int r_packets,
		unsigned long int t_packets)
{
	this->r_bytesLast = this->r_bytes;
	this->r_bytes = r_bytes;
	this->t_bytesLast = this->t_bytes;
	this->t_bytes = t_bytes;
	this->r_packetsLast = this->r_packets;
	this->r_packets = r_packets;
	this->t_packetsLast = this->t_packets;
	this->t_packets = t_packets;
}

void Interface::Print()
{
	werase(this->interfaceRow->win);
	wborder(this->interfaceRow->win, 0, 0, 0, 0, 0, 0, 0, 0);

	if (this->active) {
		wattron(this->interfaceRow->win, COLOR_PAIR(ACTIVE_COLOR));
	}

	unsigned long int printableRBytes = (this->r_bytes - this->r_bytesZeroed);
	char rBytesUnit[3];
	strcpy(rBytesUnit, "B");
	if (printableRBytes > 1000)
	{
		printableRBytes = printableRBytes / 1000;
		strcpy(rBytesUnit, "KB");
	}
	if (printableRBytes > 1000)
	{
		printableRBytes = printableRBytes / 1000;
		strcpy(rBytesUnit, "MB");
	}
	if (printableRBytes > 1000)
	{
		printableRBytes = printableRBytes / 1000;
		strcpy(rBytesUnit, "GB");
	}
	if (printableRBytes > 1000)
	{
		printableRBytes = printableRBytes / 1000;
		strcpy(rBytesUnit, "TB");
	}
	unsigned long int printableRPackets = (this->r_packets - this->r_packetsZeroed);
	unsigned long int printableTBytes = (this->t_bytes - this->t_bytesZeroed);
	char tBytesUnit[3];
	strcpy(tBytesUnit, "B");
	if (printableTBytes > 1000)
	{
		printableTBytes = printableTBytes / 1000;
		strcpy(tBytesUnit, "KB");
	}
	if (printableTBytes > 1000)
	{
		printableTBytes = printableTBytes / 1000;
		strcpy(tBytesUnit, "MB");
	}
	if (printableTBytes > 1000)
	{
		printableTBytes = printableTBytes / 1000;
		strcpy(tBytesUnit, "GB");
	}
	if (printableTBytes > 1000)
	{
		printableTBytes = printableTBytes / 1000;
		strcpy(tBytesUnit, "TB");
	}
	unsigned long int printableTPackets = (this->t_packets - this->t_packetsZeroed);

	mvwprintw(this->interfaceRow->win, 1, 1,
			"%*s | %10lu %2s | %9lu | %10lu %2s | %9lu",
			longest,
			this->name,
			printableRBytes,
			rBytesUnit,
			printableRPackets,
			printableTBytes,
			tBytesUnit,
			printableTPackets);


	if (this->active) {
		wattroff(this->interfaceRow->win, COLOR_PAIR(ACTIVE_COLOR));
	}

	wrefresh(this->interfaceRow->win);
}

void Interface::SetActive(bool active)
{
	this->active = active;
}

void Interface::Zero()
{
	r_bytesZeroed = r_bytes;
	t_bytesZeroed = t_bytes;
	r_packetsZeroed = r_packets;
	t_packetsZeroed = t_packets;
}

void Interface::UnZero()
{
	r_bytesZeroed = 0;
	t_bytesZeroed = 0;
	r_packetsZeroed = 0;
	t_packetsZeroed = 0;
}

void Interface::setInterfaceRow(InterfaceRow *interfaceRow)
{
	this->interfaceRow = interfaceRow;
}

InterfaceDetailWindow::InterfaceDetailWindow(int placementX, int placementY, int width, int height, Interface *interface)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

	wrefresh(this->win);
	this->Update();
}

void InterfaceDetailWindow::Update()
{
	//char graphTypeString[graphTypeStringSize];
	//for (int i = 0; i < GT_END; i++)
	//{
	//	getGraphTypeString((GraphType)i, graphTypeString);
	//	if (i == activeItem)
	//	{
	//		wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
	//	}
	//	mvwprintw(this->win, i + 1, 1, "%s", graphTypeString);
	//	if (i == activeItem)
	//	{
	//		wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
	//	}
	//}

	//wrefresh(this->win);
}

