#ifndef INTERFACE_H
#define INTERFACE_H

#include <string.h>
#include <vector>

#include <ncurses.h>

#include "utils.h"
#include "colors.h"
#include "logger.h"

typedef enum
{
	IDO_SELECT,
	IDO_HIDE,
	IDO_END
} InterfaceDetailOption;

#define interfaceDetailOptionStringSize 25

const char interfaceDetailOptionStrings[5][interfaceDetailOptionStringSize] =
{
	"Select for graph",
	"Hide interface",
	"END"
};

void getInterfaceDetailOptionString(InterfaceDetailOption interfaceDetailOption, char *interfaceDetailOptionString);


class InterfaceHeader
{
	public:
		InterfaceHeader(int longest);

		~InterfaceHeader();

		void PrintTabTitle(int index, char *text);

		void Print();

		int GetTabCount();

	public:
		WINDOW *win;
		int sortingHeader = -1;
		int activeTab = -1;

	private:
		int tabCount = 5;
		int longest;
};


class InterfaceRow
{
	public:
		InterfaceRow(int placementX, int placementY, int width, int height);

		~InterfaceRow();

		int GetPlacementX();

		int GetPlacementY();

		int GetWidth();

		int GetHeight();

	public:
		WINDOW *win;

	private:
		int placementX;
		int placementY;
		int width;
		int height;
};


// /sys/class/net/iface/type -> /usr/include/linux/if_arp.h
// /sys/class/net/iface/operstate
class Interface
{
	public:
		Interface(char *name, int longest, bool wireless);

		~Interface();

		void Refresh();

		void Update();

		void Print();

		void SetActive(bool active);

		void Zero();

		void UnZero();

		void setInterfaceRow(InterfaceRow *interfaceRow);

		InterfaceRow * getInterfaceRow();

		void RemoveFromUI();
		void AddToUI();
		bool IsHidden();

	public:
		char *name;
		unsigned long int r_bytes;
		unsigned long int r_bytesLast = 0;
		unsigned long int r_bytesLastLast = 0;
		unsigned long int r_bytesZeroed = 0;
		unsigned long int t_bytes;
		unsigned long int t_bytesLast = 0;
		unsigned long int t_bytesLastLast = 0;
		unsigned long int t_bytesZeroed = 0;
		unsigned long int r_packets;
		unsigned long int r_packetsLast = 0;
		unsigned long int r_packetsZeroed = 0;
		unsigned long int t_packets;
		unsigned long int t_packetsLast = 0;
		unsigned long int t_packetsZeroed = 0;
		char operstate[20];
		bool wireless;
		int linkQual;
		int levelQual;
		int noiseQual;
		int r_bps = 0;
		int t_bps = 0;

	private:
		InterfaceRow * interfaceRow = NULL;
		bool active = false;
		bool hidden = false;
		int longest;
};


class InterfaceDetailWindow
{
	public:
		InterfaceDetailWindow(int placementX, int placementY, int width, int height, Interface *interface);

		void Update();

		Interface *GetInterface();

		int GetActiveItem();
		void SetActiveItem(int activeItem);
		void IncrementActiveItem();
		void DecrementActiveItem();

	public:

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		Interface *interface;
		int activeItem = -1;
};


Interface *getMatchingInterface(char *ifname);

Interface *getMatchingInterface(const char *ifname);


#endif
