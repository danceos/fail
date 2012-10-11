
#include <config/VariantConfig.hpp>

#if defined WIN32 || defined WIN64
#ifndef MS_WINDOWS
#define MS_WINDOWS
#endif
#define LOW_HIGH
#endif

#ifdef DEC_VMS
#define LOW_HIGH_BYTE
#endif

#ifdef DEC_OSF1
#define UNIX_V
#define LOW_HIGH_BYTE
#endif

#ifdef HP_UX
#define UNIX_V
#define HIGH_LOW_BYTE
#endif

#ifdef IBM_AIX
#define UNIX_V
#define HIGH_LOW_BYTE
#endif

#ifdef __linux__
# ifndef LINUX
#  define LINUX
# endif
# define UNIX_V
# define LOW_HIGH_BYTE
#endif

#ifdef T32HOST_LINUX_X64
# define UNIX_V
# define LOW_HIGH_BYTE
#endif

#ifdef LINUX_PPC
# ifndef LINUX
#  define LINUX
# endif
# define UNIX_V
# define HIGH_LOW_BYTE
#endif

#ifdef T32HOST_MACOSX_X86
# define UNIX_V
# define LOW_HIGH_BYTE
#endif

#ifdef SUN_SOL
#define UNIX_V
#define HIGH_LOW_BYTE
#endif


typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;


#ifdef HIGH_LOW_BYTE

#define SETWORDCONST(A,B) ( ((unsigned char *)&(A))[0] = (B),((unsigned char *)&(A))[1] = (B)>>8 )
#define SETLONGCONST(A,B) ( ((unsigned char *)&(A))[0] = (B),((unsigned char *)&(A))[1] = (B)>>8,((unsigned char *)&(A))[2] = (B)>>16,((unsigned char *)&(A))[3] = (B)>>24 )

#define SETWORDVAR(A,B) ( ((unsigned char *)&(A))[0] = ((unsigned char *)&(B))[1],((unsigned char *)&(A))[1] = ((unsigned char *)&(B))[0] )
#define SETLONGVAR(A,B) ( ((unsigned char *)&(A))[0] = ((unsigned char *)&(B))[3],((unsigned char *)&(A))[1] = ((unsigned char *)&(B))[2],((unsigned char *)&(A))[2] = ((unsigned char *)&(B))[1],((unsigned char *)&(A))[3] = ((unsigned char *)&(B))[0] )

#endif

#ifdef HIGH_LOW

#define SETWORDCONST(A,B) ((*((unsigned short *)&(A))) = (unsigned short)((((B)>>8)&0xff)|(((B)<<8)&0xff00)))
#define SETLONGCONST(A,B) ((*((int *)&(A))) = (int)((((B)>>24)&0xffl)|(((B)>>8)&0xff00l)|(((B)<<8)&0xff0000l)|(((B)<<24)&0xff000000l)))

#define SETWORDVAR(A,B) ( ((unsigned char *)&(A))[0] = ((unsigned char *)&(B))[1],((unsigned char *)&(A))[1] = ((unsigned char *)&(B))[0] )
#define SETLONGVAR(A,B) ( ((unsigned char *)&(A))[0] = ((unsigned char *)&(B))[3],((unsigned char *)&(A))[1] = ((unsigned char *)&(B))[2],((unsigned char *)&(A))[2] = ((unsigned char *)&(B))[1],((unsigned char *)&(A))[3] = ((unsigned char *)&(B))[0] )

#endif

#ifdef LOW_HIGH

#define SETWORDCONST(A,B) ((*((unsigned short *)&(A))) = (unsigned short)(B))
#define SETLONGCONST(A,B) ((*((int *)&(A))) = (int)(B))

#define SETWORDVAR(A,B) ((*((unsigned short *)&(A))) = (*((unsigned short *)&(B))))
#define SETLONGVAR(A,B) ((*((int *)&(A))) = (*((int *)&(B))))

#endif

#ifdef LOW_HIGH_BYTE

#define SETWORDCONST(A,B) ( ((unsigned char *)&(A))[0] = (B),((unsigned char *)&(A))[1] = (B)>>8 )
#define SETLONGCONST(A,B) ( ((unsigned char *)&(A))[0] = (B),((unsigned char *)&(A))[1] = (B)>>8,((unsigned char *)&(A))[2] = (B)>>16,((unsigned char *)&(A))[3] = (B)>>24 )

