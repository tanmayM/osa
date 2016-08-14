#include "osa.h"
#include "errno.h"

#define FUNC_CREATE	1
#define FUNC_BIND 	2

char * osa_enum2Str(osa_sockDomain_e domain)
{

	switch(domain)
	{
		case OSA_AF_UNIX	: 	return "UNIX";
		case OSA_AF_INET	:	return "INET";
		case OSA_AF_INET6	:	return "INET6";
		case OSA_AF_NETLINK :	return "NETLINK";
		case OSA_AF_PACKET	:	return "PACKET";
		default 			:	return "DOMAIN_UNKNOWN";
	}
}

char * osa_enum2Str(osa_sockType_e type)
{
	switch(type)
	{
		case OSA_SOCK_DGRAM		:	return "DGRAM";
		case OSA_SOCK_STREAM	:	return "STREAM";
		case OSA_SOCK_SEQPACKET :	return "SEQPACKET";
		case OSA_SOCK_RAW		: 	return "RAW";
	}

}

char * osa_enum2Str(osa_SockErr_e err)
{
	switch(err)
	{
		case OSA_SOCKERR_ACCESS				:	return "SOCKERR_ACCESS";
		case OSA_SOCKERR_UNKNOWNPROTO		: 	return "SOCKERR_UNKNOWNPROTO";
		case OSA_SOCKERR_EMFILE				:	return "SOCKERR_EMFILE";
		case OSA_SOCKERR_ENFILE				: 	return "SOCKERR_ENFILE";
		case OSA_SOCKERR_INSUFFMEM			:	return "SOCKERR_INSUFFMEM";
		case OSA_SOCKERR_EPROTONOSUPPORT	:	return "SOCKERR_EPROTONOSUPPORT";
		case OSA_SOCKERR_ADDRINUSE 			:	return "SOCKERR_ADDRINUSE";
		case OSA_SOCKERR_BADHANDLE 			:	return "SOCKERR_BADHANDLE";
		case OSA_SOCKERR_SOCKINUSE 			:   return "SOCKERR_SOCKINUSE";
		default 							:	return "SOCKERR_UNKNOWN";
	}
}
	 
static int getUnixSockDomain(osa_sockDomain_e domain)
{
	switch(domain)
	{
		case OSA_AF_UNIX 	:	return AF_UNIX;
		case OSA_AF_INET	:	return AF_INET;
		case OSA_AF_INET6	:	return AF_INET6;
		case OSA_AF_NETLINK :	return AF_NETLINK;
		case OSA_AF_PACKET	: 	return AF_PACKET;
		default				:	return -1;
	}
}

static int getUnixSockType(osa_sockType_e type)
{
	switch(type)
	{
		case OSA_SOCK_DGRAM		:	return SOCK_DGRAM;
		case OSA_SOCK_STREAM	:	return SOCK_STREAM;
		case OSA_SOCK_SEQPACKET	:	return SOCK_SEQPACKET;
		case OSA_SOCK_RAW		:	return SOCK_RAW;
		default					:	return -1;
	}
}

static osa_SockErr_e unixSockErr2osaSockErr(int funcId=0)
{
	switch(errno)
	{
		case EACCES				:	return OSA_SOCKERR_ACCESS;
		case EINVAL				:	
		{	
			if(FUNC_CREATE==funcId)
				return OSA_SOCKERR_UNKNOWNPROTO;
			else if(FUNC_BIND==funcId)
				return OSA_SOCKERR_SOCKINUSE;
		}
		break;
		case EMFILE 			:	return OSA_SOCKERR_EMFILE;
		case ENFILE 			:	return OSA_SOCKERR_ENFILE;
		case ENOBUFS 			:	return OSA_SOCKERR_INSUFFMEM;
		case ENOMEM 			:	return OSA_SOCKERR_INSUFFMEM;
		case EPROTONOSUPPORT 	: 	return OSA_SOCKERR_EPROTONOSUPPORT;
		case EADDRINUSE 		:	return OSA_SOCKERR_ADDRINUSE;
		case EBADF 				:	return OSA_SOCKERR_BADHANDLE;
		case ENOTSOCK 			: 	return OSA_SOCKERR_BADHANDLE;
		default 				: 	return OSA_SOCKERR_UNKNOWN;
	}

	return OSA_SOCKERR_UNKNOWN;
}

