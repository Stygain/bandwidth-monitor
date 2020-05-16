#include "settingsWindow.h"


extern Settings *settings;
extern Logger *logger;

SettingsWindow::SettingsWindow(int placementX, int placementY, int width)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = SWO_END + settings->root["hiddenInterfaces"].size() + 3;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);

	this->Update();
}

void SettingsWindow::Update()
{
	werase(this->win);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

	char settingsWindowOptionString[settingsWindowOptionStringSize];
	int placementYIndex = 0;
	for (int i = 0; i < SWO_END; i++)
	{
		placementYIndex++;
		getSettingsWindowOptionString((SettingsWindowOption)i, settingsWindowOptionString);
		if ((SettingsWindowOption)i == SWO_HIDDEN_INTERFACES)
		{
			mvwprintw(this->win, placementYIndex, 1, "%s", settingsWindowOptionString);
			for (int j = 0; j < settings->root["hiddenInterfaces"].size(); j++)
			{
				placementYIndex++;
				if (j == activeItem)
				{
					wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
				}
				mvwprintw(this->win, placementYIndex, 5, "%s", settings->root["hiddenInterfaces"][j].asString().c_str());
				if (j == activeItem)
				{
					wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
				}
			}
		}
		else if ((SettingsWindowOption)i == SWO_ZERO_ON_START)
		{
			mvwprintw(this->win, placementYIndex, 1, "%s", settingsWindowOptionString);
			if (settings->root["hiddenInterfaces"].size() == activeItem)
			{
				wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
			}
			placementYIndex++;
			mvwprintw(this->win, placementYIndex, 5, "0");
			if (settings->root["hiddenInterfaces"].size() == activeItem)
			{
				wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
			}
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
	this->max = SWO_END + settings->root["hiddenInterfaces"].size() - 1;
	logger->Log("MAXIMUM: ");
	logger->Log(std::to_string(this->max));
	logger->Log("\n");
	this->activeItem = activeItem;
	this->activeItem = modulo(this->activeItem, this->max);

	this->Update();
}

void SettingsWindow::IncrementActiveItem()
{
	this->max = SWO_END + settings->root["hiddenInterfaces"].size() - 1;
	this->activeItem++;
	this->activeItem = modulo(this->activeItem, this->max);

	this->Update();
}

void SettingsWindow::DecrementActiveItem()
{
	this->max = SWO_END + settings->root["hiddenInterfaces"].size() - 1;
	this->activeItem--;
	this->activeItem = modulo(this->activeItem, this->max);

	this->Update();
}
