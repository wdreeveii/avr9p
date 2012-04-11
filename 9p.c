/* 9p.c -- 9p synthetic file server
 * 11/13/2011
 * Dedicated to my long friend Matt Brown.
 * AUTHOR: Whitham D. Reeve II
 * Heavily copied from Nigel Roles of Vita Nuova styx.c styx on a brick project.
 * Updated to 9p2000 and converted to skeleton for others to extend
 * 
 * BEWARE: This code makes heavy use of typecasts that are not safe on all architectures.
 * 			9p is a little endian formatted protocol, and avr-gcc produces little endian code.
 *			This was a deliberate and carefully made decision while considering both architecture
 *			safe and architecture unsafe methods upon examination of the resulting assembly. 
 *			This will run on a 1mip/mhz @ 20mhz atmega644pa chip with 4k ram. Careful attention has been paid
 *			to the size of the stack to avoid unnecessary duplication of data such as temporary local variables.
 *			Internally it stores all data using the native sizes in the 9p2000 specification, 
 *			and quite a bit of fat may? be trimmed in this area.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "9p.h"
#include "buffer.h"
#include "usart.h"
#include "9p_config.h"
#include "9p_pos.h"
#include "9p_pwm.h"

#define QID_ROOT 0
#define QID_POS 1
#define QID_CONFIG 2
#define QID_PWM 3

#define QID_MAP_MAX (sizeof(qid_map) / sizeof(qid_map[0]))

Qid qid_root = {QTDIR, 0, QID_ROOT};

int16_t p9_nowrite(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *data)
{
	return *count;
}

int16_t p9_noread(uint8_t outchannel, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[4] = {0};
	p9_send_reply(outchannel, Rread, tag, reply, 4);
	return 0;
}

DirectoryEntry dir_root[],dir_slash[];

DirectoryEntry dir_root[] = {
	{ "..",     {QTDIR, 0, QID_ROOT}, dir_slash },
	{ "pos",    {QTDIR, 0, QID_POS}, 0 },
	{ "config", {QTDIR, 0, QID_CONFIG}, 0 },
	{ "pwm",    {QTDIR, 0, QID_PWM}, 0 },
	{ 0 }
};

DirectoryEntry dir_slash[] = {
	{ "/", {QTDIR, 0, QID_ROOT}, dir_root },
	{ 0 }
};

#define QID_MAP_SIZE 20
DirectoryEntry *qid_map[QID_MAP_SIZE] = {
	/* QID_ROOT */      &dir_slash[0],
	/* QID_POS */       &dir_root[1],
	/* QID_CONFIG */    &dir_root[2],
	/* QID_PWM */       &dir_root[3],
};

uint8_t p9_register_de(DirectoryEntry * entry)
{
	uint8_t index;
	for(index = 0; index < QID_MAP_SIZE; index++)
	{
		if (!qid_map[index])
		{
			qid_map[index] = entry;
			break;
		}
	}
	return index;
}

void p9_init()
{
	dir_root[1].sub = p9_build_pos_dir(QID_ROOT, dir_root);
	dir_root[2].sub = p9_build_config_dir(QID_ROOT, dir_root);
	dir_root[3].sub = p9_build_pwm_dir(QID_ROOT, dir_root);
}

typedef struct Fid {
	struct Fid *next;
	uint32_t fid;
	uint8_t open;
	Qid		qid;
} Fid;

Fid *fids;

void flushtag(uint16_t oldtag)
{
	// no pending requests with a stored tag that need to be removed.
	
}

Fid * findfid(uint32_t fid)
{
	Fid *fp;
	for (fp = fids; fp && fp->fid != fid; fp = fp->next)
		;
	return fp;
}

Fid * fidcreate(uint32_t fid, const Qid *qid)
{
	Fid *fp;
	fp = malloc(sizeof(Fid));
	/* check fp here PLZ FIX */
	fp->open = 0;
	fp->fid = fid;
	fp->next = fids;
	fp->qid = *qid;
	fids = fp;
	return fp;
}

void fiddelete(Fid *fp)
{
	Fid **fpp;
	/* clobber any outstanding reads on this fid */
	//allreaderlistfindanddestroy(matchfp, fp);
	/* now clobber the fid */
	for (fpp = &fids; *fpp; fpp = &(*fpp)->next)
		if (*fpp == fp) {
			*fpp = fp->next;
			free(fp);
			return;
		}
	return;
}

