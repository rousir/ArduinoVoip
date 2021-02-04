#include "SipMachine.h"
#include "Debug.h"

/**
 * [SipMachine::SipMachine sip类]
 *
 * @DateTime 2021-01-20
 * @param    user 用户名
 * @param    pwd 密码
 * @param    telNr 本机号码
 * @param    userAgent 本机代理
 * @param    proxyRegistrar sip服务地址
 * @param    port sip服务器端口
 */
SipMachine::SipMachine(String user, String pwd, String telNr, String userAgent, String proxyRegistrar, int port)
{
    this->user = user;
    this->pwd = pwd;
    this->telNr = telNr;
    this->userAgent = userAgent;
    this->userClient = userClient;
    this->proxyRegistrar = proxyRegistrar;
    this->port = port;
    timeStOut = micros();
    timeStOut = timeStOut / 20000;
    timeStOut = timeStOut * 20000;
}

/**
 * [SipMachine::setup 设置本机地址和服务地址]
 *
 * @DateTime 2021-01-20
 * @param    userClient 本机ip地址
 * @param    proxyServer 服务地址
 */
void SipMachine::setup(String userClient, String proxyServer)
{
    this->proxyServer = proxyServer;
    this->userClient = userClient;

    debug_println(String(" Connect SIP Server ") + " Port " + String(port));

    udpIpWrite = strToIP(proxyServer);
}

/**
 * [SipMachine::strToIP ip字符串转ip]
 *
 * @DateTime 2021-01-20
 * @param    str ip字符串
 * @return
 */
IPAddress SipMachine::strToIP(String str)
{
    byte adr[4];
    adr[0] = str.substring(0, str.indexOf('.')).toInt();
    str = str.substring(str.indexOf('.') + 1);
    adr[1] = str.substring(0, str.indexOf('.')).toInt();
    str = str.substring(str.indexOf('.') + 1);
    adr[2] = str.substring(0, str.indexOf('.')).toInt();
    str = str.substring(str.indexOf('.') + 1);
    adr[3] = str.toInt();
    IPAddress ip(adr[0], adr[1], adr[2], adr[3]);
    return ip;
}

/**
 * [SipMachine::setEvent 设置事件回调]
 *
 * @DateTime 2021-01-20
 * @param    event 回调函数
 */
void SipMachine::setEvent(callbackEvent event)
{
    this->event_cb = event;
}

/**
 * [SipMachine::setReadSpeachPcmCallback 设置读取到音频数据回调]
 *
 * @DateTime 2021-01-22
 * @param    cb
 */
void SipMachine::setReadSpeachPcmCallback(readSpeachPcmCallback cb)
{
    read_speach_pcm_cb = cb;
}

/**
 * [SipMachine::setWriteSpeachPcmCallback 设置发送音频数据回调]
 *
 * @DateTime 2021-01-22
 * @param    cb
 */
void SipMachine::setWriteSpeachPcmCallback(writeSpeachPcmCallback cb)
{
    write_speach_pcm_cb = cb;
}

/**
 * [SipMachine::loop sip类客户端主循环]
 *
 * @DateTime 2021-01-20
 * @param    pcmOut pcm数据输入
 * @return
 */
