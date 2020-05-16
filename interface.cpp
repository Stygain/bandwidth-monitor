#include "interface.h"


void getInterfaceDetailOptionString(InterfaceDetailOption interfaceDetailOption, char *interfaceDetailOptionString)
{
	strcpy(interfaceDetailOptionString, interfaceDetailOptionStrings[interfaceDetailOption]);
};

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

int InterfaceRow::GetPlacementX()
{
	return this->placementX;
}

int InterfaceRow::GetPlacementY()
{
	return this->placementY;
}

int InterfaceRow::GetWidth()
{
	return this->width;
}

int InterfaceRow::GetHeight()
{
	return this->height;
}



Interface::Interface(char *name, int longest, bool wireless)
{
	this->longest = longest;
	this->wireless = wireless;
	this->name = new char[strlen(name) + 1];
	strcpy(this->name, name);
}

Interface::~Interface()
{
	delete this->name;
}

void Interface::Refresh()
{
	if (!this->hidden)
	{
		wrefresh(this->interfaceRow->win);
	}
}

void Interface::Update()
{
	extern Logger *logger;
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
		sscanf(buf, "%s %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
				ifname, &r_bytes, &r_packets, &t_bytes, &t_packets);
		ifname[strlen(ifname) - 1] = '\0';

		if (strcmp(ifname, this->name) == 0)
		{
			this->r_bytesLastLast = this->r_bytesLast;
			this->r_bytesLast = this->r_bytes;
			this->r_bytes = r_bytes;
			this->t_bytesLastLast = this->t_bytesLast;
			this->t_bytesLast = this->t_bytes;
			this->t_bytes = t_bytes;
			this->r_packetsLast = this->r_packets;
			this->r_packets = r_packets;
			this->t_packetsLast = this->t_packets;
			this->t_packets = t_packets;

			if (((this->r_bytes - this->r_bytesLast) + (this->r_bytesLast - this->r_bytesLastLast)) == 0)
			{
				this->r_bps = 0;
			}
			else
			{
				this->r_bps = (int)(((this->r_bytes - this->r_bytesLast) + (this->r_bytesLast - this->r_bytesLastLast)) / 2);
			}
			if (((this->t_bytes - this->t_bytesLast) + (this->t_bytesLast - this->t_bytesLastLast)) == 0)
			{
				this->t_bps = 0;
			}
			else
			{
				this->t_bps = (int)(((this->t_bytes - this->t_bytesLast) + (this->t_bytesLast - this->t_bytesLastLast)) / 2);
			}
			break;
		}
	}

	fclose(fp);

	char ifaceType[] = "/sys/class/net/";
	strcat(ifaceType, this->name);
	strcat(ifaceType, "/operstate");
	fp = fopen(ifaceType, "r");

	logger->Log("Iface file: ");
	logger->Log(ifaceType);
	logger->Log("\n");
	if (fp != NULL)
	{
		fgets(buf, 200, fp);
		sscanf(buf, "%s", this->operstate);
		logger->Log("This operstate: ");
		logger->Log(this->operstate);
		logger->Log("\n");

		fclose(fp);
	}

	if (this->wireless)
	{
		fp = fopen("/proc/net/wireless", "r");

		if (fp != NULL)
		{
			logger->Log("In /proc/net/wireless\n");
			// Skip first two lines
			for (int i = 0; i < 2; i++) {
				fgets(buf, 200, fp);
			}

			int linkQual;
			int levelQual;
			int noiseQual;

			char wirelessInterfaceName[20];
			while (fgets(buf, 200, fp)) {
				sscanf(buf, "%s %*lu %d. %d. %d",
						wirelessInterfaceName, &linkQual, &levelQual, &noiseQual);
				wirelessInterfaceName[strlen(wirelessInterfaceName) - 1] = '\0';

				if (strcmp(wirelessInterfaceName, this->name) == 0)
				{
					logger->Log("Read: ");
					logger->Log(std::to_string(linkQual));
					logger->Log(", ");
					logger->Log(std::to_string(levelQual));
					logger->Log(", ");
					logger->Log(std::to_string(noiseQual));
					logger->Log("\n");
					this->linkQual = linkQual;
					this->levelQual = levelQual;
					this->noiseQual = noiseQual;
				}
			}
		}
	}
}