void fidwalk(uint8_t outchannel, uint16_t tag, Fid *fp, uint32_t * fid, uint32_t * newfid, uint16_t numwalks, uint8_t *namesarr)
{
	const DirectoryEntry *sdp;
	const DirectoryEntry *dp;
	uint16_t namesize = 0;
	uint16_t walks = numwalks;
	
	struct walklist {
		uint16_t size;
		Qid		walklist[MAXWELEM];	
	} data;
	uint8_t walklistend = 0;
	uint8_t filefound = 0;
	if (numwalks == 0)
	{
		if (*fid != *newfid)
			fidcreate(*newfid, &fp->qid);
		
		p9_send_reply(outchannel, Rwalk, tag, (uint8_t *)&numwalks, 2);
		return;
	
	}

	dp = qid_map[fp->qid.path];
	if (dp->sub == 0)
	{
		p9_send_error_reply(outchannel, tag, "Not a directory.");
		return;
	}
	
	for (; walks > 0; walks--)
	{
		namesize = *((uint16_t *)(namesarr));
		filefound = 0;
		if (dp == qid_map[QID_ROOT] && 0 == strncmp((char*)namesarr + 2, "..", 2))
		{
			data.walklist[walklistend++] = dp->qid;
			goto continue_loop;
		}
		// find the file
		for (sdp = dp->sub; sdp->name; sdp++)
		{
			if (strncmp(sdp->name, (char*)namesarr + 2, namesize) == 0) {
				filefound = 1;
				data.walklist[walklistend++] = sdp->qid;
				dp = sdp;
				break;
			}
		}
		if (!filefound)
		{ 
			// first loop when walks == numwalks
			if(walks == numwalks)
			{
				p9_send_error_reply(outchannel, tag, "No such file or directory.");
				return;
			}
			break;
		}
		continue_loop:
		namesarr += 2 + namesize;
	}
	
	if (walklistend == numwalks)
	{
		if (*fid == *newfid)
			fp->qid = data.walklist[walklistend - 1];
		else
			fidcreate(*newfid, &data.walklist[walklistend-1]);
	}
	
	data.size = walklistend;
	
	p9_send_reply(outchannel, Rwalk, tag, (uint8_t *)&data, 2 + sizeof(Qid)*walklistend);
	
	return;
}

typedef struct Stat {
	uint16_t size;
	uint16_t type;
	uint32_t dev;
	uint8_t qidtype;
	uint32_t qidvers;
	uint64_t qidpath;
	uint32_t mode;
	uint32_t atime;
	uint32_t mtime;
	uint64_t length;
	uint8_t strings[100];
} Stat;

void mkstat(const DirectoryEntry * dp, Stat * data)
{
	uint16_t stringlen = 0;
	uint8_t *spos;
	data->type = 0xEFBE;
	data->dev = 0xEEFFC0DA;
	memcpy(&(data->qidtype), &(dp->qid), sizeof(Qid));
	
	data->mode = ((uint32_t)(dp->qid.type & 0xFC)) << 24;
	data->mode |= dp->sub ? 0555 : 0666;
	data->atime = 0xDEC0CDAB;
	data->mtime = 0xEFBEEFBE;
	//data->length
	
	spos = data->strings;
	*((uint16_t *)(spos)) = stringlen = strlen(dp->name);
	memcpy(spos + 2, dp->name, stringlen);
	
	spos += 2 + stringlen;
	
	*((uint16_t *)(spos)) = 3;
	memcpy(spos + 2, "sys", 3);
	spos += 5;
	
	*((uint16_t *)(spos)) = 3;
	memcpy(spos + 2, "sys", 3);
	spos += 5;
	
	*((uint16_t *)(spos)) = 3;
	memcpy(spos + 2, "sys", 3);
	spos += 5;
	
	data->size = 41 + (spos - data->strings);
}
struct Rstat_data {
	uint16_t	size_dup;
	Stat		data;
};
void fidstat(uint8_t outchannel, uint16_t tag, Fid *fp)
{
 	struct Rstat_data data = {};
	const DirectoryEntry *dp;
	
	dp = qid_map[fp->qid.path];
	
	mkstat(dp, &data.data);
	data.size_dup = data.data.size + 2;
	
	p9_send_reply(outchannel, Rstat, tag, (uint8_t *)&data, data.size_dup);
}

