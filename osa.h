#ifndef __O_S_ABS__
#define __O_S_ABS__

#ifdef __linux__
#include <cstdint>  /* for cpp;  In case you are using c, the parallel header is stdint.h */
#endif

#ifdef _WIN32

#endif

/****************************************************
* 		D A T A    T Y P E   W R A P E R S 
*****************************************************/

typedef unsigned char 			u8_t;
typedef signed char   			s8_t;
typedef unsigned short int		u16_t;	
typedef signed short int		i16_t;
typedef uint32_t				u32_t;
typedef int32_t					i32_t;
typedef uint64_t				u64_t;
typedef int64_t					i64_t;


/****************************************
* 		R E T U R N     T Y P E S
*****************************************/

typedef enum
{
	OSA_SUCCESS,
	OSA_ERR_BADPARAM,
	OSA_ERR_INSUFFMEM,
	OSA_ERR_COREFUNCFAIL,	/* OSA functions will usually call some OS provided core function. This error value tells that that 
							   function returned an error */
}ret_e;


/*************************************************
* 		U T I L I T Y    F U N C T I O N S 
**************************************************/

/* Get enum names as character strings. Very useful in debugging huge logs */
char * osa_enum2Str(ret_e ret);
char * osa_enum2Str(osa_sockDomain_e domain);
char * osa_enum2Str(osa_sockType_e type);

/****************************************
* 		String Processing
*****************************************/

ret_e osa_strlen(char * str);		/* Get length of string (NULL character at the end is not counted) */

ret_e osa_strcpy(char *dst, char * src, i32_t dstSz);			/* copy string src to dst. Max (dstSz-1) bytes will be 
																   copied */
ret_e osa_strncpy(char * dst, char *src, i32_t n, i32_t dstSz); /* copy n bytes from src to dst. Max(dstSz-1) bytes will be
																   copied */
/* osa_strcmp: Compares strings s1 and s2. Returns an integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) 
   is found,  respectively, to be less than, to match, or be greater than s2.
   IMPORTANT: If either of the strings s1, s2 are NULL, the behavior is undefined.
 */
i32_t osa_strcmp(char *s1, char *s2);

/* osa_strncmp: Compares first n bytes of strings s1 and s2.
	Returns an integer less than, equal to, or greater than zero if s1 (or the first n bytes thereof) is found,  respectively,
       to be less than, to match, or be greater than s2.
    IMPORTANT: If either of the strings s1, s2 are NULL, the behavior is undefined.
 */
i32_t osa_strncmp(char *s1, char *s2, i32_t n);
i32_t osa_strcasecmp(char *s1, char *s2); /* Same as osa_strcmp but ignores case while comparing */
i32_t osa_strncasecmp(char *s1, char *s2, i32_t n); /* Same as osa_strncmp but ignores case while comparing */

i32_t osa_strcat(char *dst, char *src, i32_t dstSz);
i32_t osa_strncat(char *dst, char *src, i32_t n, i32_t dstSz);

char * osa_strchr(char * s1, int c);
char * osa_index(char *s1, int c);
char * osa_strstr(char * haystack, char * needle);



/****************************************
*			M E M O R Y
*****************************************/

void * osa_malloc(u32_t sz);									/* Allocates 'sz' bytes on heap and returns the pointer. 
																	Returns NULL on failure */
void * osa_calloc(u32_t sz);    								/* Allocates 'sz' bytes on heap. Sets all bytes to 0 (zero) and 
																	returns the pointer. Returns NULL on failure */
void * osa_realloc(void * buf, u32_t sz);						/* Allocates a new buffer of size 'sz' and copies the old buffer
																	content to new one and frees the old buffer. If new size is 
																	less than old, only new sz bytes of data will be copied. */
void osa_free(void * buf);										/* Free the buffer */


/****************************************
	*			S O C K E T S
*****************************************/

#ifdef __linux__
typedef int32_t  				osa_ioHd_t; 	/** TO DO: int32_t or int ?? **/
#endif

#define SOCKADDR_MAX_STR_SZ 108		/* IPV4 text representation takes 16 bytes, IPV6 45 at max, For unix domain sockets, linux's
										equivalent structure uses 108. Hences using the maximum value available */