int16_t SipMachine::loop(short pcmOut)
{
    int ret;

    // write Data to outputbuffer
    // if (status == call)
    // {
    //     rtpOut.put(pcmOut);
    // }

    if (timeStOut < micros())
    {
        timeStOut += 10000;

        //每隔20ms取一帧数据
        if ((status == call) & (timeStOut % 20000 == 0))
        {
            ret = udp.parsePacket();
            debugL2_println(String("Nr. of parsePacket: ") + String(ret) + String(" Byte"));
            if (ret)
            {
                if (ret == 16)
                {
                    getDtmfData();
                }
                else
                {
                    //取得音频数据
                    getSpeachData();
                }
            }
        }
        else
        {
            switch (status)
            {
            case init:
                if (timeExpires < millis())
                {
                    sipRegister();
                    status = reg;
                    timeExpires = millis() + 5000;
                }
                break;
            case reg:
                if (timeExpires < millis())
                {
                    sock_sip.stop();
                    debug_println(String(" Stop  SIP Client on IP ") + proxyServer + " Port " + String(port));
                    status = init;
                    timeExpires = millis() + 5000;
                }
                break;
            case idle:
                // if ((timeExpires - millis() < 0))
                //     status = init;
                break;
            case call:
                //发送音频数据
                writeSpeachData();
                break;
            case ringIn:
                break;
            case ringOut:
                break;
            case messageIn:
                status = idle;
                break;
            default:
                break;
            }
            if (sock_sip.connected())
            {
                if (sock_sip.available())
                {
                    String str = sock_sip.readStringUntil('\n'); //terminator, buffer, sizeof(buffer));

                    if (sipHeader.contentLength > 0)
                    {
                        sipHeader.contentLength -= (str.length() + 1);
                        debugL1_println(str.c_str());
                        parserSdp(str);

                        if (sipHeader.contentLength < 0)
                        {
                            exec();

                            if (status == messageIn) {
                                debugL1_println(str.c_str());
                                messageData = str;
                            }
                        }
                    }
                    else
                    {
                        debugL1_print("-_ ");
                        debugL1_println(str.c_str());
                        parserSip(str);
                        if (str.equals("\r") || str.equals("\n")) {
                            exec();
                        }
                    }
                }
            }
            else
            {
                status = init;
                timeExpires -= 5000;
                debug_println("sip server disconnected, reconnect...");
            }
        }
    }

    // return rtpIn.get((micros() % 20000) / 125);
    //20ms 一帧 160个样本 8000hz 125us一个样本

    return 0;
}

/**
 * [SipMachine::parserSdp 处理SDP协议数据]
 *
 * @DateTime 2021-01-20
 * @param    in 待处理的SDP协议数据
 */
void SipMachine::parserSdp(String in)
{
    sdpHeader.parse(in);
}

/**
 * [SipMachine::parserSip 处理SIP协议数据]
 *
 * @DateTime 2021-01-20
 * @param    in 待处理的SIP协议数据
 */
void SipMachine::parserSip(String in)
{
    sipHeader.parse(in);
}