int8_t fidopen(Fid *fp, uint8_t mode)
{
	if (fp->open
	    || (mode & ORCLOSE)
	    /*|| (mode & OTRUNC) */)
		return 0;
	if (fp->qid.type && (mode == OWRITE || mode == ORDWR))
		/* can't write directories */
		return 0;
	fp->open = 1;
	return 1;
}
struct Rreaddir_data {
	Stat		data;
	uint16_t size_dup;
};
#define MIN(a,b) (a < b) ? a : b
int8_t fidreaddir(uint8_t outchannel, uint16_t tag, const DirectoryEntry *dp, uint64_t *offset, uint32_t * count)
{
	const DirectoryEntry *sdp;
	uint8_t reply[*count + 4];
	uint8_t *replyptr = reply + 4;
	
	struct Rreaddir_data data = {};
	
	uint32_t statsize = 0;
	
	uint16_t numcpybytes = 0;
	*((uint32_t *)reply) = 0;
	
	for (sdp = dp->sub; sdp->name; sdp++)
	{
		mkstat(sdp, &data.data);
		statsize += data.data.size + 2;
		
		if (*offset >= statsize)
			continue;
		
		// offset < statsize
		
		// not enough space for this directory so just breakout and return what we have so far
		if (*count < statsize - *offset)
			break;
		numcpybytes = MIN(*count, statsize - *offset);
		
		memcpy(replyptr,
				(((uint8_t *)(&data)) + (*offset - (statsize - data.data.size - 2))),
				numcpybytes);
		replyptr += numcpybytes;
		*((uint32_t *)(reply)) += numcpybytes;
		*offset += numcpybytes;
		*count -= numcpybytes;
	}
	p9_send_reply(outchannel, Rread, tag, reply, replyptr - reply);
	return 0;
}

int8_t fidread(uint8_t outchannel, uint16_t tag, Fid * fp, uint64_t * offset, uint32_t * count)
{
	const DirectoryEntry *dp;
	dp = qid_map[fp->qid.path];
	
	if (!dp)
		return -1;
	
	if (fp->qid.type & QTDIR) {
		if (!fp->open)
			return -1;
		return fidreaddir(outchannel, tag, dp, offset, count);
	}
	/* right, now that that's out of the way */
	if (!dp->read)
		return -1;
	
	return (*dp->read)(outchannel, dp, tag, offset, count);
}

int16_t fidwrite(Fid *fp, uint64_t *offset, uint32_t *count, uint8_t *buf)
{
	const DirectoryEntry *dp;

	if (fp->qid.type & QTDIR)
		return -1;		/* can't write directories */
	if (!fp->open)
		return -1;

	dp = qid_map[fp->qid.path];
	if (!dp || !dp->write)
		return -1;		/* no write method */
	return (*dp->write)(dp, offset, count, buf);
}

/* size[4]type[1]tag[2]data_size[2]*/
#define ERR_HEADER_SIZE 9

void p9_send_error_reply(uint8_t outchannel, uint16_t tag, char *msg)
{
	uint16_t len = strlen(msg);
	uint8_t data[ERR_HEADER_SIZE];
	/* size[4]type[1]tag[2]data_size[2]data[len] */
	*((uint32_t *)(data)) = ERR_HEADER_SIZE + len;	
	data[4] = Rerror;
	*((uint16_t *)(data + 5)) = tag;
	*((uint16_t *)(data + 7)) = len;
	
	USART_Send(outchannel, data, ERR_HEADER_SIZE);
	USART_Send(outchannel, (uint8_t *)msg, len);
}

void p9_send_reply(uint8_t outchannel, uint8_t type, uint16_t tag, uint8_t *msg, uint16_t len)
{
	/* 7 = size[4]type[1]tag[2] */
	uint8_t data[7];
	*((uint32_t *)(data)) = 7 + len;
	*((uint8_t *)(data + 4)) = type;
	*((uint16_t *)(data + 5)) = tag;
	
	USART_Send(outchannel, data, 7);
	
	if (msg)
	{
		USART_Send(outchannel, msg, len);
	}
}