typedef enum osa_sockErr_e
{
	OSA_SOCK_SUCCESS;
	OSA_SOCKERR_ACCESS,				/* Permission to create socket denied by OS */
	OSA_SOCKERR_UNKNOWNPROTO,		/* Unknown protocol */
	OSA_SOCKERR_EMFILE,				/* Process file table overflow. Process has open max allowed open files already */
	OSA_SOCKERR_ENFILE,				/* Globally, max allowed open files (including all running processes) is reached */
	OSA_SOCKERR_INSUFFMEM,			/* OS doesn't have enough memory to open socket */
	OSA_SOCKERR_EPROTONOSUPPORT,	/* Protocol not supported in this domain */
	OSA_SOCKERR_ADDRINUSE, 			/* Address is in use */
	OSA_SOCKERR_BADHANDLE, 			/* Socket handle is not valid */
	OSA_SOCKERR_SOCKINUSE, 			/* Socket descriptor already in use */
	OSA_SOCKERR_UNKNOWN,

}osa_sockErr_e;

/* Socket domain :: This determines/indicates the kind of socket - unix-socket/ipv4-socket/ipv6-socket etc */
typedef enum
{
	OSA_AF_INET,
	OSA_AF_INET6,
	OSA_AF_UNIX,
	OSA_AF_NETLINK,
	OSA_AF_PACKET,
}osa_sockDomain_e;

typedef enum 
{
	OSA_SOCK_DGRAM,
	OSA_SOCK_STREAM,
	OSA_SOCK_SEQPACKET,
	OSA_SOCK_RAW,
}osa_sockType_e;

/* osa_sockAddrIn_t: Structure to hold IP socket address.
	addr : This is plaintext ip address (as character string e.g. "100.1.2.3").
	port : This is TCP/UDP port in HOST BYTE ORDER.
*/
typedef struct osa_sockAddrIn_t
{
	osa_sockDomain_e domain;
	char 			 addr[SOCKADDR_MAX_STR_SZ];
	u16_t			 port; 
	/* IPv6 specific. Do we need them?
	u32_t 			 flowInfo;
	u32_t 			 scope_id; 
	*/
}osa_sockAddrIn_t;

/* osa_sockAddrGeneric_t : The Socket IPC has become ubiquitous and many address families (domains) are created to
						   use sockets for various tasks. This generic structure is used to wrap all of those.
	addr : This should be a pointer to domain specific structure. Library will internally typecast it according to domain
	addrLen : Size of the buffer/structure pointed to by 'addr'
*/
typedef struct osa_sockAddrGeneric_t
{
	osa_sockDomain_e domain;
	void *			 addr;
	u32_t 			 addrLen;
}


/* osa_sendCompleteCb : Send complete indication callback for non-blocking (asynchronous) io send (e.g. socket send).
					 This function will be called by osa-lib after data is sent on fd. User can do any post-processing
					 needed (freeing memory etc) in this callback.

   IN hd 		   : Handle for which this callback is called.
   IN appData	   : Caller provided pointer. This could point to a structure/buffer in caller  memory.
				     Caller can deference and use it in this callback function.
*/
void (*osa_sendCompleteCb)(osa_socket &sock, void * appData);


/* recvReadyCb    : Recv ready indication callback for non-blocking io recv (e.g. socket recv). 
					This function will be called by osa-lib when fd is ready for recv operation i.e. there is data 
					available to be read on this handle (e.g. packet available on a socket). Caller should call recv() 
					(e.g. osa_recv/recvfrom) inside this callback.
					
   IN osa_ioHd_t  : The handle on which data is ready.
   IN appData	  : Caller provided pointer. This could point to a structure/buffer in caller  memory.
				    Caller can deference and use it in this callback function.
*/
void (*osa_recvReadyCb)(osa_socket &sock, void * appData);


