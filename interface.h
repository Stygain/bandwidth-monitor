#ifndef INTERFACE_H
#define INTERFACE_H

#include <string.h>
#include <ncurses.h>

#include "utils.h"
#include "colors.h"



class InterfaceFooter
{
	public:
		InterfaceFooter();

		~InterfaceFooter();

		void UpdateMode(Mode mode);

		void Print();

	public:
		WINDOW *win;

	private:
		Mode mode;
};


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

	public:
		WINDOW *win;

	private:
		int placementX;
		int placementY;
		int width;
		int height;
};


class Interface
{
	public:
		Interface(char *name, int longest);

		Interface(char *name, int longest, InterfaceRow * interfaceRow);

		~Interface();

		void Refresh();

		void Update(unsigned long int r_bytes,
				unsigned long int t_bytes,
				unsigned long int r_packets,
				unsigned long int t_packets);

		void Print();

		void SetActive(bool active);

		void Zero();

		void UnZero();

		void setInterfaceRow(InterfaceRow *interfaceRow);

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
		int longest;
};


class InterfaceDetailWindow
{
	public:
		InterfaceDetailWindow(int placementX, int placementY, int width, int height, Interface *interface);

		void Update();

	public:

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		Interface *interface;
};


#endif
