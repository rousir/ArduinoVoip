#ifndef MD5_h
#define MD5_h

#include "Arduino.h"
#include <string.h>
#include "SipHeader.h"

typedef unsigned long MD5_u32plus;

typedef struct
{
    MD5_u32plus lo, hi;
    MD5_u32plus a, b, c, d;
    char buffer[64];
    MD5_u32plus block[16];
} MD5_CTX;

class MD5
{
public:
    MD5();
    static unsigned  char* make_hash(  char *arg);
    static  unsigned char* make_hash(  char *arg,size_t size);
    static  char* make_digest(const unsigned char *digest, int len);
    static const void *body(void *ctxBuf, const void *data, size_t size);
    static void MD5Init(void *ctxBuf);
    static void MD5Final( unsigned char *result, void *ctxBuf);
    static void MD5Update(void *ctxBuf, const void *data, size_t size);
    String encrypt(String encrypts);
    String getUri(SipHeader::Authenticate auth);
    String getAuth(SipHeader::Authenticate auth);
};

#endif
