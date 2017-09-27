#ifndef EVENT_LIST_INCLUDED
#define EVENT_LIST_INCLUDED

class EventList;

#include <wx/wx.h>
#include "sequence-editor.h"

#define EVENT_LIST_LAST_COLUMN_NUMBER 7

class EventList
{
public:
	SequenceEditor *sequence_editor;
	wxFont font;
	wxColour current_cell_border_color;
	long row_height;
	long column_widths[8];

	EventList(SequenceEditor* sequence_editor);
	void RefreshData();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetFirstVisibleRowNumber();
	long GetLastVisibleRowNumber();
	long GetLastVisiblePopulatedRowNumber();
	long GetColumnWidth(long column_number);
	long GetXFromColumnNumber(long column_number);
	long GetYFromRowNumber(long row_number);
	long GetRowNumberFromY(long y);
};

#endif
