#include "urandom.h"
#include "xbase32.h"
#include "xbase57.h"
#include "xnet.h"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <alloca.h>

#ifdef XSLIB_RCSID
static const char rcsid[] = "$Id: urandom.c,v 1.3 2013/03/15 15:07:29 gremlin Exp $";
#endif

static int get_random_fd(void)
{
	static int fd = -2;

	if (fd == -2)
	{
		struct timeval tv;
		int i;

		gettimeofday(&tv, 0);
		fd = open("/dev/urandom", O_RDONLY);
		if (fd == -1)
			fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
		srandom((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);

		/* Crank the random number generator a few times */
		gettimeofday(&tv, 0);
		for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
			random();
	}
	return fd;
}

bool urandom_has_device()
{
	return (get_random_fd() >= 0);
}

/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
void urandom_get_bytes(void *buf, size_t nbytes)
{
	ssize_t k, n = nbytes;
	int fd = get_random_fd();
	int lose_counter = 0;
	unsigned char *cp = (unsigned char *)buf;

	if (fd >= 0)
	{
		while (n > 0)
		{
			k = read(fd, cp, n);
			if (k <= 0)
			{
				if (lose_counter++ > 8)
				{
					struct timeval tv;
					int i;
					/* Crank the random number generator a few times */
					gettimeofday(&tv, 0);
					for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
					{
						random();
					}
					break;
				}
				continue;
			}
			n -= k;
			cp += k;
			lose_counter = 0;
		}
	}
	
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	for (cp = (unsigned char *)buf, k = 0; k < (ssize_t)nbytes; k++)
		*cp++ ^= (random() >> 7) & 0xFF;
}

int urandom_get_int(int a, int b)
{
	double range = (double)b - a;
	if (range > 0)
	{
		unsigned int n = range * random() / (RAND_MAX + 1.0);
		return a + n;
	}
	else
	{
		unsigned int n = -range * random() / (RAND_MAX + 1.0);
		return b + n;
	}
}

ssize_t urandom_generate_base32id(char id[], size_t size)
{
	size_t bsize, len, i;
	unsigned char *buf;
	char *b32str;
	uint32_t u32;

	len = size - 1;
	if ((ssize_t)len <= 0)
		return len;

	bsize = (len * 5 + 7) / 8;
	if (bsize < sizeof(uint32_t))
		bsize = sizeof(uint32_t);

       	buf = alloca(bsize);
	urandom_get_bytes(buf, bsize);

	u32 = (uint8_t)buf[0];
	for (i = 1; i < sizeof(uint32_t); ++i)
	{
		u32 <<= 8;
		u32 += buf[i];
	}

	b32str = alloca(XBASE32_ENCODED_LEN(bsize) + 1);
	len = xbase32_encode(b32str, buf, bsize);
	if (len >= size)
	{
		len = size - 1;
		b32str[len] = 0;
	}

	memcpy(id, b32str, len + 1);
	id[0] = xbase32_alphabet[10 + (u32 / (UINT32_MAX / 22 + 1))]; /* make it always letter instead of digit */
	return len;
}

ssize_t urandom_generate_base57id(char id[], size_t size)
{
	size_t bsize, len, i;
	unsigned char *buf;
	char *b57str;
	uint32_t u32;

	len = size - 1;
	if ((ssize_t)len <= 0)
		return len;

	bsize = XBASE57_DECODED_LEN(len) + 1;
	if (bsize < sizeof(uint32_t))
		bsize = sizeof(uint32_t);

       	buf = alloca(bsize);
	urandom_get_bytes(buf, bsize);

	u32 = (uint8_t)buf[0];
	for (i = 1; i < sizeof(uint32_t); ++i)
	{
		u32 <<= 8;
		u32 += buf[i];
	}

	b57str = alloca(XBASE57_ENCODED_LEN(bsize) + 1);
	len = xbase57_encode(b57str, buf, bsize);
	if (len >= size)
	{
		len = size - 1;
		b57str[len] = 0;
	}

	memcpy(id, b57str, len + 1);
	id[0] = xbase57_alphabet[10 + (u32 / (UINT32_MAX / 47 + 1))]; /* make it always letter instead of digit */
	return len;
}