void lib9p_process_message(uint8_t outchannel, buffer_t *msg)
{
	uint8_t reply[sizeof(Qid) + 4]; // 12 for Tversion, 17 for Topen
	uint16_t oldtag; 	// Tflush
	int32_t written; // Twrite

	msg->p_out += 4;
	msg->count -= 4;
	
	uint8_t type;
	Buffer_Pull(msg, &type);
	
	uint16_t tag = *((uint16_t *)(msg->p_out));
	msg->p_out += 2;
	msg->count -= 2;

	switch (type)
	{
		case Tversion:
			*((uint32_t *)reply) = BUFFER_SIZE;
			*((uint16_t *)(reply + 4)) = 6;
			memcpy(reply + 6, "9P2000", 6);
			
			p9_send_reply(outchannel, Rversion, tag, reply, 12);
			return;
		case Tflush:
			oldtag = *((uint16_t *) (msg->p_out));
			flushtag(oldtag);
			p9_send_reply(outchannel, Rflush, tag, 0, 0);
			return;
		case Tauth:
			p9_send_error_reply(outchannel, tag, "Auth not required.");
			return;
		case Twstat:
			p9_send_reply(outchannel, Rwstat, tag, 0, 0); // fake response instead of error for v9fs
			//send_error_reply(tag, "Unable to change file stat.");
			return;
	}
	
	uint32_t fid = *((uint32_t *) (msg->p_out));
	msg->p_out += 4;
	msg->count -= 4;
	
	Fid *fp = findfid(fid);
	
	switch (type)
	{
		case Tattach:
			if (fp)
			{
				p9_send_error_reply(outchannel, tag, "fid in use");	
			}
			else
			{	// other fields in this transaction
				// afid[4] -- no auth
				// uname[s] -- no users
				// aname[s] -- no other filesystems
				
				fp = fidcreate(fid, &qid_root);
				p9_send_reply(outchannel, Rattach, tag, (uint8_t *) &fp->qid, sizeof(Qid));
			}
			return;
		case Tclunk:
		case Tremove:
			if (!fp)
			{
				p9_send_error_reply(outchannel, tag, "no such fid");
			}
			else
			{
				fiddelete(fp);
				if (type == Tremove)
					p9_send_error_reply(outchannel, tag, "can't remove");
				else
					p9_send_reply(outchannel, Rclunk, tag, 0, 0);
			}
			return;
		case Twalk:
			// other fields in this transaction
			// newfid[4]
			// nwname[2]
			// nwname*(wname[s])
	
			if (fp->open)
			{
				p9_send_error_reply(outchannel, tag, "Fid opened");
				return;
			}
			if (fid  != *((uint32_t *)(msg->p_out)))
			{
				if ( findfid( *((uint32_t *)(msg->p_out)) ) )
				{
					p9_send_error_reply(outchannel, tag, "New fid in use.");
					return;
				}
			}
			fidwalk (outchannel, tag, 
						fp, 
						&fid, 
						(uint32_t *)(msg->p_out), 
						*((uint16_t *) (msg->p_out + 4)), 
						msg->p_out + 6);
			return;
		case Tstat:
			fidstat(outchannel, tag, fp);
			return;
		case Tcreate:
			p9_send_error_reply(outchannel, tag, "Can't create");
			return;
		case Topen:
			if (!fidopen(fp, msg->p_out[0]))
				p9_send_error_reply(outchannel, tag, "Can't open");
			else
			{
				// send qid and iounit (maximum length to be sent before the request is segmented)
				memcpy(reply, (uint8_t *)&fp->qid, sizeof(Qid));
				*((uint32_t *)(reply + sizeof(Qid))) = IOUNIT;
				p9_send_reply(outchannel, Ropen, tag, reply, sizeof(Qid) + 4);
			}
			return;
		case Tread:
			if (*((uint32_t *)(msg->p_out + 8)) > IOUNIT)
				*((uint32_t *)(msg->p_out + 8)) = IOUNIT;
			
			if (fidread(outchannel, tag, fp, (uint64_t *)msg->p_out, (uint32_t *)(msg->p_out + 8)) < 0)
				p9_send_error_reply(outchannel, tag, "Can't read");
			return;
		case Twrite:
			written = fidwrite(fp, 
								(uint64_t *)(msg->p_out),
								(uint32_t *)(msg->p_out + 8), 
								msg->p_out + 12);
			if (written < 0)
				p9_send_error_reply(outchannel, tag, "can't write");
			else {
				p9_send_reply(outchannel, Rwrite, tag, (uint8_t *)&written, 4);
			}
			return;
	}
	
}
