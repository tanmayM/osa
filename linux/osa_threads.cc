#include "osa.h"
#include "osa_threads.h"
#include <string.h>
#include <assert.h>

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


/********************************************************
*					M U T E X
*********************************************************/


osa_mutex :: osa_mutex(void)
{
	isAlive=0;
}


osa_mutex :: ~osa_mutex(void)
{
	if(1 == isAlive)
	{
		pthread_mutex_destroy(&mutex);
		osa_logi("osa_mutex::destructor: Mutex %x destroyed", &mutex);
	}
}

ret_e osa_mutex :: create(void)
{
	int result = pthread_mutex_init(&mutex, NULL);

	if(0 == result)
	{
		osa_logd("osa_mutex::create Mutex %x created", &mutex);
	}
	else
	{
		osa_logd("osa_mutex::create Mutex %x creation failed", &mutex);
		return OSA_ERR_COREFUNCFAIL;
	}

	isAlive=1;
	return OSA_SUCCESS;
}



ret_e osa_mutex :: destroy()
{
	if(1 == isAlive)
	{
		pthread_mutex_destroy(&mutex);
		osa_logd("osa_mutex::destroy: Mutex %x destroyed", &mutex);
		isAlive = 0;
	}
}

ret_e osa_mutex :: lock(char * locker)
{
	char * func = "osa_mutex::lock";

	if(1 == isAlive)
	{
		int result = pthread_mutex_lock(&mutex);

		if(0 != result)
		{
			osa_loge("%s:error: failed. result=%d\n", func, result);
			return OSA_ERR_COREFUNCFAIL;
		}

		osa_logv("%s: mutex %x locked by ", func, &mutex, locker?locker:"--");
	}
	else
	{
		osa_loge("%s: error: mutex %x is destroyed. locker=%s. Dying", func, &mutex, locker?locker:"--");
		osa_assert(0);
	}

	return OSA_SUCCESS;
}

ret_e osa_mutex :: unlock(char * unlocker)
{
	char * func = "osa_mutex::unlock";

	if(1 == isAlive)
	{
		int result = pthread_mutex_unlock(&mutex);

		if(0 != result)
		{
			osa_loge("%s: error: failed. result=%d\n", func, result);
			return OSA_ERR_COREFUNCFAIL;
		}
		osa_logv("%s: mutex %x unlocked by ", func, &mutex, unlocker?unlocker:"--");
	}
	else
	{
		osa_loge("%s: error: mutex %x is destroyed. unlocker=%s. Dying", func, &mutex, unlocker?unlocker:"--");
		osa_assert(0);
	}

	osa_logd("mutex %x unlocked by ", &mutex, unlocker?unlocker:"--");

	return OSA_SUCCESS;	
}


/********************************************************
*					S E M A P H O R E
*********************************************************/


osa_semaphore :: osa_semaphore()
{
	maxCount=0;
	isAlive = 0;
}


ret_e osa_semaphore :: create(uint32_t count)
{
	int result = sem_init(&sem, 0, count);

	if(0 == result)
	{
		osa_logd("osa_semaphore:: created. count=%d", count);
	}
	else
	{
		osa_logd("osa_semaphore:: creation failed. count=%d, errno=%s (%d)", count, strerror(errno), errno);
		return OSA_ERR_COREFUNCFAIL;
	}

	maxCount = count;
	isAlive = 1;

	return OSA_SUCCESS;
}

osa_semaphore :: ~osa_semaphore()
{
	char * func = "~osa_semaphore";
	if(1 == isAlive)
	{
		int currCount=0;
		int result = sem_getvalue(&sem, &currCount);

		if(0 != result)
		{
			osa_logd("%s: sem_getvalue failed - %s (%d). something wrong", func, strerror(errno), errno);
			return; /* TO DO: Do we just return? assert? call destroy anyways?? */
		}

		result = sem_destroy(&sem);
		if(0 != result)
		{
			osa_logd("%s: sem_destroy failed - %s (%d). something wrong", func, strerror(errno), errno);
		return; /* TO DO: Do we just return? assert? call destroy anyways?? */
		}

		osa_logd("%s: %x destroyed", func, &sem);
		isAlive = 0;
	}
}

ret_e osa_semaphore :: destroy()
{
	char * func = "osa_semaphore::destroy";
	if(1 == isAlive)
	{
		int currCount=0;
		int result = sem_getvalue(&sem, &currCount);

		if(0 != result)
		{
			osa_loge("%s: sem_getvalue failed - %s (%d). something wrong", func, strerror(errno), errno);
			return OSA_ERR_COREFUNCFAIL; /* TO DO: Do we just return? assert? call destroy anyways?? */
		}

		result = sem_destroy(&sem);
		if(0 != result)
		{
			osa_loge("%s: sem_destroy failed - %s (%d). something wrong", func, strerror(errno), errno);
			return OSA_ERR_COREFUNCFAIL; /* TO DO: Do we just return? assert? call destroy anyways?? */
		}

		isAlive = 0;
		osa_logd("%s: %x destroyed", func, &sem);
	}
}

