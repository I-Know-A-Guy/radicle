#include "radicle/config.h"
#include "radicle/print.h"

int radicle_load_config(const char* file, json_t** data) {
	FILE* stream = fopen(file, "r");
	json_error_t error;
	*data = json_loadf(stream, 0, &error);
	fclose(stream);
	if(data == NULL) {
		ERROR("Config init failed. %s\n", error.text);
		return 1;
	}
	return 0;
}
