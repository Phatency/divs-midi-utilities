#ifndef EVENT_TYPE_LABEL_LANE_INCLUDED
#define EVENT_TYPE_LABEL_LANE_INCLUDED

class EventTypeLabelLane;

#include "label-lane.h"
#include "window.h"

class EventTypeLabelLane: public LabelLane
{
public:
	EventTypeLabelLane(Window* window);
	~EventTypeLabelLane();
	virtual void PopulateLabels();
};

#endif