#ifndef RTP_H
#define RTP_H

#include <Arduino.h>
#include <complex>
#include <valarray>
#include "Debug.h"
#define SWAP_INT32(x) (((x) >> 24) | (((x)&0x00FF0000) >> 8) | (((x)&0x0000FF00) << 8) | ((x) << 24))
#define SWAP_INT16(x) (((x & 0xff) << 8) | ((x & 0xff00) >> 8))



class RTP
{
private:
    typedef std::complex<double> Complex;
    typedef std::valarray<Complex> CArray;

    bool is_big;
    int8_t ALaw_Encode(int16_t number);
    int16_t ALaw_Decode(int8_t number);

    int8_t MuLaw_Encode(int16_t number);
    int16_t MuLaw_Decode(int8_t number);
    bool is_big_endian(void);
    void fft(CArray &x);

public:
    RTP();
    
    int8_t alaw_encode(int16_t pcm);
    int16_t alaw_decode(int8_t alaw);

    struct Rtp
    {
        uint8_t cc : 4;
        uint8_t x : 1;
        uint8_t p : 1;
        uint8_t version : 2;
        // byte 0

        uint8_t pt : 7;
        uint8_t m : 1;
        //byte 1
        
        uint16_t sequenceNumber;
        //byte 3
        uint32_t timestamp;
        //byte 7
        uint32_t SSRCIdentifier;
        //byte 11
        int8_t b[160];
    };
    struct Dtmf
    {
        uint8_t cc : 4;
        uint8_t x : 1;
        uint8_t p : 1;
        uint8_t version : 2;
        // byte 0
        uint8_t pt : 7;
        uint8_t m : 1;

        //byte 1
        uint16_t sequenceNumber;
        //byte 3
        uint32_t timestamp;
        //byte 7
        uint32_t SSRCIdentifier;
        //byte 11
        uint8_t event;
        //byte 12
        uint8_t volume : 6;
        uint8_t r : 1;
        uint8_t e : 1;
        //byte 13
        uint16_t duration;
        //byte 15
    };
    size_t rtpPos=0;

    Rtp rtpBuffer;
    Dtmf dtmfBuffer;
    uint16_t getSequenceNumber()
    {
        return is_big ? rtpBuffer.sequenceNumber : SWAP_INT16(rtpBuffer.sequenceNumber);
    };
    uint32_t getTimestamp()
    {
        return is_big ? rtpBuffer.timestamp : SWAP_INT32(rtpBuffer.timestamp);
    };
    uint32_t getSSRCIdentifier()
    {
        return is_big ? rtpBuffer.SSRCIdentifier : SWAP_INT32(rtpBuffer.SSRCIdentifier);
    };
    void setSequenceNumber(uint16_t x)
    {
        rtpBuffer.sequenceNumber = is_big ? x : SWAP_INT16(x);
    };
    void setTimestamp(uint32_t x)
    {
        rtpBuffer.timestamp = is_big ? x : SWAP_INT32(x);
    };
    void setSSRCIdentifier(uint32_t x)
    {
        rtpBuffer.SSRCIdentifier = is_big ? x : SWAP_INT32(x);
    };

    void put(int16_t value);
    int16_t get(uint8_t pos);
    char getDtmf();
};

#endif
