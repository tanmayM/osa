#include "osa.h"
#include "errno.h"
#include "sys/socket.h`"

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
	 
static int osa2UnixSockDomain(osa_sockDomain_e domain)
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

static int unix2OsaSockDomain(int unixDomain)
{
	switch(unixDomain)
	{
		case AF_UNIX 	:	return OSA_AF_UNIX;
		case AF_INET	:	return OSA_AF_INET;
		case AF_INET6	:	return OSA_AF_INET6;
		case AF_NETLINK :	return OSA_AF_NETLINK;
		case AF_PACKET	: 	return OSA_AF_PACKET;
		default			:	return -1;
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

static osa_SockErr_e unixSockErr2osaSockErr()
{
	switch(errno)
	{
		case EACCES				:	return OSA_SOCKERR_ACCESS;
		case EPERM 				:	return OSA_SOCKERR_ACCESS;
		case EINVAL				:	return OSA_SOCKERR_INVAL;
		case EMFILE 			:	return OSA_SOCKERR_EMFILE;
		case ENFILE 			:	return OSA_SOCKERR_ENFILE;
		case ENOBUFS 			:	return OSA_SOCKERR_INSUFFMEM;
		case ENOMEM 			:	return OSA_SOCKERR_INSUFFMEM;
		case EPROTONOSUPPORT 	: 	return OSA_SOCKERR_EPROTONOSUPPORT;
		case EADDRINUSE 		:	return OSA_SOCKERR_ADDRINUSE;
		case EBADF 				:	return OSA_SOCKERR_BADHANDLE;
		case ENOTSOCK 			: 	return OSA_SOCKERR_BADHANDLE;
		case ENETDOWN 			:	return OSA_SOCKERR_IFACEDOWN;
		case EOPNOTSUPP 		:	return OSA_SOCKERR_OPNOTSUPP;
		default 				: 	return OSA_SOCKERR_UNKNOWN;
	}
}

ret_e osa_socket::create(osa_socskDomain_e domain, osa_sockType_e type, i32_t proto, osa_SockErr_e &sockErr)
{
	char * func="osa_socket::create";
	int unixDomain, unixType;
	ret_e ret=OSA_SUCCESS;

	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
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

	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = osa2UnixSockDomain(domain);
	unixType   = getUnixSockType(type);

	sockFd = socket(unixDomain, unixType, proto);

	if(-1 == sockFd)
	{
		osa_log_error("%s: socket creation failed. Domain=%s, Type=%s, Proto=%d, errno=%s (%d)", func, 
							osa_enum2Str(domain), osa_enum2Str(type), proto, strerror(errno), errno);

		sockErr = unixSockErr2osaSockErr();
		ret = OSA_ERR_COREFUNCFAILED;
	}
	else
	{
		osa_log_info("%s: socket created successfully. Domain=%s, Type=%s, Proto=%d", func, 
							osa_enum2Str(domain), osa_enum2Str(type), proto);

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
	
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
	osa_log_debug("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", osa_enum2Str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_INET != sockAddr.domain && OSA_AF_INET6 != sockAddr.domain)
	{
		osa_log_error("%s: Wrong domain provided: %d. Use this API for IPv4 and IPv6 only. returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = osa2UnixSockDomain(sockAddr.domain);

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
				osa_log_error("%s: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, errno=%s (%d)", func,
						osa_enum2str(sockAddr.domain), sAddr.addr, sAddr,port, strerror(errno), errno);

				sockErr = unixSockErr2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_log_info("%s: socket bind success. Addr.domain=%s, Addr.ip=%s, Addr.port=%d", func,
						osa_enum2str(sockAddr.domain), sAddr.addr, sAddr,port);
	
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
				osa_log_error("%s: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, errno=%s (%d)", 
									func, osa_enum2str(sockAddr.domain), sAddr6.addr, sAddr6,port, strerror(errno), errno);

				sockErr = unixSockErr2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_log_info("%s: socket bind success. Addr.domain=%s, Addr.ip=%s, Addr.port=%d", 
									func, osa_enum2str(sockAddr.domain), sAddr6.addr, sAddr6,port);

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	osa_log_debug("%s: success. returning", func);

	return ret;
}


ret_e osa_socket::bind(osa_sockAddrGeneric_t &sockAddr, osa_SockErr_e &sockErr)
{
	char * func="osa_socket::bind";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	struct sockaddr_un *sAddrUn;
	struct sockaddr_ll *sAddrPkt;
	
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
	osa_log_debug("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", func, osa_enum2Str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_UNIX != sockAddr.domain && OSA_AF_NETLINK != sockAddr.domain && OSA_AF_PACKET != sockAddr.domain)
	{
		osa_log_error("%s: Wrong domain provided: %d. Use the API for UNIX|NETLINK|PACKET domain. Returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == sockAddr.addr || 0 == sockAddr.addrLen)
	{
		osa_log_error("%s: Bad input parameters. addr=%x, addrLen=%d", func, sockAddr.addr, sockAddr.addrLen);
	}
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = osa2UnixSockDomain(sockAddr.domain);

	switch(sockAddr.domain)
	{
		case OSA_AF_UNIX:
		{
			if(sizeof(struct sockaddr_un) != sockAddr.addrLen)
			{
				osa_log_error("%s: struct sockaddr_un expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrUn = (struct sockaddr_un *)sockAddr.addr;

			result = bind(sockFd, (struct sockaddr *)sAddrUn, sizeof(struct sockaddr_un));

			if(0 != result)
			{
				osa_log_error("%s: socket bind failed. Addr.domain=%s, socket-path=%s, errno=%s (%d)", func,
						osa_enum2str(sockAddr.domain), saddrun.sun_path, strerror(errno), errno);

				sockErr = unixSockErr2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_log_info("%s: socket bind success. Addr.domain=%s, socket-path=%s", func,
						osa_enum2str(sockAddr.domain), saddrun.sun_path);

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
		case OSA_AF_PACKET:
		{
			if(sizeof(struct sockaddr_ll) != sockAddr.addrLen)
			{
				osa_log_error("%s: struct sockaddr_ll expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrPkt = (struct sockaddr_ll *)sockAddr.addr;

			result = bind(sockFd, (struct sockaddr *)sAddrPkt, sizeof(struct sockaddr_ll));
		
			if(0 != result)
			{
				char *t = sAddrPkt->sll_addr;
				osa_log_error("%s: socket bind failed. Addr.domain=%s, proto=%d (in networkByteOrder:%d), ifIndex=%d,
					hatype=%d, pktType=%d, halen=%d, addr=%x:%x:%x:%x:%x:%x:%x:%x, errno=%s (%d)", 
						osa_enum2str(sockAddr.domain), ntohs(sAddrPkt->sll_protocol), sAddrPkt->sll_protocol, 
						sAddrPkt->sll_ifindex, sAddrPkt->sll_hatype, sAddrPkt->sll_pkttype, sAddrPkt->sll_halen,
						t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
						strerror(errno), errno);

				sockErr = unixSockErr2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_log_info("%s: socket bind success. Addr.domain=%s, proto=%d (in networkByteOrder:%d), ifIndex=%d,
					hatype=%d, pktType=%d, halen=%d, addr=%x:%x:%x:%x:%x:%x:%x:%x", 
						osa_enum2str(sockAddr.domain), ntohs(sAddrPkt->sll_protocol), sAddrPkt->sll_protocol, 
						sAddrPkt->sll_ifindex, sAddrPkt->sll_hatype, sAddrPkt->sll_pkttype, sAddrPkt->sll_halen,
						t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]);

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	osa_log_debug("%s: success. returning", func);

	return ret;
}


ret_e osa_socket :: listen(i32_t maxCon, osa_SockErr_e &sockErr)
{
	char * func = "osa_socket::listen"
	int result;
	ret_e ret=OSA_SUCCESS;

	osa_log_debug("%s: entered. maxCon=%d", func, maxCon);
	result = listen(sockFd, maxCon);

	if(0 != result)
	{
		osa_log_error("%s: socket listen failed. maxCon=%d, errno=%s (%d)", func, maxCon, strerror(errno), errno);
	
		sockErr = unixSockErr2osaSockErr();
		ret = OSA_ERR_COREFUNCFAILED;
	}
	else
	{
		osa_log_info("%s: socket listen success. maxCon=%d", func, maxCon);
	
		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}
	
	osa_log_debug("%s: success. returning", func);
	return ret;
}

void osa_socket :: setSockFd(int newSockFd)
{
	sockFd = newSockFd;
}

ret_e osa_socket :: accept(osa_socket &newStreamSock, osa_SockErr_e &sockErr)
{
	char * func = "osa_socket::accept"
	int newSockFd, result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	struct sockaddr sAddr;
	osa_log_debug("%s: entered.", func);

	newSockFd = accept(sockFd, &sAddr, sizeof(struct sockaddr));

	if(-1 == newSockFd)
	{
		osa_log_error("%s: socket accept failed. errno=%s (%d)", func, strerror(errno), errno);
	
		sockErr = unixSockErr2osaSockErr();
		ret = OSA_ERR_COREFUNCFAILED;	
	}
	else
	{
		newStreamSock.setSockFd(newSockFd);

		osa_sockDomain_e domain;
		domain = unix2OsaSockDomain(sockAddr.sa_family);
		osa_log_info("%s: socket accept success. Domain=%s", func, osa_enum2str(domain));
		switch(domain)
		{
			case OSA_AF_INET:
				struct sockaddr_in sAddrIn, dAddrIn;
				getsockname(newSockFd, &sAddrIn, sizeof(struct sockaddr_in));
				getpeername(newSockFd, &dAddrIn, sizeof(struct sockaddr_in));
				/* TO DO : Add getIp, getPort functions */
				osa_log_info();
		}
		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}

	osa_log_debug("%s: success. returning", func);
}