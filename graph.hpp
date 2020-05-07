#pragma once

#include <iostream>
#include <string.h>
#include <vector>
#include <time.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>

#include <ncurses.h>

#include "interface.hpp"
#include "colors.hpp"
#include "utils.hpp"

class GraphDataColumn
{
	public:
		GraphDataColumn(GraphType graphType, std::vector<Interface *> *interfaces, Interface *interface)
		{
			this->graphType = graphType;
			this->interfaces = interfaces;
			this->interface = interface;
		}

		void SetNext(GraphDataColumn *next)
		{
			this->next = next;
		}

		void Update()
		{
			if (next == NULL)
			{
				if (graphType == GT_PKTS_RECV)
				{
					int pktsRecvDiff = 0;
					if (this->interface != NULL)
					{
						if (this->interface->r_packetsLast != 0)
						{
							pktsRecvDiff += this->interface->r_packets - this->interface->r_packetsLast;
						}
					}
					else
					{
						for (size_t i = 0; i < this->interfaces->size(); i++)
						{
							if (this->interfaces->at(i)->r_packetsLast != 0)
							{
								pktsRecvDiff += this->interfaces->at(i)->r_packets - this->interfaces->at(i)->r_packetsLast;
							}
						}
					}
					//logfile << "Pkts Recv Diff: " << pktsRecvDiff << "\n";
					this->value = pktsRecvDiff;
				}
				else if (graphType == GT_BYTES_RECV)
				{
					int bytesRecvDiff = 0;
					if (this->interface != NULL)
					{
						if (this->interface->r_bytesLast != 0)
						{
							bytesRecvDiff += this->interface->r_bytes - this->interface->r_bytesLast;
						}
					}
					else
					{
						for (size_t i = 0; i < this->interfaces->size(); i++)
						{
							if (this->interfaces->at(i)->r_bytesLast != 0)
							{
								bytesRecvDiff += this->interfaces->at(i)->r_bytes - this->interfaces->at(i)->r_bytesLast;
							}
						}
					}
					//logfile << "Bytes Recv Diff: " << bytesRecvDiff << "\n";
					this->value = bytesRecvDiff;
				}
				else if (graphType == GT_PKTS_SEND)
				{
					int pktsSendDiff = 0;
					if (this->interface != NULL)
					{
						if (this->interface->t_packetsLast != 0)
						{
							pktsSendDiff += this->interface->t_packets - this->interface->t_packetsLast;
						}
					}
					else
					{
						for (size_t i = 0; i < this->interfaces->size(); i++)
						{
							if (this->interfaces->at(i)->t_packetsLast != 0)
							{
								pktsSendDiff += this->interfaces->at(i)->t_packets - this->interfaces->at(i)->t_packetsLast;
							}
						}
					}
					//logfile << "Pkts Send Diff: " << pktsSendDiff << "\n";
					this->value = pktsSendDiff;
				}
				else if (graphType == GT_BYTES_SEND)
				{
					int bytesSendDiff = 0;
					if (this->interface != NULL)
					{
						if (this->interface->t_bytesLast != 0)
						{
							bytesSendDiff += this->interface->t_bytes - this->interface->t_bytesLast;
						}
					}
					else
					{
						for (size_t i = 0; i < this->interfaces->size(); i++)
						{
							if (this->interfaces->at(i)->t_bytesLast != 0)
							{
								bytesSendDiff += this->interfaces->at(i)->t_bytes - this->interfaces->at(i)->t_bytesLast;
							}
						}
					}
					//logfile << "Bytes Send Diff: " << bytesSendDiff << "\n";
					this->value = bytesSendDiff;
				}
			}
			else
			{
				this->value = this->next->GetValue();
			}
		}

		void UpdateGraphInterface(Interface *interface)
		{
			this->interface = interface;

			this->Update();
		}

		void Clear()
		{
			this->value = 0;

			this->Update();
		}

		int GetValue()
		{
			return this->value;
		}


	private:
		int value = 0;
		GraphType graphType;
		std::vector<Interface *> *interfaces;
		Interface *interface = NULL;

		GraphDataColumn *next = NULL;
};


class GraphRow
{
	public:
		GraphRow(int height, int width, int placementX, int placementY, int value, int max)
		{
			this->max = max;
			this->value = value;
			this->placementY = placementY;
			this->placementX = placementX;
			this->height = height;
			this->width = width;
			this->win = newwin(this->height, this->width, this->placementY, this->placementX);

			wrefresh(this->win);
		}

		~GraphRow()
		{
		}

		void Update(std::vector<GraphDataColumn *> *gDataCols, int max)
		{
			werase(this->win);
			float currPos = (float)this->value / (float)this->max;
			for (size_t i = 0; i < gDataCols->size(); i++)
			{
				float valuePercent = (float)gDataCols->at(i)->GetValue() / (float)max;
				float split = (float)1 / (float)this->max;
				if (valuePercent >= currPos && valuePercent < (currPos + split))
				{
					mvwprintw(this->win, 0, i, "_");
				}
				else if (valuePercent > currPos)
				{
					mvwprintw(this->win, 0, i, "|");
				}
				else if (valuePercent == 0 && currPos == 0)
				{
					mvwprintw(this->win, 0, i, "_");
				}
			}

			wrefresh(this->win);
		}


	public:
		WINDOW *win;


	private:
		int value;
		int max;
		int placementY;
		int placementX;
		int height;
		int width;
};

class GraphTitle
{
	public:
		GraphTitle(GraphType graphType, int placementX, int placementY, int width, int height, Interface *interface)
		{
			this->placementX = placementX;
			this->placementY = placementY;
			this->width = width;
			this->height = height;

			this->interface = interface;

			this->win = newwin(this->height, this->width, this->placementY, this->placementX);

			this->UpdateGraphType(graphType);
		}

