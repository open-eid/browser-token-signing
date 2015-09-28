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
#include "binary_utils.h"
#include <string.h>
#include "esteid_log.h"

char *hex2bin(const char *hex) {
	size_t binLength;
	char *bin;
	char *c;
	char *h;
	int i = 0;

	LOG_LOCATION;

	binLength = strlen(hex) / 2;
	bin = (char *)malloc(binLength);
	c = bin;
	h = (char *)hex;
	while (*h) {
		int x;
		sscanf(h, "%2X", &x);
		*c = x;
		c++;
		h += 2;
		i++;
	}
	return bin;
}