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

static int o_osa2UnixSockDomain(osa_sockDomain_e domain)
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

static int o_unix2OsaSockDomain(int unixDomain)
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

static int o_getUnixSockType(osa_sockType_e type)
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

static osa_SockErr_e o_unixS2osaSockErr()
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
		case ENOTCONN 			: 	return OSA_SOCKERR_NOTCONN;
		default 				: 	return OSA_SOCKERR_UNKNOWN;
	}
}

static void o_unixStruct2OsaStruct(struct sockaddr_in &src, osa_sockaddrIn_t &dst)
{
	dst.domain = src.sin_family;
	dst.port = src.sin_port;
	strcpy(dst.addr, inet_ntoa(src.sin_addr));
}

static void o_unixStruct2OsaStruct(struct sockaddr_in6 &src6, osa_sockaddrIn_t &dst6)
{
	dst6.domain = src6.sin_family;
	dst6.port = src6.sin_port;
	strcpy(dst6.addr, inet_ntoa(src6.sin_addr));
}

static int o_osaStruct2unixStruct(osa_sockaddrIn_t &src, struct sockaddr_in &dst )
{
	dst.sin_family = o_osa2UnixSockDomain(src.domain);
	dst.sin_port = htons(src.port);
	return inet_pton(dst.sin_family, (const char *)src.addr, (void *)&dst.sin_addr);
}

static int o_osaStruct2unixStruct(osa_sockaddrIn_t &src6, struct sockaddr_in6 &dst6 )
{
	dst6.sin6_family = o_osa2UnixSockDomain(src6.domain);;
	dst6.sin6_port   = htons(src6.port);
	return inet_pton(dst6.sin_family, (const char *)src6.addr, (void *)&dst6.sin6_addr);
}

