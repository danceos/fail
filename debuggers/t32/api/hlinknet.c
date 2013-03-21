/****************************************************************
*                                                               *
*         Copyright by Lauterbach GmbH                          *
*         Alle Rechte vorbehalten - All rights reserved         *
*                                                               *
*****************************************************************

  Module:       hlinknet.c
  Function:     Low level communication protocol for talking to TRACE32.
                Is used by CAPI routined implemented in hremote.c.
                Link both files with your application.

***************************************************************/

#include "t32.h"

#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4255 )
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef MS_WINDOWS
# include <winsock2.h>
# include <windows.h>
# include <fcntl.h>
typedef int socklen_t;
#endif

#ifdef DEC_VMS
# include <file.h>
# include <time.h>
# include <errno.h>
# include <socket.h>
# include <inet.h>
# include <netdb.h>
# include <in.h>
# define DONT_USE_ASYNC
# define fd_set  int
# define FD_ZERO(fdset) (*fdset = 0)
# define FD_SET(fd,fdset) (*fdset |= (1<<(fd)))
# define FD_ISSET(fd,fdset) (*fdset & (1<<(fd)))
#endif

#ifdef __linux__
# include <fcntl.h>
# include <unistd.h>
# include <sys/time.h>
# include <errno.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <sys/select.h>
#endif

#ifdef UNIX_V
# include <fcntl.h>
# include <unistd.h>
# include <sys/time.h>
# include <errno.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# if defined(T32HOST_HPUX) || defined(HP_UX)
typedef int socklen_t;
# else
#  include <sys/select.h>
# endif
#endif

#ifdef OS_9
# include <fcntl.h>
# include <sys/time.h>
# include <errno.h>
# include <inet/socket.h>
# include <inet/in.h>
# include <inet/netdb.h>
#endif

#if defined(_MSC_VER)
# pragma warning( pop )
#endif


#define PCKLEN_MAX 1472                 /* maximum size of UDP-packet */
#define BUFLEN_MIN 6000					/* !!! keep in sync with hassist.c remoteapi !!! */

#ifndef MS_WINDOWS
# ifndef UNIX_V
extern struct hostent * gethostbyaddr();
# endif
#endif

#if defined(DEC_VMS) || defined(MS_WINDOWS) || defined(OS_9) || defined(LINUX)
# define RECEIVEADDR (&ReceiveSocketAddress)
static struct sockaddr ReceiveSocketAddress;
#else
# define RECEIVEADDR 0
#endif


typedef struct LineStruct_s {
    char                NodeName[80];               /* node name of host running T32 SW */
    int                 CommSocket;                 /* socket for communication */
    unsigned short      HostPort;		            /* Host side port */
    unsigned short      ReceivePort;                /* Receiver Port */
    unsigned short      TransmitPort;               /* Transmitter Port in T32 */
    int                 PacketSize;     	        /* Max. size of UDP-packet data */
    int                 PollTimeSec;
    int                 ReceiveToggleBit;
    unsigned char       MessageId;
    int                 LineUp;
    unsigned short      ReceiveSeq, TransmitSeq;    /* block-ids */
    unsigned short      LastReceiveSeq, LastTransmitSeq;
    unsigned char*      LastTransmitBuffer;
    int                 LastTransmitSize;
    struct sockaddr_in  SocketAddress;
} LineStruct;

#ifdef MS_WINDOWS
extern void T32_InstallAsyncSelect(HWND hwnd, int msg);
extern void T32_UnInstallAsyncSelect(HWND hwnd);
#endif

extern void LINE_SetReceiveToggleBit(int value);
extern int LINE_GetReceiveToggleBit(void);
extern int LINE_GetNextMessageId(void);
extern int LINE_GetMessageId(void);
extern int LINE_GetLineParamsSize (void);
extern void LINE_SetDefaultLineParams (LineStruct* params);
extern LineStruct* LINE_GetLine0Params (void);
extern void LINE_SetLine (LineStruct* params);
extern int LINE_LineConfig(char * in);
extern void LINE_LineExit(void);
extern int LINE_LineInit(char * message);
extern int LINE_LineTransmit(unsigned char * in, int size);
extern int LINE_LineDriverGetSocket(void);
extern int LINE_LineReceive(unsigned char * out);
extern int LINE_ReceiveNotifyMessage(unsigned char* package);
extern int LINE_LineSync(void);

