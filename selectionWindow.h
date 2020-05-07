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

	public:
		int activeItem = -1;
		int max = (int)GT_END;


	private:
		WINDOW *win;

		GraphType graphType;

		int placementX;
		int placementY;
		int width;
		int height;
};

#endif