		void Update()
		{
			werase(this->win);

			char graphTypeString[graphTypeStringSize];
			getGraphTypeString(this->graphType, graphTypeString);

			if (this->active)
			{
				wattron(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
			}
			if (this->interface == NULL)
			{
				wprintw(this->win, "%s", graphTypeString);
			}
			else
			{
				wprintw(this->win, "%s through %s", graphTypeString, interface->name);
			}
			if (this->active)
			{
				wattroff(this->win, COLOR_PAIR(HEADER_ACTIVE_COLOR));
			}

			wrefresh(this->win);
		}

		void UpdateGraphType(GraphType graphType)
		{
			this->graphType = graphType;

			this->Update();
		}

		void UpdateGraphInterface(Interface *interface)
		{
			this->interface = interface;

			this->Update();
		}

		void setActive(bool active)
		{
			this->active = active;

			this->Update();
		}

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		GraphType graphType;
		Interface *interface;

		bool active = false;
};


class GraphMaxItem
{
	public:
		GraphMaxItem(int placementX, int placementY, int width, int height)
		{
			this->placementX = placementX;
			this->placementY = placementY;
			this->width = width;
			this->height = height;

			this->win = newwin(this->height, this->width, this->placementY + 1, this->placementX);

			wrefresh(this->win);
		}

		void Update()
		{
			werase(this->win);

			wprintw(this->win, "%d", this->max);

			wrefresh(this->win);
		}

		void UpdateMaxItem(int max)
		{
			this->max = max;

			this->Update();
		}

	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		int max = 0;
};


class Graph
{
	public:
		Graph(GraphType graphType, int placementX, int placementY, int width, int height, std::vector<Interface *> *interfaces)
		{
			this->graphType = graphType;
			this->placementX = placementX;
			this->placementY = placementY;
			this->width = width;
			this->height = height;
			this->interfaces = interfaces;

			graphTitle = new GraphTitle(this->graphType, this->placementX + 1, this->placementY, this->width, 1, NULL);
			graphMaxItem = new GraphMaxItem(this->placementX + 2, this->placementY + 1, 5, 1);

			this->win = newwin(this->height, this->width, this->placementY + 1, this->placementX);
			wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

			wrefresh(this->win);
		}

		~Graph()
		{
			gDataCols.clear();
			gRows.clear();
			delete graphTitle;
			delete graphMaxItem;
		}

		void Create()
		{
			this->numCols = (this->width - 3);
			//logfile << "This width: " << this->width << " number of columns: " << this->numCols << "\n";
			for (int i = 0; i < this->numCols; i++)
			{
				this->gDataCols.push_back(new GraphDataColumn(this->graphType, this->interfaces, this->interface));
			}

			for (size_t i = 0; i < (this->gDataCols.size() - 1); i++)
			{
				gDataCols[i]->SetNext(gDataCols[i+1]);
			}

			this->numRows = (this->height - 2);
			//logfile << "This height: " << this->height << " number of rows: " << this->numRows << "\n";
			for (int i = 0; i < this->numRows; i++)
			{
				this->gRows.push_back(
					new GraphRow(
						1,
						(this->width - 3),
						(this->placementX + 1),
						(this->placementY + 1 + ((i * 1) + 1)),
						(this->numRows - i - 1),
						(this->numRows - 1)
					)
				);
			}

			wrefresh(this->win);
		}

		void Update()
		{
			// Update the data
			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				gDataCols[i]->Update();
			}

			if (this->update)
			{
				// Scan the data columns for the maximum
				int max = 0;
				for (size_t i = 0; i < this->gDataCols.size(); i++)
				{
					int tempValue = gDataCols[i]->GetValue();
					if (tempValue > max)
					{
						max = tempValue;
					}
				}

				wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);
				wrefresh(this->win);

				// Print the graph contents
				for (size_t i = 0; i < this->gRows.size(); i++)
				{
					gRows[i]->Update(&(this->gDataCols), max);
				}

				graphMaxItem->UpdateMaxItem(max);

				wrefresh(this->win);
			}
		}

		void Clear()
		{
			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				gDataCols[i]->Clear();
			}

			for (size_t i = 0; i < this->gRows.size(); i++)
			{
				gRows[i]->Update(&(this->gDataCols), 2);
			}

			wrefresh(this->win);
		}

		void UpdateGraphInterface(Interface *interface)
		{
			this->interface = interface;

			graphTitle->UpdateGraphInterface(interface);

			for (size_t i = 0; i < this->gDataCols.size(); i++)
			{
				gDataCols[i]->UpdateGraphInterface(interface);
			}

			this->Clear();
			this->Update();
		}

		void setActive(bool active)
		{
			graphTitle->setActive(active);
		}

		GraphType GetGraphType()
		{
			return this->graphType;
		}

		int GetPlacementX()
		{
			return this->placementX;
		}

		int GetPlacementY()
		{
			return this->placementY;
		}

		int GetWidth()
		{
			return this->width;
		}

		int GetHeight()
		{
			return this->height;
		}

		void SetUpdate(bool update)
		{
			this->update = update;

			this->Update();
		}

		Interface * GetInterface()
		{
			return this->interface;
		}


	private:
		WINDOW *win;

		int placementX;
		int placementY;
		int width;
		int height;

		int numRows;
		int numCols;
		bool update = true;

		std::vector<Interface *> *interfaces;
		Interface *interface = NULL;

		GraphType graphType;
		GraphTitle *graphTitle;
		GraphMaxItem *graphMaxItem;
		std::vector<GraphRow *> gRows;
		std::vector<GraphDataColumn *> gDataCols;
};


