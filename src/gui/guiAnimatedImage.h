#pragma once

#include "irrlichttypes_extrabloated.h"
#include <string>

class ISimpleTextureSource;

class GUIAnimatedImage : public gui::IGUIElement {
public:
	GUIAnimatedImage(gui::IGUIEnvironment *env, gui::IGUIElement *parent,
		int32_t id, const core::rect<s32> &rectangle, const std::string &texture_name,
		int32_t frame_count, int32_t frame_duration, ISimpleTextureSource *tsrc);

	virtual void draw() override;

	void setFrameIndex(int32_t frame);
	int32_t getFrameIndex() const { return m_frame_idx; };

private:
	ISimpleTextureSource *m_tsrc;

	video::ITexture *m_texture = nullptr;
	uint64_t m_global_time = 0;
	int32_t m_frame_idx = 0;
	int32_t m_frame_count = 1;
	uint64_t m_frame_duration = 1;
	uint64_t m_frame_time = 0;
};
