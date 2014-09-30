#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esteid_certinfo.h"
#include "esteid_sign.h"

extern char EstEID_error[1024];
extern int EstEID_errorCode;

char *promptForPIN(void *unused, const char *message, const char *name) {
	return strdup("01497");
}

void showAlert(void *nativeWindowHandle, const char *message) {

}

void sign(const char* hash, EstEID_Certs *certs) {
	for (unsigned int i = 0; i < certs->certCount; i++) {
		EstEID_CertInfo *certInfo = certs->certs + i;
		if (certInfo->usageNonRepudiation) {
			printf("signing!\n");
			EstEID_PINPromptData pinPromptData = { promptForPIN, showAlert, NULL };
			char *signature;
			if (EstEID_signHashHex(&signature, certInfo, hash, pinPromptData)) {
				printf("signature: %s\n", signature);
				free(signature);
			}
			else {
				printf("error: %s (error code: %i)\n", EstEID_error, EstEID_errorCode);
			}
		}
	}
}

int main(int argc, const char **argv) {

	EstEID_Certs *certs = EstEID_loadCerts(NULL);
	if (certs != NULL) {
		EstEID_printCerts(certs);
		while(1) {
			printf("tokensChanged: %i\n", EstEID_tokensChanged());
			getchar();
		}
		//EstEID_waitForSlotEvent();
		if (argc == 2) sign(argv[1], certs);
		EstEID_freeCerts(certs);
	}
	else {
		printf("error: %s (error code: %i)\n", EstEID_error, EstEID_errorCode);
	}
}
