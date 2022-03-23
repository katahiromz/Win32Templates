#include "Common.h"

static char base64table1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t Base64EncodeSize(size_t cbSrc)
{
    return ((cbSrc + 2) / 3) * 4 + 1;
}

char *Base64Encode(const void *pSrc, size_t cbSrc)
{
#define B64_ENC(Ch) (base64table1[(unsigned char)(Ch) & 0x3f])
    size_t cb = 0, cbDest = Base64EncodeSize(cbSrc);
    const char *pbSrc = pSrc;
    char *pszDest = (char *)malloc(cbDest);
    char *pchDest = pszDest;

    if (!pszDest)
        return NULL;

    for ( ; cbSrc > 2; cbSrc -= 3, pbSrc += 3 )
    {
        *pchDest++ = B64_ENC(pbSrc[0] >> 2);
        *pchDest++ = B64_ENC(((pbSrc[0] << 4) & 0x30) | ((pbSrc[1] >> 4) & 0x0F));
        *pchDest++ = B64_ENC(((pbSrc[1] << 2) & 0x3C) | ((pbSrc[2] >> 6) & 0x03));
        *pchDest++ = B64_ENC(pbSrc[2] & 0x3F);
        cb += 4;
    }

    if (cbSrc == 1)
    {
        *pchDest++ = B64_ENC(pbSrc[0] >> 2);
        *pchDest++ = B64_ENC((pbSrc[0] << 4) & 0x30);
        *pchDest++ = '=';
        *pchDest++ = '=';
        cb += 4;
    }
    else if (cbSrc == 2)
    {
        *pchDest++ = B64_ENC(pbSrc[0] >> 2);
        *pchDest++ = B64_ENC(((pbSrc[0] << 4) & 0x30) | ((pbSrc[1] >> 4) & 0x0F));
        *pchDest++ = B64_ENC((pbSrc[1] << 2) & 0x3C);
        *pchDest++ = '=';
        cb += 4;
    }
    *pchDest++ = 0;
    assert(pchDest - pszDest == cbDest);

    return pszDest;
#undef B64_ENC
}

static const unsigned char base64table2[0x100] =
{
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF,   62, 0xFF, 0xFF, 0xFF,   63,
  52,   53,   54,   55,   56,   57,   58,   59,
  60,   61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF,    0,    1,    2,    3,    4,    5,    6,
   7,    8,    9,   10,   11,   12,   13,   14,
  15,   16,   17,   18,   19,   20,   21,   22,
  23,   24,   25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0,   26,   27,   28,   29,   30,   31,   32,
  33,   34,   35,   36,   37,   38,   39,   40,
  41,   42,   43,   44,   45,   46,   47,   48,
  49,   50,   51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

size_t Base64DecodeSize(const char *pszSrc, size_t cbSrc)
{
    size_t cbDest, cb = 0;

    while (cbSrc--)
    {
        if (*pszSrc == '=')
            break;
        if (base64table2[(unsigned char)*pszSrc++] != (unsigned char)0xFF)
            cb++;
    }

    cbDest = cb / 4 * 3;

    switch (cb & 3)
    {
    case 1: case 2: cbDest++; break;
    case 3: cbDest += 2; break;
    }

    return cbDest;
}

void *Base64Decode(const char *pszSrc, size_t cbSrc, size_t *pcbDest)
{
    size_t i, j, n, cb = 0;
    size_t cbDest = Base64DecodeSize(pszSrc, cbSrc);
    unsigned char *pbDest = (unsigned char *)malloc(cbDest + 1);
    unsigned char b, *pb;
    *pcbDest = cbDest;

    if (!pbDest)
        return NULL;

    for (i = 0; i < cbSrc; i++)
    {
        if (pszSrc[i] == '=')
            break;
        if (base64table2[(unsigned char)pszSrc[i]] != (unsigned char)0xFF)
            cb++;
    }

    cbDest = cb / 4 * 3;
    switch(cb % 4)
    {
    case 1: case 2: cbDest++; break;
    case 3: cbDest += 2; break;
    }

    pb = pbDest;
    for (i = 0; i < cb / 4 * 4; i += 4)
    {
        for (j = n = 0; j < 4; )
        {
            b = base64table2[(unsigned char)*pszSrc++];
            if (b != (unsigned char)0xFF)
            {
                n |= (((unsigned int)b) << ((3 - j) * 6));
                j++;
            }
        }

        for (j = 0; j < 3; j++)
            *pb++ = (unsigned char) ((n >> (8 * (2 - j))) & 0xFF);
    }

    for (j = n = 0; j < cb % 4; )
    {
        b = base64table2[(unsigned char)*pszSrc++];
        if (b != (unsigned char) 0xFF)
        {
            n |= (((unsigned int)b) << ((3 - j) * 6));
            j++;
        }
    }

    for (j = 0; j < ((cb & 3) * 6 / 8); j++)
        *pb++ = (unsigned char)((n >> (8 * (2 - j))) & 0xFF);

    *pb = 0;
    assert(pb - pbDest == cbDest);

    return pbDest;
}

#ifdef UNITTEST
int main(int argc, char **argv)
{
    size_t size;
    char *encode = Base64Encode(argv[1], strlen(argv[1]));
    char *decode = Base64Decode(encode, strlen(encode), &size);
    printf("%s --> %s --> %s (%d)\n", argv[1], encode, decode, size);
    free(encode);
    free(decode);
    return 0;
}
#endif
