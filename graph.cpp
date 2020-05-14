#include "graph.h"

GraphDataColumn::GraphDataColumn(GraphType graphType, std::vector<Interface *> *interfaces, Interface *interface)
{
	this->graphType = graphType;
	this->interfaces = interfaces;
	this->interface = interface;
}

void GraphDataColumn::SetNext(GraphDataColumn *next)
{
	this->next = next;
}

void GraphDataColumn::Update()
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
					if (!this->interfaces->at(i)->IsHidden())
					{
						if (this->interfaces->at(i)->r_packetsLast != 0)
						{
							pktsRecvDiff += this->interfaces->at(i)->r_packets - this->interfaces->at(i)->r_packetsLast;
						}
					}
				}
			}
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
					if (!this->interfaces->at(i)->IsHidden())
					{
						if (this->interfaces->at(i)->r_bytesLast != 0)
						{
							bytesRecvDiff += this->interfaces->at(i)->r_bytes - this->interfaces->at(i)->r_bytesLast;
						}
					}
				}
			}
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
					if (!this->interfaces->at(i)->IsHidden())
					{
						if (this->interfaces->at(i)->t_packetsLast != 0)
						{
							pktsSendDiff += this->interfaces->at(i)->t_packets - this->interfaces->at(i)->t_packetsLast;
						}
					}
				}
			}
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
					if (!this->interfaces->at(i)->IsHidden())
					{
						if (this->interfaces->at(i)->t_bytesLast != 0)
						{
							bytesSendDiff += this->interfaces->at(i)->t_bytes - this->interfaces->at(i)->t_bytesLast;
						}
					}
				}
			}
			this->value = bytesSendDiff;
		}
	}
	else
	{
		this->value = this->next->GetValue();
	}
}

void GraphDataColumn::UpdateGraphInterface(Interface *interface)
{
	this->interface = interface;

	this->Update();
}

void GraphDataColumn::Clear()
{
	this->value = 0;
}

int GraphDataColumn::GetValue()
{
	extern Logger *logger;
	return this->value;
}

void GraphDataColumn::SetValue(int newValue)
{
	this->value = newValue;
}


GraphRow::GraphRow(int height, int width, int placementX, int placementY, int value, int max)
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

GraphRow::~GraphRow()
{
}