/**
 * [SipMachine::exec 解析sip数据后的相关处理]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::exec()
{
    int ret;
    if (!sipHeader.to.telNr.equals(telNr))
    {
        tagTo = sipHeader.to.tagTo;
    }
    else
    {
        tagFrom = sipHeader.to.tagTo;
    }

    if (sipHeader.from.telNr.equals(telNr))
    {
        tagFrom = sipHeader.from.tagFrom;
    }
    else
    {
        tagTo = sipHeader.from.tagFrom;
    }

    switch (sipHeader.responseCodes)
    {
    case 0:
        debug_println("Execute ACK");
        ret = udp.begin(sdpHeader.udpPortRead);
        debug_println(String("Start UDP") + ((ret == 1) ? "TRUE" : "FALSE") + " on Port " + sdpHeader.udpPortRead);
        status = call;
        if (event_cb) event_cb(SIPMACHINE_EVENT_ACK);
        break;
    case 1:
        debug_println("Execute BYE");
        sipOk();
        udp.stop();
        debug_println(String("Stop UDP read "));
        status = idle;
        if (event_cb) event_cb(SIPMACHINE_EVENT_BYE);
        break;
    case 2:
        debug_println("Execute INVITE");
        sdpHeader.udpPortWrite = sdpHeader.m.port.toInt();

        //听取声音端口为
        sdpHeader.udpPortRead = sdpHeader.udpPortWrite + 10;
        sipRinging();
        status = ringIn;
        if (event_cb) event_cb(SIPMACHINE_EVENT_INVITE);
        break;
    case 3:
        debug_println("Execute CANCEL");
        sipOk();
        status = idle;
        if (event_cb) event_cb(SIPMACHINE_EVENT_CANCEL);
        break;
    case 4:
        debug_println("Execute MESSAGE");
        sipOk();
        status = messageIn;
        if (event_cb) event_cb(SIPMACHINE_EVENT_MESSAGE);
        break;
    case 100:
        debug_println("Execute 100 Trying");
        break;
    case 180:
        debug_println("Execute 180 Ringing");
        status = ringOut;
        if (event_cb) event_cb(SIPMACHINE_EVENT_RINGING);
        break;
    case 200:
        if (sipHeader.cSeq.typ.equals("REGISTER"))
        {
            debug_println("Execute 200 OK Register");
            timeExpires += sipHeader.contact.expires.toInt() * 1000;
            status = idle;
            if (event_cb) event_cb(SIPMACHINE_EVENT_REGISTER_OK);
        }
        else if (sipHeader.cSeq.typ.equals("INVITE"))
        {
            debug_println("Execute 200 OK INVITE");
            sdpHeader.udpPortWrite = sdpHeader.m.port.toInt();
            sipAck();
            status = call;
            udpIpWrite = strToIP(sdpHeader.o.municastAddress);
            if (event_cb) event_cb(SIPMACHINE_EVENT_INVITE_OK);
        }
        else if (sipHeader.cSeq.typ.equals("MESSAGE"))
        {
            debug_println("Execute 200 OK MESSAGE");
            timeExpires += sipHeader.contact.expires.toInt() * 1000;
            status = idle;
            if (event_cb) event_cb(SIPMACHINE_EVENT_MESSAGE_OK);
        }
        break;
    case 401:
        debug_println("Execute 401 sipRegisterAuth");
        sipRegisterAuth();
        if (event_cb) event_cb(SIPMACHINE_EVENT_REGISTER_AUTH);
        break;
    case 403:
        debug_println("Execute 403 Forbidden");
        // sock_sip.stop();
        // debug_println(String("sock_sip.stop"));
        udp.stop();
        debug_println(String("Stop UDP read and Write"));
        if (event_cb) event_cb(sipHeader.responseCodes);
        break;
    case 404:
        debug_println("Execute 404 Not Found");
        // sock_sip.stop();
        // debug_println(String("sock_sip.stop"));
        udp.stop();
        debug_println(String("Stop UDP read  and Write"));
        if (event_cb) event_cb(SIPMACHINE_EVENT_NOT_FOUND);
        break;
    case 407:
        debug_println("Execute 407 Proxy Authentication Required");
        sipAck();
        sipAuth();
        ret = udp.begin(sdpHeader.udpPortRead);
        debug_println(String("Start UDP read ") + ((ret == 1) ? "TRUE" : "FALSE") + " on Port " + sdpHeader.udpPortRead);
        if (event_cb) event_cb(sipHeader.responseCodes);
        break;
    case 481:
        debug_println(" Execute 481 Call Leg/Transaction Does Not Exist ");
        if (event_cb) event_cb(SIPMACHINE_EVENT_NOT_EXIST);
        break;
    case 486:
        debug_println("Execute 486 Busy Here");
        // sock_sip.stop();
        // debug_println(String("sock_sip.stop"));
        udp.stop();
        debug_println(String("Stop UDP read "));
        if (event_cb) event_cb(SIPMACHINE_EVENT_BUSY_HERE);
        break;
    default:
        debug_println(String(sipHeader.responseCodes) + " does not Exists :-)");
        break;
    }
}

/**
 * [SipMachine::writeSIPdata 发送数据到SIP服务器]
 *
 * @DateTime 2021-01-20
 * @param    message
 */
void SipMachine::writeSIPdata(String message)
{
    size_t ret = sock_sip.println(message);
    debugL1_println(message.c_str());
    if (ret < 0) {
        debugL1_println(ret);
    }
}