#define SETWORDVAR(A,B) ( ((unsigned char *)&(A))[0] = ((unsigned char *)&(B))[0],((unsigned char *)&(A))[1] = ((unsigned char *)&(B))[1] )
#define SETLONGVAR(A,B) ( ((unsigned char *)&(A))[0] = ((unsigned char *)&(B))[0],((unsigned char *)&(A))[1] = ((unsigned char *)&(B))[1],((unsigned char *)&(A))[2] = ((unsigned char *)&(B))[2],((unsigned char *)&(A))[3] = ((unsigned char *)&(B))[3] )

#endif


#define LINE_MBLOCK		16384 	   /* large block mode */
#define LINE_MSIZE		(LINE_MBLOCK+256)

#define T32_DEV_OS		0
#define T32_DEV_ICE		1
#define T32_DEV_ICD		1  /* similar to ICE but clearer for user */

/* Events that can be generated by T32 */
#define T32_MAX_EVENTS 		3
#define T32_E_BREAK 		0x0 /* change in runstate, probably only "break" */
#define T32_E_EDIT              0x1 /* user requested "edit source" in external editor */
#define T32_E_BREAKPOINTCONFIG  0x2 /* breakpoint configuration changed (breakpoint set/removed) */

/* Callbacks should have the following protoypes */
/************************* WARNING *****************************/
/* 
 * If you add more callback functions here, make sure you ONLY
 * use 'int', 'unsigned int', 'integral-type *' or 'float-type *' as parameters.
 * 
 * No 'long', no 'long long', no 'short', no 'char', no floats, no typedefs 
 * and no function pointers!
 * This is because we absolutely need to avoid portability problems here.
 * 
 * Explanation:
 * The T32_NotificationCallback_t typedef'd below has an empty parameter list,
 * which we can't change anymore to a va_list because of backwards compatibility.
 * This means only the native types like 'int' and 'integral type *' can be used
 * safely, otherwise we run into hard-to-debug problems with type-promotion and
 * parameter-passing conventions on the varying platforms we support.
 */
extern void T32_callbackBreak(int generic);
extern void T32_callbackEditExtern(int generic, int lineNr, unsigned char *fileName);
extern void T32_callbackBreakpointConfig(int generic);


#define T32_API_RECEIVE         0x1
/* Missing: 0x5, */
#define T32_API_NOTIFICATION    0x6

#define T32_API_SYNCREQUEST     0x02
#define T32_API_SYNCACKN        0x12
#define T32_API_SYNCBACK        0x22

#define T32_PCKLEN_MAX 	1472



typedef struct t32_notification {

	char payload[T32_PCKLEN_MAX]; /* maximum packet size*/

	struct t32_notification *next, *prev;
	/* T32_NotificationPackage *next;
	   T32_NotificationPackage *prev; */

} T32_NotificationPackage;

extern T32_NotificationPackage *T32_NotificationHead, *T32_NotificationTail; /* Queue pending notifications */




#define T32_COM_RECEIVE_FAIL	-1
#define T32_COM_TRANSMIT_FAIL	-2
#define T32_COM_PARA_FAIL	-3
#define T32_COM_SEQ_FAIL	-4
#define T32_MAX_EVENT_FAIL 	-5
#define T32_MALLOC_FAIL 	-6


typedef void * T32_TAPACCESS_HANDLE;

#define T32_TAPACCESS_MAXBITS	0x3b00

#define T32_TAPACCESS_RELEASE	((T32_TAPACCESS_HANDLE)0)
#define T32_TAPACCESS_HOLD	((T32_TAPACCESS_HANDLE)1)

#define T32_TAPACCESS_TDO	0x00
#define T32_TAPACCESS_TDI	0x04
#define T32_TAPACCESS_TMS	0x08
#define T32_TAPACCESS_TCK	0x0c
#define T32_TAPACCESS_nTRST	0x10
#define T32_TAPACCESS_nRESET	0x14
#define T32_TAPACCESS_nRESET_LATCH 0x18
#define T32_TAPACCESS_VTREF	0x1c
#define T32_TAPACCESS_VTREF_LATCH 0x20
#define T32_TAPACCESS_nENOUT	0x24

#define T32_TAPACCESS_SET(x)	(2|((x)&1))
#define T32_TAPACCESS_SET_LOW	2
#define T32_TAPACCESS_SET_HIGH	3
#define T32_TAPACCESS_SET_0	T32_TAPACCESS_SET_LOW
#define T32_TAPACCESS_SET_1	T32_TAPACCESS_SET_HIGH

