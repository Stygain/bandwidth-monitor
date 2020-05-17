#include "selectionWindow.h"


SelectionWindow::SelectionWindow(GraphType graphType, int placementX, int placementY, int width, int height)
{
	this->graphType = graphType;
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);

	this->Update();
}

void SelectionWindow::AutoResize()
{
	this->placementX = this->graph->GetPlacementX();
	this->placementY = this->graph->GetPlacementY() + 1;
	this->width = graphTypeStringSize + 1;
	this->height = (int)GT_END + 2;

	wresize(this->win, this->height, this->width);
	mvwin(this->win, this->placementY, this->placementX);

	this->Update();
}

void SelectionWindow::Resize(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	wresize(this->win, this->height, this->width);
	mvwin(this->win, this->placementY, this->placementX);

	this->Update();
}

void SelectionWindow::Update()
{
	werase(this->win);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

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

int SelectionWindow::GetActiveItem()
{
	return this->activeItem;
}

void SelectionWindow::SetActiveItem(int activeItem)
{
	this->activeItem = activeItem;
	this->activeItem = modulo(this->activeItem, GT_END);

	this->Update();
}

void SelectionWindow::IncrementActiveItem()
{
	this->activeItem++;
	this->activeItem = modulo(this->activeItem, GT_END);

	this->Update();
}

void SelectionWindow::DecrementActiveItem()
{
	this->activeItem--;
	this->activeItem = modulo(this->activeItem, GT_END);

	this->Update();
}

void SelectionWindow::SetGraph(Graph *graph)
{
	this->graph = graph;
}