ret_e osa_socket::getSockAddr(osa_sockAddrIn_t &sAddrOsa)
{
	char * func = "osa_socket:getSockAddr";
	struct sockaddr tmp;
	int result;

	result = getsockname(sockFd, &tmp, sizeof(struct sockaddr));
	if(0 != result)
	{
		osa_loge("%s: Unable to get socket address. sockFd=%d, errno=%s (%d)", func, sockFd, strerror(errno), errno);
		return OSA_ERR_COREFUNCFAILED;
	}

	switch(tmp.sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in sAddrInUnix;
			result = getsockname(sockFd, &sAddrInUnix, sizeof(struct sockaddr_in));
			if(0 != result)
			{			
				osa_loge("%s: Unable to get socket address. sockFd=%d, domain=INET, errno=%s (%d)", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAILED;
			}
			o_unixStruct2OsaStruct(sAddrInUnix, sAddrOsa); 
		}
		break;
		case AF_INET6:
		{
			struct sockaddr_in6 sAddrIn6Unix;
			result = getsockname(sockFd, &sAddrIn6Unix, sizeof(struct sockaddr_in6));
			if(0 != result)
			{			
				osa_loge("%s: Unable to get socket address. sockFd=%d, domain=INET6, errno=%s (%d)", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAILED;
			}
			o_unixStruct2OsaStruct(sAddrIn6Unix, sAddrOsa); 
		}
		break;
	}

	return OSA_SUCCESS;
}

ret_e osa_socket::getsockPeerAddr(osa_sockAddrIn_t &peerAddrOsa)
{
	char * func = "osa_socket:getSockPeerAddr";
	struct sockaddr tmp;
	int result;

	result = getsockname(sockFd, &tmp, sizeof(struct sockaddr));
	if(0 != result)
	{
		osa_loge("%s: Unable to get socket address. sockFd=%d, errno=%s (%d)", func, sockFd, strerror(errno), errno);
		return OSA_ERR_COREFUNCFAILED;
	}

	switch(tmp.sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in sAddrInUnix;
			result = getpeername(sockFd, &sAddrInUnix, sizeof(struct sockaddr_in));
			if(0 != result)
			{			
				osa_loge("%s: Unable to get peer socket address. sockFd=%d, domain=INET, errno=%s (%d)", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAILED;
			}
			o_unixStruct2OsaStruct(sAddrInUnix, peerAddrOsa); 
		}
		break;
		case AF_INET6:
		{
			struct sockaddr_in6 sAddrIn6Unix;
			result = getpeername(sockFd, &sAddrIn6Unix, sizeof(struct sockaddr_in6));
			if(0 != result)
			{			
				osa_loge("%s: Unable to get peer socket address. sockFd=%d, domain=INET6, errno=%s (%d)", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAILED;
			}
			o_unixStruct2OsaStruct(sAddrIn6Unix, peerAddrOsa); 
		}
		break;
	}

	return OSA_SUCCESS;
}



ret_e osa_socket::create(osa_socskDomain_e domain, osa_sockType_e type, i32_t proto, osa_SockErr_e &sockErr)
{
	char * func="osa_socket::create";
	int unixDomain, unixType;
	ret_e ret=OSA_SUCCESS;

	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
	osa_logd("%s: entered. domain=%s, type=%s, proto=%d", osa_enum2Str(domain), osa_enum2Str(type), proto);
	if (OSA_AF_UNIX != domain && OSA_AF_INET != domain && OSA_AF_INET6 != domain
		&& OSA_AF_NETLINK != domain && OSA_AF_PACKET != domain)
	{
		osa_loge("%s: domain paramater incorrect: %d. Returning", func, domain);
		return OSA_ERR_BADPARAM;
	}

	if(	OSA_SOCK_DGRAM != type && OSA_SOCK_STREAM != type && OSA_SOCK_SEQPACKET != type 
		&& OSA_SOCK_RAW != type )
	{
		osa_loge("%s: type paramater incorrect: %d. Returning", func, type);
		return OSA_ERR_BADPARAM;	
	}

	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = o_osa2UnixSockDomain(domain);
	unixType   = o_getUnixSockType(type);

	sockFd = socket(unixDomain, unixType, proto);

	if(-1 == sockFd)
	{
		osa_loge("%s: socket creation failed. Domain=%s, Type=%s, Proto=%d, errno=%s (%d)", func, 
			osa_enum2Str(domain), osa_enum2Str(type), proto, strerror(errno), errno);

		sockErr = o_unixS2osaSockErr();
		ret = OSA_ERR_COREFUNCFAILED;
	}
	else
	{
		osa_logi("%s: socket created successfully. Domain=%s, Type=%s, Proto=%d", func, 
			osa_enum2Str(domain), osa_enum2Str(type), proto);

		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}

	osa_logd("%s: success. returning", func);
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
	osa_logd("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", osa_enum2Str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_INET != sockAddr.domain && OSA_AF_INET6 != sockAddr.domain)
	{
		osa_loge("%s: Wrong domain provided: %d. Use this API for IPv4 and IPv6 only. returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = o_osa2UnixSockDomain(sockAddr.domain);

	switch(sockAddr.domain)
	{
		case OSA_AF_INET:
		{
			/* TO DO: Use convertion function */
			result = inet_pton(unixDomain, (const char *)sockAddr.addr, (void *)&sAddr.sin_addr);
			if(1 != result)
			{
				osa_loge("%s: Invalid IP address:%s. inet_pton returned %d. returning", func, sockAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			sAddr.sin_family = unixDomain;
			sAddr.sin_port   = htons(sockAddr.port);

			result = bind(sockFd, &sAddr, sizeof(sAddr));

			if(0 != result)
			{
				osa_loge("%s: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, errno=%s (%d)", func,
					osa_enum2str(sockAddr.domain), sAddr.addr, sAddr,port, strerror(errno), errno);

				sockErr = o_unixS2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_logi("%s: socket bind success. Addr.domain=%s, Addr.ip=%s, Addr.port=%d", func,
					osa_enum2str(sockAddr.domain), sAddr.addr, sAddr,port);

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
		case OSA_AF_INET6:
		{
			/* TO DO: Use convertion function */
			result = inet_pton(unixDomain, (const char *)sockAddr.addr, (void *)&sAddr6.sin6_addr);
			if(1 != result)
			{
				osa_loge("%s: Invalid IP address:%s. inet_pton returned %d. returning", func, sockAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			sAddr6.sin6_family = unixDomain;
			sAddr6.sin6_port   = htons(sockAddr.port);

			result = bind(sockFd, &sAddr6, sizeof(sAddr6));

			if(0 != result)
			{
				osa_loge("%s: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, errno=%s (%d)", 
					func, osa_enum2str(sockAddr.domain), sAddr6.addr, sAddr6,port, strerror(errno), errno);

				sockErr = o_unixS2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_logi("%s: socket bind success. Addr.domain=%s, Addr.ip=%s, Addr.port=%d", 
					func, osa_enum2str(sockAddr.domain), sAddr6.addr, sAddr6,port);

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	osa_logd("%s: success. returning", func);

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
	osa_logd("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", func, osa_enum2Str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_UNIX != sockAddr.domain && OSA_AF_NETLINK != sockAddr.domain && OSA_AF_PACKET != sockAddr.domain)
	{
		osa_loge("%s: Wrong domain provided: %d. Use the API for UNIX|NETLINK|PACKET domain. Returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == sockAddr.addr || 0 == sockAddr.addrLen)
	{
		osa_loge("%s: Bad input parameters. addr=%x, addrLen=%d", func, sockAddr.addr, sockAddr.addrLen);
	}
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = o_osa2UnixSockDomain(sockAddr.domain);

	switch(sockAddr.domain)
	{
		case OSA_AF_UNIX:
		{
			if(sizeof(struct sockaddr_un) != sockAddr.addrLen)
			{
				osa_loge("%s: struct sockaddr_un expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrUn = (struct sockaddr_un *)sockAddr.addr;

			result = bind(sockFd, (struct sockaddr *)sAddrUn, sizeof(struct sockaddr_un));

			if(0 != result)
			{
				osa_loge("%s: socket bind failed. Addr.domain=%s, socket-path=%s, errno=%s (%d)", func,
					osa_enum2str(sockAddr.domain), saddrun.sun_path, strerror(errno), errno);

				sockErr = o_unixS2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_logi("%s: socket bind success. Addr.domain=%s, socket-path=%s", func,
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
				osa_loge("%s: struct sockaddr_ll expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrPkt = (struct sockaddr_ll *)sockAddr.addr;

			result = bind(sockFd, (struct sockaddr *)sAddrPkt, sizeof(struct sockaddr_ll));

			if(0 != result)
			{
				char *t = sAddrPkt->sll_addr;
				osa_loge("%s: socket bind failed. Addr.domain=%s, proto=%d (in networkByteOrder:%d), ifIndex=%d,
					hatype=%d, pktType=%d, halen=%d, addr=%x:%x:%x:%x:%x:%x:%x:%x, errno=%s (%d)", 
					osa_enum2str(sockAddr.domain), ntohs(sAddrPkt->sll_protocol), sAddrPkt->sll_protocol, 
					sAddrPkt->sll_ifindex, sAddrPkt->sll_hatype, sAddrPkt->sll_pkttype, sAddrPkt->sll_halen,
					t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
					strerror(errno), errno);

				sockErr = o_unixS2osaSockErr();
				ret = OSA_ERR_COREFUNCFAILED;
			}
			else
			{
				osa_logi("%s: socket bind success. Addr.domain=%s, proto=%d (in networkByteOrder:%d), ifIndex=%d,
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

	osa_logd("%s: success. returning", func);

	return ret;
}


ret_e osa_socket :: listen(i32_t maxCon, osa_SockErr_e &sockErr)
{
	char * func = "osa_socket::listen"
	int result;
	ret_e ret=OSA_SUCCESS;

	osa_logd("%s: entered. maxCon=%d", func, maxCon);
	result = listen(sockFd, maxCon);

	if(0 != result)
	{
		osa_loge("%s: socket listen failed. maxCon=%d, errno=%s (%d)", func, maxCon, strerror(errno), errno);

		sockErr = o_unixS2osaSockErr();
		ret = OSA_ERR_COREFUNCFAILED;
	}
	else
	{
		osa_logi("%s: socket listen success. maxCon=%d", func, maxCon);

		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}
	
	osa_logd("%s: success. returning", func);
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

	osa_logd("%s: entered.", func);

	newSockFd = accept(sockFd, &sAddr, sizeof(struct sockaddr));

	if(-1 == newSockFd)
	{
		osa_loge("%s: socket accept failed. errno=%s (%d)", func, strerror(errno), errno);

		sockErr = o_unixS2osaSockErr();
		ret = OSA_ERR_COREFUNCFAILED;	
	}
	else
	{
		newStreamSock.setSockFd(newSockFd);
		
		osa_sockAddrIn_t sAddrOsa, rAddrOsa;
		
		result = newStreamSock.getSockAddr(sAddrOsa);
		osa_assert(OSA_SUCCESS == result);

		result = newStreamSock.getSockPeerAddr(rAddrOsa);
		osa_assert(OSA_SUCCESS == result);
		
		osa_logi("%s: socket accept success. Domain=%s, lAddr=%s, lPort=%d, rAddr=%s, rPort=%d", func, 
			osa_enum2str(sAddrOsa.domain), sAddrOsa.addr, sAddrOsa.port, rAddrOsa.addr, rAddrOsa.port);
		
		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}

	osa_logd("%s: success. returning", func);
}