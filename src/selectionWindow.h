#ifndef SELECTIONWINDOW_H
#define SELECTIONWINDOW_H

#include <ncurses.h>

#include "utils.h"
#include "colors.h"


class SelectionWindow
{
	public:
		SelectionWindow(GraphType graphType, int placementX, int placementY, int width, int height);

		void Update();

		GraphType GetActiveItemGraphType();

		int GetActiveItem();
		void SetActiveItem(int activeItem);
		void IncrementActiveItem();
		void DecrementActiveItem();
	
	private:
		WINDOW *win;

		GraphType graphType;

		int placementX;
		int placementY;
		int width;
		int height;

		int activeItem = -1;
};

#endif