static int Connection(unsigned char *ipaddrused);

static struct timeval LongTime = { 0, 500000 };



static const LineStruct LineDefaultParams = {
    "localhost", -1, 0, 0, 20000, 1024, 5, -1, 0, 0};
static LineStruct  Line0Params = {
    "localhost", -1, 0, 0, 20000, 1024, 5, -1, 0, 0};

static LineStruct* pLineParams = &Line0Params;



static int str2dec (char* in)
{
    int x = 0;
    while (*in) {
        x *= 10;
        if (*in < '0' || *in > '9')
            return -1;
        x += *in - '0';
        in++;
    }
    return x;
}


void LINE_SetReceiveToggleBit(int value)
{
    pLineParams->ReceiveToggleBit = value;
}


int LINE_GetReceiveToggleBit(void)
{
    return pLineParams->ReceiveToggleBit;
}


int LINE_GetNextMessageId(void)
{
    return ++pLineParams->MessageId;
}


int LINE_GetMessageId(void)
{
    return pLineParams->MessageId;
}


int LINE_GetLineParamsSize (void)
{
    return sizeof (LineStruct);
}

void LINE_SetDefaultLineParams (LineStruct* params)
{
    *params = LineDefaultParams;
    return;
}


LineStruct* LINE_GetLine0Params (void)
{
    return &Line0Params;
}


void LINE_SetLine (LineStruct* params)
{
    pLineParams = params;
}


int LINE_LineConfig(char * in)
{
    int         x;
    LineStruct* line = pLineParams;

    if (!strncmp((char *) in, "NODE=", 5)) {
        strcpy(line->NodeName, in+5);
        return 1;
    }
    if (!strncmp((char *) in, "PORT=", 5)) {
        x = str2dec (in+5);
        if (x == -1)
            return -1;
        line->TransmitPort = x;
        return 1;
    }
    if (!strncmp((char *) in, "HOSTPORT=", 9)) {
        x = str2dec (in+9);
        if (x == -1)
            return -1;
        line->HostPort = x;
        return 1;
    }
    if (!strncmp((char *) in, "PACKLEN=", 8)) {
        x = str2dec (in+8);
        if (x == -1)
            return -1;
        line->PacketSize = x;
        return 1;
    }
    if (!strncmp((char *) in, "TIMEOUT=", 8)) {
        x = str2dec (in+8);
        if (x == -1)
            return -1;
        line->PollTimeSec = x;
        return 1;
    }
    return -1;
}


static void WinsockErrorMessage(char * out)
{
#ifdef MS_WINDOWS
    int             err;
    err = WSAGetLastError();

    switch (err) {
    case WSAHOST_NOT_FOUND:
	strcat(out, " (HOST_NOT_FOUND)");
	break;
    case WSATRY_AGAIN:
	strcat(out, " (TRY_AGAIN)");
	break;
    case WSANO_RECOVERY:
	strcat(out, " (NO_RECOVERY)");
	break;
    case WSANO_DATA:
	strcat(out, " (NO_DATA)");
	break;
    case WSAEINTR:
	strcat(out, " (INTR)");
	break;
    case WSANOTINITIALISED:
	strcat(out, " (NOTINITIALISED)");
	break;
    case WSAENETDOWN:
	strcat(out, " (NETDOWN)");
	break;
    case WSAEAFNOSUPPORT:
	strcat(out, " (WSAEAFNOSUPPORT)");
	break;
    case WSAEINPROGRESS:
	strcat(out, " (INPROGRESS)");
	break;
    case WSAEMFILE:
	strcat(out, " (WSAEMFILE)");
	break;
    case WSAENOBUFS:
	strcat(out, " (WSAENOBUFS)");
	break;
    case WSAEPROTONOSUPPORT:
	strcat(out, " (WSAEPROTONOSUPPORT)");
	break;
    case WSAEPROTOTYPE:
	strcat(out, " (WSAEPROTOTYPE)");
	break;
    case WSAESOCKTNOSUPPORT:
	strcat(out, " (WSAESOCKTNOSUPPORT)");
	break;
    }
#endif
}


