#incude "osa.h"
#include "errno.h"

char * osa_sockDomain_e_2_String(osa_sockDomain_e_2_String domain)
{

	switch(domain)
	{
		case OSA_AF_UNIX	: 	return "UNIX";
		case OSA_AF_INET4	:	return "INET4";
		case OSA_AF_INET6	:	return "INET6";
		case OSA_AF_NETLINK :	return "NETLINK";
		case OSA_AF_PACKET	:	return "PACKET";
		default 			:	return "DOMAIN_UNKNOWN";
	}
}

char * osa_sockType_e_2_String(osa_sockType_e type)
{
	switch(type)
	{
		case OSA_SOCK_DGRAM		:	return "DGRAM";
		case OSA_SOCK_STREAM	:	return "STREAM";
		case OSA_SOCK_SEQPACKET :	return "SEQPACKET";
		case OSA_SOCK_RAW		: 	return "RAW";
	}

}

char * osa_SockErr_e_2_String(osa_SockErr_e err)
{
	switch(err)
	{
		case OSA_SOCKERR_ACCESS				:	return "ACCESS";
		case OSA_SOCKERR_UNKNOWNPROTO		: 	return "UNKNOWNPROTO";
		case OSA_SOCKERR_EMFILE				:	return "EMFILE";
		case OSA_SOCKERR_ENFILE				: 	return "ENFILE";
		case OSA_SOCKERR_INSUFFMEM			:	return "INSUFFMEM";
		case OSA_SOCKERR_EPROTONOSUPPORT	:	return "EPROTONOSUPPORT";
		default 							:	return "UNKNOWN ERROR";
	}
}

static int getUnixSockDomain(osa_sockDomain_e domain)
{
	switch(domain)
	{
		case OSA_AF_UNIX 	:	return AF_UNIX;
		case OSA_AF_INET4	:	return AF_INET4;
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

static osa_SockErr_e unixSockErr2osaSockErr()
{
	switch(errno)
	{
		case EACCES				:	return OSA_SOCKERR_ACCESS;
		case EINVAL				:	return OSA_SOCKERR_UNKNOWNPROTO;
		case EMFILE 			:	return OSA_SOCKERR_EMFILE;
		case ENFILE 			:	return OSA_SOCKERR_ENFILE;
		case ENOBUFS 			:	return OSA_SOCKERR_INSUFFMEM;
		case ENOMEM 			:	return OSA_SOCKERR_INSUFFMEM;
		case EPROTONOSUPPORT 	: 	return OSA_SOCKERR_EPROTONOSUPPORT;
		default 				: 	return OSA_SOCKERR_UNKNOWNERR;
	}
}

ret_e osa_socket::create(osa_socskDomain_e domain, osa_sockType_e type, i32_t protocol, osa_SockErr_e &sockErr)
{
	char * func = "osa_socket::create";
	int ret, unixDomain, unixType;

	/*##############
	  ############## */
	if (OSA_AF_UNIX != domain && OSA_AF_INET4 != domain && OSA_AF_INET6 != domain
		&& OSA_AF_NETLINK != domain && OSA_AF_PACKET != domain)
	{
		osa_log_error("%s: domain paramater incorrect: %d", domain);
		return OSA_ERR_BADPARAM;
	}

	if(	OSA_SOCK_DGRAM != type && OSA_SOCK_STREAM != type && OSA_SOCK_SEQPACKET != type 
		&& OSA_SOCK_RAW != type )
	{
		osa_log_error("%s: type paramater incorrect: %d", type);
		return OSA_ERR_BADPARAM;	
	}

	/*##############
	  ############## */

	unixDomain = getUnixSockDomain(domain);
	unixType   = getUnixSockType(type);
	ret = socket(unixDomain, unixType, protocol);

	if(-1 == ret)
	{
		sockErr = unixSockErr2osaSockErr();
		osa_log_error("%s: socket creation failed. Domain=%s, Type=%s, Proto=%d, Error=%s (errno=%d)", func, 
			osa_sockDomain_e_2_String(domain), osa_sockType_e_2_String(type), protocol, osa_SockErr_e_2_String(sockErr), errno);
	}
	else
	{
		sockErr = OSA_SOCKERR_SUCCESS;
	}

}