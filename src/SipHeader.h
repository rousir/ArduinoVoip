#ifndef SipHeader_H
#define SipHeader_H

#include <Arduino.h>

class SipHeader
{
private:
    struct Gk
    {
        String telNr;
        String ip;
        String port;
        String transport;
    };
    Gk parsGk(String in);
    String find(String x, String in);

public:
    struct Via
    {
        String userClient;
        int port;
        String branch;
    };
    struct To
    {
        String telNr;
        String userAgent;
        String tagTo;
    };

    struct From
    {
        String telNr;
        String userAgent;
        String tagFrom;
    };

    struct CSeq
    {
        int cSeq;
        String typ;
    };
    struct CallId
    {
        String callId;
    };
    struct Contact
    {
        String telNr;
        String userClient;
        String expires;
    };
    struct Authenticate
    {
        String realm;
        String domain;
        String nonce;
        String stale;
        String algorithm;
        String qop;
        String user;
        String uri;
        String cNonce;
        String response;
        String pwd;
        String types;
        String telNr;
        String agent;
        String nonceCount;
        int nc;
    };
    struct Invite
    {
        String telNr;
        String userAgent;
        String port;
    };

    Via via;
    To to;
    From from;
    CSeq cSeq;
    CallId callId;
    Contact contact;
    Authenticate authenticate;
    Invite invite;
    String proxyRegistrar;
    int responseCodes;
    int contentLength;
    void  parse(String in);
    String getVia();
    String getMaxForwards();
    String getTo();
    String getFrom();
    String getCallID();
    String getCSeq();
    String getContact();
    String getAllow();
    String getContentLength(int contentLength = 0);
    String getContenType();
    String getAuthorisation();

    void setResposeCode(String in);
    void setFrom(String in);
    void setTo(String in);
    void setCallId(String in);
    void setCseq(String in);
    void setAuthenticate(String in);
    void setVia(String in);
    void setContentLength(String in);
    void setContact(String in);
    void setInvite(String in);
};

#endif
