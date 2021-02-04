#include "SipHeader.h"
#include "MD5.h"
#include <Arduino.h>
#include "Debug.h"

/**
 * [SipHeader::parse 解析处理Sip数据包]
 *
 * @DateTime 2021-01-20
 * @param    in 待解析的sip数据包
 */
void SipHeader::parse(String in)
{
    String ret = "";
    if (in.indexOf("SIP/2.0 ") > -1)
        setResposeCode(in);
    if (in.indexOf("From: ") > -1)
        setFrom(in);
    if (in.indexOf("To: ") > -1)
        setTo(in);
    if (in.indexOf("Call-ID: ") > -1)
        setCallId(in);
    if (in.indexOf("CSeq: ") > -1)
        setCseq(in);
    if (in.indexOf("WWW-Authenticate: ") > -1)
        setAuthenticate(in);
    if (in.indexOf("Proxy-Authenticate:") > -1)
        setAuthenticate(in);
    if (in.indexOf("Via: ") > -1)
        setVia(in);
    if (in.indexOf("Content-Length: ") > -1)
        setContentLength(in);
    if (in.indexOf("Contact: ") > -1)
        setContact(in);

    if (in.indexOf("ACK sip:") > -1)
    {
        responseCodes = 0;
    }
    if (in.indexOf("BYE sip:") > -1)
    {
        responseCodes = 1;
    }
    if (in.indexOf("INVITE sip:") > -1)
    {
        setInvite(in);
        responseCodes = 2;
    }
    if (in.indexOf("CANCEL sip:") > -1)
    {
        responseCodes = 3;
    }
    if (in.indexOf("MESSAGE sip:") > -1)
    {
        responseCodes = 4;
    }
}

/**
 * [SipHeader::parsGk 解析sip头]
 *
 * @DateTime 2021-01-20
 * @param    in 待解析的sip头
 * @return 解析后的sip 头结构体
 */
SipHeader::Gk SipHeader::parsGk(String in)
{
    Gk ret;
    ret.ip = "";
    ret.port = "";
    ret.telNr = "";
    ret.transport = "";
    String gks = in.substring(in.indexOf("<") + 1, in.indexOf(">"));
    if (gks.indexOf(":") >= 0)
    {
        gks = gks.substring(gks.indexOf(":") + 1);
    }
    if (gks.indexOf(";") >= 0)
    {
        String a = gks.substring(gks.indexOf(";"));
        gks = gks.substring(0, gks.indexOf(";"));
        ret.transport = a.substring(a.indexOf("=") + 1);
    }
    if (gks.indexOf("@") >= 0)
    {
        ret.telNr = gks.substring(0, gks.indexOf("@"));
        gks = gks.substring(gks.indexOf("@") + 1);
    }
    if (gks.indexOf(":") >= 0)
    {
        ret.port = gks.substring(gks.indexOf(":") + 1);
        gks = gks.substring(0, gks.indexOf(":"));
    }
    ret.ip = gks;
    return ret;
}

/**
 * [SipHeader::find 在in中查找x]
 *
 * @DateTime 2021-01-20
 * @param    x
 * @param    in
 * @return
 */
String SipHeader::find(String x, String in)
{
    String ret = "";
    if (in.indexOf(x) >= 0)
    {
        ret = in.substring(in.indexOf(x) + x.length());
        ret.replace("\r", "");
        ret.replace("\n", "");
        if (ret.indexOf(";") >= 0)
        {
            ret = ret.substring(0, ret.indexOf(";"));
        }
        if (ret.indexOf(",") >= 0)
        {
            ret = ret.substring(0, ret.indexOf(","));
        }
        ret.replace("\"", "");
    }
    return ret;
}

/**
 * [SipHeader::getVia 获取via字段]
 *
 * @DateTime 2021-01-20
 * @return 当前via字段
 */
String SipHeader::getVia()
{
    String str;
    str = "Via: SIP/2.0/TCP ";
    str += via.userClient;
    
    if (via.port != 0)
    {
        str += ":";
        str += via.port;
    }
    
    str += ";branch=";
    str += via.branch;
    return str;
}

/**
 * [SipHeader::getMaxForwards 获取MaxForwards字段]
 *
 * @DateTime 2021-01-20
 * @return 当前MaxForwards字段
 */
String SipHeader::getMaxForwards()
{
    String str;
    str = "Max-Forwards: 70";
    return str;
}
String SipHeader::getTo()
{
    String str;
    if (to.tagTo.length() > 0)
    {
        str = "To: <sip:";
        str += to.telNr;
        str += "@";
        str += to.userAgent;
        str += ">;tag=";
        str += to.tagTo;
    }
    else
    {
        str = "To: <sip:";
        str += to.telNr;
        str += "@";
        str += to.userAgent;
        str += ">";
    }
    return str;
}
String SipHeader::getFrom()
{
    String str;
    str = "From: <sip:";
    str += from.telNr;
    str += "@";
    str += from.userAgent;
    str += ">;tag=";
    str += from.tagFrom;
    return str;
}
String SipHeader::getCallID()
{
    String str;
    str = "Call-ID: ";
    str += callId.callId;
    return str;
}
String SipHeader::getCSeq()
{
    String str;
    str = "CSeq: ";
    str += cSeq.cSeq;
    str += " ";
    str += cSeq.typ;
    return str;
}
String SipHeader::getContact()
{
    String str;
    str = "Contact: <sip:";
    str += contact.telNr;
    str += "@";
    str += contact.userClient;
    str += ";transport=TCP>";
    return str;
}
String SipHeader::getAllow()
{
    String str;
    str = "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING";
    return str;
}
String SipHeader::getContenType()
{
    String str;
    str = "Content-Type: application/sdp";
    return str;
}
String SipHeader::getContentLength(int contentLength)
{
    String str;
    str = "Content-Length: ";
    str += String(contentLength);
    return str;
}

