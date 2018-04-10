/*
   Author: XIONG Jiagui
   Date: 2005-06-20
 */
#include "bit.h"
#include <string.h>

#ifdef XSLIB_RCSID
static const char rcsid[] = "$Id: bit.c,v 1.7 2012/09/20 03:21:47 jiagui Exp $";
#endif

ssize_t bitmap_lsb_find1(const uint8_t *bitmap, size_t start, size_t end)
{
	size_t i, s, e, b, bstart, bend;
	if (start >= end)
		return -1;

	s = start / 8;
	i = start % 8;
	if ((bitmap[s] >> i) != 0)
	{
		b = s;
		bstart = i;
		bend = 8;
	}
	else
	{
		e = (end - 1) / 8;
		for (b = s + 1; b <= e && bitmap[b] == 0; ++b)
			continue;
		if (b > e)
			return -1;

		if (b == e)
		{
			bend = end % 8;
			if (bend == 0)
				bend = 8;
		}
		else
		{
			bend = 8;
		}
		bstart = 0;
	}

	for (i = bstart; i < bend; ++i)
	{
		if (bitmap[b] & (1 << i))
			return b * 8 + i;
	}

	return -1;
}

ssize_t bitmap_lsb_find0(const uint8_t *bitmap, size_t start, size_t end)
{
	size_t i, s, e, b, bstart, bend;
	if (start >= end)
		return -1;

	s = start / 8;
	i = start % 8;
	if ((bitmap[s] >> i) != (0xFF >> i))
	{
		b = s;
		bstart = i;
		bend = 8;
	}
	else
	{
		e = (end - 1) / 8;
		for (b = s + 1; b <= e && bitmap[b] == 0xFF; ++b)
			continue;
		if (b > e)
			return -1;

		if (b == e)
		{
			bend = end % 8;
			if (bend == 0)
				bend = 8;
		}
		else
		{
			bend = 8;
		}
		bstart = 0;
	}

	for (i = bstart; i < bend; ++i)
	{
		if (!(bitmap[b] & (1 << i)))
			return b * 8 + i;
	}

	return -1;
}

int bit_parity(uint32_t w)
{
	w ^= (w >> 16);
	w ^= (w >> 8);
	w ^= (w >> 4);
	w ^= (w >> 2);
	w ^= (w >> 1);
	return (w & 1);
}

int bit_count(uintmax_t x)
{
	int n = 0;

	if (x)
	{
		do
		{
			n++;            
		} while ((x &= x-1));
	}

	return n;
}

uintmax_t round_up_power_two(uintmax_t x)
{
	int r = 1;

	if (x == 0 || (intmax_t)x < 0)
		return 0;
	--x;
	while (x >>= 1)
		r++;
	return (UINTMAX_C(1) << r);
}

uintmax_t round_down_power_two(uintmax_t x)
{
	int r = 0;

	if (x == 0)
		return 0;
	while (x >>= 1)
		r++;
	return (UINTMAX_C(1) << r);
}

bool bitmap_msb_equal(const uint8_t *bmap1, const uint8_t *bmap2, size_t prefix)
{
	int bytes = prefix / 8;
	int bits = prefix % 8;

	if (bytes)
	{
		if (memcmp(bmap1, bmap2, bytes))
			return false;
	}
	
	if (bits)
	{
		uint8_t b1 = bmap1[bytes] >> (8 - bits);
		uint8_t b2 = bmap2[bytes] >> (8 - bits);
		return b1 == b2;
	}

	return true;
}

bool bitmap_lsb_equal(const uint8_t *bmap1, const uint8_t *bmap2, size_t prefix)
{
	int bytes = prefix / 8;
	int bits = prefix % 8;

	if (bytes)
	{
		if (memcmp(bmap1, bmap2, bytes))
			return false;
	}
	
	if (bits)
	{
		uint8_t b1 = bmap1[bytes] << (8 - bits);
		uint8_t b2 = bmap2[bytes] << (8 - bits);
		return b1 == b2;
	}

	return true;
}

bool bit_msb32_equal(uint32_t a, uint32_t b, size_t prefix)
{
	// NB: a >>= 32 seems doing nothing.
	// So we need to check if prefix == 0
	if (prefix == 0)
		return true;

	if (prefix < 32)
	{
		a >>= (32 - prefix);
		b >>= (32 - prefix);
	}
	return a == b;
}

bool bit_lsb32_equal(uint32_t a, uint32_t b, size_t prefix)
{
	if (prefix == 0)
		return true;

	if (prefix < 32)
	{
		a <<= (32 - prefix);
		b <<= (32 - prefix);
	}
	return a == b;
}

bool bit_msb64_equal(uint64_t a, uint64_t b, size_t prefix)
{
	if (prefix == 0)
		return true;

	if (prefix < 64)
	{
		a >>= (64 - prefix);
		b >>= (64 - prefix);
	}
	return a == b;
}

bool bit_lsb64_equal(uint64_t a, uint64_t b, size_t prefix)
{
	if (prefix == 0)
		return true;

	if (prefix < 64)
	{
		a <<= (64 - prefix);
		b <<= (64 - prefix);
	}
	return a == b;
}

int bit_msb32_find(uint32_t n)
{
	int b = 0;
	if (!n)
		return -1;
 
#define STEP(x) if (n >= ((uint32_t)1) << x) { b += x; n >>= x; }
	STEP(16);
	STEP(8);
	STEP(4);
	STEP(2);
	STEP(1);
#undef STEP
	return b;
}

int bit_msb64_find(uint64_t n)
{
	int b = 0;
	if (!n)
		return -1;
 
#define STEP(x) if (n >= ((uint64_t)1) << x) { b += x; n >>= x; }
	STEP(32);
	STEP(16);
	STEP(8);
	STEP(4);
	STEP(2);
	STEP(1);
#undef STEP
	return b;
}

int bit_lsb32_find(uint32_t n)
{
	int b = 0;
	if (!n)
		return -1;

#define STEP(x) if (!(n & ((((uint32_t)1) << x) - 1))) { b += x; n >>= x; }
	STEP(16);
	STEP(8);
	STEP(4);
	STEP(2);
	STEP(1);
#undef STEP
	return b;
}

int bit_lsb64_find(uint64_t n)
{
	int b = 0;
	if (!n)
		return -1;

#define STEP(x) if (!(n & ((((uint64_t)1) << x) - 1))) { b += x; n >>= x; }
	STEP(32);
	STEP(16);
	STEP(8);
	STEP(4);
	STEP(2);
	STEP(1);
#undef STEP
	return b;
}

#ifdef TEST_BIT

#include <stdio.h>

int main(int argc, char **argv)
{
	uint64_t n;
	int i;
	uint32_t a = 0xF2345678;
	uint32_t b = 0x82345678;
	int x = bit_msb32_equal(a, b, 0);
	printf("%d\n", x);

	for (i = 0, n = 1; true; i++, n *= 42)
	{
		printf("42**%-2d = %#18lx: M %2d L %2d\n",
			i, n, bit_msb64_find(n), bit_lsb64_find(n));
 
		if (n >= INT64_MAX / 42)
			break;
	}
	return 0;
}

#endif