void Interface::Print()
{
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

	if (!this->hidden)
	{
		werase(this->interfaceRow->win);
		wborder(this->interfaceRow->win, 0, 0, 0, 0, 0, 0, 0, 0);

		if (this->active) {
			wattron(this->interfaceRow->win, COLOR_PAIR(ACTIVE_COLOR));
		}

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

InterfaceRow * Interface::getInterfaceRow()
{
	return this->interfaceRow;
}

void Interface::RemoveFromUI()
{
	this->hidden = true;
}

void Interface::AddToUI()
{
	this->hidden = false;
}

bool Interface::IsHidden()
{
	return this->hidden;
}

InterfaceDetailWindow::InterfaceDetailWindow(int placementX, int placementY, int width, int height, Interface *interface)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->interface = interface;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);

	this->Update();
}

void InterfaceDetailWindow::Update()
{
	werase(this->win);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

	char interfaceDetailOptionString[interfaceDetailOptionStringSize];
	for (int i = 0; i < IDO_END; ++i)
	{
		getInterfaceDetailOptionString((InterfaceDetailOption)i, interfaceDetailOptionString);
		if (i == this->activeItem)
		{
			wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
		}
		mvwprintw(this->win, i + 1, 1, "%s", interfaceDetailOptionString);
		if (i == this->activeItem)
		{
			wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
		}
	}
	// TODO get max width of IDO strings
	mvwprintw(this->win, 1, 25, "State: %s", this->interface->operstate);

	if (this->interface->wireless)
	{
		mvwprintw(this->win, 2, 25, "Link Quality: %d", this->interface->linkQual);
		mvwprintw(this->win, 3, 25, "Level Quality: %d", this->interface->levelQual);
		mvwprintw(this->win, 4, 25, "Noise Quality: %d", this->interface->noiseQual);
	}

	char r_bpsUnit[3] = "B";
	int r_bps = this->interface->r_bps;
	if (r_bps > 1000)
	{
		r_bps = (int)(r_bps / 1000);
		strcpy(r_bpsUnit, "KB");
	}
	if (r_bps > 1000)
	{
		r_bps = (int)(r_bps / 1000);
		strcpy(r_bpsUnit, "MB");
	}
	if (r_bps > 1000)
	{
		r_bps = (int)(r_bps / 1000);
		strcpy(r_bpsUnit, "GB");
	}

	char t_bpsUnit[3] = "B";
	int t_bps = this->interface->t_bps;
	if (t_bps > 1000)
	{
		t_bps = (int)(t_bps / 1000);
		strcpy(t_bpsUnit, "KB");
	}
	if (t_bps > 1000)
	{
		t_bps = (int)(t_bps / 1000);
		strcpy(t_bpsUnit, "MB");
	}
	if (t_bps > 1000)
	{
		t_bps = (int)(t_bps / 1000);
		strcpy(t_bpsUnit, "GB");
	}
	mvwprintw(this->win, 1, 50, "Recv: %d %sps", r_bps, r_bpsUnit);
	mvwprintw(this->win, 2, 50, "Send: %d %sps", t_bps, t_bpsUnit);

	wrefresh(this->win);
}

Interface * InterfaceDetailWindow::GetInterface()
{
	return this->interface;
}

int InterfaceDetailWindow::GetActiveItem()
{
	return this->activeItem;
}

void InterfaceDetailWindow::SetActiveItem(int activeItem)
{
	this->activeItem = activeItem;
	this->activeItem = modulo(this->activeItem, IDO_END);

	this->Update();
}

void InterfaceDetailWindow::IncrementActiveItem()
{
	this->activeItem++;
	this->activeItem = modulo(this->activeItem, IDO_END);

	this->Update();
}

void InterfaceDetailWindow::DecrementActiveItem()
{
	this->activeItem--;
	this->activeItem = modulo(this->activeItem, IDO_END);

	this->Update();
}


Interface *getMatchingInterface(char *ifname)
{
	extern std::vector<Interface *> interfaces;

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		if (strcmp(interfaces[i]->name, ifname) == 0)
		{
			return interfaces[i];
		}
	}
	return NULL;
}

Interface *getMatchingInterface(const char *ifname)
{
	extern std::vector<Interface *> interfaces;

	for (size_t i = 0; i < interfaces.size(); ++i)
	{
		if (strcmp(interfaces[i]->name, ifname) == 0)
		{
			return interfaces[i];
		}
	}
	return NULL;
}

