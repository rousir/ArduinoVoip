#ifndef SIPSTREAMIN_H_INCLUDED
#define SIPSTREAMIN_H_INCLUDED

#include "Stream.h"

class SipStreamIn: public Stream
{

public:
    size_t write(uint8_t);
    int available();
    int read();
    int peek();
    void flush();


};


#endif // SIPSTREAMIN_H_INCLUDED
