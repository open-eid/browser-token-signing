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
#include <string.h>
#include <stdarg.h>
#include "esteid_time.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <share.h>
#include <io.h>
#define W_OK 2
#define access _access
#endif

#include "esteid_log.h"

#ifdef _WIN32
#define MAX_LOG_FILE_NAME_LEN _MAX_ENV + 12
char filename[MAX_LOG_FILE_NAME_LEN] = "";
#endif

const char *EstEID_getLogFilename() {
#ifdef _WIN32
	if (!filename[0]) {
		char *tempValue;
		size_t length;
		if(_dupenv_s(&tempValue, &length, "TEMP")) {
			tempValue = strdup("");
		}
		sprintf_s(filename, MAX_LOG_FILE_NAME_LEN, "%s\\esteid.log", tempValue);
		free(tempValue);
	}
	return filename;
#else
	return "/tmp/esteid.log";
#endif
}

#define TIMESTAMP_BUFFER_LEN 30
FILE *EstEID_openLog(const char *func, const char *file, int line) {
	char timestamp[TIMESTAMP_BUFFER_LEN];
#ifdef _WIN32
	char delimiter = '\\';
	SYSTEMTIME now;
	FILE *log;
	GetLocalTime(&now);
	sprintf_s(timestamp, TIMESTAMP_BUFFER_LEN, "%d-%02d-%02d %02d:%02d:%02d.%03d", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
#else
	char delimiter = '/';
	struct timeval tv;
	time_t curtime;
	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec;
	strftime(timestamp, 30, "%Y-%m-%d %T", localtime(&curtime));
#ifdef __APPLE__
	sprintf(timestamp + strlen(timestamp), ".%03i ", tv.tv_usec / 1000);
#else
	sprintf(timestamp + strlen(timestamp), ".%03li ", tv.tv_usec / 1000);
#endif
#endif
#ifndef _WIN32
	FILE *log = fopen(EstEID_getLogFilename(), "a");
#else	
	log = _fsopen(EstEID_getLogFilename(), "a", _SH_DENYNO);
#endif
	fprintf(log, "%s ", timestamp);
	if (file) {
		char *f = strrchr((char *)file, delimiter);
		if (!f) f = (char *)file;
		else f++;
		fprintf(log, "%s() [%s:%i] ", func, f, line);
	}
	return log;
}

void EstEID_log_real(const char *func, const char *file, int line, const char *message, ...) {
	FILE *log;
	va_list args;

	if (access(EstEID_getLogFilename(), W_OK) == -1) {
		return;
	}
	log = EstEID_openLog(func, file, line);
	va_start(args, message);
	vfprintf(log, message, args);
	va_end(args);
	fprintf(log, "\n");
	fclose(log);
}

//#ifndef WIN_IE
void EstEID_logMap_real(const char *func, const char *file, int line, EstEID_Map map) {
	FILE *log;
	
	if (access(EstEID_getLogFilename(), W_OK) == -1) return;
	log = EstEID_openLog(func, file, line);
	fprintf(log, "entries:\n");
	EstEID_mapPrint(log, map);
	fclose(log);
}
//#endif