#include "SdpHeader.h"
#include "Debug.h"

SdpHeader::SdpHeader() {
 o.username="-";
 o.netType="IN";
 o.addrType="IP4";
 c.netType="IN";
 c.addrType="IP4";
 m.media="audio";
 m.proto="RTP/AVP";
 m.fmt="0 127";
}
void SdpHeader::parse(String in)
{
    if (in.startsWith("o="))
    {
        in.replace("o=", "");
        o.username = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" o.username                  ") + String(o.username.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        o.sessId = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" o.sessId                    ") + String(o.sessId.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        o.sessVersion = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" o.sessVersion               ") + String(o.sessVersion.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        o.netType = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" o.netType                   ") + String(o.netType.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        o.addrType = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" o.addrType                  ") + String(o.addrType.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        o.municastAddress = in.substring(0, in.indexOf("\r"));
        debugL2_println(String(" o.municastAddress           ") + String(o.municastAddress.c_str()));
    }
    if (in.startsWith("m="))
    {
        in.replace("m=", "");
        m.media = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" m.media                     ") + String(m.media.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        m.port = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" m.port                      ") + String(m.port.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        m.proto = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" m.proto                     ") + String(m.proto.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        m.fmt = in.substring(0, in.indexOf("\r"));
        debugL2_println(String(" m.fmt                       ") + String(m.fmt.c_str()));
    }
    if (in.startsWith("c="))
    {
        in.replace("c=", "");
        c.netType = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" c.netType                   ") + String(c.netType.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        c.addrType = in.substring(0, in.indexOf(" "));
        debugL2_println(String(" c.addrType                  ") + String(c.addrType.c_str()));
        in = in.substring(in.indexOf(" ") + 1);
        c.connectionAddress = in.substring(0, in.indexOf("\r"));
        debugL2_println(String(" c.connectionAddress         ") + String(c.connectionAddress.c_str()));
    }
}
String SdpHeader::getContent()
{
    String ret;
    ret = "v=0\r\n";
    ret += "o=" + o.username + " " + o.sessId + " " + o.sessVersion + " " + o.netType + " " + o.addrType + " " + o.municastAddress + "\r\n";
    ret += "s=" + s + "\r\n";
    ret += "c=" + c.netType + " " + c.addrType + " " + c.connectionAddress + "\r\n";
    ret += "t=" + t + "\r\n";
    ret += "m=" + m.media + " " + m.port + " " + m.proto + " " + m.fmt + "\r\n";
    ret += "a=rtpmap:8 PCMA/8000\r\n";
    ret += "a=rtpmap:127 telephone-event/8000";
    return ret;
}
