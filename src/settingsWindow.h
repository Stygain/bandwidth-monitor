#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <string.h>

#include <ncurses.h>

#include "interface.h"
#include "settings.h"
#include "logger.h"
#include "utils.h"
#include "colors.h"


class SettingsWindow
{
	public:
		SettingsWindow(int placementX, int placementY, int width);

		void Resize(int placementX, int placementY, int width);

		void Update();

		int GetActiveItem();
		void SetActiveItem(int activeItem);
		void IncrementActiveItem();
		void DecrementActiveItem();

		SettingsWindowSelection Selected();
	
	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		int max = 0;

		int activeItem = -1;
};

#endif