void GraphRow::Update(std::vector<GraphDataColumn *> *gDataCols, int max)
{
	werase(this->win);
	float currPos = (float)this->value / (float)this->max;
	for (size_t i = 0; i < gDataCols->size(); i++)
	{
		float valuePercent = (float)gDataCols->at(i)->GetValue() / (float)max;
		float split = (float)1 / (float)this->max;
		if (valuePercent > currPos && valuePercent <= (currPos + split))
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


GraphTitle::GraphTitle(GraphType graphType, int placementX, int placementY, int width, int height, Interface *interface)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->interface = interface;

	this->win = newwin(this->height, this->width, this->placementY, this->placementX);

	this->UpdateGraphType(graphType);
}

void GraphTitle::Resize(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	wresize(this->win, this->height, this->width);
	mvwin(this->win, this->placementY, this->placementX);

	this->Update();
}

void GraphTitle::Update()
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

void GraphTitle::UpdateGraphType(GraphType graphType)
{
	this->graphType = graphType;

	this->Update();
}

void GraphTitle::UpdateGraphInterface(Interface *interface)
{
	this->interface = interface;

	this->Update();
}

void GraphTitle::setActive(bool active)
{
	this->active = active;

	this->Update();
}


GraphMaxItem::GraphMaxItem(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->win = newwin(this->height, this->width, this->placementY + 1, this->placementX);

	wrefresh(this->win);
}

void GraphMaxItem::Resize(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	wresize(this->win, this->height, this->width);
	mvwin(this->win, this->placementY, this->placementX);

	this->Update();
}

void GraphMaxItem::Update()
{
	werase(this->win);

	wprintw(this->win, "%d", this->max);

	wrefresh(this->win);
}

void GraphMaxItem::UpdateMaxItem(int max)
{
	this->max = max;

	this->Update();
}


Graph::Graph(GraphType graphType, int placementX, int placementY, int width, int height, std::vector<Interface *> *interfaces)
{
	this->graphType = graphType;
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;
	this->interfaces = interfaces;

	this->graphTitle = new GraphTitle(this->graphType, this->placementX + 1, this->placementY, this->width, 1, NULL);
	this->graphMaxItem = new GraphMaxItem(this->placementX + 2, this->placementY + 1, 5, 1);

	this->win = newwin(this->height, this->width, this->placementY + 1, this->placementX);
	wborder(this->win, 0, 0, 0, 0, 0, 0, 0, 0);

	wrefresh(this->win);
}

Graph::~Graph()
{
	gDataCols.clear();
	gRows.clear();
	delete this->graphTitle;
	delete this->graphMaxItem;
}

void Graph::Create()
{
	this->numCols = (this->width - 3);
	for (int i = 0; i < this->numCols; i++)
	{
		this->gDataCols.push_back(new GraphDataColumn(this->graphType, this->interfaces, this->interface));
	}

	for (size_t i = 0; i < (this->gDataCols.size() - 1); i++)
	{
		gDataCols[i]->SetNext(gDataCols[i+1]);
	}

	this->numRows = (this->height - 2);
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

void Graph::Resize(int placementX, int placementY, int width, int height)
{
	this->placementX = placementX;
	this->placementY = placementY;
	this->width = width;
	this->height = height;

	this->graphTitle->Resize(this->placementX + 1, this->placementY, this->width, 1);
	this->graphMaxItem->Resize(this->placementX + 2, this->placementY + 2, 5, 1);

	//TODO update the rows and columns as well
	int oldCols = this->numCols;
	this->numCols = (this->width - 3);
	std::vector<GraphDataColumn *> newCols;
	for (int i = 0; i < this->numCols; i++)
	{
		newCols.push_back(new GraphDataColumn(this->graphType, this->interfaces, this->interface));
	}

	extern Logger *logger;
	//logger->Log("Old Width: ");
	//logger->Log(std::to_string(oldCols));
	//logger->Log("\n");
	//logger->Log("New Width: ");
	//logger->Log(std::to_string(this->numCols));
	//logger->Log("\n");
	

	int smallerCols = oldCols < this->numCols ? oldCols : this->numCols;
	//logger->Log("Smaller: ");
	//logger->Log(std::to_string(smallerCols));
	//logger->Log("\n");
	for (int i = 1; i < smallerCols + 1; i++)
	{
		//logger->Log("Indexing: ");
		//logger->Log(std::to_string(this->numCols - i));
		//logger->Log(" and: ");
		//logger->Log(std::to_string(oldCols - i));
		//logger->Log(" value: ");
		//logger->Log(std::to_string(this->gDataCols[oldCols - i]->GetValue()));
		//logger->Log("\n");
		newCols[this->numCols - i]->SetValue(this->gDataCols[oldCols - i]->GetValue());
	}
	this->gDataCols.clear();
	this->gDataCols = newCols;

	for (size_t i = 0; i < (this->gDataCols.size() - 1); i++)
	{
		this->gDataCols[i]->SetNext(gDataCols[i+1]);
	}

	int oldRows = this->numRows;
	this->numRows = (this->height - 2);
	std::vector<GraphRow *> newRows;
	for (int i = 0; i < this->numRows; i++)
	{
		newRows.push_back(
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

	this->gRows.clear();
	this->gRows = newRows;

	wresize(this->win, height, width);
	mvwin(this->win, this->placementY + 1, this->placementX);

	this->Update();

	this->graphTitle->Update();
}

void Graph::Update()
{
	// Update the data
	for (size_t i = 0; i < this->gDataCols.size(); i++)
	{
		gDataCols[i]->Update();
	}

	this->Print();
}

void Graph::Print()
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

	this->graphMaxItem->UpdateMaxItem(max);

	this->graphTitle->Update();

	wrefresh(this->win);
}

void Graph::Clear()
{
	for (size_t i = 0; i < this->gDataCols.size(); i++)
	{
		gDataCols[i]->Clear();
	}

	for (size_t i = 0; i < this->gRows.size(); i++)
	{
		gRows[i]->Update(&(this->gDataCols), 2);
	}

	this->graphMaxItem->UpdateMaxItem(0);

	wrefresh(this->win);
}

void Graph::UpdateGraphInterface(Interface *interface)
{
	this->interface = interface;

	this->graphTitle->UpdateGraphInterface(interface);

	for (size_t i = 0; i < this->gDataCols.size(); i++)
	{
		gDataCols[i]->UpdateGraphInterface(interface);
	}

	this->Clear();
	this->Update();
}

void Graph::setActive(bool active)
{
	this->graphTitle->setActive(active);
}

GraphType Graph::GetGraphType()
{
	return this->graphType;
}

int Graph::GetPlacementX()
{
	return this->placementX;
}

int Graph::GetPlacementY()
{
	return this->placementY;
}

int Graph::GetWidth()
{
	return this->width;
}

int Graph::GetHeight()
{
	return this->height;
}

Interface * Graph::GetInterface()
{
	return this->interface;
}
