#include "osa.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <string.h>
#include <sys/un.h>
#include <assert.h>

#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include "osa_sock_internal.h"

char * osa_enum2str(osa_sockDomain_e domain)
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

char * osa_enum2str(osa_sockType_e type)
{
	switch(type)
	{
		case OSA_SOCK_DGRAM		:	return "DGRAM";
		case OSA_SOCK_STREAM	:	return "STREAM";
		case OSA_SOCK_SEQPACKET :	return "SEQPACKET";
		case OSA_SOCK_RAW		: 	return "RAW";
	}
}

char * osa_enum2str(osa_sockErr_e err)
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

static sa_family_t o_osa2UnixSockDomain(osa_sockDomain_e domain)
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

static osa_sockDomain_e o_unix2OsaSockDomain(sa_family_t unixDomain)
{
	switch(unixDomain)
	{
		case AF_UNIX 	:	return OSA_AF_UNIX;
		case AF_INET	:	return OSA_AF_INET;
		case AF_INET6	:	return OSA_AF_INET6;
		case AF_NETLINK :	return OSA_AF_NETLINK;
		case AF_PACKET	: 	return OSA_AF_PACKET;
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

static osa_sockErr_e o_unix2osaSockErr()
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
		case EAFNOSUPPORT		: 	return OSA_SOCKERR_WRONGDOMAIN;
		case EAGAIN 			: 	return OSA_SOCKERR_NOFREELOCALPORT;
		case EALREADY 			:   return OSA_SOCKERR_EALREADY;
		case EINPROGRESS 		: 	return OSA_SOCKERR_INPROGRESS;
		case ECONNREFUSED 		:   return OSA_SOCKERR_CONNREFUSED;
		case EINTR 				: 	return OSA_SOCKERR_INTERRUPTED;
		case EISCONN 			:	return OSA_SOCKERR_CONNECTED;
		case ENETUNREACH 		: 	return OSA_SOCKERR_NETUNREACH;
		case ETIMEDOUT 			:	return OSA_SOCKERR_TIMEDOUT;
		case EFAULT 			:   return OSA_SOCKERR_FAULTADDR;
		case EDESTADDRREQ 		: 	return OSA_SOCKERR_NODESTSET;
		default 				: 	return OSA_SOCKERR_UNKNOWN;
	}
}

static void o_unix2OsaStruct(struct sockaddr_in &src, osa_sockAddrIn_t &dst)
{
	dst.domain = o_unix2OsaSockDomain(src.sin_family);
	dst.port = src.sin_port;
	osa_strcpy(dst.addr, inet_ntoa(src.sin_addr), SOCKADDR_MAX_STR_SZ);
}

static void o_unix2OsaStruct(struct sockaddr_in6 &src6, osa_sockAddrIn_t &dst6)
{
	dst6.domain = o_unix2OsaSockDomain(src6.sin6_family);
	dst6.port = src6.sin6_port;
	if(NULL == inet_ntop(AF_INET6, (const void *)&src6.sin6_addr, dst6.addr, SOCKADDR_MAX_STR_SZ) )
	{
		osa_loge("o_unix2OsaStruct: inet_ntop failed: errno=%s (%d)", strerror(errno), errno);
		assert(0);
	}
	//osa_strcpy(dst6.addr, inet_ntop(AF_INET6, src6.sin6_addr), SOCKADDR_MAX_STR_SZ);
}

static int o_osa2unixStruct(osa_sockAddrIn_t &src, struct sockaddr_in &dst )
{
	dst.sin_family = o_osa2UnixSockDomain(src.domain);
	dst.sin_port = htons(src.port);
	return inet_pton(dst.sin_family, (const char *)src.addr, (void *)&dst.sin_addr);
}

static int o_osa2unixStruct(osa_sockAddrIn_t &src6, struct sockaddr_in6 &dst6 )
{
	dst6.sin6_family = o_osa2UnixSockDomain(src6.domain);;
	dst6.sin6_port   = htons(src6.port);
	return inet_pton(dst6.sin6_family, (const char *)src6.addr, (void *)&dst6.sin6_addr);
}

