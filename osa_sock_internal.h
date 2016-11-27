#ifndef __O_S_ABS_SOCK_INTERNAL__
#define __O_S_ABS_SOCK_INTERNAL__

#ifdef __linux__
#include <cstdint>  /* for cpp;  In case you are using c, the parallel header is stdint.h */
#endif

#ifdef _WIN32

#endif

typedef struct pktData_t
{
	void * buf;
	i32_t len;
	i32_t flags;
	bool isSendTo;
	union
	{
		osa_sockAddrIn_t ipAddr;
		osa_sockAddrGeneric_t genAddr;
	};

}pktData_t;


#endif