/**
 * [SipMachine::sipRegister 连接sip服务器， 发送sip注册包]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipRegister()
{
    sock_sip = WiFiClient();
    int ret = sock_sip.connect(proxyServer.c_str(), port);

    debug_println(String(" Connect SIP Client  on IP ") + proxyServer + " Port " + String(port) + " ret= " + (ret ? "true" : "false"));

    if (ret) {
        sipHeader.proxyRegistrar = proxyRegistrar;
        sipHeader.via.branch = branch;
        sipHeader.via.port = port;
        sipHeader.via.userClient = userClient;
        sipHeader.from.tagFrom = tagFrom;
        sipHeader.from.telNr = telNr;
        sipHeader.from.userAgent = userAgent;
        sipHeader.to.tagTo = tagTo;
        sipHeader.to.telNr = telNr;
        sipHeader.to.userAgent = userAgent;
        sipHeader.callId.callId = callId + "@" + userAgent;
        sipHeader.contact.telNr = telNr;
        sipHeader.contact.userClient = userClient;
        sipHeader.cSeq.cSeq = 0;
        sipHeader.cSeq.typ = "REGISTER";
        debug_println("*****  Register  *****");
        String str;
        str = "REGISTER sip:";
        str += sipHeader.proxyRegistrar;
        str += " SIP/2.0\r\n";
        str += sipHeader.getVia() + "\r\n";
        str += sipHeader.getMaxForwards() + "\r\n";
        str += sipHeader.getFrom() + "\r\n";
        str += sipHeader.getTo() + "\r\n";
        str += sipHeader.getCallID() + "\r\n";
        str += sipHeader.getCSeq() + "\r\n";
        str += "User-Agent: esp32 sip\r\n";

        str += sipHeader.getContact() + "\r\n";
        str += sipHeader.getAllow() + "\r\n";
        str += sipHeader.getContentLength() + "\r\n";

        writeSIPdata(str);
    }
}

/**
 * [SipMachine::sipRinging 回应sip Ringing响铃]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipRinging()
{
    debug_println("*****  Ringing *****");
    sipHeader.to.tagTo = randomChr(30);
    sipHeader.contact.telNr = telNr;
    sipHeader.contact.userClient = userClient;

    String str;
    str = "SIP/2.0 180 Ringing";
    str += "\r\n";
    str += sipHeader.getVia() + ";received=" + userClient + "\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";
    str += sipHeader.getContact() + "\r\n";
    str += sipHeader.getContentLength() + "\r\n";

    writeSIPdata(str);
}

/**
 * [SipMachine::sipOk 回复SIP OK]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipOk()
{
    debug_println("*****  Ok *****");
    sdpHeader.o.sessId = String(sdpHeader.o.sessId.toInt() + 1);
    sdpHeader.o.sessVersion = String(sdpHeader.o.sessVersion.toInt() + 1);
    sdpHeader.o.municastAddress = userClient;

    sdpHeader.c.connectionAddress = userClient;

    sdpHeader.m.port = String(sdpHeader.udpPortRead);
    sdpHeader.m.fmt = String("8 127");

    sdpHeader.o.username = telNr;
    sdpHeader.s = "Phone Call";

    sipHeader.contact.telNr = telNr;
    sipHeader.contact.userClient = userClient;

    String str;
    str = "SIP/2.0 200 OK";

    str += "\r\n";
    str += sipHeader.getVia() + "\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";
    str += sipHeader.getContact() + "\r\n";

    if (sipHeader.cSeq.typ.equals("INVITE"))
    {
        str += sipHeader.getContenType() + "\r\n";
        str += sipHeader.getContentLength(sdpHeader.getContent().length() + 2) + "\r\n";
        str += "\r\n";
        str += sdpHeader.getContent() + "\r\n";
    }
    else
    {
        str += sipHeader.getContentLength() + "\r\n";
    }

    writeSIPdata(str);
}

/**
 * [SipMachine::sipInvite 呼叫sip分机]
 *
 * @DateTime 2021-01-20
 * @param    telNrTo 分机号码
 */
