#include "9p.h"
#include "buffer.h"

typedef struct Readable {
	Reader *reader;
} Readable;

Readable readable[3];

typedef struct Reader {
	uint16_t tag;
	uint16_t fid;
	uint16_t offset;
	uint16_t count;
	struct Reader *next;
} Reader;

uint8_t reader_count;

void readerfree(Reader *rp)
{
	free(rp);
	reader_count--;
}


void readerlistfindanddestroy(Reader **rpp, int (*action)(Reader *rp, void *magic), void *magic)
{
	while (*rpp) {
		Reader *rp = *rpp;
		if ((*action)(rp, magic)) {
			*rpp = rp->next;
			readerfree(rp);
		}
		else
			rpp = &(rp->next);
	}
}

void allreaderlistfindanddestroy(int (*action)(Reader *rp, void *magic), void *magic)
{
	short i;
	for (i = 0; i < 3; i++)
		readerlistfindanddestroy(&readable[i].reader, action, magic);
}

int matchtag(Reader *rp, void *oldtag)
{
	if (rp->tag == (uint16_t)oldtag) {
		return 1;
	}
	return 0;
}

void flushtag(uint16_t oldtag)
{
	/* a little inefficient this - there can be at most one match! */
	allreaderlistfindanddestroy(matchtag, (void *)oldtag);
}

int16_t demowrite(const DirectoryEntry *dp, uint16_t offset, uint16_t count, uint8_t *data)
{
	return count;
}

int16_t sensorread(const struct DirectoryEntry *dp, uint16_t tag, uint16_t fid, uint16_t offset, uint16_t count)
{

	return 0;
}

#define QID_ROOT 0
const DirectoryEntry dir_root[], dir_slash[];

const DirectoryEntry dir_motor[] = {
	{ "..", QID_ROOT, dir_root },
	{ "0", QID_MOTOR_0,	0, 0, demowrite },
	{ "1", QID_MOTOR_1,	0, 0, demowrite },
	{ "2", QID_MOTOR_2,	0, 0, demowrite },
	{ "012", QID_MOTOR_012, 0, 0, demowrite },
	{ 0 }
};

const DirectoryEntry dir_sensor[] = {
	{ "..", QID_ROOT, dir_root },
	{ "0", QID_SENSOR_0,	0, demoread, demowrite },
	{ "1", QID_SENSOR_1,	0, demoread, demowrite },
	{ "2", QID_SENSOR_2,	0, demoread, demowrite },
	{ 0 }
};

const DirectoryEntry dir_root[] = {
	{ "..", QID_ROOT, dir_slash },
	{ "motor", QID_MOTOR, dir_motor },
	{ "sensor", QID_SENSOR, dir_sensor },
	{ 0 }
};

const DirectoryEntry dir_slash[] = {
	{ "/", QID_ROOT, dir_root },
	{ 0 }
};

const DirectoryEntry *qid_map[] = {
	/* QID_ROOT */		&dir_slash[0],
	/* QID_MOTOR */		&dir_root[1],
	/* QID_MOTOR_0 */	&dir_motor[1],
	/* QID_MOTOR_1 */	&dir_motor[2],
	/* QID_MOTOR_2 */	&dir_motor[3],
	/* QID_MOTOR_012 */	&dir_motor[4],
	/* QID_SENSOR */	&dir_root[2],
	/* QID_SENSOR_0 */	&dir_sensor[1],
	/* QID_SENSOR_1 */	&dir_sensor[2],
	/* QID_SENSOR_2 */	&dir_sensor[3],
};

#define QID_MAP_MAX (sizeof(qid_map) / sizeof(qid_map[0]))

const uint8_t qid_root[8] = { QID_ROOT, 0, 0, 0x80 };

typedef struct DirectoryEntry {
	char *name;
	uint8_t qid;
	const struct DirectoryEntry *sub;
	int16_t (*read)(const struct DirectoryEntry *dp, uint16_t tag, uint16_t fid, uint16_t offset, uint16_t count);
	int16_t (*write)(const struct DirectoryEntry *dp, uint16_t offset, uint16_t count, uint8_t *buf);
} DirectoryEntry;


typedef struct Fid {
	struct Fid *next;
	uint16_t fid;
	uint8_t open;
	uint8_t qid[8];
} Fid;

Fid *fids;

Fid * fidfind(uint16_t fid)
{
	Fid *fp;
	for (fp = fids; fp && fp->fid != fid; fp = fp->next)
		;
	return fp;
}

Fid * fidcreate(uint16_t fid, const uint8_t qid[8])
{
	Fid *fp;
	fp = malloc(sizeof(Fid));
	/* check fp here PLZ FIX */
	fp->open = 0;
	fp->fid = fid;
	fp->next = fids;
	memcpy(fp->qid, qid, 8);
	fids = fp;
	return fp;
}

int matchfp(Reader *rp, void *magic)
{
	if (rp->fid == ((Fid *)magic)->fid) {
		return 1;
	}
	return 0;
}

