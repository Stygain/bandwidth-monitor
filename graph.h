#ifndef GRAPH_H
#define GRAPH_H

// #include <iostream>
#include <string.h>
#include <vector>
// #include <time.h>
// #include <fstream>
// #include <stdlib.h>
// #include <unistd.h>
// #include <iomanip>

#include <ncurses.h>

#include "interface.h"
#include "utils.h"
#include "colors.h"

class GraphDataColumn
{
	public:
		GraphDataColumn(GraphType graphType, std::vector<Interface *> *interfaces, Interface *interface);

		void SetNext(GraphDataColumn *next);

		void Update();

		void UpdateGraphInterface(Interface *interface);

		void Clear();

		int GetValue();


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
		GraphRow(int height, int width, int placementX, int placementY, int value, int max);

		~GraphRow();

		void Update(std::vector<GraphDataColumn *> *gDataCols, int max);


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

class GraphTitle
{
	public:
		GraphTitle(GraphType graphType, int placementX, int placementY, int width, int height, Interface *interface);

		void Update();

		void UpdateGraphType(GraphType graphType);

		void UpdateGraphInterface(Interface *interface);

		void setActive(bool active);

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		GraphType graphType;
		Interface *interface;

		bool active = false;
};


class GraphMaxItem
{
	public:
		GraphMaxItem(int placementX, int placementY, int width, int height);

		void Update();

		void UpdateMaxItem(int max);

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		int max = 0;
};


class Graph
{
	public:
		Graph(GraphType graphType, int placementX, int placementY, int width, int height, std::vector<Interface *> *interfaces);

		~Graph();

		void Create();

		void Update();

		void Clear();

		void UpdateGraphInterface(Interface *interface);

		void setActive(bool active);

		GraphType GetGraphType();

		int GetPlacementX();

		int GetPlacementY();

		int GetWidth();

		int GetHeight();

		void SetUpdate(bool update);

		Interface * GetInterface();


	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		int numRows;
		int numCols;
		bool update = true;

		std::vector<Interface *> *interfaces;
		Interface *interface = NULL;

		GraphType graphType;
		GraphTitle *graphTitle;
		GraphMaxItem *graphMaxItem;
		std::vector<GraphRow *> gRows;
		std::vector<GraphDataColumn *> gDataCols;
};

#endif