ret_e osa_socket::create(osa_socskDomain_e domain, osa_sockType_e type, i32_t proto, osa_SockErr_e &sockErr)
{
	char * func="osa_socket::create";
	int unixDomain, unixType;
	ret_e ret=OSA_SUCCESS;

	/*##############
	  ############## */
	osa_log_debug("%s: entered. domain=%s, type=%s, proto=%d", osa_enum2Str(domain), osa_enum2Str(type), proto);
	if (OSA_AF_UNIX != domain && OSA_AF_INET != domain && OSA_AF_INET6 != domain
		&& OSA_AF_NETLINK != domain && OSA_AF_PACKET != domain)
	{
		osa_log_error("%s: domain paramater incorrect: %d. Returning", func, domain);
		return OSA_ERR_BADPARAM;
	}

	if(	OSA_SOCK_DGRAM != type && OSA_SOCK_STREAM != type && OSA_SOCK_SEQPACKET != type 
		&& OSA_SOCK_RAW != type )
	{
		osa_log_error("%s: type paramater incorrect: %d. Returning", func, type);
		return OSA_ERR_BADPARAM;	
	}

	/*##############
	  ############## */

	unixDomain = getUnixSockDomain(domain);
	unixType   = getUnixSockType(type);

	sockFd = socket(unixDomain, unixType, proto);

	if(-1 == sockFd)
	{
		sockErr = unixSockErr2osaSockErr();
		osa_log_error("%s: socket creation failed. Domain=%s, Type=%s, Proto=%d, Error=%s (%d)", func, 
			osa_enum2Str(domain), osa_enum2Str(sockFd), proto, strerror(errno), errno);
		ret = OSA_ERR_COREFUNCFAILED;
	}
	else
	{
		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}

	osa_log_debug("%s: success. returning", func);
}


ret_e osa_socket::bind(osa_sockAddrIn_t &sockAddr, osa_SockErr_e &sockErr)
{
	char * func="osa_socket::bind";
	struct sockaddr_in sAddr;
	struct sockaddr_in6 sAddr6;
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	
	/*##############
	  ############## */
	osa_log_debug("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", osa_enum2Str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_INET != sockAddr.domain && OSA_AF_INET6 != sockAddr.domain)
	{
		osa_log_error("%s: Wrong domain provided: %d. Use this API for IPv4 and IPv6 only. returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}
	/*##############
	  ############## */

	unixDomain = getUnixSockDomain(sockAddr.domain);

	switch(sockAddr.domain)
	{
		case OSA_AF_INET:
		{
			result = inet_pton(unixDomain, (const char *)sockAddr.addr, (void *)&sAddr.sin_addr);
			if(1 != result)
			{
				osa_log_error("%s: Invalid IP address:%s. inet_pton returned %d. returning", func, sockAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			sAddr.sin_family = unixDomain;
			sAddr.sin_port   = htons(sockAddr.port);

			result = bind(sockFd, &sAddr, sizeof(sAddr));

			if(0 != result)
			{
				sockErr = unixSockErr2osaSockErr(FUNC_BIND);
				osa_log_error("%s: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, Error=%s (%d). returning", 
						osa_enum2str(sAddr.domain), sAddr.addr, sAddr,port, strerror(errno), errno);
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
		case OSA_AF_INET6:
		{
			result = inet_pton(unixDomain, (const char *)sockAddr.addr, (void *)&sAddr6.sin6_addr);
			if(1 != result)
			{
				osa_log_error("%s: Invalid IP address:%s. inet_pton returned %d. returning", func, sockAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			sAddr6.sin6_family = unixDomain;
			sAddr6.sin6_port   = htons(sockAddr.port);

			result = bind(sockFd, &sAddr6, sizeof(sAddr6));

			if(0 != result)
			{
				sockErr = unixSockErr2osaSockErr(FUNC_BIND);
				osa_log_error("%s: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, Error=%s (%d). returning", 
						osa_enum2str(sAddr6.domain), sAddr6.addr, sAddr6,port, strerror(errno), errno);
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	return ret;
}


ret_e osa_socket::bind(osa_sockAddrGeneric_t &sockAddr, osa_SockErr_e &sockErr)
{
	char * func="osa_socket::bind";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	struct sockaddr_un *sAddrUn;
	
	/*##############
	  ############## */
	osa_log_debug("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", osa_enum2Str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_UNIX != sockAddr.domain && OSA_AF_NETLINK != sockAddr.domain && OSA_AF_PACKET != sockAddr.domain)
	{
		osa_log_error("%s: Wrong domain provided: %d. Use the API for UNIX|NETLINK|PACKET domain. Returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == sockAddr.addr || 0 == sockAddr.addrLen)
	{
		osa_log_error("%s: Bad input parameters. addr=%x, addrLen=%d", sockAddr.addr, sockAddr.addrLen);
	}
	/*##############
	  ############## */

	unixDomain = getUnixSockDomain(sockAddr.domain);

	switch(sockAddr.domain)
	{
		case OSA_AF_UNIX:
		{
			if(OSA_AF_UNIX == sockAddr.domain && sizeof(struct sockaddr_un) != sockAddr.addrLen)
			{
				osa_log_error("%s: struct sockaddr_un expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrUn = (struct sockaddr_un *)sockAddr.addr;
			int pathLen=0;
			osa_strlen(sAddrUn.sun_path, pathLen);
			if(0 == pathLen)
			{
				osa_log_error("%s: path string has no path. osa_strlen returned zero. returning");
				return OSA_ERR_BADPARAM;
			}

			bind(sockFd, (struct sockaddr *)sAddrUn, sizeof(struct sockaddr_un));

			if(0 != result)
			{
				sockErr = unixSockErr2osaSockErr(FUNC_BIND);
				osa_log_error("%s: socket bind failed. Addr.domain=%s, socket-path=%s, Error=%s (%d). returning", 
						osa_enum2str(sAddr.domain), saddrun.sun_path, strerror(errno), errno);
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	return ret;
}
