#include "osa.h"
#include <stdlib.h>

void * osa_malloc(u32_t sz)
{
	return malloc(sz);
}

void * osa_calloc(u32_t sz)
{
	return calloc(1, sz);
}

void * osa_realloc(void * buf, u32_t sz)
{
	return realloc(buf, sz);
}

void osa_free(void * buf)
{
	return free(buf);
}