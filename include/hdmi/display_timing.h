/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */



/*
 * A single signal can be specified via a range of minimal and maximal values
 * with a typical value, that lies somewhere inbetween.
 */
struct timing_entry {
	uint32_t min;
	uint32_t typ;
	uint32_t max;
};

enum display_flags {
	DISPLAY_FLAGS_HSYNC_LOW		= 1 << 0,
	DISPLAY_FLAGS_HSYNC_HIGH	= 1 << 1,
	DISPLAY_FLAGS_VSYNC_LOW		= 1 << 2,
	DISPLAY_FLAGS_VSYNC_HIGH	= 1 << 3,

	/* data enable flag */
	DISPLAY_FLAGS_DE_LOW		= 1 << 4,
	DISPLAY_FLAGS_DE_HIGH		= 1 << 5,
	/* drive data on pos. edge */
	DISPLAY_FLAGS_PIXDATA_POSEDGE	= 1 << 6,
	/* drive data on neg. edge */
	DISPLAY_FLAGS_PIXDATA_NEGEDGE	= 1 << 7,
	DISPLAY_FLAGS_INTERLACED	= 1 << 8,
	DISPLAY_FLAGS_DOUBLESCAN	= 1 << 9,
	DISPLAY_FLAGS_DOUBLECLK		= 1 << 10,
};

struct display_timing {
	struct timing_entry pixelclock;

	struct timing_entry hactive;		/* hor. active video */
	struct timing_entry hfront_porch;	/* hor. front porch */
	struct timing_entry hback_porch;	/* hor. back porch */
	struct timing_entry hsync_len;		/* hor. sync len */

	struct timing_entry vactive;		/* ver. active video */
	struct timing_entry vfront_porch;	/* ver. front porch */
	struct timing_entry vback_porch;	/* ver. back porch */
	struct timing_entry vsync_len;		/* ver. sync len */

	enum display_flags flags;		/* display flags */
	int hdmi_monitor;			/* is hdmi monitor? */
};