ret_e osa_semaphore :: wait(char * waiter)
{
	if(isAlive)
	{
		int result = sem_wait(&sem);

		if(0 != result)
		{
			osa_loge("osa_sem::wait failed. result=%d, errno=%s (%d)\n", result, strerror(errno), errno);
			return OSA_ERR_COREFUNCFAIL;
		}

		osa_logv("semaphore %x locked by %s", &sem, waiter?waiter:"--");
	}
	else
	{
		osa_logd("semaphore %x is destroyed. waiter=%s", &sem, waiter?waiter:"--");
		osa_assert(1==0);
	}

	return OSA_SUCCESS;
}

ret_e osa_semaphore :: post(char * poster)
{
	char * func = "osa_semaphore::post";

	if(isAlive)
	{
		int result = sem_post(&sem);

		if(0 != result)
		{
			osa_loge("%s: post failed. result=%d, poster=%s, \n", func, result, poster?poster:"--");
			return OSA_ERR_COREFUNCFAIL;
		}

		osa_logv("%s: semaphore %x unlocked by %s", func, &sem, poster?poster:"--");
	}
	else
	{
		osa_loge("%s: semaphore %x is destroyed. poster=%s. Dying", func, &sem, poster?poster:"--");
		osa_assert(0);
	}

	return OSA_SUCCESS;	
}


/********************************************************
*		C O N D I T I O N A L   V A R I A B L E
*********************************************************/


osa_cond :: osa_cond()
{
	isAlive = 0;
}

ret_e osa_cond :: create()
{
	int result = pthread_cond_init(&cond, NULL	);

	if(0 == result)
	{
		osa_logd("osa_cond:: created");
	}
	else
	{
		osa_logd("osa_cond:: creation failed. Error=%s", osa_errStr(result));
		return OSA_ERR_COREFUNCFAIL;
	}

	isAlive = 1;

	return OSA_SUCCESS;
}

osa_cond :: ~osa_cond()
{
	if(1 == isAlive)
	{
		int result=0;
		result = pthread_cond_destroy(&cond);
		if(0 != result)
		{
			osa_logd("osa::cond destructor. pthread_cond_destroy failed - %s. something wrong", osa_errStr(result));
			return; /* TO DO: Do we just return? assert? call destroy anyways?? */
		}

		isAlive = 0;
	}
}

ret_e osa_cond :: destroy()
{
	if(1 == isAlive)
	{
		int result = pthread_cond_destroy(&cond);
		if(0 != result)
		{
			osa_logd("osa::cond destructor. pthread_cond_destroy failed - %s. something wrong", osa_errStr(result));
			return OSA_ERR_COREFUNCFAIL; /* TO DO: Do we just return? assert? call destroy anyways?? */
		}

		isAlive = 0;
	}

	return OSA_SUCCESS;
}

ret_e osa_cond :: wait(osa_mutex &m, char * waiter)
{
	if(isAlive)
	{
		pthread_mutex_t * mutex = (pthread_mutex_t *)m.getNativeMutex();
		int result = pthread_cond_wait(&cond, mutex);

		if(0 != result)
		{
			osa_loge("osa_cond::wait failed. result=\n", osa_errStr(result));
			return OSA_ERR_COREFUNCFAIL;
		}

		osa_logv("cond %x locked by %s", &cond, waiter?waiter:"--");
	}
	else
	{
		osa_logd("cond %x is already destroyed. waiter=%s", &cond, waiter?waiter:"--");
		osa_assert(1==0);
	}

	return OSA_SUCCESS;
}

ret_e osa_cond :: signal(char * poster)
{
	char * func = "osa_cond::signal";

	if(isAlive)
	{
		int result = pthread_cond_signal(&cond);

		if(0 != result)
		{
			osa_loge("%s: failed. result=%s, poster=%s\n", func, osa_errStr(result), poster?poster:"--");
			return OSA_ERR_COREFUNCFAIL;
		}

		osa_logv("%s: cond %x signalled by %s", func, &cond, poster?poster:"--");
	}
	else
	{
		osa_loge("%s: cond %x is destroyed. poster=%s. Dying", func, &cond, poster?poster:"--");
		osa_assert(0);
	}

	return OSA_SUCCESS;	
}

ret_e osa_cond :: broadcast(char * poster)
{
	char * func = "osa_cond::broadcast";

	if(isAlive)
	{
		int result = pthread_cond_broadcast(&cond);

		if(0 != result)
		{
			osa_loge("%s: failed. result=%s, poster=%s\n", func, osa_errStr(result), poster?poster:"--");
			return OSA_ERR_COREFUNCFAIL;
		}

		osa_logv("%s: cond %x broadcasted by %s", func, &cond, poster?poster:"--");
	}
	else
	{
		osa_loge("%s: cond %x is destroyed. broadcaster=%s. Dying", func, &cond, poster?poster:"--");
		osa_assert(0);
	}

	return OSA_SUCCESS;	
}