static long GetInetAddress(char * name, char * message)
{
    struct hostent *hp;
    int             i1, i2, i3, i4;

    char            ownname[256];

    if (name == NULL) {
	gethostname(ownname, sizeof(ownname));
	name = ownname;
    }
    i1 = i2 = i3 = i4 = 0;

    if (sscanf(name, "%d.%d.%d.%d", &i1, &i2, &i3, &i4) == 4) {
	if (i1 || i2 || i3 || i4) {
	    return (i1 << 24) | (i2 << 16) | (i3 << 8) | (i4);
	}
    }
    hp = gethostbyname(name);

    if (!hp) {
	strcpy(message, "node name (");
	strcat(message, name);
	strcat(message, ") unknown");
	WinsockErrorMessage(message);
	return -1l;
    }
    return ntohl(*(long *) (hp->h_addr));
}


void LINE_LineExit(void)
{
    int         i;
    LineStruct* line = pLineParams;
    static const unsigned char discon[] = {4, 0, 0, 0, 0, 0, 0, 0, 'T', 'R', 'A', 'C', 'E', '3', '2', 0};

    if (!line->LineUp)
        return;

    if (line->CommSocket != -1) {
        for (i = 0; i < 5; i++)
            sendto(line->CommSocket, (char *) discon, 16, 0, (struct sockaddr *) &(line->SocketAddress), sizeof(line->SocketAddress)); /* send EXIT 4 */

#ifdef MS_WINDOWS
	closesocket(line->CommSocket);
#endif
#if defined(DEC_VMS) || defined(UNIX_V) || defined(OS_9)
	close(line->CommSocket);
#endif
    }
#ifdef MS_WINDOWS
    WSACleanup();
#endif

    line->CommSocket = -1;
    line->LineUp = 0;
}