class osa_socket
{

public:

/* create 	: Create a socket. (This creates an empty socket. It needs to be configured with further 
				  API calls before it can be used)

	IN domain 	:: AF_UNIX/AF_INET/AF_INET6/AF_NETLINK/AF_PACKET (Mainly these would be the values)
	IN type   	:: SOCK_STREAM/SOCK_DGRAM/SOCK_SEQPACKET/SOCK_RAW/SOCK_RDM/SOCK_PACKET. 
				   For TCP connection, use SOCK_STREAM.
				   For UDP connection, use SOCK_DGRAM.   Others need advance understanding.
	IN protocol :: Normally it should be set to 0 (zero). Others need advance knowledge.
	OUT sockErr :: Socket specific error
*/
	ret_e create(osa_sockDomain_e domain, osa_sockType_e type, i32_t protocol, osa_SockErr_e &sockErr);

/* Set the socket for asynchronous io. Refer Asynchronous IO section for details */
	ret_e makeASynchronous(osa_sendCompleteCb sendCompleteCb, osa_recvReadyCb recvReadyCb, void * appData);


/* bind		: Bind an address to the empty socket.

	IN addr 	: This provides Domain, address and port. Address is to be provided in textual format and 
				  port is in host byte order.
				  Address and port and converted to appropriate formats internally based on the domain
				  parameter.
*/
	ret_e bind(osa_sockAddrIn_t &addr, osa_SockErr_e &sockErr);

/* bind		: Bind an address to the empty socket.

	IN addr 	: This provides Domain and address. Address is internall typecasted to specific structure
				  based on domain				  
*/
	ret_e bind(osa_sockAddrGeneric_t &addr, osa_SockErr_e &sockErr);


/* listen	: Start listening for new connections. This tells the operating systems that we are now ready to accept
				  new connections. Only valid for TCP sockets (SOCK_STREAM/SEQPACKET) and UDP (SOCK_DGRAM) doesn't have a 
				  concept of 'connection'.
				  The listening will start on address and port given in bind call earlier. If bind was not called, operating
				  system will usually assign an address and port and starts listening on that.

	IN maxCon	: Maximum number of simultanuous connections allowed. This will determine the maximum number of clients
				  that can connect to this server.
*/
	ret_e listen(i32_t maxCon, osa_SockErr_e &sockErr);


/* accept	: Accept a new connection. Only valid for TCP sockets (SOCK_STREAM/SEQPACKET).
				  If the socket is marked as asynchronous, this function should be called in osa_recvReadyCb() callback.

   IN remoteAddr: Remote address and port.
*/
   ret_e accept(osa_sockAddr_t &remoteAddr, osa_SockErr_e &sockErr);

/* connect	: Connect to a remote socket. If 'socket' is a TCP socket, then this call attempts to establish a tcp 
				  	  connection with remote server (with address remoteAddr). 
				  	  If its a UDP socket, this call tells the operating system that only packets from remoteAddr will/should 
				  	  be accepted. Packets coming from any other address will be rejected by OS and will not be delivered to 
				  	  this process.

   IN remoteAddr: Remote socket address (family, address, port). 
*/
   ret_e connect(osa_sockAddr_t &remoteAddr, osa_SockErr_e &sockErr);


/* send	: Send data on a socket. Can be used for a connected (tcp) socket.
					  If socket has been configured for asynchronous io (with osa_io_makeASynchronous), this
					  data will be added to an internal queue and once that is sent out, sendCompleteCb() will be called.

	IN buf 			: Buffer which contains the data (i.e. packet in most of the cases)
	IN len 			: Length of the data to be sent. 'buf' must be atleast 'len' bytes long
	IN flags 		: Flags control certain aspects of send operation. It is bitwise OR of the values to be given.
						e.g. MSG_DONTROUTE, MSG_OOB. (Flags may change depending on the platform/OS being used)
*/
	ret_e send(void * buf, i32_t len, i32_t flags, osa_SockErr_e &sockErr);


/* sendto	: Send data on a socket. Can be used for a datagram (udp) socket (as well as tcp. For TCP, remoteAddr
					  should be empty). If socket has been configured for asynchronous io (with osa_io_makeASynchronous), 
					  this data will be added to an internal queue and once that is sent out, sendCompleteCb() will be called.

	IN buf 			: Buffer which contains the data (i.e. packet in most of the cases)
	IN len 			: Length of the data to be sent. 'buf' must be atleast 'len' bytes long
	IN flags 		: Flags control certain aspects of send operation. It is bitwise OR of the values to be given.
						e.g. MSG_DONTROUTE, MSG_OOB. (Flags may change depending on the platform/OS being used)
*/
	ret_e sendto(void * buf, i32_t len, i32_t flags, osa_sockAddr_t &remoteAddr, osa_SockErr_e &sockErr);


/* recv 	: Receive data from socket. Can be used with connected (tcp) sockets.
					  If socket has been configured for asynchronous io (with osa_io_makeASynchronous), this function 
					  should be called in osa_recvReadyCb callback.
					  
	OUT buf 		: Buffer where data is to be copied
	IN  bufSize 	: size of the buffer 'buf'
	OUT bytesRead 	: Actual number of bytes copied in 'buf' by OS
	IN  flags 		: e.g. PEEK : This returns the length of available data. You can increase the buffer size if 
*/
	ret_e recv(void * buf, i32_t bufSize, i32_t *bytesRead, i32_t flags, osa_SockErr_e &sockErr);


/* recvfrom: Receive data from socket. Can be used with connected and datagram (tcp/udp) sockets.
					  If socket has been configured for asynchronous io (with osa_io_makeASynchronous), this function 
					  should be called in osa_recvReadyCb callback.

	OUT buf 		: Buffer where data is to be copied
	IN  bufSize 	: size of the buffer 'buf'
	OUT bytesRead 	: Actual number of bytes copied in 'buf' by OS
	IN  flags 		: e.g. MSG_PEEK //TO DO: Add enum
	OUT remoteAddr	: Remote socket details (address, port)
*/
	ret_e recvfrom(void * buf, i32_t bufSize, i32_t *bytesRead, i32_t flags, 
		osa_sockAddr_t &remoteAddr, osa_SockErr_e &sockErr);


/* destroy 	: Close the socket. This call closes the socket and frees the socket context inside kernel */
	ret_e destroy();

private:
	int sockFd;

};