ret_e osa_socket::getSockAddr(osa_sockAddrIn_t &sAddrOsa)
{
	char * func = "osa_socket:getSockAddr";
	struct sockaddr tmp;
	socklen_t sockLen=0;
	int result;

	result = getsockname(sockFd, &tmp, &sockLen);
	if(0 != result)
	{
		osa_loge("%s:error: Unable to get socket address. sockFd=%d, errno=%s (%d). returning", func, sockFd, strerror(errno), errno);
		return OSA_ERR_COREFUNCFAIL;
	}

	switch(tmp.sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in sAddrInUnix;
			result = getsockname(sockFd, (struct sockaddr *)&sAddrInUnix, &sockLen);
			if(0 != result)
			{			
				osa_loge("%s:error: Unable to get socket address. sockFd=%d, domain=INET, errno=%s (%d)", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAIL;
			}
			o_unix2OsaStruct(sAddrInUnix, sAddrOsa); 
		}
		break;
		case AF_INET6:
		{
			struct sockaddr_in6 sAddrIn6Unix;
			result = getsockname(sockFd, (struct sockaddr *)&sAddrIn6Unix, &sockLen);
			if(0 != result)
			{			
				osa_loge("%s:error: Unable to get socket address. sockFd=%d, domain=INET6, errno=%s (%d). returning", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAIL;
			}
			o_unix2OsaStruct(sAddrIn6Unix, sAddrOsa); 
		}
		break;
	}

	return OSA_SUCCESS;
}

ret_e osa_socket::getsockPeerAddr(osa_sockAddrIn_t &peerAddrOsa)
{
	char * func = "osa_socket:getSockPeerAddr";
	struct sockaddr tmp;
	socklen_t sockLen=0;
	int result;

	result = getsockname(sockFd, &tmp, &sockLen);
	if(0 != result)
	{
		osa_loge("%s:error: Unable to get socket address. sockFd=%d, errno=%s (%d). Returning", func, sockFd, strerror(errno), errno);
		return OSA_ERR_COREFUNCFAIL;
	}

	switch(tmp.sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in sAddrInUnix;
			result = getpeername(sockFd, (struct sockaddr *)&sAddrInUnix, &sockLen);
			if(0 != result)
			{			
				osa_loge("%s:error: Unable to get peer socket address. sockFd=%d, domain=INET, errno=%s (%d). Returning", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAIL;
			}
			o_unix2OsaStruct(sAddrInUnix, peerAddrOsa); 
		}
		break;
		case AF_INET6:
		{
			struct sockaddr_in6 sAddrIn6Unix;
			result = getpeername(sockFd, (struct sockaddr *)&sAddrIn6Unix, &sockLen);
			if(0 != result)
			{			
				osa_loge("%s:error: Unable to get peer socket address. sockFd=%d, domain=INET6, errno=%s (%d). Returning", func, 
					sockFd, strerror(errno), errno);
				return OSA_ERR_COREFUNCFAIL;
			}
			
			o_unix2OsaStruct(sAddrIn6Unix, peerAddrOsa); 
		}
		break;
	}

	return OSA_SUCCESS;
}

void osa_socket :: setSockFd(int newSockFd)
{
	sockFd = newSockFd;
}



ret_e osa_socket::create(osa_sockDomain_e domain, osa_sockType_e type, i32_t proto, osa_sockErr_e &sockErr)
{
	char * func="osa_socket::create";
	int unixDomain, unixType;
	ret_e ret=OSA_SUCCESS;

	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
	osa_logd("%s: entered. domain=%s, type=%s, proto=%d", osa_enum2str(domain), osa_enum2str(type), proto);
	if (OSA_AF_UNIX != domain && OSA_AF_INET != domain && OSA_AF_INET6 != domain
		&& OSA_AF_NETLINK != domain && OSA_AF_PACKET != domain)
	{
		osa_loge("%s:error: domain paramater incorrect: %d. Returning", func, domain);
		return OSA_ERR_BADPARAM;
	}

	if(	OSA_SOCK_DGRAM != type && OSA_SOCK_STREAM != type && OSA_SOCK_SEQPACKET != type 
		&& OSA_SOCK_RAW != type )
	{
		osa_loge("%s:error: type paramater incorrect: %d. Returning", func, type);
		return OSA_ERR_BADPARAM;	
	}

	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = o_osa2UnixSockDomain(domain);
	unixType   = o_getUnixSockType(type);

	sockFd = socket(unixDomain, unixType, proto);

	if(-1 == sockFd)
	{
		osa_loge("%s:error: socket creation failed. Domain=%s, Type=%s, Proto=%d, errno=%s (%d)", func, 
			osa_enum2str(domain), osa_enum2str(type), proto, strerror(errno), errno);

		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;
	}
	else
	{
#if (LOGLEVEL >= LOGLVL_INFO)
		osa_logi("%s: socket created successfully. Domain=%s, Type=%s, Proto=%d", func, 
			osa_enum2str(domain), osa_enum2str(type), proto);
#endif

		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}

	osa_logd("%s: success. returning", func);
}


ret_e osa_socket::bind(osa_sockAddrIn_t &sockAddr, osa_sockErr_e &sockErr)
{
	char * func="osa_socket::bind";
	struct sockaddr_in sAddr;
	struct sockaddr_in6 sAddr6;
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
	osa_logd("%s: entered. sockAddr.domain=%s, sockAddr.addr=%s, sockAddr.port=%d", func, osa_enum2str(sockAddr.domain), 
		sockAddr.addr, sockAddr.port);

	if (OSA_AF_INET != sockAddr.domain && OSA_AF_INET6 != sockAddr.domain)
	{
		osa_loge("%s:error: Wrong domain provided: %d. Use this API for IPv4 and IPv6 only. returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/

	unixDomain = o_osa2UnixSockDomain(sockAddr.domain);

	switch(sockAddr.domain)
	{
		case OSA_AF_INET:
		{
			result = o_osa2unixStruct(sockAddr, sAddr);

			if(1 != result)
			{
				osa_loge("%s:error: Invalid IP address:%s. inet_pton returned %d. returning", func, sockAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			result = ::bind(sockFd, (const struct sockaddr *)&sAddr, (socklen_t)sizeof(sAddr));  /* IMP: the "::" in front of bind is to indicate to compiler to use global scope while resolving this function */

			if(0 != result)
			{
				osa_loge("%s:error: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, errno=%s (%d). Returning", func,
					osa_enum2str(sockAddr.domain), sockAddr.addr, sockAddr.port, strerror(errno), errno);

				sockErr = o_unix2osaSockErr();
				ret = OSA_ERR_COREFUNCFAIL;
			}
			else
			{
#if (LOGLEVEL >= LOGLVL_INFO)
				osa_logi("%s: socket bind success. Addr.domain=%s, Addr.ip=%s, Addr.port=%d", func,
					osa_enum2str(sockAddr.domain), sAddr.addr, sAddr,port);
#endif

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
		case OSA_AF_INET6:
		{
			result = o_osa2unixStruct(sockAddr, sAddr);
			if(1 != result)
			{
				osa_loge("%s:error: Invalid IP address:%s. inet_pton returned %d. returning", func, sockAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}


			result = ::bind(sockFd, (struct sockaddr *)&sAddr6, sizeof(sAddr6));

			if(0 != result)
			{
				osa_loge("%s:error: socket bind failed. Addr.domain=%s, Addr.ip=%s, Addr.port=%d, errno=%s (%d). returning", 
					func, osa_enum2str(sockAddr.domain), sockAddr.addr, sockAddr.port, strerror(errno), errno);

				sockErr = o_unix2osaSockErr();
				ret = OSA_ERR_COREFUNCFAIL;
			}
			else
			{
#if (LOGLEVEL >= LOGLVL_INFO)
				osa_logi("%s: socket bind success. Addr.domain=%s, Addr.ip=%s, Addr.port=%d", 
					func, osa_enum2str(sockAddr.domain), sAddr6.addr, sAddr6,port);
#endif

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	osa_logd("%s: success. returning", func);

	return ret;
}


ret_e osa_socket::bind(osa_sockAddrGeneric_t &sockAddr, osa_sockErr_e &sockErr)
{
	char * func="osa_socket::bind";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	struct sockaddr_un *sAddrUn;
	struct sockaddr_ll *sAddrPkt;
	
	/*--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
	--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--*/
	osa_logd("%s: entered. sockAddr.domain=%s, sockAddr.addrLen=%d", func, osa_enum2str(sockAddr.domain), sockAddr.addrLen);

	if (OSA_AF_UNIX != sockAddr.domain && OSA_AF_NETLINK != sockAddr.domain && OSA_AF_PACKET != sockAddr.domain)
	{
		osa_loge("%s:error: Wrong domain provided: %d. Use the API for UNIX|NETLINK|PACKET domain. Returning", func, sockAddr.domain);
		return OSA_ERR_BADPARAM;
	}

	if(NULL == sockAddr.addr || 0 == sockAddr.addrLen)
	{
		osa_loge("%s:error: Bad input parameters. addr=%x, addrLen=%d", func, sockAddr.addr, sockAddr.addrLen);
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
				osa_loge("%s:error: struct sockaddr_un expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrUn = (struct sockaddr_un *)sockAddr.addr;

			result = ::bind(sockFd, (struct sockaddr *)sAddrUn, sizeof(struct sockaddr_un));

			if(0 != result)
			{
				osa_loge("%s:error: socket bind failed. Addr.domain=%s, socket-path=%s, errno=%s (%d)", func,
					osa_enum2str(sockAddr.domain), sAddrUn->sun_path, strerror(errno), errno);

				sockErr = o_unix2osaSockErr();
				ret = OSA_ERR_COREFUNCFAIL;
			}
			else
			{
#if (LOGLEVEL >= LOGLVL_INFO)
				osa_logi("%s: socket bind success. Addr.domain=%s, socket-path=%s", func,
					osa_enum2str(sockAddr.domain), sAddrun.sun_path);
#endif

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
		case OSA_AF_PACKET:
		{
			if(sizeof(struct sockaddr_ll) != sockAddr.addrLen)
			{
				osa_loge("%s:error: struct sockaddr_ll expected as addr with len=%d, But addrLen=%d. returning", func, 
					sizeof(struct sockaddr_un), sockAddr.addrLen);
				return OSA_ERR_BADPARAM;
			}
			sAddrPkt = (struct sockaddr_ll *)sockAddr.addr;

			result = ::bind(sockFd, (struct sockaddr *)sAddrPkt, sizeof(struct sockaddr_ll));

			if(0 != result)
			{
				unsigned char *t = sAddrPkt->sll_addr;
				osa_loge("%s:error: socket bind failed. Addr.domain=%s, proto=%d (in networkByteOrder:%d), ifIndex=%d,\
					hatype=%d, pktType=%d, halen=%d, addr=%x:%x:%x:%x:%x:%x:%x:%x, errno=%s (%d)", 
					osa_enum2str(sockAddr.domain), ntohs(sAddrPkt->sll_protocol), sAddrPkt->sll_protocol, 
					sAddrPkt->sll_ifindex, sAddrPkt->sll_hatype, sAddrPkt->sll_pkttype, sAddrPkt->sll_halen,
					t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
					strerror(errno), errno);

				sockErr = o_unix2osaSockErr();
				ret = OSA_ERR_COREFUNCFAIL;
			}
			else
			{
#if (LOGLEVEL >= LOGLVL_INFO)
				osa_logi("%s: socket bind success. Addr.domain=%s, proto=%d (in networkByteOrder:%d), ifIndex=%d,\
					hatype=%d, pktType=%d, halen=%d, addr=%x:%x:%x:%x:%x:%x:%x:%x", 
					osa_enum2str(sockAddr.domain), ntohs(sAddrPkt->sll_protocol), sAddrPkt->sll_protocol, 
					sAddrPkt->sll_ifindex, sAddrPkt->sll_hatype, sAddrPkt->sll_pkttype, sAddrPkt->sll_halen,
					t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]);
#endif

				sockErr = OSA_SOCK_SUCCESS;
				ret = OSA_SUCCESS;
			}
		}
		break;
	}

	osa_logd("%s: success. returning", func);

	return ret;
}


ret_e osa_socket :: listen(i32_t maxCon, osa_sockErr_e &sockErr)
{
	char * func = "osa_socket::listen";
	int result;
	ret_e ret=OSA_SUCCESS;

	osa_logd("%s: entered. maxCon=%d", func, maxCon);
	result = ::listen(sockFd, maxCon);

	if(0 != result)
	{
		osa_loge("%s:error: socket listen failed. maxCon=%d, errno=%s (%d)", func, maxCon, strerror(errno), errno);

		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;
	}
	else
	{
#if (LOGLEVEL >= LOGLVL_INFO)
		osa_logi("%s: socket listen success. maxCon=%d", func, maxCon);
#endif

		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}
	
	osa_logd("%s: success. returning", func);
	return ret;
}



ret_e osa_socket :: accept(osa_socket &newStreamSock, osa_sockErr_e &sockErr)
{
	char * func = "osa_socket::accept";
	int newSockFd, result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	socklen_t sockLen=0;
	struct sockaddr sAddr;

	osa_logd("%s: entered.", func);

	newSockFd = ::accept(sockFd, &sAddr, &sockLen);

	if(-1 == newSockFd)
	{
		osa_loge("%s:error: socket accept failed. errno=%s (%d)", func, strerror(errno), errno);

		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;	
	}
	else
	{
		newStreamSock.setSockFd(newSockFd);

#if (LOGLEVEL >= LOGLVL_INFO)
		osa_sockAddrGeneric_t sAddrOsa;
		struct sockaddr tmp;

		result = getsockname(sockFd, &tmp, &sockLen);

		switch(tmp.sun_family)
		{
			case AF_UNIX:
			{
				struct sockaddr_un unixAddr;
				int cr_len;
				struct ucred cr;
				char pidStr[40];

				sAddrOsa.addr = (void *)&unixAddr;
				sAddrOsa.addrLen = sizeof(sockaddr_un);

				result = newStreamSock.getSockAddr(sAddrOsa);
				osa_assert(OSA_SUCCESS == result);

				if ( getsockopt (newSockFd, SOL_SOCKET, SO_PEERCRED, &cr, &cr_len) == 0 && cr_len == sizeof (cr))
					sprintf(pidStr, ", pid of peer=%d", cr.pid);
				else
					sprintf(pidStr, ", pid of peer=COULDNT_RETRIEVE");

				osa_logi("%s: socket accept success. Domain=%d, socket-path=%s%s", func, unixAddr.sun_family, 
					unixAddr.sun_path, pidStr);
			}
			break;
			case AF_INET:
			case AF_INET6:
			{
				osa_sockAddrIn_t sAddrOsa, rAddrOsa;

				result = newStreamSock.getSockAddr(sAddrOsa);
				osa_assert(OSA_SUCCESS == result);

				result = newStreamSock.getSockPeerAddr(rAddrOsa);
				osa_assert(OSA_SUCCESS == result);

				osa_logi("%s: socket accept success. Domain=%s, lAddr=%s, lPort=%d, rAddr=%s, rPort=%d", func, 
					osa_enum2str(sAddrOsa.domain), sAddrOsa.addr, sAddrOsa.port, rAddrOsa.addr, rAddrOsa.port);			
			}
			break;
		}
#endif

		sockErr = OSA_SOCK_SUCCESS;
		ret = OSA_SUCCESS;
	}

	osa_logd("%s: success. returning", func);
}

ret_e osa_socket :: connect(osa_sockAddrIn_t &rAddr, osa_sockErr_e &sockErr)
{
	char * func = "osa_socket::connect";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	struct sockaddr sAddr;

	osa_logd("%s: entered. rAddr.domain=%s, rAddr.addr=%s, rAddr.port=%d", func, osa_enum2str(rAddr.domain), 
		rAddr.addr, rAddr.port);

	switch(rAddr.domain)
	{
		case OSA_AF_INET:
		{
			struct sockaddr_in rAddrIn;

			result = o_osa2unixStruct(rAddr, rAddrIn);
			if(1 != result)
			{
				osa_loge("%s:error: Invalid remote IP address:%s. inet_pton returned %d. returning", func, rAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			result = ::connect(sockFd, (struct sockaddr *)&rAddrIn, sizeof(struct sockaddr_in));
			
		}
		break;
		case OSA_AF_INET6:
		{
			struct sockaddr_in6 rAddrIn6;

			result = o_osa2unixStruct(rAddr, rAddrIn6);
			if(1 != result)
			{
				osa_loge("%s:error: Invalid remote IP address:%s. inet_pton returned %d. returning", func, rAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			result = ::connect(sockFd, (struct sockaddr *)&rAddrIn6, sizeof(struct sockaddr_in6));
		}
		break;
	}

	if(-1 == result)
	{
		osa_loge("%s:error: socket connect failed. errno=%s (%d). returning", func, strerror(errno), errno);

		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;	
		return ret;
	}

	sockErr = OSA_SOCK_SUCCESS;
	ret = OSA_SUCCESS;
	osa_logd("%s: socket connect successful. returning", func);	

	return ret;
}


ret_e osa_socket :: connect(osa_sockAddrGeneric_t &rAddr, osa_sockErr_e &sockErr)
{
	char * func = "osa_socket::connect";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;
	struct sockaddr sAddr;

	osa_logd("%s: entered. rAddr.domain=%s, rAddr.addrLen=%d", func, osa_enum2str(rAddr.domain), rAddr.addrLen);

	switch(rAddr.domain)
	{
		case OSA_AF_UNIX:
		{
			struct sockaddr_un * rAddrUn = (struct sockaddr_un *)rAddr.addr;

			if(0 == rAddr.addrLen || NULL == rAddr.addr)
			{
				osa_loge("%s:error: unix domain expects sockaddr_un(len=%d). Instead passed addr is %d, len=%d. returning", func, 
					sizeof(struct sockaddr_un), rAddr.addr, result);
				return OSA_ERR_BADPARAM;
			}

			result = ::connect(sockFd, (struct sockaddr *)&rAddrUn, sizeof(struct sockaddr_un));
		}
		break;
	}

	if(-1 == result)
	{
		osa_loge("%s:error: socket connect failed. errno=%s (%d). returning", func, strerror(errno), errno);

		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;	
		return ret;
	}

	sockErr = OSA_SOCK_SUCCESS;
	ret = OSA_SUCCESS;
	osa_logd("%s: socket connect successful. returning", func);	
}

ret_e osa_socket :: send(void * buf, i32_t len, i32_t flags, osa_sockErr_e &sockErr)
{
	char * func = "osa_socket::send";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;

	if(NULL == buf || 0 == len)
	{
		osa_loge("%s:error: param error: buf=%x, len=%d. returning", func, buf, len);
		return OSA_ERR_BADPARAM;
	}

	osa_logd("%s: entered. buf=%x, len=%d, flags=%x", func, buf, len, flags);

	if(1 == isAsync)
	{
		pktData_t *pktData = (pktData_t *)malloc(sizeof(pktData_t)); /* This needs to be freed in receiver function OR socket destructor */
		pktData->buf = buf;
		pktData->len = len;
		pktData->isSendTo = false;

		q_data_t qObj;
		qObj.obj = pktData;
		qObj.size = sizeof(pktData);

		sockQ.push(qObj);
		osa_logi("%s: data pushed for async send. returning", func);
		sockErr =  OSA_SOCKERR_INPROGRESS;
		return OSA_SUCCESS;
	}

	/* TO DO: Flags is unused right now */
	result = ::send(sockFd, buf, len, 0);
	if(-1 ==  result)
	{
		osa_loge("%s:error: socket blocking-send failed. errorno=%s (%d). returning", func, strerror(errno), errno);
		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;	
		return ret;
	}

	osa_logd("%s: success. %d bytes sent. returning", func, result);

	return ret;
}


ret_e osa_socket :: sendto(void *buf, i32_t len, i32_t flags, osa_sockAddrIn_t &rAddr, osa_sockErr_e &sockErr)
{
	char *func = "osa_socket::sendto";
	int result, unixDomain;
	ret_e ret=OSA_SUCCESS;

	if(NULL == buf || 0 == len)
	{
		osa_loge("%s:error: param error: buf=%x, len=%d. returning", func, buf, len);
		return OSA_ERR_BADPARAM;
	}

	osa_logd("%s: entered. buf=%x, len=%d, flags=%x, rAddr.domain=%s, rAddr.addr=%s, rAddr.port=%d", func, 
		buf, len, flags, osa_enum2str(rAddr.domain), rAddr.addr, rAddr.port);

	if(1 == isAsync)
	{
		pktData_t *pktData = (pktData_t *)malloc(sizeof(pktData_t)); /* This needs to be freed in receiver function OR socket destructor */
		pktData->buf = buf;
		pktData->len = len;
		pktData->isSendTo = true;
		pktData->ipAddr = rAddr;

		q_data_t qObj;
		qObj.obj = pktData;
		qObj.size = sizeof(pktData);

		sockQ.push(qObj);
		osa_logi("%s: data pushed for async send. returning", func);
		sockErr =  OSA_SOCKERR_INPROGRESS;
		return OSA_SUCCESS;
	}

	switch(rAddr.domain)
	{
		case OSA_AF_INET:
		struct sockaddr_in rAddrIn;
		result = o_osa2unixStruct(rAddr, rAddrIn);
		if(1 != result)
		{
			osa_loge("%s:error: Invalid remote IP address:%s. inet_pton returned %d. returning", func, rAddr.addr, result);
			return OSA_ERR_BADPARAM;
		}

		result = ::sendto(sockFd, buf, len, 0, (struct sockaddr *)&rAddrIn, sizeof(struct sockaddr_in));
		break;
		case OSA_AF_INET6:
		struct sockaddr_in rAddrIn6;
		result = o_osa2unixStruct(rAddr, rAddrIn6);
		if(1 != result)
		{
			osa_loge("%s:error: Invalid remote IP address:%s. inet_pton returned %d. returning", func, rAddr.addr, result);
			return OSA_ERR_BADPARAM;
		}

		result = ::sendto(sockFd, buf, len, 0, (struct sockaddr *)&rAddrIn6, sizeof(struct sockaddr_in6));
		break;
	}
	
	/* TO DO: Flags is unused right now */
	if(-1 ==  result)
	{
		osa_loge("%s:error: socket blocking-send failed. errorno=%s (%d). returning", func, strerror(errno), errno);
		sockErr = o_unix2osaSockErr();
		ret = OSA_ERR_COREFUNCFAIL;	
		return ret;
	}

	osa_logd("%s: success. %d bytes sent. returning", func, result);

	return ret;
}



int main()
{}