String SipHeader::getAuthorisation()
{
    authenticate.nc = 1;
    MD5 md5;
    authenticate.cNonce = md5.encrypt("das ist ein Chaos");
    authenticate.uri = md5.getUri(authenticate);
    authenticate.response = md5.getAuth(authenticate);

    String str;
    str += "Authorization: Digest username=\"";
    str += authenticate.user;

    str += "\",realm=\"";
    str += authenticate.realm;

    str += "\",nonce=\"";
    str += authenticate.nonce;

    str += "\",uri=\"sip:";
    str += authenticate.uri;

    str += "\",response=\"";
    str += authenticate.response;

    str += "\",algorithm=MD5";
    return str;
}

void SipHeader::setResposeCode(String in)
{
    responseCodes = in.substring(in.indexOf("SIP/2.0 ") + 8, in.indexOf(" ", 8)).toInt();
    debugL2_println(String(" Response Code               ") + String(responseCodes));
}
void SipHeader::setFrom(String in)
{
    Gk gk;
    gk = parsGk(in);
    from.telNr = gk.telNr;
    debugL2_println(String(" sipHeaderR.from.telNr       ") + String(from.telNr.c_str()));
    from.userAgent = gk.ip;
    debugL2_println(String(" sipHeaderR.from.userAgent   ") + String(from.userAgent.c_str()));
    from.tagFrom = find("tag=", in);
    debugL2_println(String(" sipHeaderR.from.tagFrom     ") + String(from.tagFrom.c_str()));
}
void SipHeader::setTo(String in)
{
    Gk gk;
    gk = parsGk(in);
    to.telNr = gk.telNr;
    debugL2_println(String(" sipHeaderR.to.telNr         ") + String(to.telNr.c_str()));
    to.userAgent = gk.ip;
    debugL2_println(String(" sipHeaderR.to.userAgent     ") + String(to.userAgent.c_str()));
    to.tagTo = find("tag=", in);
    debugL2_println(String(" sipHeaderR.to.tagTo         ") + String(to.tagTo.c_str()));
}
void SipHeader::setCallId(String in)
{
    callId.callId = in.substring(in.indexOf(" ") + 1, in.indexOf("\r"));
    debugL2_println(String(" sipHeaderR.callId.callId    ") + String(callId.callId.c_str()));
}
void SipHeader::setCseq(String in)
{
    cSeq.cSeq = in.substring(in.indexOf(": ") + 2, in.indexOf(" ", in.indexOf(" ") + 1)).toInt();
    debugL2_println(String(" sipHeaderR.Cseq.Cseq        ") + String(cSeq.cSeq));
    cSeq.typ = in.substring(in.indexOf(" ", 7) + 1, in.indexOf("\r"));
    debugL2_println(String(" sipHeaderR.Cseq.userAgent   ") + String(cSeq.typ.c_str()));
}
void SipHeader::setAuthenticate(String in)
{
    authenticate.realm = find("realm=", in);
    debugL2_println(String(" sipHeaderR.auth...realm     ") + String(authenticate.realm.c_str()));
    authenticate.domain = find("domain=", in);
    debugL2_println(String(" sipHeaderR.auth...domain    ") + String(authenticate.domain.c_str()));
    authenticate.nonce = find("nonce=", in);
    debugL2_println(String(" sipHeaderR.auth...nonce     ") + String(authenticate.nonce.c_str()));
    authenticate.stale = find("stale=", in);
    debugL2_println(String(" sipHeaderR.auth...stale     ") + String(authenticate.stale.c_str()));
    authenticate.algorithm = find("algorithm=", in);
    debugL2_println(String(" sipHeaderR.auth...algorithm ") + String(authenticate.algorithm.c_str()));
    authenticate.qop = find("qop=", in);
    debugL2_println(String(" sipHeaderR.auth...qop       ") + String(authenticate.qop.c_str()));
}
void SipHeader::setVia(String in)
{
    via.port = in.substring(in.indexOf(":", 5) + 1, in.indexOf(";")).toInt();
    debugL2_println(String(" sipHeaderR.via.port         ") + String(via.port));
    via.branch = find("branch=", in);
    debugL2_println(String(" sipHeaderR.via.branch       ") + String(via.branch.c_str()));
    // via.userClient = in.substring(in.indexOf(" ", in.indexOf("Via: ") + 5) + 1, in.indexOf(":", in.indexOf("Via: ") + 5));
    debugL2_println(String(" sipHeaderR.via.userClient   ") + String(via.userClient.c_str()));
}
void SipHeader::setContentLength(String in)
{
    contentLength = in.substring(in.indexOf(": ") + 2).toInt();
    debugL2_println(String(" sipHeaderR.contentLength    ") + String(contentLength));
}
void SipHeader::setContact(String in)
{
    Gk gk;
    gk = parsGk(in);
    contact.telNr = gk.telNr;
    debugL2_println(String(" contact.telNr               ") + String(contact.telNr));
    contact.userClient = gk.ip;
    debugL2_println(String(" contact.userClient          ") + String(contact.userClient));
    contact.expires = find("expires=", in);
    debugL2_println(String(" contact.expires             ") + String(contact.expires));
}
void SipHeader::setInvite(String in)
{
    Gk gk;
    gk = parsGk(in);
    invite.telNr = gk.telNr;
    debugL2_println(String(" invite.telNr                ") + String(invite.telNr));
    invite.userAgent = gk.ip;
    debugL2_println(String(" invite.userAgent            ") + String(invite.userAgent));
    invite.port = gk.port;
    debugL2_println(String(" invite.port                 ") + String(invite.port));
}

