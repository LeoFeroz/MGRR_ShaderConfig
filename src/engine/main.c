
#include "definitions.h"

int main(void) {
	char parameter[30];
	Operation option = -1;

	scanf("%s", parameter);

	if (strcmp(parameter, "--Unpack") == 0) {
		option = UNPACK;
	} else if (strcmp(parameter, "--Repack") == 0) {
		option = REPACK;
	} else {
		printf("Invalid operation\n");
		printf("Options: --Unpack --Repack\n");
		printf("Usage: MGRR_ShaderConfig.exe <parameter>\n");
		return 0;
	}

	switch (option) {
		case UNPACK:
			ReadBinnaryFiles();
			break;
		case REPACK:
			break;
	}
}