/********************************************************
*			A S Y N C H R O N O U S    I O
*********************************************************/

/* Asynchronous IO is very important and useful tool for writing optimized and efficient programs. Without async io, a process
   will waste lot of time waiting for device to be ready for io (i.e. network card ready to send packet or one video frame 
   ready to be captured). The Read/write calls will simply block (sleep) till the actual action can be performed. Thus wasting
   lot of time doing nothing.
   By using asychronous notifications along with multi-threading, a process can keep on doing other work and only when it gets
   an io-ready notification, it can do the read/write calls.
*/


/* osa_io_makeAsynchronous :: Go for asynchronous io with a given fd.
	By default, when you open a device, it will be configured for synchronous io i.e. your write will 
    block until data is actually written to the device and read will block till data is available to be read. This behavior 
    can be changed by using this function. After this function is called, the 'send()' function will internally push the
    data to a queue and immediately return. Another thread will take care of sending it out. Once thats done, 'sendCompleteCb'
    will be called. On the other hand, the 'recvReadyCb()' will be called whenever data is available on the fd. Internally 
    these  functions will make use of OS provided async io premitives such as select/poll/epoll.

	IN fd 	   	   		:: File descriptor for a file/device. 
	IN osa_sendCompleteCb  :: Caller needs to provide this callback function. osa library will call this when actual send
							  is done. 
	IN osa_recvReadyCb  :: Caller needs to provide this callback function. osa library will call this. 
						   User can call actual 'recv' in this callback.
*/
ret_e osa_io_makeASynchronous(osa_ioHd_t fd, osa_sendCompleteCb sendCompleteCb, osa_recvReadyCb recvReadyCb);

/** TO DO : Do we add inotify type api here?? **/




/********************************************************
*			F I L E    P R O C E S S I N G
*********************************************************/

#ifdef __linux__
typedef FILE  				osa_fileHd_t;
#endif

typedef osa_fileErr_e
{}osa_fileErr_e;

