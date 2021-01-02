/*
Copyright (C) 2002-2013 Nikolaus Gebhardt
This file is part of the "Irrlicht Engine".
For conditions of distribution and use, see copyright notice in irrlicht.h

Modified 2019.05.01 by stujones11, Stuart Jones <stujones111@gmail.com>

This is a heavily modified copy of the Irrlicht CGUIScrollBar class
which includes automatic scaling of the thumb slider and hiding of
the arrow buttons where there is insufficient space.
*/

#pragma once

#include "irrlichttypes_extrabloated.h"

using namespace irr;
using namespace gui;

class GUIScrollBar : public IGUIElement
{
public:
	GUIScrollBar(IGUIEnvironment *environment, IGUIElement *parent, int32_t id,
			core::rect<s32> rectangle, bool horizontal, bool auto_scale);

	enum ArrowVisibility
	{
		HIDE,
		SHOW,
		DEFAULT
	};

	virtual void draw();
	virtual void updateAbsolutePosition();
	virtual bool OnEvent(const SEvent &event);

	int32_t getMax() const { return max_pos; }
	int32_t getMin() const { return min_pos; }
	int32_t getLargeStep() const { return large_step; }
	int32_t getSmallStep() const { return small_step; }
	int32_t getPos() const;

	void setMax(const int32_t &max);
	void setMin(const int32_t &min);
	void setSmallStep(const int32_t &step);
	void setLargeStep(const int32_t &step);
	void setPos(const int32_t &pos);
	void setPageSize(const int32_t &size);
	void setArrowsVisible(ArrowVisibility visible);

private:
	void refreshControls();
	int32_t getPosFromMousePos(const core::position2di &p) const;
	float range() const { return f32(max_pos - min_pos); }

	IGUIButton *up_button;
	IGUIButton *down_button;
	ArrowVisibility arrow_visibility = DEFAULT;
	bool is_dragging;
	bool is_horizontal;
	bool is_auto_scaling;
	bool dragged_by_slider;
	bool tray_clicked;
	int32_t scroll_pos;
	int32_t draw_center;
	int32_t thumb_size;
	int32_t min_pos;
	int32_t max_pos;
	int32_t small_step;
	int32_t large_step;
	int32_t drag_offset;
	int32_t page_size;
	int32_t border_size;

	core::rect<s32> slider_rect;
	video::SColor current_icon_color;
};
