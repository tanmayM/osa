#ifndef __O_S_ABS_THREADS__
#define __O_S_ABS_THREADS__

#include <pthread.h>


typedef struct osa_ThreadHandle_t
{
#ifdef __linux__
	pthread_t t;
#endif
}osa_ThreadHandle_t;

#endif