/* File opening modes. "Read-Write" mode is deliberately avoided as the behavior is confusing/not intuitive.
   If you want to do both the operations on the file, you should open it twice in different modes */
typedef enum osa_file_mode_e
{
	OSA_FILE_READONLY,		/* File is opened for read and file pointer is placed at the beginning of the file */
	OSA_FILE_WRITEONLY,     /* File is opened for write (only). If the file has any data, it is truncated.
							   File pointer is placed at the beginning of the file */
	OSA_FILE_APPEND_ONLY,   /* File is opened for append (only). 
							   If the file exists already, the file pointer is placed at the end of the file (where you can 
							   append). If the file doesnt exist, new file is created for writing. */
 }osa_file_mode_e;


/* osa_file_open : Open a file for operation.
	IN/OUT  hd 		: File Handle.
	IN      path	: File path on the disk
	IN 		mode 	: Mode in which to open a file. Should be of type #osa_file_mode_e
	OUT     fileErr : File operations specific error
*/
ret_e osa_file_open(osa_fileHd_t &hd, char * path, osa_file_mode_e mode, osa_fileErr_e &fileErr);

/* osa_file_close : Close the file descriptor 
	IN 		hd 		: File Handle.
	OUT     fileErr : File operations specific error
	*/
ret_e osa_file_close(osa_fileHd_t &hd, osa_fileErr_e &fileErr);


/* osa_file_read : Read data from file.
	IN hd 			: File Handle
	IN buf 			: Buffer handle to read the data from. It is the responsibility of the caller to check that 'buf' is not 
					  NULL. 
	IN/OUT numBytes : Number of bytes to read. It is the responsibility of caller to make sure that 'buf' is large enough 
					  to hold 'numBytes'. When returned, this variable holds the number of bytes actually read from the file.
	OUT     fileErr : File operations specific error
*/
ret_e osa_file_read(osa_fileHd_t &hd, void * buf, int &numBytes, osa_fileErr_e &fileErr);

/* osa_file_write : Write data to file.
	IN hd 			: File Handle
	IN buf 			: Buffer handle to write the data from. It is the responsibility of the caller to check that 'buf' is not 
					  NULL.
	IN/OUT numBytes : Number of bytes to write. It is the responsibility of caller to make sure that 'buf' is at least 
					  'numBytes long. When returned, this variable holds the number of bytes actually written to the file.
	OUT     fileErr : File operations specific error
*/
ret_e osa_file_write(osa_fileHd_t &hd, void * buf, int &numBytes, osa_fileErr_e &fileErr);

/* osa_file_setCurPos : Set position of file pointer to 'pos' bytes from the start
	pos 			: Position to set the filepointer to (from the start)
	OUT     fileErr : File operations specific error
*/
ret_e osa_file_setCurPos(osa_fileHd_t &hd, int pos, osa_fileErr_e &fileErr);

/* osa_file_getCurPos : Get position of file pointer to 'pos' bytes from the start.
	pos 			: Position to get the filepointer to (from the start)
	OUT     fileErr : File operations specific error
*/
ret_e osa_file_getCurPos(osa_fileHd_t &hd, int &pos, osa_fileErr_e &fileErr);


/********************************************************
*					T H R E A D S
*********************************************************/

typedef void *	osa_threadHd_t;


/* To Do : pthread_sigmask, sigaltstack, detachable thread,	pthread_kill
*/

/* Thread Attributes::
	1. cpu affinity
	2. detach state (joinable or not joinable)
	3. guard size 			:: A guard area of size 'guard size' around the thread stack will be kept vacant.
	4. inherit-shed 		:: Whether to take the scheduling related params from the parent thread or should be taken
					  		   from input parameters supplied (see below)					  
	5. scheduling priority 	:: Linux allows this value to be from 0 to 99. 99 being the highest.
	6. scheduling policy 	:: standard, batch, idle, fifo, round-robin. Here fifo and round-robin are 'real-time' policies.
							   which means threads with these priorities will get scheduled before standard/batch/idle threads.
	7. contention scope 	:: system or process. This decides whether the thread competes for scheduling with all other threads/
						       processes in the system or with other threads in the process. I read somewhere that process scope 
						       is better since it allows for thread to bound to any available LWP (light weight process) at a given
						       time and that in turn improves performance. 
	8. stack 				:: User can allocate and provide the memory area where the thread's stack should reside. If this attribute
						       is not provide, thread library will take care of it internally. Should be used only in environment with
						       special needs/restrictions.
*/


