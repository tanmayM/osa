#include "osa.h"
#include "osa_threads.h"

char * osa_enum2str(osa_thread_priority_e prio)
{
	switch(prio)
	{
		case OSA_THREAD_PRIO_DEFAULT	: return "DEFAULT";
		case OSA_THREAD_PRIO_LOW		: return "LOW";
		case OSA_THREAD_PRIO_HIGH		: return "HIGH";
		case OSA_THREAD_PRIO_CRITICAL	: return "CRITICAL";
		default 						: return "UNKNOWN_PRIO";
	}		
}

char * osa_errStr(int err)
{
	switch(err)
	{
		case EAGAIN	: return "EAGAIN";
		case EINVAL	: return "EINVAL";
		case EPERM	: return "EPERM";
		default 	: return "UNKNOWN_ERR"; 
	}
}

static ret_e o_set_attributes(pthread_attr_t &attr, osa_thread_priority_e &prio, osa_thread_stack_t *stack)
{
	char * func = "o_set_attributes";
	if(0 != pthread_attr_init (&attr))
	{
		osa_loge("o_set_attributes: pthread_attr_init failed");
		return OSA_ERR_COREFUNCFAIL;
	}

	switch(prio)
	{
		case OSA_THREAD_PRIO_HIGH:
		{
			pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
			pthread_attr_setschedpolicy (&attr, SCHED_RR);
			int min = sched_get_priority_min(SCHED_RR);
			int max = sched_get_priority_max(SCHED_RR);

			struct sched_param schdPrio;
			schdPrio.sched_priority = min + ((max-min)/4);
			pthread_attr_setschedparam(&attr, &schdPrio);
			osa_loge("%s: Priority: (OS-min=%d, OS-max=%d), ThreadPrioritySelected=%d", min, max, schdPrio.sched_priority);
		}
		break;
		case OSA_THREAD_PRIO_CRITICAL:
		{
			pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
			pthread_attr_setschedpolicy (&attr, SCHED_RR);
			int min = sched_get_priority_min(SCHED_RR);
			int max = sched_get_priority_max(SCHED_RR);

			struct sched_param schdPrio;
			schdPrio.sched_priority = min + 3 * ((max-min)/4);
			pthread_attr_setschedparam(&attr, &schdPrio);
			osa_loge("%s: Priority: (OS-min=%d, OS-max=%d), ThreadPrioritySelected=%d", func, min, max, schdPrio.sched_priority);
		}
		break;
		case OSA_THREAD_PRIO_DEFAULT:
		case OSA_THREAD_PRIO_LOW:
			osa_loge("%s: Thread priority kept to default");
		break;
	}

	if(NULL != stack && NULL != stack->buf && 0 < stack->sz) 
	{
		if(0 != pthread_attr_setstack(&attr, stack->buf, stack->sz))
		{
			osa_loge("o_set_attributes: pthread_attr_setstack failed. user provided stack=%x, size=%d", stack->buf, stack->sz);
			return OSA_ERR_COREFUNCFAIL;
		}
	}
}

ret_e osa_thread_create(osa_threadHd_t &tHd, ThreadFunc tFunc, osa_thread_priority_e &prio, char thrName[15], void * arg,
			osa_thread_stack_t *stack)
{
	char * func = "osa_thread_create";

	if(NULL == func || osa_strlen(thrName)> 15)
	{
		osa_loge("%s: error: in params. func=%x, thrName=%s", func, thrName);
	}

	osa_logd("%s: entered. ThreadName=%d, tFunc=%x, prio=%s, arg=%d, stack=%x", func, 
			thrName, tFunc, osa_enum2str(prio), arg, stack);

	osa_ThreadHandle_t *hd = (osa_ThreadHandle_t *)&tHd;
	pthread_attr_t attr;

	o_set_attributes(attr, prio, stack);
	int result = pthread_create(&(hd->t), &attr, tFunc, arg);
	if(0 != result)
	{
		osa_loge("%s: pthread_create failed. err=%s", func, osa_errStr(result));
		return OSA_ERR_COREFUNCFAIL;
	}

	if(0 != pthread_setname_np(hd->t, thrName))
	{
		osa_loge("%s: pthread_setname_np failed, but we will continue", func);
	}

	osa_logi("%s: success. ThreadId=%x, ThreadName=%s", hd->t, thrName);
}

ret_e osa_thread_exit(void *retVal)
{
	pthread_t t;
	char thrName[15] = {0};
	
	t = pthread_self();
	pthread_getname_np(t, thrName, 15);
	
	osa_logi("osa_thread_exit: Thread=%x, Name=%s", t, thrName);

	pthread_exit(retVal);
}


osa_mutex :: osa_mutex(void)
{
	int result = pthread_mutex_init(&mutex, NULL);

	if(0 == result)
	{
		osa_logd("osa_mutex::init Mutex %x created", &mutex);
	}
	else
	{
		osa_logd("osa_mutex::init Mutex %x creation failed", &mutex);
	}
}

osa_mutex :: ~osa_mutex(void)
{
	pthread_mutex_destroy(&mutex);
	osa_logi("osa_mutex::destructor: Mutex %x destroyed", &mutex);
}

ret_e osa_mutex :: destroy()
{
	pthread_mutex_destroy(&mutex);
	osa_logi("osa_mutex::destroy: Mutex %x destroyed", &mutex);
}

ret_e osa_mutex :: lock(char * locker)
{
	int result = pthread_mutex_lock(&mutex);

	if(0 != result)
	{
		osa_loge("osa_mutex::lock failed. result=%d\n", result);
		return OSA_ERR_COREFUNCFAIL;
	}

	osa_logd("mutex %x locked by ", &mutex, locker?locker:"--");

	return OSA_SUCCESS;
}

ret_e osa_mutex :: unlock(char * unlocker)
{
	int result = pthread_mutex_unlock(&mutex);

	if(0 != result)
	{
		osa_loge("osa_mutex::unlock failed. result=%d\n", result);
		return OSA_ERR_COREFUNCFAIL;
	}

	osa_logd("mutex %x unlocked by ", &mutex, unlocker?unlocker:"--");

	return OSA_SUCCESS;	
}

