#ifndef MARKER_LANE_INCLUDED
#define MARKER_LANE_INCLUDED

class MarkerLane;

#include "label-lane.h"
#include "window.h"

class MarkerLane: public LabelLane
{
public:
	MarkerLane(Window* window);
	~MarkerLane();
	virtual void PopulateLabels();
};

#endif
