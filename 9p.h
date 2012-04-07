
#ifndef _9P_H
#define _9P_H

#include "buffer.h"

/* Maximum walk elements */
#define MAXWELEM 16
/* Maximum entries in a directory */
#define MAXDIRENTRIES 16

#define IOUNIT BUFFER_SIZE - 23

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

typedef struct Qid
{
	uint8_t		type;
	uint32_t	vers;
	uint64_t	path;
} Qid;

typedef struct DirectoryEntry {
	char	*name;
	Qid		qid;
	const	struct DirectoryEntry *sub;
	int16_t (*read)(uint8_t outchannel, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count);
	int16_t (*write)(const struct DirectoryEntry *dp, uint64_t * offset, uint32_t * count, uint8_t *buf);
} DirectoryEntry;

void p9_send_reply(uint8_t outchannel, uint8_t type, uint16_t tag, uint8_t *msg, uint16_t len);
void p9_send_error_reply(uint8_t outchannel, uint16_t tag, char *msg);

void lib9p_process_message(uint8_t outchannel, buffer_t *msg);
uint8_t p9_register_de(DirectoryEntry * entry);
void p9_init();

int16_t p9_nowrite(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *data);
int16_t p9_noread(uint8_t outchannel, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count);

#endif