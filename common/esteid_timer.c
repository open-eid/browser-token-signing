/*
 * Estonian ID card plugin for web browsers
 *
 * Copyright (C) 2010-2011 Codeborne <info@codeborne.com>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdlib.h>
#include "esteid_timer.h"
#include "esteid_log.h"

struct timeval EstEID_startTimer() {
	struct timeval start;
	gettimeofday(&start, NULL);
	return start;
};

void EstEID_stopTimerAndLog(struct timeval start, const char *message) {
	struct timeval end;
	long s;
	long ms;
	gettimeofday(&end, NULL);

	s = end.tv_sec - start.tv_sec;
	ms = (end.tv_usec - start.tv_usec) / 1000;
	if (ms < 0) {
		s--;
		ms = 1000 + ms;
	}

	EstEID_log("%s time: %li.%03li sec", message, s, ms);
};

