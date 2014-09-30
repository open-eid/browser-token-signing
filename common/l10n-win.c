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

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "esteid_log.h"
#include "l10n.h"

#define LOCALE_BUFFER_SIZE 16

int LoadValueFromRegistry(char* destination, HKEY key, WCHAR* subKey, WCHAR* value) {
	WCHAR locale[2 * LOCALE_BUFFER_SIZE] = {L""};
	DWORD localeBufferSize = 2 * LOCALE_BUFFER_SIZE; 
	int returnValue = 0;	
	HKEY hKey;
	
	LOG_LOCATION;

	destination[0] = '\0';

	RegOpenKeyEx(key, subKey, 0, KEY_QUERY_VALUE, &hKey);
	if(RegQueryValueEx( hKey, value, NULL, NULL, (LPBYTE) locale, &localeBufferSize ) == ERROR_SUCCESS) {
		returnValue = WideCharToMultiByte(CP_UTF8, 0, locale, -1, destination, LOCALE_BUFFER_SIZE, NULL, NULL);
		if(returnValue > 0){
			EstEID_log("value in registry is %s ", destination);			
		}
		else {
			EstEID_log("unable to load value from registry");
		}
	}
	RegCloseKey(hKey);
	return returnValue;
}

int LoadUserLocaleFromIDCardConfiguration(char* destination) {
	LOG_LOCATION;	
	return LoadValueFromRegistry(destination, HKEY_CURRENT_USER, L"Software\\Estonian ID Card\\OrganizationDefaults\\Main", L"Language");	
}

int LoadUserLocaleBySystemCharset(char* destination) {
	char codePageBuffer[LOCALE_BUFFER_SIZE] = {""};

	LOG_LOCATION;

	LoadValueFromRegistry(codePageBuffer, HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage", L"ACP");	
	EstEID_log("ACP = %s", codePageBuffer);
	if(!strcmp("1252", codePageBuffer)){
		strcpy(destination, "en");
	}
	else if(!strcmp("1251", codePageBuffer)){
		strcpy(destination, "ru");
	}
	else{
		strcpy(destination, "et");
	}
	return 1;
}

char *EstEID_getUserLocale() {
	char locale[LOCALE_BUFFER_SIZE];
	if(!LoadUserLocaleFromIDCardConfiguration(locale)) {
		LoadUserLocaleBySystemCharset(locale);
	}		
	return strdup(locale);
}