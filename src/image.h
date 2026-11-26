#pragma once

#include <utility>

#include <graphics/image-file.h>
#include <obs.h>

#include "dispatch_queue.h"

class Image {
public:
	Image() = default;
	Image(const char *path)
	{
		obs_enter_graphics();
		me_ = gs_texture_create_from_file(path);
		if (me_) {
			cx_ = gs_texture_get_width(me_);
			cy_ = gs_texture_get_height(me_);
		}
		obs_leave_graphics();

		if (!me_)
			throw std::runtime_error("Failed to load image");
	}
	~Image() { reset(); }

	Image(const Image &) = delete;
	Image &operator=(const Image &) = delete;

	Image(Image &&o) noexcept
		: me_(std::exchange(o.me_, nullptr)),
		  cx_(std::exchange(o.cx_, 0)),
		  cy_(std::exchange(o.cy_, 0))
	{
	}

	Image &operator=(Image &&o) noexcept
	{
		if (this == &o)
			return *this;

		reset();
		me_ = std::exchange(o.me_, nullptr);
		cx_ = std::exchange(o.cx_, 0);
		cy_ = std::exchange(o.cy_, 0);
		return *this;
	}

	gs_texture_t *texture() const { return me_; }
	uint32_t cx() const { return cx_; }
	uint32_t cy() const { return cy_; }

private:
	void reset()
	{
		if (!me_)
			return;

		auto *me = std::exchange(me_, nullptr);
		gTeardownQueue->async([me]() {
			obs_enter_graphics();
			gs_texture_destroy(me);
			obs_leave_graphics();
		});
	}

	gs_texture_t *me_ = nullptr;
	uint32_t cx_ = 0;
	uint32_t cy_ = 0;
};
