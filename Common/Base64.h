#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

size_t Base64EncodeSize(size_t cbSrc);
char *Base64Encode(const void *pSrc, size_t cbSrc);
size_t Base64DecodeSize(const char *pszSrc, size_t cbSrc);
void *Base64Decode(const char *pszSrc, size_t cbSrc, size_t *pcbDest);

#ifdef __cplusplus
}
#endif
