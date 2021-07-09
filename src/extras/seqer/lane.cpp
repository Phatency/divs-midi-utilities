
#include <QAction>
#include <QBrush>
#include <QColor>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QSettings>
#include <QWidget>
#include "lane.h"
#include "midifile.h"
#include "window.h"

Lane::Lane(Window* window)
{
	this->window = window;
	this->setFocusPolicy(Qt::StrongFocus);

	QAction* edit_event_action = new QAction(tr("Edit Event"));
	this->addAction(edit_event_action);
	edit_event_action->setShortcut(QKeySequence(Qt::Key_Return));
	connect(edit_event_action, SIGNAL(triggered()), this, SLOT(editEvent()));

	QAction* select_event_action = new QAction(tr("Select Event"));
	this->addAction(select_event_action);
	select_event_action->setShortcut(QKeySequence(Qt::Key_Space));
	connect(select_event_action, SIGNAL(triggered()), this, SLOT(selectEvent()));

	QAction* toggle_event_selection_action = new QAction(tr("Toggle Event Selection"));
	this->addAction(toggle_event_selection_action);
	toggle_event_selection_action->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Space));
	connect(toggle_event_selection_action, SIGNAL(triggered()), this, SLOT(toggleEventSelection()));

	QAction* cursor_left_action = new QAction(tr("Cursor Left"));
	this->addAction(cursor_left_action);
	cursor_left_action->setShortcut(QKeySequence(Qt::Key_Left));
	connect(cursor_left_action, SIGNAL(triggered()), this, SLOT(cursorLeft()));

	QAction* cursor_right_action = new QAction(tr("Cursor Right"));
	this->addAction(cursor_right_action);
	cursor_right_action->setShortcut(QKeySequence(Qt::Key_Right));
	connect(cursor_right_action, SIGNAL(triggered()), this, SLOT(cursorRight()));

	QAction* cursor_up_action = new QAction(tr("Cursor Up"));
	this->addAction(cursor_up_action);
	cursor_up_action->setShortcut(QKeySequence(Qt::Key_Up));
	connect(cursor_up_action, SIGNAL(triggered()), this, SLOT(cursorUp()));

	QAction* cursor_down_action = new QAction(tr("Cursor Down"));
	this->addAction(cursor_down_action);
	cursor_down_action->setShortcut(QKeySequence(Qt::Key_Down));
	connect(cursor_down_action, SIGNAL(triggered()), this, SLOT(cursorDown()));

	QSettings settings;
	QColor button_color = QGuiApplication::palette().color(QPalette::Button);
	QColor highlight_color = QGuiApplication::palette().color(QPalette::Highlight);
	this->background_color = settings.value("lane/background-color", QColorConstants::White).value<QColor>();
	this->white_note_background_color = settings.value("lane/white-note-background-color", QColorConstants::White).value<QColor>();
	this->black_note_background_color = settings.value("lane/black-note-background-color", QColor::fromHsl(button_color.hslHue(), button_color.hslSaturation(), 230)).value<QColor>();
	this->unselected_event_pen = QPen(settings.value("lane/unselected-event-border-color", QColorConstants::Black).value<QColor>());
	this->unselected_event_brush = QBrush(settings.value("lane/unselected-event-color", QColorConstants::White).value<QColor>());
	this->unselected_event_text_pen = QPen(settings.value("lane/unselected-event-text-color", QColorConstants::Black).value<QColor>());
	this->selected_event_pen = QPen(settings.value("lane/selected-event-border-color", QColorConstants::Black).value<QColor>());
	this->selected_event_brush = QBrush(settings.value("lane/selected-event-color", QColor::fromHsl(highlight_color.hslHue(), highlight_color.hslSaturation(), 200)).value<QColor>());
	this->selected_event_text_pen = QPen(settings.value("lane/selected-event-text-color", QColorConstants::White).value<QColor>());
	this->cursor_pen = QPen(settings.value("lane/cursor-color", QColor::fromHsl(highlight_color.hslHue(), highlight_color.hslSaturation(), 63)).value<QColor>());
	this->selection_rect_pen = QPen(settings.value("lane/selection-rect-color", QColor::fromHsl(highlight_color.hslHue(), highlight_color.hslSaturation(), 127)).value<QColor>(), 1.5, Qt::DashLine);
	this->mouse_drag_threshold = settings.value("lane/mouse-drag-threshold", 8).toInt();

	this->track = MidiFile_getTrackByNumber(this->window->sequence->midi_file, 1, 1);
}

