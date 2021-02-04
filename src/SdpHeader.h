#ifndef SdpHeader_H
#define SdpHeader_H

#include <Arduino.h>

class SdpHeader
{
public:
    struct O
    {
        String username;
        String sessId;
        String sessVersion;
        String netType;
        String addrType;
        String municastAddress;
    };

    struct M
    {
        String media;
        String port;
        String proto;
        String fmt;
    };
    struct C
    {
        String netType;
        String addrType;
        String connectionAddress;
    };
    String v = "0";
    O o;
    String s = "-";
    C c;
    String t = "0 0";
    M m;
    String a = "";

    int udpPortRead=0;
    int udpPortWrite=0;

    void parse(String in);
    String getContent();
    SdpHeader();

};

#endif
