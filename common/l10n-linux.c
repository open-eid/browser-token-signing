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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "esteid_log.h"
#include "l10n.h"

char *EstEID_findLine(FILE *file, const char *prefix) {
	char *result = NULL;
	for (char *line = NULL; !result; free(line)) {
		size_t size = 0;
		int len = getline(&line, &size, file);
		if (len == -1) break;
		char *l = line;
		while (isspace(*l)) l++;
		if (*l == '#' || *l == ';') continue;
		if (!strncmp(prefix, l, strlen(prefix))) {
			l += strlen(prefix);
			while (isspace(*l) || *l == '=') l++;
			while	(isspace(l[strlen(l) - 1])) l[strlen(l) - 1] = 0;
			result = strdup(l);
		}
	}
	return result;
}

#define CONFIG_FILE "/.config/Estonian ID Card.conf"

char *EstEID_getUserLocale() {
	char *home = getenv("HOME");
	char *filename = malloc(strlen(home) + strlen(CONFIG_FILE) + 1);
	sprintf(filename, "%s%s", home, CONFIG_FILE);
	FILE *f = fopen(filename, "r");
	free(filename);
	char *locale = NULL;
	if (f) {
		locale = EstEID_findLine(f, "Language");
		EstEID_log("EstEID_getUserLocale\t locale from conf file: %s", locale);
		fclose(f);
	}
	return locale;
}