#define T32_TAPACCESS_SHIFT	0x30
#define T32_TAPACCESS_SHIFTTMS	0x40
#define T32_TAPACCESS_SHIFTIR	0x50
#define T32_TAPACCESS_SHIFTDR	0x60

#define T32_TAPACCESS_SLEEP_MS 0x7c
#define T32_TAPACCESS_SLEEP_US 0x7d
#define T32_TAPACCESS_SLEEP_HALF_CLOCK 0x7e
#define T32_TAPACCESS_SLEEP	T32_TAPACCESS_SLEEP_MS /*just for backwards compatibility*/


#define SHIFTRAW_OPTION_INTERNAL_TMS 0x0001 /*do not use the internal options*/
#define SHIFTRAW_OPTION_INTERNAL_TDI 0x0002
#define SHIFTRAW_OPTION_INTERNAL_TDO 0x0004
#define SHIFTRAW_OPTION_INTERNAL_ALL 0x0007

#define SHIFTRAW_OPTION_NONE         0x0000
#define SHIFTRAW_OPTION_LASTTMS_ONE  0x0008
#define SHIFTRAW_OPTION_TMS_ONE      0x0010
#define SHIFTRAW_OPTION_TMS_ZERO     0x0000
#define SHIFTRAW_OPTION_TDI_ONE      0x0040
#define SHIFTRAW_OPTION_TDI_ZERO     0x0000
#define SHIFTRAW_OPTION_TDI_LASTTDO  0x0020

#define T32_TAPACCESS_SPECIAL	0x80

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef _NO_PROTO

extern int T32_Config( const char *, const char * );
extern int T32_Init(void);
extern int T32_Exit(void);
extern int T32_Nop(void);
extern int T32_NopFail(void);
extern int T32_Ping(void);
extern int T32_Stop(void);
extern int T32_GetPracticeState(int * );
extern int T32_Attach( int );
extern int T32_GetState( int * );
extern int T32_GetCpuInfo( char **, word *, word *, word * );
extern int T32_GetRam(dword *, dword *, word *);
extern int T32_ResetCPU(void);

extern int T32_WriteMemory( dword, int, byte * , int );
extern int T32_WriteMemoryPipe( dword, int, byte * , int );
extern int T32_WriteMemoryEx( dword, int, int, int, byte * , int );
extern int T32_ReadMemory( dword, int, byte *, int );
extern int T32_ReadMemoryEx( dword, int, int, int, byte * , int );


#define T32_MEMORY_ACCESS_DATA			0x0000
#define T32_MEMORY_ACCESS_PROGRAM		0x0001
#define T32_MEMORY_ACCESS_ARM_CP0		0x0002
#define T32_MEMORY_ACCESS_ARM_ICE		0x0003
#define T32_MEMORY_ACCESS_ARM_ETM		0x0004
#define T32_MEMORY_ACCESS_ARM_CP14		0x0005
#define T32_MEMORY_ACCESS_ARM_CP15		0x0006
#define T32_MEMORY_ACCESS_ARM_ARM		0x0007
#define T32_MEMORY_ACCESS_ARM_THUMB		0x0008
#define T32_MEMORY_ACCESS_ARM_PHYSICAL_ARM	0x0009
#define T32_MEMORY_ACCESS_ARM_PHYSICAL_THUMB	0x000a
#define T32_MEMORY_ACCESS_ARM_ETB		0x000b
#define T32_MEMORY_ACCESS_ARM_PHYSICAL_DATA	0x000c
#define T32_MEMORY_ACCESS_ARM_PHYSICAL_PROGRAM	0x000d
#define T32_MEMORY_ACCESS_ARM_DAP		0x000e
#define T32_MEMORY_ACCESS_ARM_USR		0x000f

#define T32_MEMORY_ATTR_WIDTHMASK		0x000f
#define T32_MEMORY_ATTR_DUALPORT		0x0400
#define T32_MEMORY_ATTR_NOINCREMENT	0x4000


