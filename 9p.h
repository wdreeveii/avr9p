
#ifndef _9P_H
#define _9P_H

#include "buffer.h"


/* Maximum walk elements */
#define MAXWELEM 16

/* STATFIXLEN includes leading 16-bit count */
/* The count, however, excludes itself; total size is BIT16SZ+count */
#define STATFIXLEN	(BIT16SZ+QIDSZ+5*BIT16SZ+4*BIT32SZ+1*BIT64SZ)	/* amount of fixed length data in a stat buffer */

#define	NOTAG		(uint16_t)~0U	/* Dummy tag */
#define	NOFID		(uint32_t)~0U	/* Dummy fid */
#define	IOHDRSZ		24	/* ample room for Twrite/Rread header (iounit) */


enum
{
	Tversion =	100,
	Rversion,
	Tauth =	102,
	Rauth,
	Tattach =	104,
	Rattach,
	Terror =	106,	/* illegal */
	Rerror,
	Tflush =	108,
	Rflush,
	Twalk =		110,
	Rwalk,
	Topen =		112,
	Ropen,
	Tcreate =	114,
	Rcreate,
	Tread =		116,
	Rread,
	Twrite =	118,
	Rwrite,
	Tclunk =	120,
	Rclunk,
	Tremove =	122,
	Rremove,
	Tstat =		124,
	Rstat,
	Twstat =	126,
	Rwstat,
	Tmax,
};

enum
{
	OREAD   = 0,     	//  open for read 
	OWRITE  = 1,		// write 
	ORDWR   = 2,		// read and write 
	OEXEC   = 3,		// execute, == read but check execute permission 
	OTRUNC  = 16,		// or'ed in (except for exec), truncate file first 
	OCEXEC  = 32,		// or'ed in, close on exec 
	ORCLOSE = 64,		// or'ed in, remove on close 
	ODIRECT = 128,		// or'ed in, direct access 
	ONONBLOCK = 256,	// or'ed in, non-blocking call 
	OEXCL   = 0x1000,	// or'ed in, exclusive use (create only) 
	OLOCK   = 0x2000,	// or'ed in, lock after opening 
	OAPPEND = 0x4000,	// or'ed in, append only 
};

enum
{
	AEXIST  = 0,	// accessible: exists 
	AEXEC   = 1,	// execute access 
	AWRITE  = 2,	// write access 
	AREAD   = 4,	// read access 
};


// Qid.type
enum
{
	QTDIR       =0x80,	// type bit for directories 
	QTAPPEND    =0x40,	// type bit for append only files 
	QTEXCL      =0x20,	// type bit for exclusive use files 
	QTMOUNT     =0x10,	// type bit for mounted channel 
	QTAUTH      =0x08,	// type bit for authentication file 
	QTTMP       =0x04,	// type bit for non-backed-up file 
	QTSYMLINK   =0x02,	// type bit for symbolic link 
	QTFILE      =0x00,	// type bits for plain file 
};

// Dir.mode

enum
{
	DMDIR       =0x80000000,	// mode bit for directories 
	DMAPPEND    =0x40000000,	// mode bit for append only files 
	DMEXCL      =0x20000000,	// mode bit for exclusive use files 
	DMMOUNT     =0x10000000,	// mode bit for mounted channel 
	DMAUTH      =0x08000000,	// mode bit for authentication file 
	DMTMP       =0x04000000,	// mode bit for non-backed-up file 
	DMREAD      =0x4,			// mode bit for read permission 
	DMWRITE     =0x2,			// mode bit for write permission 
	DMEXEC      =0x1,			// mode bit for execute permission 
};

/*DMSYMLINK   =0x02000000  # mode bit for symbolic link (Unix, 9P2000.u) 
DMDEVICE    =0x00800000  # mode bit for device file (Unix, 9P2000.u) 
DMNAMEDPIPE =0x00200000  # mode bit for named pipe (Unix, 9P2000.u) 
DMSOCKET    =0x00100000  # mode bit for socket (Unix, 9P2000.u) 
DMSETUID    =0x00080000  # mode bit for setuid (Unix, 9P2000.u) 
DMSETGID    =0x00040000  # mode bit for setgid (Unix, 9P2000.u)
*/
void lib9p_process_message(buffer_t *msg);

#endif