/****************************************************************
*                                                               *
*         Copyright by Lauterbach GmbH                          *
*         Alle Rechte vorbehalten - All rights reserved         *
*                                                               *
*****************************************************************

  Module:       hremote.c
  Function:     CAPI routines for communication with TRACE32.
                Uses low level protocol implemented in hlinknet.c.
                Link both files with your application.
                General Protocol T32OUT_BUFFER:
                 -5...-1   0 1    2       3     4...5    6...
                ---------------------------------------------------------
                | Header | CMD | SCMD | MSGID | Length | PayLoad
                ---------------------------------------------------------

  Author:       A. Nieser
  Date:         15.10.98

***************************************************************/

#include "t32.h"

#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4255 )
#endif

#include <stdio.h>
#include <string.h>
#ifdef MS_WINDOWS
# include <windows.h>
#else
# include <stdlib.h>
# include <sys/time.h>
#endif

#if defined(_MSC_VER)
# pragma warning( pop )
#endif


int T32_Errno;

/* Remote Api commands used on Host */
#define RAPI_CMD_NOP              0x70 /* NOP */
#define RAPI_CMD_ATTACH           0x71 /* Attach to Device */
#define RAPI_CMD_EXECUTE_PRACTICE 0x72 /* Execute generic Practice command */
#define RAPI_CMD_PING             0x73 /* Ping */
#define RAPI_CMD_DEVICE_SPECIFIC  0x74 /* Device-Specific command */
#define RAPI_CMD_CMDWINDOW        0x75 /* T32_CmdWin: Generic PRACTICE command with remote window */
#define RAPI_CMD_GETMSG           0x76 /* T32_GetMessage */
#define RAPI_CMD_EDITNOTIFY       0x78 /* T32_EditNotifyEnable */
#define RAPI_CMD_TERMINATE        0x79 /* T32_Terminate */

#define MAXRETRY	5

#define LINE_SBLOCK		4096       /* small block mode */

#define T32_MSG_LHANDLE	0x10
#define T32_MSG_LRETRY	0x08


unsigned char LINE_OutBuffer[LINE_MSIZE+256];
unsigned char LINE_InBuffer[LINE_MSIZE+256];

#define T32_OUTBUFFER (LINE_OutBuffer+13+4)
#define T32_INBUFFER (LINE_InBuffer+13)


/* forward declaration, see hlinknet.c for definition */
typedef struct LineStruct_s LineStruct;

extern int LINE_LineConfig(char * in);
extern int LINE_LineInit(char * message);
extern void LINE_LineExit(void);
extern int LINE_LineDriverGetSocket(void);
extern int LINE_LineTransmit(unsigned char * in, int size);
extern int LINE_LineReceive(unsigned char * out);
extern int LINE_ReceiveNotifyMessage(unsigned char* package);
extern int LINE_LineSync(void);
extern int LINE_GetLineParamsSize (void);
extern void LINE_SetDefaultLineParams (LineStruct* params);
extern LineStruct* LINE_GetLine0Params (void);
extern void LINE_SetLine (LineStruct* params);
extern void LINE_SetReceiveToggleBit(int value);
extern int LINE_GetReceiveToggleBit(void);
extern int LINE_GetNextMessageId(void);
extern int LINE_GetMessageId(void);

/* prototypes for local helper functions */
int LINE_Transmit(int len);
int LINE_Receive(void);
int LINE_Sync(void);

static const int MaxPacketSize = 2048;

static T32_NotificationCallback_t notificationCallback[T32_MAX_EVENTS];


/**************************************************************************

  T32_GetLineSize - Get sizeof line structure for Multi-Line usage

  Parameter:    -

  Return:       sizeof line structure

  Usage:        void* line = malloc (T32_GetLineSize());

***************************************************************************/

int T32_GetChannelSize(void)
{
    return LINE_GetLineParamsSize();
}

/**************************************************************************

  T32_GetLineDefaults - Get default parameters for Multi-Line usage

  Parameter:    out  pointer to line receiving the defaults

  Return:       -

  Usage:        T32_GetLineDefaults (line);

***************************************************************************/

void T32_GetChannelDefaults (void* line)
{
    LINE_SetDefaultLineParams ((LineStruct *)line);
    return;
}

/**************************************************************************

  T32_SetLine - Set active line for Multi-Line usage

  Parameter:    in  pointer to now active line

  Return:       -

  Usage:        T32_SetChannel (line);

***************************************************************************/

void T32_SetChannel (void* line)
{
    LINE_SetLine ((LineStruct *)line);
    return;
}

/**************************************************************************

  T32_GetLine0 - Get default line used by Single-Line usage

  Parameter:    -

  Return:       pointer to default line

  Usage:        void* line = T32_GetChannel0;

***************************************************************************/

void* T32_GetChannel0 (void)
{
    return (void*) LINE_GetLine0Params ();
}