/* Thread entry point :: User needs to define a function matching this signature and provide it in 'osa_thread_create()'.
						 When the thread is started, it starts executing this function.	This function is like 'main()' for 
						 the calling thread. You can call other functions and do everything else (loops, if/else etc). All of that will
						 be executed in the new thread.
						 When this function returns, the thread ends (just like your program ends when you return from main())
*/
void * (*ThreadFunc)(void *);

/* Check #Thread Termination and osa_thread_cleanup_push() for details */
void (*CleanupCb) (void *arg);

typedef enum osa_thread_priority_e
{
	OSA_THREAD_PRIO_DEFAULT,
	OSA_THREAD_PRIO_IDLE,
	OSA_THREAD_PRIO_LOW,
	OSA_THREAD_PRIO_HIGH,
	OSA_THREAD_PRIO_CRITICAL,
}osa_thread_priority_e;

typedef struct osa_thread_stack_t 
{
	i32_t sz;
	u8_t  *buf;
}osa_thread_stack_t;


/* osa_thread_create :: Create a new thread
		OUT t 		:: Thread handle. After the call returns, 't' will be populated with thread handle which should be used in
					   all further API calls for this thread.
		IN func 	:: Entry point for the thread. This is equivalent to 'main()' for this thread. When the thread starts, it will
					   start from this function and it will end when this function exits. user can call other functions and do
					   everything that you do normally from within this function and it will be executed as part of thread 't'.
		IN prio 	:: Thread priority. Set priority for the thread according to the application needs. Value should be an
					   enum from #osa_thread_priority_e. User should set it to OSA_THREAD_PRIO_DEFAULT if no special requirement.
		IN stack    :: User can provide where thread stack should reside. This should be used only if user has specialized
					   requirements. Otherwise it should be set to NULL.
		
		IN thrName 	:: Give a name to this thread. Helpful for debugging. Can be set to NULL. IMP: Max length of the string 
					   should be 15.
*/
ret_e osa_thread_create(osa_threadHd_t &t, ThreadFunc func, osa_thread_priority_e prio, osa_thread_stack_t &stack,  
							void * arg, char thrName[15]);
	


/* THREAD TERMINATION : The easiest way to terminate a thread is to return from the ThreadFunc 't' that is its entry point.
						However, there are 2 other ways provided for thread termination ::
						1. osa_thread_exit() : self exit
						2. osa_thread_kill() : Kill another thread
					
						These 2 APIs are for abrupt exit. Hence some cleanup might be needed so that program doesn't remain in
						inconsistent state (e.g. unlocking previously acquired mutexes)
						For this purpose, following API is provided to register cleanup callback functions ::
						
						1. osa_thread_cleanup_push(callbackFunc)
						2. osa_thread_cleanup_pop()

						Lets assume that your thread starts in function 't'. Then 't' calls 'u', 'u' call 'v' and 'v' calls 'w'.
						Now, 'w' calls osa_thread_exit(). Each of  t,u,v may want to do some cleanup. To achieve this, every 
						function can push their cleanup function using osa_thread_cleanup_push() on a sort of cleanup stack 
						before calling next function. So, when 'w' calls exit, these cleanup functions will be popped from 
						stack and executed i.e. v-cleanup() then u-cleanup and finally t-cleanup.

						If 'w' doesn't exit then functions v,u,t can pop their cleanup functions manually using osa_thread_cleanup_pop()
						before returning as it won't be needed.
*/

/* osa_thread_exit	:: Exit the current thread. 
					   This function can be used to exit the current thread. Useful if you are in some nested function (t call u calls v
					   calls w ...) and wants this thread to stop/die immediately. 
					   Note: This function can also be called from 'main()' function of the 'process'. Normally, if main returns then
					   all other threads get killed. But if you call osa_thread_exit() from main(), then only main thread exits and
					   program continues untill all other threads exit.
		OUT retVal 	:: Return value to be returned to any other thread which has called pthread_join() on this thread.
*/
ret_e osa_thread_exit(void * retVal);