void SipMachine::sipInvite(String telNrTo)
{
    debug_println("*****  Invite *****");

    sdpHeader.o.sessId = String(random(90000000));
    sdpHeader.o.sessVersion = String(random(90000000));
    sdpHeader.o.municastAddress = userClient;
    sdpHeader.c.connectionAddress = userClient;
    sdpHeader.m.port = String(random(10000, 90000));//随机生成本机接收声音端口
    sdpHeader.udpPortRead = sdpHeader.m.port.toInt();
    sdpHeader.m.fmt = String("8 127");

    sdpHeader.o.username = telNr;
    sdpHeader.s = "esp32 sip";

    sipHeader.via.branch = branch;
    sipHeader.via.port = 0;
    sipHeader.via.userClient = proxyServer;

    sipHeader.from.tagFrom = tagFrom;
    sipHeader.from.telNr = telNr;
    sipHeader.from.userAgent = userClient;

    sipHeader.to.telNr = telNrTo;
    sipHeader.to.userAgent = proxyServer;
    sipHeader.to.tagTo = "";

    sipHeader.callId.callId = String(randomChr(7)) + "@" + proxyServer;
    sipHeader.cSeq.cSeq = 1;
    sipHeader.cSeq.typ = "INVITE";
    authType = "INVITE";
    sipHeader.contact.telNr = telNr;
    sipHeader.contact.userClient = userClient;

    String str;
    str = "INVITE sip:";
    str += telNrTo;
    str += "@";
    str += proxyServer;
    str += " SIP/2.0";

    str += "\r\n";
    str += sipHeader.getVia() + ";rport;alias\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";
    str += sipHeader.getContact() + "\r\n";

    str += "User-Agent: esp32 sip\r\n";
    str += "Supported: replaces\r\n";
    str += "Allow: ACK,PRACK,BYE,CANCEL,INVITE,UPDATE,MESSAGE,INFO,OPTIONS,SUBSCRIBE,NOTIFY,REFER\r\n";
    str += "Allow-Events: presence,refer,telephone-event,keep-alive,dialog\r\n";
    str += "Accept: application/sdp,application/dtmf-relay,text/plain\r\n";

    str += sipHeader.getContenType() + "\r\n";
    str += sipHeader.getContentLength(sdpHeader.getContent().length() + 2) + "\r\n";
    str += "\r\n";
    str += sdpHeader.getContent() + "\r\n";
    writeSIPdata(str);
}