int LINE_LineInit(char * message)
{
    int             i, j;
    socklen_t       length;
    int             val;
    unsigned char   ipaddrused[4];
    long            remote_ip;
    int             buflen;
    LineStruct*     line = pLineParams;

    if (line->LineUp)
        return 0;

    if (line->CommSocket == -1) {
#ifdef MS_WINDOWS
	WSADATA         wsaData;
	if (WSAStartup(0x0101, &wsaData)) {
	    strcpy(message, "TCP/IP not ready, check configuration");
	    return -1;
	}
#endif
    }
    if ((remote_ip = GetInetAddress(line->NodeName, message)) == -1l)
	return -1;

    if (line->CommSocket == -1) {
	line->SocketAddress.sin_family = AF_INET;
	line->SocketAddress.sin_addr.s_addr = INADDR_ANY;
	line->SocketAddress.sin_port = line->HostPort;	/* Port can be determined by
						 * host */

	if ((line->CommSocket = (int)socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	    strcpy(message, "cannot create socket");
	    WinsockErrorMessage(message);
	    goto done;
	}
	if ((i = bind(line->CommSocket, (struct sockaddr *) & line->SocketAddress, sizeof(line->SocketAddress))) == -1) {
	    strcpy(message, "cannot bind socket");
	    WinsockErrorMessage(message);
	    goto done;
	}
	length = sizeof(line->SocketAddress);
	if (getsockname(line->CommSocket, (struct sockaddr *) & line->SocketAddress, &length) == -1) {
	    strcpy(message, "cannot identify port");
	    WinsockErrorMessage(message);
	    goto done;
	}
	line->ReceivePort = ntohs(line->SocketAddress.sin_port);

	line->SocketAddress.sin_family = AF_INET;
	line->SocketAddress.sin_addr.s_addr = INADDR_ANY;
	line->SocketAddress.sin_port = htons(line->TransmitPort);
    }
    line->SocketAddress.sin_addr.s_addr = htonl(remote_ip);

    buflen = 18000;

#if !defined(OS_9)
    val = 0;
    length = sizeof(val);
    getsockopt(line->CommSocket, SOL_SOCKET, SO_RCVBUF, (char *) &val, &length);

    if (val > 0 && val < line->PacketSize)
        line->PacketSize = val;

    if (val < buflen) {
	val = buflen;
	length = sizeof(val);
	if (setsockopt(line->CommSocket, SOL_SOCKET, SO_RCVBUF, (char *) &val, length) == -1) {
	    strcpy(message, "cannot alloc buffer for rcvsocket");
	    goto done;
	}
	val = 0;
	length = sizeof(val);
	getsockopt(line->CommSocket, SOL_SOCKET, SO_RCVBUF, (char *) &val, &length);
	if (val < buflen) {
	    strcpy(message, "cannot alloc buffer for rcvsocket");
	    goto done;
	}
    }
    val = 0;
    length = sizeof(val);
    getsockopt(line->CommSocket, SOL_SOCKET, SO_SNDBUF, (char *) &val, &length);
    if (val < buflen) {
	val = buflen;
	length = sizeof(val);
	if (setsockopt(line->CommSocket, SOL_SOCKET, SO_SNDBUF, (char *) &val, length) == -1) {
	    strcpy(message, "cannot alloc buffer for sndsocket");
	    goto done;
	}
	val = 0;
	length = sizeof(val);
	getsockopt(line->CommSocket, SOL_SOCKET, SO_SNDBUF, (char *) &val, &length);
	if (val < buflen) {
	    strcpy(message, "cannot alloc buffer for sndsocket");
	    goto done;
	}
    }
#endif

    for (i = 0; i < 10; i++) {
	j = Connection(ipaddrused);

	if (j == 0)
	    continue;

	if (j == 1) {
	    line->LineUp = 1;
	    return 1;
	}

	strcpy(message, "TRACE32 access refused");
	goto done;
    }

    strcpy(message, "TRACE32 not responding");

done:
    LINE_LineExit();			/* Close connection if no success */
    return -1;
}


#ifdef MS_WINDOWS

void T32_InstallAsyncSelect(HWND hwnd, int msg)
{
#ifndef DONT_USE_ASYNC
    WSAAsyncSelect(pLineParams->CommSocket, hwnd, msg, FD_READ);
#endif
}

void T32_UnInstallAsyncSelect(HWND hwnd)
{
#ifndef DONT_USE_ASYNC
    WSAAsyncSelect(pLineParams->CommSocket, hwnd, 0, 0);
#endif
}

#endif

/**
   Sends a message to T32. Handles segmentation of _message_ into (one
   or multiple) _packages_.

   @param in pointer to outgoing message (already includes 5 byte message header)
   @param size size of message
   @note the message must be allocated such that there is space in
    front of the message for adding the packet header
   */

int LINE_LineTransmit(unsigned char * in, int size)
{
    int             packetSize;
    unsigned int    tmpl;
    int             result;
    unsigned int    bytesTransmitted = 0;
    LineStruct*     line = pLineParams;
    unsigned char   receivebuffer[HANDSHAKE_RECEIVEBUFFERSIZE];

    line->LastTransmitBuffer = in;
    line->LastTransmitSize = size;
    line->LastTransmitSeq = line->TransmitSeq;
    in -= 4; /* space for packet header */


    /* Assumed that Host SocketReceivebuffersize is 6000 (see hassist.c) */
    do {
	packetSize = (size > line->PacketSize - 4) ?  line->PacketSize - 4 : size;

	/* When sending multiple packets, the packet header is written inside the
	 message's payload. Original contents needs to be saved/restored */

	SETLONGVAR(tmpl, in[0]);               /* save */

	in[0] = 0x11;                          /* transmit data package */
	in[1] = (size > packetSize) ? 1 : 0;   /* more packets follow */
	SETWORDVAR(in[2], line->TransmitSeq);  /* packet sequence ID */

	if (sendto( line->CommSocket, (char *) in, packetSize + 4, 0, (struct sockaddr *) & line->SocketAddress, sizeof(line->SocketAddress) ) != packetSize + 4) { /* send Data Package 0x11 */
	    SETLONGVAR(in[0], tmpl);        /* restore buffer */
	    return 0;
	}
	bytesTransmitted += packetSize + 4;
	if( (bytesTransmitted + line->PacketSize) > BUFLEN_MIN ) {
	    socklen_t length;

	    length=sizeof(struct sockaddr);

	    if( (result = recvfrom(line->CommSocket, (char *)receivebuffer, 1, 0, (struct sockaddr *) RECEIVEADDR, &length)) < 0) {
		    return -1;
	    }
	}
	SETLONGVAR(in[0], tmpl);                  /* restore buffer */

	line->TransmitSeq++;
	in += packetSize;
	size -= packetSize;
    }
    while (size > 0);                          /* more packets required? */

    return line->LastTransmitSize;
}

/** Receives a package from the socket, with timeout handling.
  @return number of received bytes or error number (<0)
*/
static int ReceiveWithTimeout(struct timeval *tim, unsigned char *dest, int size)
{
    int             i;
    fd_set          readfds;
    socklen_t       length;
    struct timeval  timeout;
    LineStruct*     line = pLineParams;

    timeout = *tim;

#ifdef POLL_NET
    DWORD           endpoll;
    static struct timeval tival = {0};

    if (tim->tv_usec || tim->tv_sec)
	endpoll = GetCurrentTime() + tim->tv_usec / 1000 + tim->tv_sec * 1000;
    else
	endpoll = 0;
retry:
#endif

    FD_ZERO(&readfds);
    FD_SET( (unsigned int)line->CommSocket, &readfds);

#ifdef POLL_NET
    i = select(FD_SETSIZE, &readfds, (fd_set *) NULL, (fd_set *) NULL, &tival);

    if (i < 0)
	return i;

    if (i == 0) {
	if (endpoll) {
	    if (GetCurrentTime() < endpoll) {
		ScreenDispatcher();
		goto retry;
	    }
	}
	return i;
    }
#else
    i = select(FD_SETSIZE, &readfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);
#endif

    if (i <= 0) {
	return i;
    }
    length = sizeof(struct sockaddr);
    return recvfrom(line->CommSocket, (char *)dest, size, 0, (struct sockaddr *) RECEIVEADDR, &length);
}


int LINE_LineDriverGetSocket(void)
{
    return pLineParams->CommSocket;
}

/* Queue pending notifications */
T32_NotificationPackage *T32_NotificationHead = NULL, *T32_NotificationTail = NULL;

/** Receives messages from the socket. Assembles multiple packets into
   a single message.

  @param out output buffer for storing payload. Attention: there must
         be some space _before_ the output buffer (4 bytes or more)
         for temporary usage.
  @return number of bytes of the message or error number (<0)
 */

int LINE_LineReceive(unsigned char * out)
{
    int             i, flag;
    int             count;
    unsigned short  tmpw;
    unsigned int    tmpl;
    unsigned short  s;
    LineStruct*     line = pLineParams;
    register unsigned char  *dest;
    static unsigned char     handshake[] = {7, 0, 0, 0, 0, 0, 0, 0, 'T', 'R', 'A', 'C', 'E', '3', '2', 0};

retry:
    dest = out-4; /* adjust pointer so we place header BEFORE "out" and thus payload AT "out" */
    count = 0;
    s = line->ReceiveSeq;

    do {

        /* multiple packets are merged in-place: backup data that is
	   overwritten package aby header */
        SETLONGVAR(tmpl, dest[0]);

		do {
			struct timeval PollTime = {0, 0};
			PollTime.tv_sec = line->PollTimeSec;
			if ((i = ReceiveWithTimeout(&PollTime, dest, line->PacketSize)) <= 0) {
				if (i == -2)
					goto retry;
				return -1;
			}
			/* we got a handshakepackage */
			if( (i == 1) && (dest[0] == '+') )
			{
				goto retry;
			}

			if (i <= 4) {
				return -1;
			}


			/* Detect and enqeue async notification that slipped into a request/reply pair */
			if (dest[0] == T32_API_NOTIFICATION)
			{
				T32_NotificationPackage *newPackage, *oldHead;

				newPackage = (T32_NotificationPackage *)malloc(sizeof(T32_NotificationPackage));
				if (newPackage==NULL)
					return -1;

				memcpy(newPackage, dest, i); /* in theory i should always be the package size, at least for ethernet */
				oldHead = T32_NotificationHead;
				newPackage->prev = NULL;
				newPackage->next = oldHead;
				if (oldHead)
					oldHead->prev = newPackage;
				T32_NotificationHead = newPackage;
				if (T32_NotificationTail==NULL) {
					T32_NotificationTail=newPackage;
				}
				goto retry;
			}

			if (dest[0] != T32_API_RECEIVE) {
				return -1;
			}
			SETWORDVAR(tmpw, dest[2]);

			if (tmpw == line->LastReceiveSeq && line->LastTransmitSize) {
				line->TransmitSeq = line->LastTransmitSeq;
				LINE_LineTransmit(line->LastTransmitBuffer, line->LastTransmitSize);
			}
		}
		while (tmpw != line->ReceiveSeq);

	line->ReceiveSeq++;
	flag = dest[1];
	SETLONGVAR(dest[0], tmpl); /* restore payload overwritten by package header */
	dest += i - 4;
	count += i - 4;

	if (count > LINE_MSIZE) {
	    return -1;
	}
	if (flag == 2) {
	    if (sendto(line->CommSocket, (char *) handshake, 16, 0, (struct sockaddr *) &line->SocketAddress, sizeof(line->SocketAddress)) != 16) { /* send Handshake 7 */
		return -1;
	    }
	}
    }
    while (flag);

    line->LastReceiveSeq = s;

    return count;
}


/** Receives notification messages. First checks for a queued
    notification and if none is available polls the socket for new
    notifications. For getting all pending notifications call the
    function until it returns -1.

    @param package output buffer of size T32_PCKLEN_MAX for the package
    @return -1  no notification pending,
            >=0 notificataion type T32_E_BREAK, T32_E_EDIT, T32_E_BREAKPOINTCONFIG

 */


int LINE_ReceiveNotifyMessage(unsigned char* package)
{
	 int             len;
	 static struct timeval LongTime = {0, 0};

	 /* Check for asynchronous notifications */
	 if (T32_NotificationTail) {
		 T32_NotificationPackage *prev  = T32_NotificationTail->prev;
		 if (prev)
			 prev->next = NULL;
		 memcpy(package, T32_NotificationTail->payload, T32_PCKLEN_MAX);
		 free(T32_NotificationTail);
		 T32_NotificationTail = prev;
		 if (prev==NULL) /* deleted last message */
			 T32_NotificationHead = NULL;
	 } else {
		 len = ReceiveWithTimeout(&LongTime, package, T32_PCKLEN_MAX);
		 if (len < 2)
			 return -1;
	 }

	 if (package[0] != T32_API_NOTIFICATION)
		 return -1;

	 return package[1]; /* type of notification: T32_E_BREAK, T32_E_EDIT, T32_E_BREAKPOINTCONFIG  */
 }

/** Sends sync packets */
int LINE_LineSync(void)
{
    int             i, j;
    unsigned char   packet[T32_PCKLEN_MAX];
    static char     magicPattern[] = "TRACE32";
    LineStruct*     line = pLineParams;

    j = 0;
    memset(packet, 0, sizeof(packet));

retry:
    packet[0] = T32_API_SYNCREQUEST;
    packet[1] = 0;
    SETWORDVAR(packet[2], line->TransmitSeq);
    SETWORDCONST(packet[4], 0);
    SETWORDCONST(packet[6], 0);
    strcpy((char *) (packet + 8), magicPattern);

    if (sendto(line->CommSocket, (char *) packet, 16 /*size*/, 0, (struct sockaddr *) & line->SocketAddress, sizeof(line->SocketAddress)) == -1) { /* Send SyncReq 2 */
	return -1;
    }
    while (1) {		/* empty queue */
	if (++j > 20) {
	    return -1;
	}
	if ((i = ReceiveWithTimeout(&LongTime, packet, T32_PCKLEN_MAX)) <= 0) {
	    return -1;
	}
	if (i != 16 || packet[0] != T32_API_SYNCACKN || strcmp((char *) packet + 8, magicPattern)) {
	    if (i == 16 && packet[0] == 5)
		goto retry;
	    continue;
	}
	break;
    }

    SETWORDVAR(line->ReceiveSeq, packet[2]);
    line->LastReceiveSeq = line->ReceiveSeq - 100;


    packet[0] =  T32_API_SYNCBACK;
    packet[1] = 0;
    SETWORDVAR(packet[2], line->TransmitSeq);
    SETWORDCONST(packet[4], 0);
    SETWORDCONST(packet[6], 0);
    strcpy((char *) (packet + 8), magicPattern);

    if (sendto(line->CommSocket, (char *) packet, 16, 0, (struct sockaddr *) & line->SocketAddress, sizeof(line->SocketAddress)) == -1) { /* Send SyncBack 0x22 */
	return -1;
    }
    return 1;
}


static int Connection(unsigned char *ipaddrused)
{
    int             i;
    unsigned char   buffer[T32_PCKLEN_MAX];
    LineStruct*     line = pLineParams;
    static const char magicPattern[] = "TRACE32";

    memset(buffer, 0, sizeof(buffer));

    buffer[0] = 3;		/* CONNECTREQUEST */
    buffer[1] = 0;

    line->TransmitSeq = 1;
    SETWORDVAR(buffer[2], line->TransmitSeq);
    SETWORDVAR(buffer[4], line->TransmitPort);
    SETWORDVAR(buffer[6], line->ReceivePort);

    strcpy((char *) (buffer + 8), magicPattern);

    if (sendto(line->CommSocket, (char *) buffer, line->PacketSize, 0, (struct sockaddr *) & line->SocketAddress, sizeof(line->SocketAddress)) == -1) { /* send ConnectReq 3 */
	return 0;
    }
    if ((i = ReceiveWithTimeout(&LongTime, buffer, line->PacketSize)) <= 0) {
	return 0;
    }
    if (strcmp((char *) (buffer + 8), magicPattern)) {
	return 0;
    }
    SETWORDVAR(line->ReceiveSeq, buffer[2]);

    if (buffer[0] == 0x53) {	/* POSITIVE CONNECTACKN from Debug Unit ? */
	ipaddrused[0] = 1;
	ipaddrused[1] = 1;
	ipaddrused[2] = 1;
	ipaddrused[3] = 11;
	return 2;
    }

    if (buffer[0] != 0x13) {	/* POSITIVE CONNECTACKN ? */
	if (buffer[0] == 0x23) {/* NEGATIVE ? */
	    ipaddrused[0] = buffer[4];
	    ipaddrused[1] = buffer[5];
	    ipaddrused[2] = buffer[6];
	    ipaddrused[3] = buffer[7];
	    return 2;
	}
	return 0;
    }
    line->PacketSize = i;

    return 1;
}


/* .NET helpers: additional functions to interface managed and unmanaged memory */
#ifdef	__cplusplus
extern "C" void *LINE_AllocNewChannel()
{
    return malloc(sizeof(LineStruct));
}
extern "C" void LINE_FreeAllocChannel(void * p)
{
    free(p);
}
#endif