/* osa_thread_kill :: Kill another thread  (TO DO)
					  Not sure how this API should behave, at this point. Should it kill the thread immediately or should that
					  thread be given a chance to exit itself
*/
ret_e osa_thread_kill(osa_threadHd_t &t);

/* osa_thread_cleanup_push() : Register cleanup callback.
		If osa_thread_exit() is called after this, this callback function will automatically be called. 'arg' will be passed as its 
		argument.
*/
ret_e osa_thread_cleanup_push(CleanupCb cleanupCb, void * arg);

/* osa_thread_cleanup_pop(): Remove the topmost cleanup function from cleanup stack
   IMP: This needs to be called ONLY in case of normal execution. If thread-exit is called, it will be executed automatically.
   Also, Its important to call pop if you had called push before. Otherwise it could lead to errors.
   		IN execute :: Do you want to execute it after popping. 0 : Don't execute. Non-zero value means execute.
*/
ret_e osa_thread_cleanup_pop(int execute);


/* THREAD SYNCHRONIZATION */

/* MUTEX : Mutex means locking. Its like you enter your house and lock it from the inside. Only you can open/unlock it. 
		   Unless you unlock it, nobody else can enter the house or lock it again.
		   Similarly, a thread can lock a mutex. Only that thread can unlock it. And once a mutex is locked by a thread,
		   it can not be locked again by another thread unless the first thread unlocks it.

		   By default the mutex is non-recursive (i.e. a thread can not lock the mutex again e.g. if it is recursing).
		   TO DO: Should the mutex be recursive by default??
		   Right now additional mutex parameters such as 'recursive-mutex' are not supported (deliberately). Simply because I
		   haven't seen the need for it in my experience. If there is a real need, this feature can be added.
*/
typedef void * osa_mutex_t;

/* osa_mutex_init : Initialize a mutex. Before using a mutex, you need to initialize it. Basically operating system will do
				    the necessary preparation to handle lock/unlock operations in future */
ret_e osa_mutex_init(osa_mutex_t &mutex);

/* osa_mutex_destroy : Destroy a mutex. After this API is called, this mutex should not be used. Or there will be undefined behaviour
*/
ret_e osa_mutex_destroy(osa_mutex_t &mutex);

/* osa_mutex_lock() : Acquire the lock.
					Once this API returns successfully, nobody else can get the lock unless the same thread calls osa_mutex_unlock().
					It is mainly used for thread synchronization. Whenever there is a piece of code that multiple threads want to call/
					exercise (popularly called critical-section), it should be wrapped within these lock-unlock APIs. So that only one 
					thread will execute it at any time and there won't be a race condition
*/
ret_e osa_mutex_lock(osa_mutex_t &mutex);

/* osa_mutex_unlock() : Release/Unlock a mutex.
					This call must be made by the thread if it has called osa_mutex_lock() before. It should be called at the end
					of critical section. If 'lock' was called and unlock is never called then other threads might remain locked 
					forever
*/
ret_e osa_mutex_unlck(osa_mutex_t &mutex);


/* SEMAPHORES : Semaphore are also a form of lock that allow multiple people (threads) in.
				As described here : http://niclasw.mbnet.fi/MutexSemaphore.html, they are like a set of keys to identical toilets.
				For the sake of analogy, assume that keys are hanging just outside door. If there is at least one key available, 
				anybody can take it and get inside the toilet. If there is no key, you need to 'wait'.

				When somebody goes out, s/he 'posts' the key i.e. puts it back outside the door which can be used by 
				anybody waiting outside in a queue 
*/

typedef void * osa_sem_t;

/* osa_sem_init :: Initialize a semaphore.
	IN/OUT s 		:: Semaphore handle. Its populated with correct value if call is successful.
	IN     count	:: Initial count of the semaphore
*/
ret_e osa_sem_init(osa_sem_t &s, uint32_t count);