void Lane::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	this->paintBackground(&painter);

	int selected_events_x_offset = 0;
	int selected_events_y_offset = 0;

	if (this->mouse_down && (this->mouse_down_midi_event == NULL))
	{
 		if ((this->mouse_drag_x != this->mouse_down_x) && (this->mouse_drag_y != this->mouse_down_y))
		{
			painter.setPen(this->selection_rect_pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(this->mouse_down_x, this->mouse_down_y, this->mouse_drag_x - this->mouse_down_x, this->mouse_drag_y - this->mouse_down_y);
		}
	}
	else
	{
		if (this->mouse_drag_x_allowed) selected_events_x_offset = this->mouse_drag_x - this->mouse_down_x;
		if (this->mouse_drag_y_allowed) selected_events_y_offset = this->mouse_drag_y - this->mouse_down_y;
	}

	painter.setPen(this->cursor_pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawLine(this->cursor_x, 0, this->cursor_x, this->height());

	this->paintEvents(&painter, selected_events_x_offset, selected_events_y_offset);
}

void Lane::mousePressEvent(QMouseEvent* event)
{
	qDebug("mousePressEvent(%d, %d)", (int)(event->position().x()), (int)(event->position().y()));

	if (event->button() == Qt::LeftButton)
	{
		this->mouse_down = true;
		this->mouse_down_x = event->position().x();
		this->mouse_down_y = event->position().y();
		this->mouse_down_midi_event = this->getEventFromXY(this->mouse_down_x, this->mouse_down_y);
		this->mouse_down_midi_event_is_new = false;
		this->mouse_drag_x = this->mouse_down_x;
		this->mouse_drag_y = this->mouse_down_y;
		this->mouse_drag_x_allowed = false;
		this->mouse_drag_y_allowed = false;

		if ((this->mouse_down_midi_event == NULL) && ((event->modifiers() & Qt::ShiftModifier) == 0))
		{
			this->window->selectNone();

			if ((this->mouse_down_x == this->cursor_x) && (this->mouse_down_y == this->cursor_y))
			{
				this->mouse_down_midi_event = this->addEventAtXY(this->mouse_down_x, this->mouse_down_y);
				MidiFileEvent_setSelected(this->mouse_down_midi_event, 1);
				this->mouse_down_midi_event_is_new = true;
			}

			this->window->sequence->updateWindows();
		}
	}
}

void Lane::mouseReleaseEvent(QMouseEvent* event)
{
	qDebug("mouseReleaseEvent(%d, %d)", (int)(event->position().x()), (int)(event->position().y()));

	bool should_update = false;

	if (event->button() == Qt::LeftButton)
	{
		int mouse_x = event->position().x();
		int mouse_y = event->position().y();

		if (this->mouse_down_midi_event == NULL)
		{
			if ((mouse_x == this->mouse_down_x) && (mouse_y == this->mouse_down_y))
			{
				this->cursor_x = mouse_x;
				this->cursor_y = mouse_y;
			}
			else
			{
				this->selectEventsInRect(std::min(this->mouse_down_x, mouse_x), std::min(this->mouse_down_y, mouse_y), std::abs(mouse_x - this->mouse_down_x), std::abs(mouse_y - this->mouse_down_y));
			}

			should_update = true;
		}
		else
		{
			if ((this->mouse_drag_x_allowed && (mouse_x != this->mouse_down_x)) || (this->mouse_drag_y_allowed && (mouse_y != this->mouse_down_y)))
			{
				int x_offset = this->mouse_drag_x_allowed ? (this->mouse_drag_x - this->mouse_down_x) : 0;
				int y_offset = this->mouse_drag_y_allowed ? (this->mouse_drag_y - this->mouse_down_y) : 0;

				while (MidiFileEvent_t midi_event = MidiFile_iterateEvents(this->window->sequence->midi_file))
				{
					if (MidiFileEvent_isSelected(midi_event))
					{
						this->moveEventByXY(midi_event, x_offset, y_offset);
						should_update = true;
					}
				}
			}
			else
			{
				if (((event->modifiers() & Qt::ShiftModifier) == 0) && !this->mouse_down_midi_event_is_new && (mouse_x == this->cursor_x) && (mouse_y == this->cursor_y))
				{
					this->window->focusInspector();
				}
			}
		}

		this->mouse_down = false;
		this->mouse_down_midi_event = NULL;
		this->mouse_down_midi_event_is_new = false;
		this->mouse_drag_x_allowed = false;
		this->mouse_drag_y_allowed = false;
		if (should_update) this->window->sequence->updateWindows();
	}
}

void Lane::mouseMoveEvent(QMouseEvent* event)
{
	// qDebug("mouseMoveEvent(%d, %d)", (int)(event->position().x()), (int)(event->position().y()));

	if (this->mouse_down)
	{
		this->mouse_drag_x = event->position().x();
		this->mouse_drag_y = event->position().y();
		if (abs(this->mouse_drag_x - this->mouse_down_x) > this->mouse_drag_threshold) this->mouse_drag_x_allowed = true;
		if (abs(this->mouse_drag_y - this->mouse_down_y) > this->mouse_drag_threshold) this->mouse_drag_y_allowed = true;
		this->update();
	}
}

void Lane::editEvent()
{
	bool selection_is_empty = true;
	bool should_update = false;

	for (MidiFileEvent_t midi_event = MidiFile_getFirstEvent(this->window->sequence->midi_file); midi_event != NULL; midi_event = MidiFileEvent_getNextEventInFile(midi_event))
	{
		if (MidiFileEvent_isSelected(midi_event))
		{
			selection_is_empty = false;
			break;
		}
	}

	if (selection_is_empty)
	{
		MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->cursor_x, this->cursor_y);

		if (cursor_midi_event == NULL)
		{
			cursor_midi_event = this->addEventAtXY(this->cursor_x, this->cursor_y);
			MidiFileEvent_setSelected(cursor_midi_event, 1);
			should_update = true;
		}
		else
		{
			MidiFileEvent_setSelected(cursor_midi_event, 1);
			this->window->focusInspector();
		}
	}
	else
	{
		this->window->focusInspector();
	}

	if (should_update) this->window->sequence->updateWindows();
}

void Lane::selectEvent()
{
	MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->cursor_x, this->cursor_y);

	if (cursor_midi_event == NULL)
	{
		this->window->selectNone();
	}
	else
	{
		this->window->selectNone();
		MidiFileEvent_setSelected(cursor_midi_event, 1);
	}

	this->window->sequence->updateWindows();
}

void Lane::toggleEventSelection()
{
	MidiFileEvent_t cursor_midi_event = this->getEventFromXY(this->cursor_x, this->cursor_y);

	if (cursor_midi_event == NULL)
	{
		this->window->selectNone();
	}
	else
	{
		MidiFileEvent_setSelected(cursor_midi_event, !MidiFileEvent_isSelected(cursor_midi_event));
	}

	this->window->sequence->updateWindows();
}

void Lane::cursorLeft()
{
	this->cursor_x--;
	this->update();
}

void Lane::cursorRight()
{
	this->cursor_x++;
	this->update();
}

void Lane::cursorUp()
{
	this->cursor_y--;
	this->update();
}

void Lane::cursorDown()
{
	this->cursor_y++;
	this->update();
}
