#include "selectionWindow.h"

SelectionWindow::SelectionWindow(GraphType graphType, int placementX, int placementY, int width, int height)
{
	this->graphType = graphType;
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

	wrefresh(this->win);
	this->Update();
}

void SelectionWindow::Update()
{
	char graphTypeString[graphTypeStringSize];
	for (int i = 0; i < GT_END; i++)
	{
		getGraphTypeString((GraphType)i, graphTypeString);
		if (i == activeItem)
		{
			wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
		}
		mvwprintw(this->win, i + 1, 1, "%s", graphTypeString);
		if (i == activeItem)
		{
			wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
		}
	}

	wrefresh(this->win);
}

GraphType SelectionWindow::GetActiveItemGraphType()
{
	return (GraphType)activeItem;
}

