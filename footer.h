#ifndef FOOTER_H
#define FOOTER_H

#include <ncurses.h>

#include "utils.h"
#include "colors.h"

class Footer
{
	public:
		Footer(int placementX, int placementY, int width, int height);

		~Footer();

		void Resize(int placementX, int placementY, int width, int height);

		void UpdateMode(Mode mode);

		void Print();

	public:
		WINDOW *win;

	private:
		Mode mode = MODE_NORMAL;

		int placementX;
		int placementY;
		int width;
		int height;
};

#endif