/* osa_sem_destroy :: Destroy a sempahore
	IN s  			:: Semaphore to be destroyed
*/
ret_e osa_sem_destroy(osa_sem_t &s);

/* osa_sem_wait 	:: Acquire a semaphore.
					   Its more like osa--sem--get, but the thread waits (blocks) if no semaphore is available, hence the name wait.
					   Once the semaphore is available, the thread can start executing the critical section (more like shared 
					   critical section)

					   Internally, 't' decrements the 'count' (of available semaphores)
*/
ret_e osa_sem_wait(osa_sem_t &s);

/* osa_sem_post		:: Release the semaphore
					   Once the critical section code is executed, the thread should call osa_sem_post() to release the semaphore
					   for somebody else to use
					   Internally, t increments the 'count' (of available semaphores)
*/
ret_e osa_sem_post(osa_sem_t &s);


/* CONDITIONAL VARIABLES : Conditional variables are signalling mechanisms. It can be used to stop/start the execution of a thread until
						   another thread signals it.
						   Conditional variables have to be used inside a critical section that use mutexes. E.g. Threadpool.
						   You can have multiple worker threads and one master thread. All threads will call the 'wait' function and 
						   wait for a signal from master. Master can call 'signal' function to wake up one or all threads and then they
						   will continue the execution. Master can assign work by using global variables (e.g. callback function pointers
						   that those thread can call & execute).

						   Conditional variables are little confusing. The 'wait' api will unlock the mutex internally and go to sleep.
						   Once another thread signals the conditional variable, it will call 'lock mutex' again. Once the lock is acquired,
						   'wait' api will return and thread can continue. Due to this internal ugliness, the caller should not assume that
						   values of global variables will be same as before the 'wait' call.
*/
typedef void * osa_cond_t;

/* osa_cond_init() : Initialize a conditional variable.
		OUT c 	   : After successful initialization, 'c' can be used for further actions
*/
ret_e osa_cond_init(osa_cond_t &c);

/* osa_cond_destroy() : Destroy a conditional variable.
 		OUT c 		  : Conditional variable to be destroyed. 'c' can no longer be used after this api call.
*/
ret_e osa_cond_destroy(osa_cond_t &c);

/* osa_cond_wait()	: Wait for signal on conditional variable c
	IN c 			: Conditional variable to be waited on. Must be initialized using osa_cond_init() before
	IN m 			: Paired mutex. Conditional variables are always tied with mutexes. This mutex will be unlocked before going
					  to sleep and locked again after other threads wakes us up.
*/
ret_e osa_cond_wait(osa_cond_t &c, osa_mutex_t &m);

/* osa_cond_signal() : Signal/wake-up one of the waiting threads.
					   If multiple threads are waiting, only one thread will be chosen based on scheduling policy and woken up.
	IN c 			 : conditional variable to be signaled.
*/
ret_e osa_cond_signal(osa_cont_t &c);

/* osa_cond_broadcast() : Signal/wake-up all the waiting threads.
					   All threads will be woken up. All will try to get a lock (internally). One by one, as threads complete
					   critical section, they will release mutex and new thread will acquire it and proceed.
*/
ret_e osa_cond_broadcast(osa_cond_t &c);



/********************************************************
*					Q U E U E S
*********************************************************/

/* Queues are important data structures for asynchronous io. When different threads that are waiting on different devices
   want to communicate, queue is the best data structure. The sender thread can push the data to the queue and receiver
   thread will read the data from the queue. The Queue API will internally implement mutex based access control so that
   there is no race condition.
*/

typedef void *	osa_q_t;

typedef struct q_data_t
{
	void *obj;
	uint32_t size;
}q_data_t;

ret_e osa_q_create(osa_q_t &q, uint32_t maxSize);

ret_e osa_q_destroy(osa_q_t &q);

ret_e osa_q_flush(osa_q_t &q);

ret_e osa_q_push(osa_q_t &q, q_data_t &data);

ret_e osa_q_pop(osa_q_t &q, q_data_t &data);

ret_e osa_q_peek(osa_q_t &q, q_data_t &data);

ret_e osa_q_curSize(osa_q_t &q, uint32_t &curSize);



#endif

