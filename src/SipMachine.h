#ifndef SipMachine_H
#define SipMachine_H

#include <Arduino.h>
#include "WiFiClient.h"
#include "WiFiUdp.h"

#include "SipHeader.h"
#include "SdpHeader.h"
#include "RTP.h"
#include "SipStreamIn.h"

#include "driver/i2s.h"

#define SIPMACHINE_EVENT_REGISTER_AUTH 0x01
#define SIPMACHINE_EVENT_ACK          0x02
#define SIPMACHINE_EVENT_CANCEL       0x03
#define SIPMACHINE_EVENT_MESSAGE      0x04
#define SIPMACHINE_EVENT_INVITE       0x05
#define SIPMACHINE_EVENT_BYE          0x06
#define SIPMACHINE_EVENT_RINGING      0x07
#define SIPMACHINE_EVENT_REGISTER_OK  0x08
#define SIPMACHINE_EVENT_INVITE_OK    0x09
#define SIPMACHINE_EVENT_MESSAGE_OK   0x0A
#define SIPMACHINE_EVENT_NOT_FOUND    0x0B
#define SIPMACHINE_EVENT_NOT_EXIST    0x0C
#define SIPMACHINE_EVENT_BUSY_HERE    0x0D

extern "C" {
  typedef void (*callbackEvent)(int status);
  typedef void (*readSpeachPcmCallback)(int16_t pcm);
  typedef int16_t (*writeSpeachPcmCallback)();
}

class SipMachine
{
public:
    typedef enum
    {
        init,
        reg,
        idle,
        ringIn,
        ringOut,
        callAccept,
        call,
        messageIn,
        messageOut,
    } Status;

    SipMachine(String user, String pwd, String telNr, String userAgent, String proxyRegistrar, int port = 5060);

    void setup(String userClient, String proxyServer);
    int16_t loop(int16_t pcmOut);

    void setEvent(callbackEvent event);

    String getTelNrIncomingCall();//来电号码
    void acceptIncomingCall();//接听来电

    void makeCall(String telNrTo);//拨打电话
    void cancelCall();//响铃中,主动取消电话

    void bye(); //通话中挂断电话

    String getKeyPressedLast20();
    Status getStatus();

    String getMessageInData();
    void sendMessage(String telNrTo, String message);
    
    void setReadSpeachPcmCallback(readSpeachPcmCallback cb);
    void setWriteSpeachPcmCallback(writeSpeachPcmCallback cb);
    
private:
    unsigned long timeExpires = 0;
    unsigned long timeStOut=0;

    String user;
    String pwd;
    String telNr;
    String telNrTo;
    String userAgent;
    String userClient;
    String proxyServer;
    String proxyRegistrar;
    String tagTo;
    String authType="";

    int port = 5060;

    WiFiClient sock_sip;
    WiFiUDP udp;

    RTP rtpIn;
    RTP rtpOut;

    String branch = "z9hG4bK-" + randomChr(30);
    String tagFrom = randomChr(30);
    String callId = randomChr(7);

    String randomChr(int size);

    void parserSip(String in);
    void parserSdp(String in);
    void exec();

    void writeSIPdata(String message);

    void sipRegister();
    void sipRegisterAuth();
    void sipRinging();
    void sipOk();
    void sipInvite(String telNrTo);
    void sipAuth();
    void sipAck();
    void sipBye();
    void sipSendMessage(String telNrTo, String message);
    void sipCancel();
    
    void getDtmfData();
    void getSpeachData();
    void writeSpeachData();

    IPAddress strToIP(String str);

    SipHeader sipHeader;
    SdpHeader sdpHeader;

    String dtmf;

    IPAddress udpIpWrite;

    String messageData;

    callbackEvent event_cb;
    readSpeachPcmCallback read_speach_pcm_cb;
    writeSpeachPcmCallback write_speach_pcm_cb;

protected:
    Status status = init;
};

#endif
