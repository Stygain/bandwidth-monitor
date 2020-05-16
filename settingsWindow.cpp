#include "settingsWindow.h"


extern Settings *settings;
extern Logger *logger;

SettingsWindow::SettingsWindow(int placementX, int placementY, int width)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = SWO_END + settings->root["hiddenInterfaces"].size() + 3;

	this->max = SWO_END + settings->root["hiddenInterfaces"].size() - 1;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);

	this->Update();
}

void SettingsWindow::Resize(int placementX, int placementY, int width)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = SWO_END + settings->root["hiddenInterfaces"].size() + 3;

	wresize(this->win, this->height, this->width);
	mvwin(this->win, this->placementY, this->placementX);

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
			mvwprintw(this->win, placementYIndex, 5, "%s", settings->root["zeroOnStart"].asString().c_str());
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
	this->activeItem = activeItem;
	this->activeItem = modulo(this->activeItem, this->max);

	this->Update();
}

void SettingsWindow::IncrementActiveItem()
{
	this->activeItem++;
	this->activeItem = modulo(this->activeItem, this->max);

	this->Update();
}

void SettingsWindow::DecrementActiveItem()
{
	this->activeItem--;
	this->activeItem = modulo(this->activeItem, this->max);

	this->Update();
}

SettingsWindowSelection SettingsWindow::Selected()
{
	if (this->activeItem != -1)
	{
		if (this->activeItem < settings->root["hiddenInterfaces"].size())
		{
			// Unhide this interface
			logger->Log("Selected interface: ");
			logger->Log(settings->root["hiddenInterfaces"][this->activeItem].asString());
			logger->Log("\n");

			Interface *toUnhide = getMatchingInterface(settings->root["hiddenInterfaces"][this->activeItem].asString().c_str());

			Json::Value newArr;
			for (int i = 0; i < settings->root["hiddenInterfaces"].size(); i++)
			{
				if (i != this->activeItem)
				{
					newArr.append(settings->root["hiddenInterfaces"][i].asString());
				}
			}
			settings->root["hiddenInterfaces"] = newArr;
			settings->SaveSettings();
			
			// Resize window
			this->activeItem = -1;
			this->Resize(this->placementX, this->placementY, this->width);
			this->Update();

			extern std::vector<InterfaceRow *> interfaceRows;
			extern std::vector<Interface *> interfaces;

			InterfaceRow *interfaceRow = new InterfaceRow(0, (interfaceRows.size() * 3) + 1, COLS, 3);
			interfaceRows.push_back(interfaceRow);
			toUnhide->AddToUI();
			toUnhide->setInterfaceRow(interfaceRow);

			// Return a value so that in main we create an interface row for the unhidden interface
			return CREATE_NEW_INTERFACE;
		}
		else if (this->activeItem == settings->root["hiddenInterfaces"].size())
		{
			settings->root["zeroOnStart"] = settings->root["zeroOnStart"] == 0 ? 1 : 0;
			settings->SaveSettings();

			this->Update();
		}
	}

	return NO_ACTION;
}
