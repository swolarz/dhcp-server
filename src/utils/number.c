#include "number.h"

#include <stdlib.h>


int number_uint32_parse(char* value, u_int32_t* num, u_int32_t max) {
	char* ptr;
	u_int32_t old = *num;
	unsigned long long int parsed = strtoll(value, &ptr, 10);

	if (value[0] == '\0' || *ptr != '\0' || parsed > max) {
		*num = old;
		return -1;
	}

	*num = (u_int32_t) parsed;
	return 0;
}