/**
 * [SipMachine::sipAck sip ack数据包，通常在呼叫搜到对方响应后回应]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipAck()
{
    debug_println("*****  ACK *****");
    sipHeader.cSeq.typ = "ACK";
    String str;
    str = "ACK sip:";
    str += telNrTo;
    str += "@";
    str += userAgent;
    str += " SIP/2.0";

    str += "\r\n";
    str += sipHeader.getVia() + "\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";
    str += sipHeader.getContact() + "\r\n";
    str += sipHeader.getContentLength() + "\r\n";
    writeSIPdata(str);
}

/**
 * [SipMachine::sipBye sip挂断通话]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipBye()
{
    debug_println("*****  BYE *****");

    branch = "z9hG4bK-" + randomChr(30);
    sipHeader.via.branch = branch;
    sipHeader.cSeq.typ = "BYE";
    authType = "BYE";
    sipHeader.cSeq.cSeq++;
    sipHeader.from.tagFrom = tagFrom;
    sipHeader.from.telNr = telNr;
    sipHeader.from.userAgent = userAgent;
    sipHeader.to.telNr = telNrTo;
    sipHeader.to.userAgent = userAgent;
    sipHeader.to.tagTo = tagTo;
    String str;
    str = "BYE sip:";
    str += telNrTo;
    str += "@";
    str += userAgent;
    str += " SIP/2.0";

    str += "\r\n";
    str += sipHeader.getVia() + "\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";
    str += sipHeader.getContentLength() + "\r\n";
    writeSIPdata(str);

    debug_println("*****  BYE *****");
}

/**
 * [SipMachine::sipAuth 搜到480请求重新验证]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipAuth()
{
    debug_print("***** ");
    debug_print(authType);
    debug_println("in Auth *****");
    sipHeader.cSeq.cSeq++;
    sipHeader.cSeq.typ = authType;
    sipHeader.authenticate.types = authType;
    if (authType.equals("INVITE"))
    {
        sipHeader.authenticate.telNr = telNrTo;
        sipHeader.authenticate.agent = userAgent;
        sipHeader.to.tagTo = "";
        sipHeader.contact.telNr = telNr;
        sipHeader.contact.userClient = userClient;
    }
    String str;
    str = authType;
    str += " sip:";
    str += telNrTo;
    str += "@";
    str += userAgent;
    str += " SIP/2.0";

    str += "\r\n";
    str += sipHeader.getVia() + "\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";
    str += sipHeader.getContact() + "\r\n";
    str += sipHeader.getAuthorisation() + "\r\n";

    if (authType.equals("INVITE"))
    {
        str += sipHeader.getContenType() + "\r\n";
        str += sipHeader.getContentLength(sdpHeader.getContent().length() + 2) + "\r\n";
        str += "\r\n";
        str += sdpHeader.getContent() + "\r\n";
        status = ringOut;
    }
    else
    {
        str += sipHeader.getContentLength() + "\r\n";
        status = idle;
    }
    writeSIPdata(str);
}

/**
 * [SipMachine::sipRegisterAuth 连接sipserver后回应401请求]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipRegisterAuth()
{
    sipHeader.cSeq.cSeq++;
    sipHeader.cSeq.typ = "REGISTER";
    sipHeader.to.tagTo = "";
    sipHeader.authenticate.user = user;
    sipHeader.authenticate.agent = proxyRegistrar;
    sipHeader.authenticate.pwd = pwd;
    sipHeader.authenticate.types = "REGISTER";
    sipHeader.authenticate.telNr = "";
    sipHeader.authenticate.nonceCount = "00000001";

    if (sipHeader.cSeq.cSeq < 3)
    {
        debug_println("*****  Register Auth *****");
        String str;
        str = "REGISTER sip:";
        str += sipHeader.proxyRegistrar;
        str += " SIP/2.0";

        str += "\r\n";
        str += sipHeader.getVia() + "\r\n";
        str += sipHeader.getTo() + "\r\n";
        str += sipHeader.getFrom() + "\r\n";
        str += sipHeader.getCallID() + "\r\n";
        str += sipHeader.getCSeq() + "\r\n";
        str += sipHeader.getMaxForwards() + "\r\n";
        str += "Expires: 120\r\n";
        str += sipHeader.getAuthorisation() + "\r\n";
        str += sipHeader.getContact() + "\r\n";
        str += "User-Agent: esp32 sip\r\n";
        str += "Supported: replaces\r\n";
        str += sipHeader.getAllow() + "\r\n";
        str += "Allow-Events: presence,refer,telephone-event,keep-alive,dialog\r\n";
        str += "Accept: application/sdp,application/dtmf-relay,text/plain\r\n";
        str += sipHeader.getContentLength() + "\r\n";

        writeSIPdata(str);
        debugL1_println("*****  Register  Auth out *****");
    }
}

/**
 * [SipMachine::sipSendMessage 发送短信到其他分机]
 *
 * @DateTime 2021-01-20
 * @param    telNrTo 分机号
 * @param    message 发送的消息
 */
void SipMachine::sipSendMessage(String telNrTo, String message)
{
    debug_println("*****  Message *****");

    sipHeader.via.branch = branch;
    sipHeader.via.port = 0;
    sipHeader.via.userClient = proxyServer;

    sipHeader.from.tagFrom = tagFrom;
    sipHeader.from.telNr = telNr;
    sipHeader.from.userAgent = userClient;

    sipHeader.to.telNr = telNrTo;
    sipHeader.to.userAgent = proxyServer;
    sipHeader.to.tagTo = "";

    sipHeader.callId.callId = String(randomChr(7)) + "@" + proxyServer;
    sipHeader.cSeq.cSeq = 1;
    sipHeader.cSeq.typ = "MESSAGE";

    authType = "MESSAGE";

    String str;
    str = "MESSAGE sip:";
    str += telNrTo;
    str += "@";
    str += proxyServer;
    str += " SIP/2.0";

    str += "\r\n";
    str += sipHeader.getVia() + ";rport\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";

    str += "User-Agent: esp32 sip\r\n";
    str += "Content-Type: text/plain\r\n";
    str += sipHeader.getContentLength(message.length() + 2) + "\r\n";
    str += "\r\n";
    str += message + "\r\n";

    writeSIPdata(str);
}

