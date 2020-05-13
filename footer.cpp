#include "footer.h"

Footer::Footer(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);
}

Footer::~Footer()
{
}

void Footer::Resize(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	wresize(this->win, height, width);
	mvwin(this->win, this->placementY, this->placementX);
}

void Footer::UpdateMode(Mode mode)
{
	this->mode = mode;
}

void Footer::Print()
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
	else if (mode == MODE_INTERFACE_DETAIL)
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



