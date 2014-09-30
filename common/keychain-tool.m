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
#include <Security/Security.h>

void secOp(OSStatus result) {
	if (!result) return;
	CFStringRef error = SecCopyErrorMessageString(result, NULL);
	printf("error: %s (%li)\n", CFStringGetCStringPtr(error, CFStringGetFastestEncoding(error)), (long)result);
	CFRelease(error);
	exit(1);
}

int main(int argc, char **argv) {
	
	SecKeychainRef keychain;
	secOp(SecKeychainOpen("login.keychain", &keychain));
	printf("keychain: %p\n", keychain);
	
	SecKeychainSearchRef searchRef;
	SecItemClass itemClass = kSecGenericPasswordItemClass;
	secOp(SecKeychainSearchCreateFromAttributes(keychain, itemClass, NULL, &searchRef));
	for (;;) {
		SecKeychainItemRef item;
		OSStatus result = SecKeychainSearchCopyNext(searchRef, &item);
		if (result == errSecItemNotFound) {
			break;
		}
		secOp(result);
		
		SecKeychainRef keychain;
		secOp(SecKeychainItemCopyKeychain(item, &keychain));

		SecKeychainAttributeInfo *info;
		secOp(SecKeychainAttributeInfoForItemID(keychain, CSSM_DL_DB_RECORD_GENERIC_PASSWORD, &info));
		
		SecKeychainAttributeList *attrList;
		secOp(SecKeychainItemCopyAttributesAndData(item, info, &itemClass, &attrList, NULL, NULL));
		
		for (int i = 0; i < info->count; i++) {
			uint32 n = OSSwapHostToBigInt32(info->tag[i]);
			char *p = (char *)&n;
			if (strncmp("type", p,	4)) continue;
			SecKeychainAttribute *attribute = &attrList->attr[i];
			if (!attribute->length || !attribute->data) continue;
			int len = attribute->length / sizeof(UInt32);
			if (len != 1 && strncmp("iprf", (char *)attribute->data, 4)) continue;
			printf("item %p type: iprf, deleting!\n", item);
			SecKeychainItemDelete(item);
		}
	}
	printf("done\n");
	CFRelease(searchRef);
	CFRelease(keychain);
	return 0;
}