/**
 * [SipMachine::sipCancel 发送取消命令]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::sipCancel()
{
    debug_println("*****  CANCEL *****");

    sipHeader.via.branch = branch;
    sipHeader.via.port = 0;
    sipHeader.via.userClient = proxyServer;

    sipHeader.from.tagFrom = tagFrom;
    sipHeader.from.telNr = telNr;
    sipHeader.from.userAgent = userClient;

    sipHeader.to.telNr = telNrTo;
    sipHeader.to.userAgent = proxyServer;
    sipHeader.to.tagTo = "";

    sipHeader.callId.callId = String(randomChr(7)) + "@" + proxyServer;
    sipHeader.cSeq.cSeq = 1;
    sipHeader.cSeq.typ = "CANCEL";

    authType = "CANCEL";

    String str;
    str = "CANCEL sip:";
    str += telNrTo;
    str += "@";
    str += proxyServer;
    str += " SIP/2.0";

    str += "\r\n";
    str += sipHeader.getVia() + ";rport\r\n";
    str += sipHeader.getTo() + "\r\n";
    str += sipHeader.getFrom() + "\r\n";
    str += sipHeader.getCallID() + "\r\n";
    str += sipHeader.getCSeq() + "\r\n";

    str += "User-Agent: esp32 sip\r\n";
    str += "Content-Type: text/plain\r\n";
    str += sipHeader.getContentLength(0) + "\r\n";

    writeSIPdata(str);
}

/**
 * [SipMachine::randomChr 生成随机字符串0-9a-z]
 *
 * @DateTime 2021-01-20
 * @param    size 生成的字符串长度
 * @return
 */
String SipMachine::randomChr(int size)
{
    String ret = "";
    for (int i = 0; i < size; i++)
    {
        if (i % 3 == 0)
        {
            ret += (char)random(48, 57);
        }
        else if (i % 3 == 1)
        {
            ret += (char)random(65, 90);
        }
        else if (i % 3 == 2)
        {
            ret += (char)random(97, 122);
        }
    }
    return ret;
}

/**
 * [SipMachine::getTelNrIncomingCall 获取呼入的号码]
 *
 * @DateTime 2021-01-20
 * @return 呼入的号码
 */
String SipMachine::getTelNrIncomingCall()
{
    return sipHeader.from.telNr;
}

/**
 * [SipMachine::acceptIncomingCall 接听呼叫]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::acceptIncomingCall()
{
    sipOk();
    status = callAccept;
}

/**
 * [SipMachine::makeCall 拨打电话]
 *
 * @DateTime 2021-01-20
 * @param    telNrTo 拨打的分机号
 */
void SipMachine::makeCall(String telNrTo)
{
    // this->telNrTo = telNrTo;
    sipInvite(telNrTo);
    status = ringOut;
}

/**
 * [SipMachine::cancelCall 取消呼叫]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::cancelCall()
{
    sipCancel();
    status = idle;
}

/**
 * [SipMachine::bye 挂断电话]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::bye()
{
    sipBye();
    status = idle;
}

/**
 * [SipMachine::sendMessage 发送消息]
 *
 * @DateTime 2021-01-20
 * @param    telNrTo 收信号码
 * @param    message 发送的消息
 */
void SipMachine::sendMessage(String telNrTo, String message)
{
    sipSendMessage(telNrTo, message);
    status = messageOut;
}

/**
 * [SipMachine::getKeyPressedLast20 获取drmf]
 *
 * @DateTime 2021-01-20
 * @return dtmf数据
 */
String SipMachine::getKeyPressedLast20()
{
    return dtmf;
}

/**
 * [SipMachine::getStatus 获取当前状态]
 *
 * @DateTime 2021-01-20
 * @return 当前状态
 */
SipMachine::Status SipMachine::getStatus()
{
    return status;
}