/**************************************************************************

  T32_Nop - Sends one NIL message to the system

  Parameter:  -

  Return:     int  0 or Number of Error

***************************************************************************/
int T32_Nop(void)
{
    T32_OUTBUFFER[0] = 0x02;
    T32_OUTBUFFER[1] = RAPI_CMD_NOP;
    T32_OUTBUFFER[2] = 0x00;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  T32_NopFail - Sends one NIL message to the system

  Parameter:  -

  Return:     int  -1 or Number of Error

***************************************************************************/
int T32_NopFail(void)
{
    T32_OUTBUFFER[0] = 0x02;
    T32_OUTBUFFER[1] = RAPI_CMD_NOP;
    T32_OUTBUFFER[2] = 0x00;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    return -1;
}


/**************************************************************************

  T32_Ping - Sends one PING message to the system

  Parameter:  -

  Return:     int  0 or Number of Error

***************************************************************************/
int T32_Ping(void)
{
    T32_OUTBUFFER[0] = 0x02;
    T32_OUTBUFFER[1] = RAPI_CMD_PING;
    T32_OUTBUFFER[2] = 0x00;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  T32_Stop - Stop the actual running PRACTICE program

  Parameter:  -

  Return:     int  0 or Number of Error

***************************************************************************/
int T32_Stop(void)
{
    T32_OUTBUFFER[0] = 0x00;
    T32_OUTBUFFER[1] = 0x50;
    T32_OUTBUFFER[2] = 0x02;
    T32_OUTBUFFER[3] = 0x72;
    T32_OUTBUFFER[4] = 0x00;
    T32_OUTBUFFER[5] = LINE_GetNextMessageId();

    if (LINE_Transmit(6) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  T32_Terminate - Terminate executable

  Parameter:  -

  Return:     int  0 or Number of Error

***************************************************************************/
int T32_Terminate(int retval)
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_TERMINATE;
    T32_OUTBUFFER[2] = retval;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}



/**************************************************************************

  T32_Attach - Attaches control to a device

  Parameter:  in int dev Number of device (T32_DEV_OS, T32_DEV_ICE, T32_DEV_ICD)

  Return:     int  0 or Number of Error

***************************************************************************/
int T32_Attach(int dev)
{
    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_ATTACH;
    T32_OUTBUFFER[2] = dev;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  T32_GetState - Return information about the state of TRACE32

  Parameter:  out int * pstate Pointer to variable to return the state
                        0 System down
                        1 System halted
                        2 Emulation stopped
                        3 Emulation running

  Return:     int  0 or Number of Error


***************************************************************************/
int T32_GetState(int * pstate)
{
    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x10;	/* T32_GetState: Read Status Information */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    *pstate = T32_INBUFFER[4];

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  T32_GetCpuInfo - Get information about the used CPU

  Parameter:  out char ** string Pointer to a string decribing the CPU
              out word * fpu     Pointer to catch the FPU type ( not used, always 0 )
              out word * endian  Pointer to catch the Endian mode
                         1  BigEndian
                         0  LittleEndian
              out word * type    Pointer to get internal information
                                 (for internal use only)

  Return:     int  0 or Number of Error

***************************************************************************/
int T32_GetCpuInfo( char **string, word * fpu, word * endian, word * type )	/* Get CPU information report */
{
    int             i;
    static char     cpu_string[16];

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x13; /* T32_GetCpuInfo: Get CPU information */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    for (i = 0; i < 15; i++)
	cpu_string[i] = (T32_INBUFFER + 4)[i];

    cpu_string[15] = 0;
    *string = cpu_string;
    *fpu = *endian = *type = 0;
    SETWORDVAR(*fpu, T32_INBUFFER[22]);
    SETWORDVAR(*endian, T32_INBUFFER[24]);
    SETWORDVAR(*type, T32_INBUFFER[20]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_GetRam (dword *startaddr, dword *endaddr, word *access)	/* Get Data or Program RAM addresses */
{
    dword           t32access;

    T32_OUTBUFFER[0] = 8;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x16; /* T32_GetRam: Get Memory Mapping */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    SETLONGVAR(T32_OUTBUFFER[4], *startaddr);
    SETWORDVAR(T32_OUTBUFFER[8], *access);

    if (LINE_Transmit(10) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    SETLONGVAR(*startaddr, T32_INBUFFER[4]);
    SETLONGVAR(*endaddr, T32_INBUFFER[8]);
    SETLONGVAR(t32access, T32_INBUFFER[12]);

    if (!t32access)
	*access = 0;

    return T32_Errno = T32_INBUFFER[2];
}


int T32_ResetCPU(void)					/* Reset CPU Registers */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x11; /* T32_ResetCpu: Reset CPU */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    return T32_Errno = T32_INBUFFER[2];
}


/*
 * ADDRESS   memory address
 * ACCESS    memory access flags
 * BUFFER    data to be written
 * SIZE      size of data
 */

int T32_WriteMemory(dword address, int access, byte * buffer, int size)	/* Write Memory */
{
    int             result;

    if (size > MaxPacketSize) {
	result = T32_WriteMemoryPipe(address, access, buffer, size);
	if (result)
	    return result;
	return T32_WriteMemoryPipe(address, access, buffer, 0);
    }

    /* Protocol Header */
    T32_OUTBUFFER[0] = 10;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x31; /* Comemu remote command Write memory see comemu12.c COMEMU_Remote*/
    T32_OUTBUFFER[3] = LINE_GetNextMessageId(); /* Message ID */

	/* Command specific part */
    SETLONGVAR(T32_OUTBUFFER[4], address);
    T32_OUTBUFFER[8] = access;
    T32_OUTBUFFER[9] = 0;
    T32_OUTBUFFER[10] = size;
    T32_OUTBUFFER[11] = size >> 8;

	/* payload */
    memcpy(T32_OUTBUFFER + 12, buffer, size);

    if (LINE_Transmit(12 + ((size + 1) & (~1))) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    return T32_Errno = T32_INBUFFER[2];
}


/*
 * ADDRESS   memory address
 * ACCESS    memory access flags
 * BUFFER    data to be written
 * SIZE      size of data
 */

int T32_WriteMemoryPipe(dword address, int access, byte * buffer, int size)	/* Write Memory, Pipelined */
{
    int             len;

    if (size == 0) {
	T32_OUTBUFFER[0] = 2;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0x32; /* T32_WriteMemoryPipe */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	if (LINE_Transmit(4) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	return T32_Errno = T32_INBUFFER[2];
    }
    while (size > 0) {
	len = size;
	if (len > MaxPacketSize)
	    len = MaxPacketSize;

	T32_OUTBUFFER[0] = 10;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0x32; /* T32_WriteMemoryPipe */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	SETLONGVAR(T32_OUTBUFFER[4], address);
	T32_OUTBUFFER[8] = access;
	T32_OUTBUFFER[9] = 0;
	T32_OUTBUFFER[10] = len;
	T32_OUTBUFFER[11] = len >> 8;

	memcpy(T32_OUTBUFFER + 12, buffer, len);

	if (LINE_Transmit(12 + ((len + 1) & (~1))) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	size -= len;
	address += len;
	buffer += len;
    }
    return 0;
}


int T32_WriteMemoryEx(dword address, int segment, int access, int attr, byte * buffer, int size)	/* Write Memory, extended version */
{
    int             len, hlen;

    while (size > 0) {
	len = size;
	if (len > MaxPacketSize)
	    len = MaxPacketSize;

	hlen = 0;
	T32_OUTBUFFER[hlen] = 0;
	T32_OUTBUFFER[hlen + 1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[hlen + 2] = 0x33; /* MCD API transaction list, T32_ReadMemoryEx, T32_WriteMemoryEx */
	T32_OUTBUFFER[hlen + 3] = LINE_GetNextMessageId();

	T32_OUTBUFFER[hlen + 4] = 0x02;
	T32_OUTBUFFER[hlen + 5] = (segment == -1) ? 0x00 : 0x40;
	hlen += 6;
	SETLONGVAR(T32_OUTBUFFER[hlen], address);
	hlen += 4;
	if (segment != -1) {
	    SETLONGVAR(T32_OUTBUFFER[hlen], segment);
	    hlen += 4;
	}
	SETLONGVAR(T32_OUTBUFFER[hlen], access);
	hlen += 2;
	SETLONGVAR(T32_OUTBUFFER[hlen], attr);
	hlen += 4;
	SETLONGVAR(T32_OUTBUFFER[hlen], len);
	hlen += 2;

	memcpy(T32_OUTBUFFER + hlen, buffer, len);
	hlen += ((len + 1) & (~1));

	T32_OUTBUFFER[hlen] = 0;
	T32_OUTBUFFER[hlen + 1] = 0;
	hlen += 2;

	if (LINE_Transmit(hlen) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	if ((T32_Errno = T32_INBUFFER[4]) != 0)
	    return T32_Errno;

	size -= len;
	if (!(attr & T32_MEMORY_ATTR_NOINCREMENT))
	    address += len;
	buffer += len;
    }
    return 0;
}



/*
 * ADDRESS   memory address in target memory
 * ACCESS    memory access specifier
 * BUFFER    output buffer
 * SIZE      size of data in bytes
 */

int T32_ReadMemory(dword address, int access, byte * buffer, int size)	/* Read Memory */
{
    int             len;

    while (size > 0) {
	len = size;
	if (len > MaxPacketSize)
	    len = MaxPacketSize;

	T32_OUTBUFFER[0] = 10;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0x30; /* T32_ReadMemory */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	SETLONGVAR(T32_OUTBUFFER[4], address);
	T32_OUTBUFFER[8] = access;
	T32_OUTBUFFER[9] = 0;
	T32_OUTBUFFER[10] = len;
	T32_OUTBUFFER[11] = len >> 8;

	if (LINE_Transmit(12) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	memcpy(buffer, T32_INBUFFER + 4, len);

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	size -= len;
	address += len;
	buffer += len;
    }
    return 0;
}


int T32_ReadMemoryEx(dword address, int segment, int access, int attr, byte * buffer, int size)	/* Read Memory, extended version */
{
    int             len, hlen;

    while (size > 0) {
	len = size;
	if (len > MaxPacketSize)
	    len = MaxPacketSize;

	hlen = 0;
	T32_OUTBUFFER[hlen] = 0;
	T32_OUTBUFFER[hlen + 1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[hlen + 2] = 0x33; /* MCD API transaction list, T32_ReadMemoryEx, T32_WriteMemoryEx */
	T32_OUTBUFFER[hlen + 3] = LINE_GetNextMessageId();

	T32_OUTBUFFER[hlen + 4] = 0x01;
	T32_OUTBUFFER[hlen + 5] = (segment == -1) ? 0x00 : 0x40;
	hlen += 6;
	SETLONGVAR(T32_OUTBUFFER[hlen], address);
	hlen += 4;
	if (segment != -1) {
	    SETLONGVAR(T32_OUTBUFFER[hlen], segment);
	    hlen += 4;
	}
	SETLONGVAR(T32_OUTBUFFER[hlen], access);
	hlen += 2;
	SETLONGVAR(T32_OUTBUFFER[hlen], attr);
	hlen += 4;
	SETLONGVAR(T32_OUTBUFFER[hlen], len);
	hlen += 2;
	T32_OUTBUFFER[hlen] = 0;
	T32_OUTBUFFER[hlen + 1] = 0;
	hlen += 2;

	if (LINE_Transmit(hlen) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	if ((T32_Errno = T32_INBUFFER[4]) != 0)
	    return T32_Errno;

	memcpy(buffer, T32_INBUFFER + 8, len);

	size -= len;
	if (!(attr & T32_MEMORY_ATTR_NOINCREMENT))
	    address += len;
	buffer += len;
    }
    return 0;
}


/*
 * MASK1     defines the registers to be set
 * MASK2
 * BUFFER    array of all registers
 */

int T32_WriteRegister(dword mask1, dword mask2, dword *buffer)	/* Write Registers */
{
    int             index, i;
    dword           tmp;

    index = 12;
    tmp = mask1;
    SETLONGVAR(T32_OUTBUFFER[4], tmp);
    tmp = mask2;
    SETLONGVAR(T32_OUTBUFFER[8], tmp);

    for (i = 0; i < 32; i++) {
	if (mask1 & 1l) {
	    tmp = *buffer;
	    SETLONGVAR(T32_OUTBUFFER[index], tmp);
	    index += 4;
	}
	buffer++;
	mask1 >>= 1;
    }

    for (; i < 64; i++) {
	if (mask2 & 1l) {
	    tmp = *buffer;
	    SETLONGVAR(T32_OUTBUFFER[index], tmp);
	    index += 4;
	}
	buffer++;
	mask2 >>= 1;
    }

    T32_OUTBUFFER[0] = index;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x21; /* T32_WriteRegister: Write Registers */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(index) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    return T32_Errno = T32_INBUFFER[2];
}


/*
 * MASK1     defines the registers to be read
 * MASK2
 * BUFFER    array of all registers
 */

int T32_ReadRegister(dword mask1, dword mask2, dword *buffer)	/* Read Registers */
{
    int             index, i;
    dword           tmp;

    tmp = mask1;
    SETLONGVAR(T32_OUTBUFFER[4], tmp);
    tmp = mask2;
    SETLONGVAR(T32_OUTBUFFER[8], tmp);

    T32_OUTBUFFER[0] = 12;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x20; /* T32_ReadRegister: Read Registers */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(12) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    index = 4;
    for (i = 0; i < 32; i++) {
	if (mask1 & 1l) {
	    SETLONGVAR(tmp, T32_INBUFFER[index]);
	    index += 4;
	    *buffer = tmp;
	} else
	    *buffer = 0x0;
	buffer++;
	mask1 >>= 1;
    }

    for (; i < 64; i++) {
	if (mask2 & 1l) {
	    SETLONGVAR(tmp, T32_INBUFFER[index]);
	    index += 4;
	    *buffer = tmp;
	} else
	    *buffer = 0x0;
	buffer++;
	mask2 >>= 1;
    }

    return T32_Errno = T32_INBUFFER[2];
}


int T32_ReadRegisterByName (char *regname, dword *value, dword *hvalue)
{
    int len;

    len = (unsigned int) strlen(regname);

    if (len >= 256)
	return T32_Errno = T32_COM_PARA_FAIL;

    T32_OUTBUFFER[0] = len + 3;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x23; /* T32_ReadRegisterByName */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    strcpy((char *) (T32_OUTBUFFER + 4), regname);

    if (LINE_Transmit((len + 5 + 1) & (~1)) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    SETLONGVAR(*value,  T32_INBUFFER[4]);
    SETLONGVAR(*hvalue, T32_INBUFFER[8]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_ReadPP ( dword *pp )		/* Returns Program Pointer */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x22; /* T32_ReadPP: Read Program Pointer */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    SETLONGVAR(*pp, T32_INBUFFER[4]);

    return T32_Errno = T32_INBUFFER[2];
}


/*
 * ADDRESS  memory address
 * FLAGS    memory access flags
 * BREAKP   breakpoint type, bit 8 set to clear breakpoints
 * SIZE     size of range
 */

int T32_WriteBreakpoint(dword address, int flags, int breakp, int size)	/* Set/Clear Breakpoints */
{
    int             len;

    while (size > 0) {
	len = size;
	if (len > MaxPacketSize)
	    len = MaxPacketSize;

	T32_OUTBUFFER[0] = 10;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = (breakp & 0x100) ? 0x42 : 0x41; /* T32_WriteBreakpoint: Clear/Set Breakpoints */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	SETLONGVAR(T32_OUTBUFFER[4], address);
	T32_OUTBUFFER[8] = flags;
	T32_OUTBUFFER[9] = breakp;
	T32_OUTBUFFER[10] = len;
	T32_OUTBUFFER[11] = len >> 8;

	if (LINE_Transmit(12) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	size -= len;
	address += len;
    }
    return 0;
}


/*
 * ADDRESS    memory address
 * FLAGS      memory access flags
 * BUFFER     buffer to receive breakpoint information
 * SIZE       size of range
 */

int T32_ReadBreakpoint(dword address, int flags, word *buffer, int size)	/* Get Breakpoint/Flag Information */
{
    int             i, len;
    int             outindex = 0;

    while (size > 0) {
	len = size;
	if (len > MaxPacketSize / 2)
	    len = MaxPacketSize / 2;

	T32_OUTBUFFER[0] = 10;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0x40; /* T32_ReadBreakpoint:  Get Breakpoints */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	SETLONGVAR(T32_OUTBUFFER[4], address);
	T32_OUTBUFFER[8] = flags;
	T32_OUTBUFFER[9] = 0;
	T32_OUTBUFFER[10] = len;
	T32_OUTBUFFER[11] = len >> 8;

	if (LINE_Transmit(12) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	for (i = 0; i < len; i++) {
	    buffer[outindex++] = T32_INBUFFER[4 + 2 * i] | (T32_INBUFFER[4 + 2 * i + 1] << 8);
	}

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	size -= len;
	address += len;
    }
    return 0;
}


int T32_GetBreakpointList (int* numbps, T32_Breakpoint* bps, int max)
{
    word wnum;
    int  i, loc;
    
    T32_OUTBUFFER[0] = 5;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x64; /* T32_GetBreakpointList */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    T32_OUTBUFFER[4] = 0x77; /* sample payload */
    T32_OUTBUFFER[5] = 0; /* padding for even byte count */

    if (LINE_Transmit(6) == -1)
        return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
        return T32_Errno = T32_COM_RECEIVE_FAIL;

    SETWORDVAR(wnum, T32_INBUFFER[4]);
    *numbps = wnum;

    for (i=0, loc=6; (i < wnum) && (i < max); i++, loc+=13)
    {
        SETLONGVAR(bps[i].address, T32_INBUFFER[loc]);
        bps[i].enabled = T32_INBUFFER[loc+4] & 0x1;
        SETLONGVAR(bps[i].type, T32_INBUFFER[loc+5]);
        SETLONGVAR(bps[i].auxtype, T32_INBUFFER[loc+9]);
    }

    if ((T32_Errno = T32_INBUFFER[2]) != 0)
        return T32_Errno;

    return 0;
}


int T32_Step(void)					/* Single Step */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x50; /* T32_Step: Single-Step */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;


    return T32_Errno = T32_INBUFFER[2];
}

/*
 * MODE 0=ASM, 1=HLL, 2=MIX
 */

int T32_StepMode(int mode)				/* Single Step in Mode */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x54; /* T32_StepMode: Step with Options */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    T32_OUTBUFFER[4] = mode;
    T32_OUTBUFFER[5] = 0;

    if (LINE_Transmit(6) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}

/*
 * MODE 0=ASM, 1=HLL, 2=MIX
 */

int T32_SetMode(int mode)				/* Set Mode */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x53; /* T32_Mode: Mode-Selection */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    T32_OUTBUFFER[4] = mode;
    T32_OUTBUFFER[5] = 0;

    if (LINE_Transmit(6) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


int T32_Go(void)					/* Start Realtime */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x51; /* T32_Go: Go */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


int T32_Break(void)					/* Stop Realtime */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x52; /* T32_Break: Break */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


/*
 * NAME commandline
 */

int T32_Cmd(const char *name)			/* Executes a command line */
{
    word            wlen;
    int             len;

    len = (int) strlen(name);

    if (len > MaxPacketSize)
	return T32_Errno = T32_COM_PARA_FAIL;

    if (len + 3 >= 0xff) {
	wlen = (word) (len + 5);
	T32_OUTBUFFER[0] = 0;
	T32_OUTBUFFER[1] = RAPI_CMD_EXECUTE_PRACTICE;
	T32_OUTBUFFER[2] = 0x02;
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();
	SETWORDVAR(T32_OUTBUFFER[4], wlen);
	strcpy((char *) (T32_OUTBUFFER + 6), name);
	if (LINE_Transmit((len + 7 + 1) & (~1)) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;
    } else {
	T32_OUTBUFFER[0] = len + 3;
	T32_OUTBUFFER[1] = RAPI_CMD_EXECUTE_PRACTICE;
	T32_OUTBUFFER[2] = 0x02;
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();
	strcpy((char *) (T32_OUTBUFFER + 4), name);

	if (LINE_Transmit((len + 5 + 1) & (~1)) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;
    }

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  T32_GetPracticeState - Return information about the state of PRACTICE

  Parameter:  out int * pstate Pointer to variable to return the state
                        0 not running
                        1 running

  Return:     int  0 or Number of Error


***************************************************************************/

int T32_GetPracticeState(int * pstate)		/* Returns the run-state of PRACTICE */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_EXECUTE_PRACTICE;
    T32_OUTBUFFER[2] = 0x03;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    *pstate = T32_INBUFFER[4];

    return T32_Errno = T32_INBUFFER[2];
}


/****** Executing a PRACTICE command with given windows handler *****/

int T32_CmdWin (dword handle, char* command)
{
    word            wlen;
    int             len;

    len = (int) strlen(command);

    if (len >= MaxPacketSize)
	return T32_Errno = T32_COM_PARA_FAIL;

    if (len + 3 >= 0xff) {
	wlen = (word) (len + 9);

	T32_OUTBUFFER[0] = 0;
	T32_OUTBUFFER[1] = RAPI_CMD_CMDWINDOW;
	T32_OUTBUFFER[2] = 0x02;
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();
	SETWORDVAR(T32_OUTBUFFER[4], wlen);

	SETLONGVAR(T32_OUTBUFFER[6], handle);
	strcpy((char *) (T32_OUTBUFFER + 10), command);

	if (LINE_Transmit((len + 11 + 1) & (~1)) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;
    } else {
	T32_OUTBUFFER[0] = len + 7;
	T32_OUTBUFFER[1] = RAPI_CMD_CMDWINDOW;
	T32_OUTBUFFER[2] = 0x02;
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	SETLONGVAR(T32_OUTBUFFER[4], handle);
	strcpy((char *) (T32_OUTBUFFER + 8), command);

	if (LINE_Transmit((len + 9 + 1) & (~1)) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;
    }

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    return T32_Errno = T32_INBUFFER[2];
}


int T32_EvalGet (dword *peval)		/* Returns Evaluation Result */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x14; /* T32_EvalGet */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    SETLONGVAR(*peval, T32_INBUFFER[4]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_GetMessage (char *message, word *mode)
{
    dword           dmode;

    T32_OUTBUFFER[0] = 0x02;
    T32_OUTBUFFER[1] = RAPI_CMD_GETMSG;
    T32_OUTBUFFER[2] = 0x00;
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4))
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    strcpy(message, (char *) T32_INBUFFER + 8);

    SETLONGVAR(dmode, T32_INBUFFER[4]);
    *mode = (word) dmode;

    return T32_Errno = T32_INBUFFER[2];
}


int T32_GetTriggerMessage (char *message)
{
    T32_OUTBUFFER[0] = 0x02;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x63; /* T32_GetTriggerMessage */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    if (LINE_Transmit(4))
	return T32_Errno = T32_COM_TRANSMIT_FAIL;
    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;
    strcpy(message, (char *) T32_INBUFFER + 4);
    return T32_Errno = T32_INBUFFER[2];
}


int T32_GetSymbol (char *symbol, dword *address, dword *size, dword *access)
{
    int             len;

    len = (unsigned int)strlen(symbol);

    if (len >= 256)
	return T32_Errno = T32_COM_PARA_FAIL;

    T32_OUTBUFFER[0] = len + 3;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x62; /* T32_GetSymbol: Get Symbol Address, Size & Class */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    strcpy((char *) (T32_OUTBUFFER + 4), symbol);

    if (LINE_Transmit((len + 5 + 1) & (~1)) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    SETLONGVAR(*address, T32_INBUFFER[4]);
    SETLONGVAR(*size, T32_INBUFFER[8]);
    SETLONGVAR(*access, T32_INBUFFER[12]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_GetSource (dword address, char *filename, dword *line)
{

    T32_OUTBUFFER[0] = 6;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x60; /* T32_GetSource: Get Filename & Line */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    SETLONGVAR(T32_OUTBUFFER[4], address);

    if (LINE_Transmit(8) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    strcpy(filename, (char *) T32_INBUFFER + 8);

    *line = 0;
    SETLONGVAR(*line, T32_INBUFFER[4]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_GetSelectedSource (char *filename, dword *line)
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x61; /* T32_GetSelectedSource: Get Filename & Line of Selection */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    strcpy(filename, (char *) T32_INBUFFER + 8);

    *line = 0;
    SETLONGVAR(*line, T32_INBUFFER[4]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_ReadVariableValue (char *symbol, dword *value, dword *hvalue)
{
    int len;

    len = (unsigned int) strlen(symbol);

    if (len >= 256)
	return T32_Errno = T32_COM_PARA_FAIL;

    T32_OUTBUFFER[0] = len + 3;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x65; /* T32_ReadVariable */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    strcpy((char *) (T32_OUTBUFFER + 4), symbol);

    if (LINE_Transmit((len + 5 + 1) & (~1)) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    SETLONGVAR(*value,  T32_INBUFFER[4]);
    SETLONGVAR(*hvalue, T32_INBUFFER[8]);

    return T32_Errno = T32_INBUFFER[2];
}


int T32_ReadVariableString (char *symbol, char *string, int maxlen)
{
    int len;

    len = (unsigned int) strlen(symbol);

    if (len > 250)
	return T32_Errno = T32_COM_PARA_FAIL;

    T32_OUTBUFFER[0] = len + 3;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x66; /* T32_ReadVariableString */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    strcpy((char *) (T32_OUTBUFFER + 4), symbol);

    if (LINE_Transmit((len + 5 + 1) & (~1)) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;
    
    len = strlen ((char *) T32_INBUFFER + 4);
    if (len >= maxlen)
        len = maxlen - 1;
    strncpy(string, (char *) T32_INBUFFER + 4, len);
    string[len] = 0;

    return T32_Errno = T32_INBUFFER[2];
}


/**************************************************************************

  Trace interface

***************************************************************************/

/*
 * STATE Analyzer state
 * SIZE  Size of Tracebuffer
 * MIN   Min. Record Number
 * MAX   Max. Record Number
 */

int T32_AnaStatusGet (byte *state, long *size, long *min, long *max )	/* Get Analyzer Status */
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x80; /* T32_AnaStatusGet */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    if (LINE_Transmit(4) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    *state = T32_INBUFFER[4];
    SETLONGVAR(*size, T32_INBUFFER[8]);
    SETLONGVAR(*min, T32_INBUFFER[12]);
    SETLONGVAR(*max, T32_INBUFFER[16]);

    if ((T32_Errno = T32_INBUFFER[2]) != 0)
	return T32_Errno;

    return 0;
}


int T32_AnaRecordGet(long record, byte * buffer, int len)
{
    word            wlen;
    dword           drecord;

    T32_OUTBUFFER[0] = 8;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x81; /* T32_AnaRecordGet */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    drecord = (dword) record;
    wlen = (word) len;
    SETLONGVAR(T32_OUTBUFFER[4], drecord);
    SETWORDVAR(T32_OUTBUFFER[8], wlen);

    if (LINE_Transmit(10) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    if (!len)
	len = 128;

    memcpy(buffer, T32_INBUFFER + 4, len);

    if ((T32_Errno = T32_INBUFFER[2]) != 0)
	return T32_Errno;

    return 0;
}


int T32_GetTraceState(int tracetype, int * state, long * size, long * min, long * max)
{

    T32_OUTBUFFER[0] = 2;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0x82; /* T32_GetTraceState */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    T32_OUTBUFFER[4] = tracetype;
    T32_OUTBUFFER[5] = 0;

    if (LINE_Transmit(6) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if (LINE_Receive() == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    *state = T32_INBUFFER[4];
    SETLONGVAR(*size, T32_INBUFFER[8]);
    SETLONGVAR(*min, T32_INBUFFER[12]);
    SETLONGVAR(*max, T32_INBUFFER[16]);

    if ((T32_Errno = T32_INBUFFER[2]) != 0)
	return T32_Errno;

    return 0;
}


int T32_ReadTrace(int tracetype, long record, int nrecords, unsigned long mask, byte * buffer)
{
    int             nbytes;
    word            nrecs;
    dword           drecord, dmask;
    unsigned long   mask2;

    if (nrecords < 0) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    dmask = (dword) mask;
    nbytes = 0;
    for (mask2 = mask; mask2; mask2 >>= 1) {
	if (mask2 & 1)
	    nbytes += 4;
    }

    while (nrecords > 0) {
	if (nrecords * nbytes > LINE_SBLOCK)
	    nrecs = LINE_SBLOCK / nbytes;
	else
	    nrecs = (word) nrecords;
	drecord = (dword) record;

	T32_OUTBUFFER[0] = 8;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0x83; /* T32_ReadTrace */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();
	T32_OUTBUFFER[4] = tracetype;
	T32_OUTBUFFER[5] = 0;

	SETLONGVAR(T32_OUTBUFFER[6], drecord);
	SETLONGVAR(T32_OUTBUFFER[10], dmask);
	SETWORDVAR(T32_OUTBUFFER[14], nrecs);

	if (LINE_Transmit(16) == -1)
	    return T32_Errno = T32_COM_TRANSMIT_FAIL;

	if (LINE_Receive() == -1)
	    return T32_Errno = T32_COM_RECEIVE_FAIL;

	memcpy(buffer, T32_INBUFFER + 4, nrecs * nbytes);

	if ((T32_Errno = T32_INBUFFER[2]) != 0)
	    return T32_Errno;

	record += nrecs;
	nrecords -= nrecs;
	buffer += nrecs * nbytes;
    }

    return 0;
}


/**************************************************************************

  T32_Fdx* - Fast Data Exchange

***************************************************************************/

int T32_Fdx_Open(char * name, char * mode)
{
    unsigned int len, lenm;
    dword           id;

    len  = (unsigned int)strlen(name);
    lenm = (unsigned int)strlen(mode);
    T32_OUTBUFFER[0] = 3 + len+lenm+1;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0xa1; /* T32_FdxOpen */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    strcpy((char *) (T32_OUTBUFFER + 4), name);
    strcpy((char *) (T32_OUTBUFFER + 4+len+1), mode);

    if (LINE_Transmit((len + lenm + 6 + 1) & (~1)) == -1) {
	T32_Errno = T32_COM_TRANSMIT_FAIL;
	return -1;
    }

    if (LINE_Receive() == -1) {
	T32_Errno = T32_COM_RECEIVE_FAIL;
	return -1;
    }

    if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	return -1;
    }

    SETLONGVAR(id, T32_INBUFFER[4]);

    return (int) id;
}


int T32_Fdx_Close(int channel)
{
    dword           id;

    id = (dword) channel;
    T32_OUTBUFFER[0] = 6;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0xa6; /* T32_FdxClose */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    SETLONGVAR(T32_OUTBUFFER[4], id);

    if (LINE_Transmit(8) == -1) {
	T32_Errno = T32_COM_TRANSMIT_FAIL;
	return -1;
    }
    if (LINE_Receive() == -1) {
	T32_Errno = T32_COM_RECEIVE_FAIL;
	return -1;
    }
    if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	return -1;
    }
    return 0;
}


int T32_Fdx_Resolve(char * name)
{
    unsigned int    len;
    dword           id;

    len = (unsigned int)strlen(name);
    T32_OUTBUFFER[0] = 3 + len;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0xa0; /* T32_FdxResolve */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    strcpy((char *) (T32_OUTBUFFER + 4), name);

    if (LINE_Transmit((len + 5 + 1) & (~1)) == -1) {
	T32_Errno = T32_COM_TRANSMIT_FAIL;
	return -1;
    }

    if (LINE_Receive() == -1) {
	T32_Errno = T32_COM_RECEIVE_FAIL;
	return -1;
    }

    if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	return -1;
    }

    SETLONGVAR(id, T32_INBUFFER[4]);

    return (int) id;
}


int T32_Fdx_ReceivePoll(int channel, void * data, int width, int maxsize)
{
    int             size, len;
    dword           id;

    id = (dword) channel;
    if (width <= 0 || width > 16) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    if (maxsize <= 0) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    maxsize *= width;
    if (maxsize > LINE_SBLOCK)
	maxsize = LINE_SBLOCK;

    T32_OUTBUFFER[0] = 6;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0xa2; /* T32_FdxReceivePoll */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    SETLONGVAR(T32_OUTBUFFER[4], id);

    T32_OUTBUFFER[8] = width;

#if defined(HIGH_LOW) || defined(HIGH_LOW_BYTE)
    T32_OUTBUFFER[8] |= 0x80;
#endif

    T32_OUTBUFFER[9] = 0;

    if (LINE_Transmit(10) == -1) {
	T32_Errno = T32_COM_TRANSMIT_FAIL;
	return -1;
    }
    if ((len = LINE_Receive()) == -1) {
	T32_Errno = T32_COM_RECEIVE_FAIL;
	return -1;
    }
    if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	return -1;
    }
    len -= 4;
    if (len < 0) {
	T32_Errno = T32_COM_SEQ_FAIL;
	return -1;
    }
    size = (len > maxsize) ? maxsize : len;

    if (size > 0)
	memcpy(data, T32_INBUFFER + 4, size);

    if (width <= 1)
	return size;
    return size / width;
}


int T32_Fdx_Receive(int channel, void * data, int width, int maxsize)
{
    int             size, len, paramWidth;
    dword           id;

    paramWidth=width;
    width&=0x3F;
    if (width <= 0 || width > 16) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    if (maxsize <= 0) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    maxsize *= width;
    if (maxsize > LINE_SBLOCK)
	maxsize = LINE_SBLOCK;

    do {
	id = (dword) channel;
	T32_OUTBUFFER[0] = 6;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0xa3; /* T32_FdxReceive */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();

	SETLONGVAR(T32_OUTBUFFER[4], id);

	T32_OUTBUFFER[8] = paramWidth;
#if defined(HIGH_LOW) || defined(HIGH_LOW_BYTE)
	T32_OUTBUFFER[8] |= 0x80;
#endif

	T32_OUTBUFFER[9] = 0;

	if (LINE_Transmit(10) == -1) {
	    T32_Errno = T32_COM_TRANSMIT_FAIL;
	    return -1;
	}
	if ((len = LINE_Receive()) == -1) {
	    T32_Errno = T32_COM_RECEIVE_FAIL;
	    return -1;
	}
	if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	    return -1;
	}
	len -= 4;
	if (len < 0) {
	    T32_Errno = T32_COM_SEQ_FAIL;
	    return -1;
	}
	size = (len > maxsize) ? maxsize : len;

    } while (size <= 0);

    memcpy(data, T32_INBUFFER + 4, size);

    if (width <= 1)
        return size;
    return size / width;
}


int T32_Fdx_SendPoll(int channel, void * data, int width, int size)
{
    int             bsize;
    dword           id;

    if (width <= 0 || width > 16) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    bsize = size * width;
    if (bsize > LINE_SBLOCK || bsize <= 0) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    id = (dword) channel;
    T32_OUTBUFFER[0] = 0;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = 0xa4; /* T32_FdxTransmitPoll */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();

    SETLONGVAR(T32_OUTBUFFER[4], id);

#if defined(HIGH_LOW) || defined(HIGH_LOW_BYTE)
    width |= 0x80;
#endif

    T32_OUTBUFFER[8] = width;
    T32_OUTBUFFER[9] = 0;
    T32_OUTBUFFER[10] = (10 + bsize) & 0xff;
    T32_OUTBUFFER[11] = ((10 + bsize) >> 8) & 0xff;
    memcpy(T32_OUTBUFFER + 12, data, bsize);

    if (LINE_Transmit((bsize + 12 + 1) & (~1)) == -1) {
	T32_Errno = T32_COM_TRANSMIT_FAIL;
	return -1;
    }
    if (LINE_Receive() == -1) {
	T32_Errno = T32_COM_RECEIVE_FAIL;
	return -1;
    }
    if (T32_INBUFFER[2] == 1)
	return 0;
    if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	return -1;
    }
    return size;
}


int T32_Fdx_Send(int channel, void * data, int width, int size)
{
    int             bsize;
    dword           id;

    if (width <= 0 || width > 16) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    bsize = size * width;
    if (bsize > LINE_SBLOCK || bsize <= 0) {
	T32_Errno = T32_COM_PARA_FAIL;
	return -1;
    }
    id = (dword) channel;

    do {
	T32_OUTBUFFER[0] = 0;
	T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
	T32_OUTBUFFER[2] = 0xa5; /* T32_FdxTransmit */
	T32_OUTBUFFER[3] = LINE_GetNextMessageId();
	SETLONGVAR(T32_OUTBUFFER[4], id);

#if defined(HIGH_LOW) || defined(HIGH_LOW_BYTE)
	width |= 0x80;
#endif

	T32_OUTBUFFER[8] = width;
	T32_OUTBUFFER[9] = 0;
	T32_OUTBUFFER[10] = (10 + bsize) & 0xff;
	T32_OUTBUFFER[11] = ((10 + bsize) >> 8) & 0xff;
	memcpy(T32_OUTBUFFER + 12, data, bsize);

	if (LINE_Transmit((bsize + 12 + 1) & (~1)) == -1) {
	    T32_Errno = T32_COM_TRANSMIT_FAIL;
	    return -1;
	}
	if (LINE_Receive() == -1) {
	    T32_Errno = T32_COM_RECEIVE_FAIL;
	    return -1;
	}
    } while (T32_INBUFFER[2] == 1);

    if ((T32_Errno = T32_INBUFFER[2]) != 0) {
	return -1;
    }
    return size;
}


T32_Fdx_Stream *T32_Fdx_OpenStream(char *name, char *mode)
{
    int channel;
    T32_Fdx_Stream *pstream;

    pstream = (T32_Fdx_Stream *) malloc(sizeof(T32_Fdx_Stream));
    if (!pstream)
	return (T32_Fdx_Stream *) 0;

    channel = T32_Fdx_Open(name, mode);
    if (channel == -1)
	return (T32_Fdx_Stream *) 0;

    memset(pstream, 0, sizeof(T32_Fdx_Stream));

    pstream->channel = channel;
    return pstream;
}

int T32_Fdx_CloseStream(T32_Fdx_Stream * pstream)
{
    T32_Fdx_Close(pstream->channel);
    free(pstream);
    return 0;
}

int T32_Fdx_ReceiveStreamNext(T32_Fdx_Stream * pstream, unsigned char *target, int width, int size)
{
    int len;

    len = T32_Fdx_Receive(pstream->channel, pstream->buffer, width | 0x40, sizeof(pstream->buffer));
    if (len <= 0)
	return len;
    pstream->ptr = pstream->buffer;
    pstream->ptrend = pstream->buffer + len;
    return T32_Fdx_ReceiveStream(pstream, target, width, size);
}

/**************************************************************************

  T32_TapAccess* - Low level JTAG access functions

***************************************************************************/

typedef struct tapAccessHeader
{
    struct tapAccessHeader * next;
    int cmd;
    int numberofbits;
	int numberofinbits;
    byte * pinbits;
    unsigned char outbits[1];
}
tapAccessHeader;


typedef struct tapAccessHandle
{
    struct tapAccessHeader * first;
    struct tapAccessHeader * current;
}
tapAccessHandle;


static int TAPAccess(int cmd, T32_TAPACCESS_HANDLE connection, int numberofbits, byte * poutbits, byte * pinbits, int numberofinbits)
{
    int             size, len;
    unsigned char           *ptr;
	int indatasize;

    ptr = T32_OUTBUFFER + 6;
	indatasize = (numberofinbits + 7)/8;

    if (numberofbits >= 0) {
	if (poutbits) {
	    cmd |= 2;
	}
	if (pinbits) {
	    cmd |= 1;
	}
	if (numberofbits < 0x0f) {
	    *ptr++ = (cmd << 4) | numberofbits;
	} else if (numberofbits < 0xff) {
	    *ptr++ = (cmd << 4) | 0x0f;
	    *ptr++ = numberofbits;
	} else {
	    *ptr++ = (cmd << 4) | 0x0f;
	    *ptr++ = 0xff;
	    ptr[0] = (numberofbits & 0xff);
	    ptr[1] = ((numberofbits >> 8) & 0xff);
	    ptr[2] = ((numberofbits >> 16) & 0xff);
	    ptr[3] = ((numberofbits >> 24) & 0xff);
	    ptr += 4;
	}

	size = (numberofbits + 7) / 8;
	if (size && (cmd & 2)) {
	    memcpy(ptr, poutbits, size);
	    ptr += size;
	}
    }

    if (connection != T32_TAPACCESS_HOLD)
        *ptr++ = 0;

    size = (int) (ptr - (T32_OUTBUFFER + 6));

    T32_OUTBUFFER[0] = 0;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = (connection == T32_TAPACCESS_HOLD) ? 0x93 : 0x92; /* T32_JtagDebugLock/T32_JtagDebug */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    T32_OUTBUFFER[4] = (4 + size) & 0xff;
    T32_OUTBUFFER[5] = ((4 + size) >> 8) & 0xff;

    if (LINE_Transmit(6 + ((size + 1) & (~1))) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if ((len = LINE_Receive()) == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    T32_Errno = T32_INBUFFER[2];

    if (pinbits && len >= indatasize + 4)
		memcpy(pinbits, T32_INBUFFER + 4, indatasize);

    return T32_Errno;
}


static int TAPAccessStore(int cmd, T32_TAPACCESS_HANDLE connection, int numberofbits, byte * poutbits, byte * pinbits, int numberofinbits)
{
    tapAccessHandle *taphandle;
    tapAccessHeader *tapptr;
    int             size;

    taphandle = (tapAccessHandle *) connection;

    size = (numberofbits + 7) / 8;
    tapptr = (tapAccessHeader *) malloc(sizeof(tapAccessHeader) + size);
    if (!tapptr) {
	T32_Errno = T32_MALLOC_FAIL;
	return T32_Errno;
    }

    if (poutbits) {
	memcpy(tapptr->outbits, poutbits, size);
	cmd |= 2;
    }
    if (pinbits) {
	cmd |= 1;
    }

    tapptr->next = NULL;
    tapptr->cmd = cmd;
    tapptr->numberofbits = numberofbits;
    tapptr->pinbits = pinbits;
	tapptr->numberofinbits = numberofinbits;

    if (taphandle->current)
	taphandle->current->next = tapptr;
    else
	taphandle->first = tapptr;
    taphandle->current = tapptr;

    T32_Errno = 0;
    return T32_Errno;
}


T32_TAPACCESS_HANDLE T32_TAPAccessAlloc(void)
{
    tapAccessHandle *taphandle;

    taphandle = (tapAccessHandle *) malloc(sizeof(tapAccessHandle));
    if (!taphandle) {
	T32_Errno = T32_MALLOC_FAIL;
	return NULL;
    }

    taphandle->first = NULL;
    taphandle->current = NULL;

    T32_Errno = 0;
    return taphandle;
}


static int TAPAccessSend(T32_TAPACCESS_HANDLE connection, int size, byte ** bitptr, int * bitsizesenddata, int *bitsizereceivedata, int npairs)
{
    int             i, len;
    unsigned char           *ptr;

    T32_OUTBUFFER[0] = 0;
    T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
    T32_OUTBUFFER[2] = (connection == T32_TAPACCESS_HOLD) ? 0x93 : 0x92; /* T32_JtagDebugLock/T32_JtagDebug */
    T32_OUTBUFFER[3] = LINE_GetNextMessageId();
    T32_OUTBUFFER[4] = (4 + size) & 0xff;
    T32_OUTBUFFER[5] = ((4 + size) >> 8) & 0xff;

    if (LINE_Transmit(6 + ((size + 1) & (~1))) == -1)
	return T32_Errno = T32_COM_TRANSMIT_FAIL;

    if ((len = LINE_Receive()) == -1)
	return T32_Errno = T32_COM_RECEIVE_FAIL;

    T32_Errno = T32_INBUFFER[2];

    ptr = T32_INBUFFER + 4;
    for (i = 0; i < npairs; i++) {
	if (bitptr[i]) {
	    memcpy(bitptr[i], ptr, (bitsizereceivedata[i] + 7) / 8);
		ptr += ((bitsizesenddata[i] + 7) / 8);
	}
    }

    return T32_Errno;
}


int T32_TAPAccessExecute(T32_TAPACCESS_HANDLE connection, T32_TAPACCESS_HANDLE connectionhold)
{
    unsigned char           *ptr;
    int             datasize, size, maxsize, n, cmd, numberofbits;
    tapAccessHandle *taphandle;
    tapAccessHeader *tapptr;
    unsigned char  *bitptr[2048];
    int             bitsizessend[2048];
	int             bitsizesreceive[2048];

    if (!(connection > T32_TAPACCESS_HOLD)) {
	T32_Errno = T32_COM_PARA_FAIL;
	return T32_Errno;
    }

    taphandle = (tapAccessHandle *) connection;

    ptr = T32_OUTBUFFER + 6;
    tapptr = taphandle->first;
    datasize = 0;
    n = 0;
    maxsize = EMU_CBMAXDATASIZE;

    while (tapptr) {

	cmd = tapptr->cmd;
	numberofbits = tapptr->numberofbits;
	size = (numberofbits + 7) / 8;

	if ( (datasize >= maxsize) || ( datasize + size > maxsize) )
	{
	    if (TAPAccessSend(T32_TAPACCESS_HOLD, (int) (ptr - (T32_OUTBUFFER + T32_TAPACCESSSEND_HEADERSIZE)), bitptr, bitsizessend, bitsizesreceive, n))
		return T32_Errno;
	    ptr = T32_OUTBUFFER + T32_TAPACCESSSEND_HEADERSIZE;
	    datasize = 0;
	    n = 0;
	}
	if (numberofbits < 0x0f) {
	    *ptr++ = (cmd << 4) | numberofbits;
	    datasize++;
	} else if (numberofbits < 0xff) {
	    *ptr++ = (cmd << 4) | 0x0f;
	    *ptr++ = numberofbits;
	    datasize += 2;
	} else {
	    *ptr++ = (cmd << 4) | 0x0f;
	    *ptr++ = 0xff;
	    ptr[0] = (numberofbits & 0xff);
	    ptr[1] = ((numberofbits >> 8) & 0xff);
	    ptr[2] = ((numberofbits >> 16) & 0xff);
	    ptr[3] = ((numberofbits >> 24) & 0xff);
	    ptr += 4;
	    datasize += T32_TAPACCESSSEND_HEADERSIZE;
	}

	if (size && (cmd & 2)) {
	    memcpy(ptr, tapptr->outbits, size);
	    ptr += size;
	}
	bitptr[n] = tapptr->pinbits;
	bitsizessend[n] = tapptr->numberofbits;
	bitsizesreceive[n] = tapptr->numberofinbits;
	n++;

	datasize += size;
	tapptr = tapptr->next;
    }

    if (connectionhold != T32_TAPACCESS_HOLD)
        *ptr++ = 0;

    return TAPAccessSend(connectionhold, (int) (ptr - (T32_OUTBUFFER + 6)), bitptr, bitsizessend, bitsizesreceive, n);
}


int T32_TAPAccessFree(T32_TAPACCESS_HANDLE connection)
{
    tapAccessHandle *taphandle;
    tapAccessHeader *tapptr;
    tapAccessHeader *tapptrnext;

    taphandle = (tapAccessHandle *) connection;

    tapptr = taphandle->first;
    while (tapptr) {
	tapptrnext = tapptr->next;
	free(tapptr);
	tapptr = tapptrnext;
    }

    free(taphandle);

    T32_Errno = 0;
    return 0;
}

int T32_TAPAccessSetInfo2(T32_TAPACCESS_HANDLE connection, int irpre, int irpost, int drpre, int drpost, int tristate, int tapstate, int tcklevel, int slave)
{
    unsigned char   outbits[20];

    outbits[0] = irpre & 0xff;
    outbits[1] = (irpre >> 8) & 0xff;
    outbits[2] = (irpre >> 16) & 0xff;
    outbits[3] = (irpre >> 24) & 0xff;
    outbits[4] = irpost & 0xff;
    outbits[5] = (irpost >> 8) & 0xff;
    outbits[6] = (irpost >> 16) & 0xff;
    outbits[7] = (irpost >> 24) & 0xff;
    outbits[8] = drpre & 0xff;
    outbits[9] = (drpre >> 8) & 0xff;
    outbits[10] = (drpre >> 16) & 0xff;
    outbits[11] = (drpre >> 24) & 0xff;
    outbits[12] = drpost & 0xff;
    outbits[13] = (drpost >> 8) & 0xff;
    outbits[14] = (drpost >> 16) & 0xff;
    outbits[15] = (drpost >> 24) & 0xff;
    outbits[16] = tristate;
    outbits[17] = tapstate;
    outbits[18] = tcklevel;
    outbits[19] = slave;

    return TAPAccess(0, connection, 160, outbits, NULL, 0);
}

int T32_TAPAccessSetInfo(int irpre, int irpost, int drpre, int drpost, int tristate, int tapstate, int tcklevel, int slave)
{
    return T32_TAPAccessSetInfo2(T32_TAPACCESS_RELEASE, irpre, irpost, drpre, drpost, tristate, tapstate, tcklevel, slave);
}

/* T32_GetMaxRawShiftSize
 * calculates the maximal shiftsize dependendent on the given buffers
*/
unsigned int T32_GetMaxRawShiftSize(unsigned int bitsToTransmit, byte* tmsbuf, byte* tdibuf, byte* tdobuf, unsigned int *options)
{
	unsigned int usedBits = 0, maxAllowedBits = 0;

	if(tmsbuf)
	{
		usedBits += bitsToTransmit;
	}
	if(tdibuf)
	{
		usedBits += bitsToTransmit;
	}
	if(tdobuf && !tdibuf && !tmsbuf)
	{
		usedBits += bitsToTransmit;
	}

	if(tdibuf && tmsbuf)
	{
		/* we have to transmit two buffers so the maximum shift is only half */
		maxAllowedBits = (T32_TAPACCESS_MAXBITS / 2) - SHIFTRAW_HEADERSIZE_BITS;
		if(bitsToTransmit > maxAllowedBits)
			return maxAllowedBits;
		else
			return bitsToTransmit;
	}
	else
	{
		maxAllowedBits = T32_TAPACCESS_MAXBITS - SHIFTRAW_HEADERSIZE_BITS;
		if(bitsToTransmit > maxAllowedBits)
		{
			return maxAllowedBits;
		}
		else
		{
			*options |= SHIFTRAW_OPTION_LASTTMS_ONE;
			return bitsToTransmit;
		}
	}
}



int T32_TAPAccessShiftRaw(T32_TAPACCESS_HANDLE connection, int numberofbits, byte * pTMSBits, byte * pTDIBits, byte * pTDOBits, int options)
{
	int usedbytes = 0;
	unsigned char *poutbuffer;
	unsigned char *poutptr;
	int nDataByteSize = 0;
	if (!numberofbits)
		return T32_Errno = 0;
	nDataByteSize = (numberofbits + 7) / 8 ;

	/* allocate twice the size - if TMS and TDI are given we need the double buffer size */
	poutbuffer = (unsigned char*)malloc(nDataByteSize*2+10);
	if (poutbuffer == NULL)
		return T32_MALLOC_FAIL;

	poutptr = poutbuffer;

	poutbuffer[0]  = 0x90; /* EMUMCI CMD Special Shift - see targetsystemtools.cpp */
	poutbuffer[1]  = 0x00; /* EMUMCI CMD Special Shift sub function 0x0*/

	/* setup options */
	options &= ~SHIFTRAW_OPTION_INTERNAL_ALL;
	if (pTMSBits)
		options |= SHIFTRAW_OPTION_INTERNAL_TMS;
	if (pTDIBits)
		options |= SHIFTRAW_OPTION_INTERNAL_TDI;
	if (pTDOBits)
		options |= SHIFTRAW_OPTION_INTERNAL_TDO;
	poutbuffer[2] = options & 0xFF;
	poutbuffer[3] = (options >> 8) & 0xFF;
	/* store bitlength */
	poutbuffer[4] = numberofbits & 0xff;
	poutbuffer[5] = (numberofbits >> 8) & 0xff;
	poutbuffer[6] = (numberofbits >> 16) & 0xff;
	poutbuffer[7] = (numberofbits >> 24) & 0xff;
	usedbytes += 8;
	poutptr = poutbuffer + 8;

	if (pTMSBits)
	{
		memcpy(poutptr, pTMSBits, nDataByteSize);
		poutptr += nDataByteSize;
		usedbytes += nDataByteSize;
	}
	if (pTDIBits)
	{
		memcpy(poutptr, pTDIBits, nDataByteSize);
		poutptr += nDataByteSize;
		usedbytes += nDataByteSize;
	}
	if (pTDOBits && !pTMSBits && !pTDIBits)
		usedbytes += nDataByteSize;

	if ( usedbytes > EMU_CBMAXDATASIZE )
	{
		free(poutbuffer);
		return T32_Errno = T32_COM_PARA_FAIL;
	}

	if (connection > T32_TAPACCESS_HOLD)
		T32_Errno = TAPAccessStore(12, connection, usedbytes*8 , poutbuffer, pTDOBits, ((pTDOBits)?numberofbits:0) );
	else
	{
		T32_Errno = TAPAccess(12, connection, usedbytes*8, poutbuffer, pTDOBits, ((pTDOBits)?numberofbits:0) );
	}

	free(poutbuffer);
	return T32_Errno;
}


int T32_TAPAccessShiftIR(T32_TAPACCESS_HANDLE connection, int numberofbits, byte * poutbits, byte * pinbits)
{
    if (numberofbits > T32_TAPACCESS_MAXBITS) {
	T32_Errno = T32_COM_PARA_FAIL;
	return T32_Errno;
    }
    if (connection > T32_TAPACCESS_HOLD)
		return TAPAccessStore(4, connection, numberofbits, poutbits, pinbits, numberofbits);
    else
		return TAPAccess(4, connection, numberofbits, poutbits, pinbits, numberofbits);
}


int T32_TAPAccessShiftDR(T32_TAPACCESS_HANDLE connection, int numberofbits, byte * poutbits, byte * pinbits)
{
    if (numberofbits > T32_TAPACCESS_MAXBITS) {
	T32_Errno = T32_COM_PARA_FAIL;
	return T32_Errno;
    }
    if (connection > T32_TAPACCESS_HOLD)
	return TAPAccessStore(8, connection, numberofbits, poutbits, pinbits, numberofbits);
    else
	return TAPAccess(8, connection, numberofbits, poutbits, pinbits, numberofbits);
}


int T32_TAPAccessDirect(T32_TAPACCESS_HANDLE connection, int nbytes, byte * poutbytes, byte * pinbytes)
{
    if (poutbytes == NULL && pinbytes == NULL) {
	T32_Errno = T32_COM_PARA_FAIL;
	return T32_Errno;
    }
    if (nbytes > EMU_CBMAXDATASIZE) {
	T32_Errno = T32_COM_PARA_FAIL;
	return T32_Errno;
    }
    if (connection > T32_TAPACCESS_HOLD)
	return TAPAccessStore(12, connection, nbytes * 8, poutbytes, pinbytes, nbytes * 8);
    else
	return TAPAccess(12, connection, nbytes * 8, poutbytes, pinbytes, nbytes * 8);
}


int T32_TAPAccessRelease(void)
{
    return TAPAccess(0, T32_TAPACCESS_RELEASE, -1, NULL, NULL, 0);
}


int T32_Config(const char *str1, const char *str2)
{
    char            configline[256];
    strcpy(configline, str1);
    strcat(configline, str2);
    if (LINE_LineConfig(configline) == -1)
	return -1;
    return 0;
}


int T32_Init(void)
{
    char errorline[256];
    int i;

    if (LINE_LineInit(errorline) == -1)
	return -1;

    LINE_SetReceiveToggleBit(-1);

    if (LINE_Sync() == -1)
	return -1;

    for(i=0; i<T32_MAX_EVENTS; i++) {
      notificationCallback[i] = 0;
    }

    return 0;
}


/*

	exit all drivers

*/

int T32_Exit(void)
{
    LINE_LineExit();

    return 0;
}





/** T32_NotifyStateEnable - enables notifications regarding state
   changes in T32. Register callback function for getting
   notifications related to events (state changes) in T32 or target.

   @param eventNr: T32_EMASK_BREAK,
                   T32_EMASK_EDIT,
                   T32_EMASK_BREAKPOINTCONFIG

   @param  func    callback function connected to this event

   @return    0   ... OK
              !=0 ... Number of Error

   @note currently there is not way to deregister events.
 */


static int T32_EventMask = 0;

int T32_NotifyStateEnable(int eventNr, T32_NotificationCallback_t func) {

  if (eventNr >= T32_MAX_EVENTS)
    return T32_Errno = T32_MAX_EVENT_FAIL;

  T32_EventMask |= 1 << eventNr;

  if (eventNr==T32_E_EDIT)  /* device independent -> win/main.c */
    {
      T32_OUTBUFFER[0] = 2;
      T32_OUTBUFFER[1] = RAPI_CMD_EDITNOTIFY;
      T32_OUTBUFFER[2] = 0x0;
      T32_OUTBUFFER[3] = LINE_GetNextMessageId();
      T32_OUTBUFFER[4] = 0x1;  /* No mask, just an enable bit */
    }
  else                      /* device dependent -> debug/comemu12.c */
    {
      T32_OUTBUFFER[0] = 2;
      T32_OUTBUFFER[1] = RAPI_CMD_DEVICE_SPECIFIC;
      T32_OUTBUFFER[2] = 0x12;  /* -> T32_NotifyStateEnable */
      T32_OUTBUFFER[3] = LINE_GetNextMessageId();
      T32_OUTBUFFER[4] = T32_EventMask;
    }

  if (LINE_Transmit(6) == -1)
    return (T32_Errno = T32_COM_TRANSMIT_FAIL);

  if (LINE_Receive() == -1)
    return (T32_Errno = T32_COM_RECEIVE_FAIL);

  notificationCallback[eventNr] = func; /* update pointer in case of success only */

  return 0;
}




/**************************************************************************

  T32_CheckStateNotify -  Checks socket for all pending notification
  messages and invokes callback functions if required for each of them.

  Parameter:  in unsigned param1 Parameter to give to the Event Callback

  Return:     int  OK=0 or Number of Error (<0)

***************************************************************************/

int T32_CheckStateNotify(unsigned param1)
{

	unsigned char package[T32_PCKLEN_MAX];
	int notifyid = LINE_ReceiveNotifyMessage(package);   /* Check if there is a pending notification message */

	while (notifyid!=-1) {

		switch (notifyid) {

		case T32_E_BREAK:
			{
				int state;
        //, err;
				/*err =*/ T32_GetState(&state);
				switch (state) {
				case 0: break;	/* system down */
				case 1: break;	/* system halted */
				case 2:		/* break (stopped) */
					if (notificationCallback[T32_E_BREAK]) {
						((T32_NotificationCallback_i_t) notificationCallback[T32_E_BREAK]) (param1);
					}
					break;
				case 3: break;	/* running */
				}
			}
			break;

		case T32_E_EDIT:
			if (notificationCallback[T32_E_EDIT]) {
				int off=16;
				int lineNr = package[off] + (package[off+1]<<8) + (package[off+2]<<16)+ (package[off+3]<<24); /* LE */
				unsigned char *fileName = package+20;
				((T32_NotificationCallback_iicp_t) notificationCallback[T32_E_EDIT]) (param1, lineNr, fileName);
			}
			break;


		case T32_E_BREAKPOINTCONFIG:
			if (notificationCallback[T32_E_BREAKPOINTCONFIG]) {
				((T32_NotificationCallback_i_t) notificationCallback[T32_E_BREAKPOINTCONFIG]) (param1);
			}
			break;
		}

		notifyid = LINE_ReceiveNotifyMessage(package);
	}
	return 0;
}


/**************************************************************************

  T32_GetSocketHandle - Get socket handle of TRACE32 remote connection

  Parameter:  in SOCKET *t32soc

  Return:   -

***************************************************************************/

void T32_GetSocketHandle(int *t32soc)
{
    *t32soc = LINE_LineDriverGetSocket();
}



/**************************************************************************

	network layer
	transport layer

***************************************************************************/

/** Sends message with payload contained in T32_OUTBUFFER[0...len-1].
    Adds the (empty) message header and calls LINE_LineTransmit.
 */
int LINE_Transmit(int len)
{
    int LastTransmitLen = 0;

    if (len)
	LastTransmitLen = len + 4 + 1;

    T32_OUTBUFFER[-5] = 0; /* message header */

    T32_OUTBUFFER[-4] = 0;
    T32_OUTBUFFER[-3] = 0;
    T32_OUTBUFFER[-2] = 0;
    T32_OUTBUFFER[-1] = 0;

    if (LINE_LineTransmit(T32_OUTBUFFER - 5, LastTransmitLen) == -1) {
	T32_Errno = T32_COM_TRANSMIT_FAIL;
	return -1;
    }
    return 0;
}


int LINE_Receive(void)
{
    int             len;
    int             retry;

    for (retry = 0; retry < MAXRETRY; retry++) {
	len = LINE_LineReceive(T32_INBUFFER - 1);
	if (len == -1) {
	    T32_Errno = T32_COM_RECEIVE_FAIL;
	    return -1;
	}
	if (T32_INBUFFER[2] == 0xfe) {
	    retry = 0;
	    LINE_SetReceiveToggleBit(-1);
	    continue;
	}
	if (T32_INBUFFER[3] != LINE_GetMessageId())
	    continue;

	if (T32_INBUFFER[-1] & T32_MSG_LRETRY) {
	    if (LINE_GetReceiveToggleBit() == (!!(T32_INBUFFER[-1] & T32_MSG_LHANDLE))) {
		if (LINE_Transmit(0) == -1)
		    LINE_Sync();
		continue;
	    }
	}
	LINE_SetReceiveToggleBit(!!(T32_INBUFFER[-1] & T32_MSG_LHANDLE));

	return len - 1;
    }

    T32_Errno = T32_COM_RECEIVE_FAIL;
    return -1;
}


int LINE_Sync(void)
{
    int             retry;

    for (retry = MAXRETRY; --retry > 0;) {
	if (LINE_LineSync() != -1)
	    return 0;
    }

    return -1;
}