void fiddelete(Fid *fp)
{
	Fid **fpp;
	/* clobber any outstanding reads on this fid */
	allreaderlistfindanddestroy(matchfp, fp);
	/* now clobber the fid */
	for (fpp = &fids; *fpp; fpp = &(*fpp)->next)
		if (*fpp == fp) {
			*fpp = fp->next;
			free(fp);
			return;
		}
	return;
}

int fidwalk(Fid *fp, char name[28])
{
	const DirectoryEntry *sdp;
	const DirectoryEntry *dp;

	if (fp->open)
		return -1;
	//ASSERT(fp->qid[0] < QID_MAP_MAX);
	dp = qid_map[fp->qid[0]];
	if (dp->sub == 0)
		return -1;
	for (sdp = dp->sub; sdp->name; sdp++)
		if (strcmp(sdp->name, name) == 0) {
			fp->qid[0] = sdp->qid;
			fp->qid[3] = sdp->sub ? 0x80 : 0;
			return 1;
		}
	return 0;
}

void mkdirent(const DirectoryEntry *dp, uint8_t *dir)
{
	memset(dir, 0, DIRLEN);
	strcpy(dir, dp->name);
	strcpy(dir + 28, "lego");
	strcpy(dir + 56, "lego");
	dir[84] = dp->qid;
	dir[92] = dp->sub ? 0555 : 0666;
	dir[93] = dp->sub ? (0555 >> 8) : (0666 >> 8);
	dir[95] = dp->sub ? 0x80 : 0;
}

int fidstat(Fid *fp, uint8_t *dir)
{
	const DirectoryEntry *dp;
	if (fp->open)
		return -1;
	//ASSERT(fp->qid[0] < QID_MAP_MAX);
	dp = qid_map[fp->qid[0]];
	mkdirent(dp, dir);
	return 1;
}
int fidopen(Fid *fp, uint8_t mode)
{
	if (fp->open
	    || (mode & ORCLOSE)
	    /*|| (mode & OTRUNC) */)
		return 0;
	if (fp->qid[3] && (mode == OWRITE || mode == ORDWR))
		/* can't write directories */
		return 0;
	fp->open = 1;
	return 1;
}


void send_error_reply(uint16_t tag, char *msg)
{
	uint16_t len = strlen(msg);
	uint8_t data[BUFFER_SIZE];
	/* size[4]type[1]tag[2]data_size[2]data[len] */
	*((uint32_t *)data) = 9 + len;	
	data[4] = Rerror;
	*((uint16_t *)(data + 5)) = tag;
	*((uint16_t *)(data + 7)) = len;
	memcpy(data + 9, msg, len);
	
	USART_Send(0, data, 9 + len);
}

void send_reply(uint8_t type, uint16_t tag, uint8_t *msg, uint16_t len)
{
	uint8_t data[BUFFER_SIZE];
	/* 7 = size[4]type[1]tag[2] */
	if (len > BUFFER_SIZE - 7)	
		len = BUFFER_SIZE - 7;
	*((uint32_t *)data) = 7 + len;
	data[4] = type;
	*((uint16_t *)(data + 5)) = tag;
	if (msg)
		memcpy(data + 7, msg, len);
	
	USART_Send(0, data, 7 + len);
}

void send_fid_reply(uint8_t type, uint16_t tag, uint16_t fid, uint8_t *msg, uint16_t len)
{
	uint8_t data[BUFFER_SIZE];
	/* 9 = size[4]type[1]tag[2]fid[2] */
	if (len > BUFFER_SIZE - 9)
		len = BUFFER_SIZE - 9;
	*((uint32_t *)data) = 9 + len;
	data[4] = type;
	*((uint16_t *)(data + 5)) = tag;
	*((uint16_t *)(data + 7)) = fid;
	if (msg)
		memcpy(data + 9, msg, len);
		
	USART_Send(0, data, 9 + len);
}