/**
 * [SipMachine::getDtmfData 解析读取到的dtmf数据帧]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::getDtmfData()
{
    if (udp.available())
    {
        int ret = udp.readBytes((uint8_t *) & (rtpIn.dtmfBuffer), sizeof(rtpIn.dtmfBuffer));
        debugL1_print(String("UDP DTMF read count ") + String(ret));
        debugL1_print(String("          Sequence Number : ") + String(rtpIn.getSequenceNumber()));
        debugL1_print(String(" Timestamp: ") + String(rtpIn.getTimestamp()));
        debugL1_print(String(" event ") + String(rtpIn.dtmfBuffer.event));
        debugL1_print(String(" E ") + String(rtpIn.dtmfBuffer.e));
        debugL1_print(String(" R ") + String(rtpIn.dtmfBuffer.r));
        debugL1_print(String(" volume ") + String(rtpIn.dtmfBuffer.volume));
        debugL1_println(String(" duration ") + String(rtpIn.dtmfBuffer.duration));
        if (rtpIn.dtmfBuffer.e == 1)
        {
            if (dtmf.length() > 20)
                dtmf = dtmf.substring(1);
            String s = String(rtpIn.dtmfBuffer.event, 10);
            if (s.equals("10"))
                s = "*";
            if (s.equals("11"))
                s = "#";
            dtmf += s;
        }
    }
}

/**
 * [SipMachine::getSpeachData 解析收到的音频数据]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::getSpeachData()
{
    if (udp.available())
    {
        int ret = udp.readBytes((uint8_t *) & (rtpIn.rtpBuffer), sizeof(rtpIn.rtpBuffer));

        for (int i = 0; i < 160; ++i)
        {
            int16_t pcm = rtpIn.alaw_decode(rtpIn.rtpBuffer.b[i]);
            if (read_speach_pcm_cb) {
                read_speach_pcm_cb(pcm);
            }
        }

        debugL2_print(String("UDP read count ") + String(ret));
        debugL2_print(String(" Byte      Sequence Number : ") + String(rtpIn.getSequenceNumber()));
        debugL2_println(String(" Timestamp: ") + String(rtpIn.getTimestamp()));

        if (((rtpIn.getSequenceNumber() % 5) == 0))
        {
            char dtm = rtpIn.getDtmf();
            if (dtm != ' ')
            {
                if (dtmf.length() > 20)
                    dtmf = dtmf.substring(1);
                dtmf += String(dtm);
            }
        }
    }
}

/**
 * [SipMachine::writeSpeachData 发送一帧音频数据]
 *
 * @DateTime 2021-01-20
 */
void SipMachine::writeSpeachData()
{
    int16_t pcm = 0;
    int8_t pcm_buf[4];

    for (int i = 0; i < 160; ++i)
    {
        if (write_speach_pcm_cb) {
            pcm = write_speach_pcm_cb();
        }
        rtpOut.rtpBuffer.b[i] = rtpIn.alaw_encode(pcm);
    }

    int ret = 0;
    ret = udp.beginPacket(udpIpWrite, sdpHeader.udpPortWrite);
    debugL2_print(String("UDP write ") + ((ret == 1) ? "TRUE" : "FALSE"));
    rtpOut.setSequenceNumber(rtpOut.getSequenceNumber() + 1);
    rtpOut.setTimestamp(rtpOut.getTimestamp() + 160);
    ret = udp.write((uint8_t *)&rtpOut.rtpBuffer, sizeof(rtpOut.rtpBuffer));
    debugL2_print(String(" count  ") + String(ret));
    ret = udp.endPacket();
    debugL2_print(String("  ") + ((ret == 1) ? "TRUE " : "FALSE "));
    rtpOut.rtpPos = 0;
    debugL2_print(String(" Sequence Number : ") + String(rtpOut.getSequenceNumber()));
    debugL2_print(String(" Timestamp: ") + String(rtpOut.getTimestamp()));
    debugL2_print(String(" ADDR: ") + udpIpWrite.toString() + " on Port " + (sdpHeader.udpPortWrite));
    debugL2_println("");
}

/**
 * [SipMachine::getMessageInData 获取收到的信息]
 *
 * @DateTime 2021-01-20
 * @return 收到的信息
 */
String SipMachine::getMessageInData() {
    return messageData;
}
