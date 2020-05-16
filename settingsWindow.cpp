#include "settingsWindow.h"


extern Settings *settings;
extern Logger *logger;

SettingsWindow::SettingsWindow(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);

	this->Update();
}

void SettingsWindow::Update()
{
	werase(this->win);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

	char graphTypeString[graphTypeStringSize];
	for (int i = 0; i < SWO_END; i++)
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

int SettingsWindow::GetActiveItem()
{
	return this->activeItem;
}

void SettingsWindow::SetActiveItem(int activeItem)
{
	this->max = SWO_END + settings->root["hiddenInterfaces"].size();
	logger->Log("MAXIMUM: ");
	logger->Log(std::to_string(this->max));
	logger->Log("\n");
	this->activeItem = activeItem;
	this->activeItem = modulo(this->activeItem, SWO_END);

	this->Update();
}

void SettingsWindow::IncrementActiveItem()
{
	this->activeItem++;
	this->activeItem = modulo(this->activeItem, SWO_END);

	this->Update();
}

void SettingsWindow::DecrementActiveItem()
{
	this->activeItem--;
	this->activeItem = modulo(this->activeItem, SWO_END);

	this->Update();
}
