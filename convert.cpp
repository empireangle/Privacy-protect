#include "doc.h"
/**
#include <iconv.h>

int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,
		char *outbuf, size_t outlen) {
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
		return -1;
	iconv_close(cd);
	*pout = '\0';

	return 0;
}

int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}

int u8tou16(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("utf-8", "UCS-2LE", inbuf, inlen, outbuf, outlen);
}
*/
int bin_to_hex(byte* addr, int len)
{
	int hex = 0;

	if ((addr[len - 1] & 0x80) == 0x80) //负数
	{
		for (int i = len - 1; i >= 0; i--) //Little-Endian
		{
			byte temp = ~addr[i];
			hex = hex * 16 * 16 + temp;
		}
		hex = (hex + 1)*(-1); //取反加1
	}
	else //正数
	{
		for (int i = len - 1; i >= 0; i--) //Little-Endian
		{
			hex = hex * 16 * 16 + addr[i];
		}
	}
	return hex;
}

int UTF8ToUnicode(unsigned char *ch, int *unicode)
{
	unsigned char *p = NULL;
	int e = 0, n = 0;
	if ((p = ch) && unicode)
	{
		if (*p >= 0xfc)
		{

			e = (p[0] & 0x01) << 30;
			e |= (p[1] & 0x3f) << 24;
			e |= (p[2] & 0x3f) << 18;
			e |= (p[3] & 0x3f) << 12;
			e |= (p[4] & 0x3f) << 6;
			e |= (p[5] & 0x3f);
			n = 6;
		}
		else if (*p >= 0xf8)
		{

			e = (p[0] & 0x03) << 24;
			e |= (p[1] & 0x3f) << 18;
			e |= (p[2] & 0x3f) << 12;
			e |= (p[3] & 0x3f) << 6;
			e |= (p[4] & 0x3f);
			n = 5;
		}
		else if (*p >= 0xf0)
		{

			e = (p[0] & 0x07) << 18;
			e |= (p[1] & 0x3f) << 12;
			e |= (p[2] & 0x3f) << 6;
			e |= (p[3] & 0x3f);
			n = 4;
		}
		else if (*p >= 0xe0)
		{

			e = (p[0] & 0x0f) << 12;
			e |= (p[1] & 0x3f) << 6;
			e |= (p[2] & 0x3f);
			n = 3;
		}
		else if (*p >= 0xc0)
		{

			e = (p[0] & 0x1f) << 6;
			e |= (p[1] & 0x3f);
			n = 2;
		}
		else
		{
			e = p[0];
			n = 1;
		}
		*unicode = e;
	}

	return n;
}

int UTF8StrToUnicodeStr(unsigned char * utf8_str,
	unsigned short * unicode_str, int unicode_str_size)
{
	int unicode = 0;
	int n = 0;
	int count = 0;
	unsigned char *s = NULL;
	unsigned short *e = NULL;
	if (unicode_str_size == 0){
		s = utf8_str;

		if ((utf8_str) && (unicode_str))
		{
			while (*s)
			{
				if ((n = UTF8ToUnicode(s, &unicode)))
				{
					unicode_str_size = unicode_str_size + 1;
					s += n;
				}
			}
			return 2 * unicode_str_size;
		}
	}
	s = utf8_str;
	e = unicode_str;

	if ((utf8_str) && (unicode_str))
	{
		while (*s)
		{
			if ((n = UTF8ToUnicode(s, &unicode)) > 0)
			{
				if ((count + 1) > unicode_str_size)
				{
					return count;
				}
				else
				{
					*e = (unsigned short)unicode;
					e++;
					*e = 0;
					s += n;
				}
			}
			else
			{
				return count;
			}
		}
	}
	return count;
}
