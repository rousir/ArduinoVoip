# ArduinoVoip
Arduino voip phone library, Use sip protocol tcp connection.

Arduino voip电话,sip客户端，esp32 voip，使用tcp连接，已完成接听拨打电话，收发消息等功能

使用i2s连接麦克风和喇叭，目前实现了RTP G.711A(PCMA)音频协议，音频8k采样率，16bit

ps：测试了下2.0.0以上的sdk，i2s驱动有改变，可以使用1.0.6的sdk

## 功能列表
* 拨打电话
* 接听电话
* 发送短信
* 接受短信

## 使用方法
1. 需要一个i2s麦克风和一个i2s DAC功放驱动喇叭，麦克风我用的inmp441，麦克风不能使用pdm输出的，pdm的麦克风esp32驱动和i2s输出有冲突。DAC用的MAX98357用其他替代也可以。这两种是淘宝常见的。

2. 安装一个sip server，可以使用任意sip协议的服务器，我使用的是[miniSIPServer](https://www.myvoipapp.com/)

3. 安装sip server后，添加几个分机，设置不同的号码

4. 电脑或者手机安装sip客户端，我使用的是[linphone](http://www.linphone.org/)

5. 修改代码中的sip分机号码
```
String telNr = "10013";
String serverIp = "192.168.31.21";

String user = "10013";
String pwd  = "1234";

SipMachine sipMachine = SipMachine(user, pwd, telNr, serverIp, serverIp);
```
6. 使用linphone拨打esp32 sip

# 接线
|ESP32| MIC I2S(INMP441)  |
| ------------ | ------------ |
|  GPIO26 | BCLK  |
|  GPIO27 | WS  |
|  GPIO34 | DIN  |

|ESP32| DAC I2S(MAX98357)  |
| ------------ | ------------ |
|  GPIO26 | BCLK  |
|  GPIO27 | WS  |
|  GPIO25 | DOUT  |

# 未完成的
还没有实现stun/rtun功能，还不能远程通讯

# 感谢
https://github.com/RetepRelleum/SipMachine 库基于RetepRelleum SipMachine库修改