void 9p_process_message(buffer_t *msg)
{
	uint32_t len = *((uint32_t *)(msg->p_out));
	msg->p_out += 4;
	msg->count -= 4;
	
	uint8_t type;
	buffer_pull(msg, &type);
	
	uint16_t tag = *((uint16_t *)(msg->p_out));
	msg->p_out += 2;
	msg->count -= 2;
	
	switch (type)
	{
		case Tversion:
			uint8_t reply[12];
			*((uint32_t *)reply) = BUFFER_SIZE;
			*((uint16_t *)(reply + 4)) = 6;
			memcpy(reply + 6, "9P2000", 6);
			
			send_reply(Rversion, tag, reply, 12);
			return;
		case Tflush:
			uint16_t oldtag = *((uint16_t *) (msg->p_out));
			flushtag(oldtag);
			send_reply(Rflush, tag, 0, 0);
			return;
		case Tauth:
			send_error_reply(tag, "Auth not required.");
			return;
	}
	
	uint16_t fid = *((uint16_t *) (msg->p_out));
	msg->p_out += 2;
	msg->count -= 2;
	
	Fid *fp = findfid(fid);
	
	switch (type)
	{
		case Tattach:
			if (fp)
			{
				send_error_reply(tag, "fid in use");	
			}
			else
			{
				fp = fidcreate(fid, qid_root);
				send_fid_reply(Rattach, tag, fid, fp->qid, 8);
			}
			return;
		case Tclunk:
		case Tremove:
			if (!fp)
			{
				send_error_reply(tag, "no such fid");
			}
			else
			{
				fiddelete(fp);
				if (type == Tremove)
					send_error_reply(tag, "can't remove");
				else
					send_reply(Rclunk, tag, 0, 0);
			}
			return;
		case Twalk:
			if (!fidwalk(fp, msg))
				send_error_reply(tag, "no such name");
			else
				send_fid_reply(Rwalk, tag, fid, fp->qid, 8);
			return;
		case Tstat:
			if (!fidstat(fp, dir))
				send_error_reply(tag, "can't stat");
			else
				send_fid_reply(Rstat, tag, fid, dir, 116);
			break;
		case Tcreate:
			send_error_reply(tag, "can't create");
			break;
		case Topen:
		//ASSERT(len == 1);
			if (!fidopen(fp, msg[0]))
				send_error_reply(tag, "can't open");
			else
				send_fid_reply(Ropen, tag, fid, fp->qid, 8);
			break;
	}
	
}

void
process_styx_message(unsigned char *msg, short len)
{
	unsigned char type;
	ushort tag, oldtag, fid, newfid;
	ushort offset, count;
	short extra;
	Fid *fp, *nfp;
	short written;
	uchar buf[2];

	ASSERT(len >= 3);
	
	type = *msg++; len--;
	tag = (msg[1] << 8) | msg[0]; len -= 2; msg += 2;

	switch (type) {
	case Tversion:
		send_reply(Rversion, tag, 0, 0);
		goto done;
	case Tflush:
		ASSERT(len == 2);
		oldtag = (msg[1] << 8) | msg[0];
		flushtag(oldtag);
		send_reply(Rflush, tag, 0, 0);
		goto done;
	}
	/* all other messages take a fid as well */
	ASSERT(len >= 2);
	fid = (msg[1] << 8) | msg[0]; len -= 2; msg += 2;
	fp = fidfind(fid);
	
	switch (type) {
	case Tattach:
		ASSERT(len == 56);
		if (fp) {
		fid_in_use:
			send_error_reply(tag, "fid in use");
		}
		else {
			fp = fidcreate(fid, qid_root);
			send_fid_reply(Rattach, tag, fid, fp->qid, 8);
		}
		break;
	case Tclunk:
	case Tremove:
		ASSERT(len == 0);
		if (!fp) {
		no_such_fid:
			send_error_reply(tag, "no such fid");
		}
		else {
			fiddelete(fp);
			if (type == Tremove)
				send_error_reply(tag, "can't remove");
			else
				send_fid_reply(Rclunk, tag, fid, 0, 0);
		}
		break;
	case Tclone:
		ASSERT(len == 2);
		newfid = (msg[1] << 8) | msg[0];
		nfp = fidfind(newfid);
		if (!fp)
			goto no_such_fid;
		if (fp->open) {
			send_error_reply(tag, "can't clone");
			break;
		}
		if (nfp)
			goto fid_in_use;
		nfp = fidcreate(newfid, fp->qid);
		send_fid_reply(Rclone, tag, fid, 0, 0);
		break;
	case Twalk:
		ASSERT(len == 28);
		if (!fidwalk(fp, msg))
			send_error_reply(tag, "no such name");
		else
			send_fid_reply(Rwalk, tag, fid, fp->qid, 8);
		break;
	case Tstat:
		ASSERT(len == 0);
		if (!fidstat(fp, dir))
			send_error_reply(tag, "can't stat");
		else
			send_fid_reply(Rstat, tag, fid, dir, 116);
		break;
		ASSERT(len == 0);
	case Tcreate:
		ASSERT(len == 33);
		send_error_reply(tag, "can't create");
		break;
	case Topen:
		ASSERT(len == 1);
		if (!fidopen(fp, msg[0]))
			send_error_reply(tag, "can't open");
		else
			send_fid_reply(Ropen, tag, fid, fp->qid, 8);
		break;
	case Tread:
		ASSERT(len == 10);
		offset = (msg[1] << 8) | msg[0];
		count = (msg[9] << 8) | msg[8];
		if (fidread(fp, tag, offset, count) < 0)
			send_error_reply(tag, "can't read");
		break;
	case Twrite:
		ASSERT(len >= 11);
		offset = (msg[1] << 8) | msg[0];
		count = (msg[9] << 8) | msg[8];
		msg += 11;
		len -= 11;
		ASSERT(count == len);
		written = fidwrite(fp, offset, count, msg);
		if (written < 0)
			send_error_reply(tag, "can't write");
		else {
			buf[0] = written;
			buf[1] = written >> 8;
			send_fid_reply(Rwrite, tag, fid, buf, 2);
		}
		break;
	default:
		FATAL;
	}
done:
	;
}