extern int T32_WriteRegister( dword, dword, dword * );
extern int T32_ReadRegister( dword, dword, dword * );
extern int T32_ReadPP( dword * );
extern int T32_WriteBreakpoint( dword, int, int, int );
extern int T32_ReadBreakpoint( dword, int, word * , int );
extern int T32_Step(void);
extern int T32_StepMode(int);
extern int T32_SetMode(int);
extern int T32_Go(void);
extern int T32_Break(void);
extern int T32_Terminate(int retval);
extern int T32_Cmd( char * );
extern int T32_CmdWin( dword, char * );
extern int T32_EvalGet ( dword * );
extern int T32_GetMessage ( char *, word * );
extern int T32_GetTriggerMessage ( char* );
extern int T32_GetSymbol ( char *, dword *, dword * , dword * );
extern int T32_GetSource ( dword, char *, dword * );
extern int T32_GetSelectedSource( char *, dword * );

extern int T32_AnaStatusGet( byte *, long *, long *, long * );
extern int T32_AnaRecordGet( long , byte *, int );
extern int T32_GetTraceState(int tracetype, int * state, long * size, long * min, long * max);
extern int T32_ReadTrace(int tracetype, long record, int nrecords, unsigned long mask, byte * buffer);

typedef void (*T32_NotificationCallback_i_t)(int);
typedef void (*T32_NotificationCallback_iicp_t)(int, int, unsigned char *);

#ifndef SUPPRESS_FUNCTION_TYPE_WARNING
typedef void (*T32_NotificationCallback_t)();
extern int T32_NotifyStateEnable( int event, T32_NotificationCallback_t func);
#endif

extern int T32_CheckStateNotify( unsigned param1);
extern void T32_GetSocketHandle( int *t32soc );

extern T32_TAPACCESS_HANDLE T32_TAPAccessAlloc(void);
extern int T32_TAPAccessExecute(T32_TAPACCESS_HANDLE connection, T32_TAPACCESS_HANDLE connectionhold);
extern int T32_TAPAccessFree(T32_TAPACCESS_HANDLE connection);
extern int T32_TAPAccessSetInfo(int irpre, int irpost, int drpre, int drpost, int tristate, int tapstate, int tcklevel, int slave);
extern int T32_TAPAccessShiftIR(T32_TAPACCESS_HANDLE connection, int numberofbits, byte * poutbits, byte * pinbits);
extern int T32_TAPAccessShiftDR(T32_TAPACCESS_HANDLE connection, int numberofbits, byte * poutbits, byte * pinbits);
extern int T32_TAPAccessShiftRaw(T32_TAPACCESS_HANDLE connection, int numberofbits, byte * pTMSBits, byte * pTDIBits, byte * pTDOBits, int options);
extern int T32_TAPAccessDirect(T32_TAPACCESS_HANDLE connection, int nbytes, byte * poutbytes, byte * pinbytes);
extern int T32_TAPAccessRelease(void);

extern int T32_Fdx_Resolve(char * name);
extern int T32_Fdx_Open(char * name, char * mode);
extern int T32_Fdx_Close(int channel);
extern int T32_Fdx_ReceivePoll(int channel, void * data, int width, int maxsize);
extern int T32_Fdx_Receive(int channel, void * data, int width, int maxsize);
extern int T32_Fdx_SendPoll(int channel, void * data, int width, int size);
extern int T32_Fdx_Send(int channel, void * data, int width, int size);

/*
 * New stream structure, to improve performance for PIPE mode.
 */
typedef struct
{
	int blen;
	unsigned char *ptr;
	unsigned char *ptrend;
	int channel;
	unsigned char buffer[4096];
} T32_Fdx_Stream;

extern T32_Fdx_Stream *T32_Fdx_OpenStream(char *name, char *mode);
extern int T32_Fdx_CloseStream(T32_Fdx_Stream * pstream);
extern int T32_Fdx_ReceiveStreamNext(T32_Fdx_Stream * pstream, unsigned char *target, int width, int size);

#define T32_Fdx_ReceiveStream(__pstream,__target,__width,__len)		( \
			(((__pstream)->ptr >= (__pstream)->ptrend) ? \
			    T32_Fdx_ReceiveStreamNext(__pstream,__target,__width,__len) : \
			((__pstream)->blen = ((__pstream)->ptr[0]|((__pstream)->ptr[1]<<8)), memcpy((__target),(__pstream)->ptr+2,(__pstream)->blen), (__pstream)->ptr += 2+(__pstream)->blen, (__pstream)->blen)))


extern int   T32_GetChannelSize(void);
extern void  T32_GetChannelDefaults(void* line);
extern void  T32_SetChannel(void* line);
extern void* T32_GetChannel0(void);

#endif

#ifdef	__cplusplus
}
#endif
