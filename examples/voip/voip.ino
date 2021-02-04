#include <Arduino.h>
#include "WiFi.h"

#include "SipMachine.h"
#include "Debug.h"

#include "driver/i2s.h"

const char *ssid = "xxx";            // your network SSID (name)
const char *password = "xxxxxxx"; // your network password (use for WPA, or use as key for WEP)

String telNr = "10013";
String serverIp = "192.168.2.21";
String serverAgent = "192.168.2.21:5060";

String user = "10013";
String pwd  = "1234";

SipMachine sipMachine = SipMachine(user, pwd, telNr, serverAgent, serverIp); //esp
SipMachine::Status status;

#define I2S_NUM_TXRX I2S_NUM_0

#define PIN_I2S_BCLK    26
#define PIN_I2S_LRC     27
#define PIN_I2S_DIN     34
#define PIN_I2S_DOUT    25

//m5stack core2
// #define PIN_I2S_BCLK    12
// #define PIN_I2S_LRC     0
// #define PIN_I2S_DIN     34
// #define PIN_I2S_DOUT    2

void I2S_Init(i2s_bits_per_sample_t BPS, i2s_port_t I2S_NUM, int rate) {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
        .sample_rate = rate,
        .bits_per_sample = BPS,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 256
    };

    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = PIN_I2S_BCLK;
    pin_config.ws_io_num = PIN_I2S_LRC;
    pin_config.data_in_num = PIN_I2S_DIN;
    pin_config.data_out_num = PIN_I2S_DOUT;

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);

    // i2s_set_clk(I2S_NUM, rate, BPS, I2S_CHANNEL_STEREO);
}

void wifiEvent(WiFiEvent_t event) {
    switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WIFISTA Connected");
        Serial.print("WIFISTA MAC: ");
        Serial.print(WiFi.macAddress());
        Serial.print(", IPv4: ");
        Serial.println(WiFi.localIP());
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WIFISTA Disconnected");
        WiFi.reconnect();
        break;
    default:
        break;
    }
}

void connectToNetwork()
{
    WiFi.onEvent(wifiEvent);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println(String("connection to WiFi ") + String(ssid));
    Serial.println(String("password: ") + String(password));
}

void sipMachineEvent(int code) {
    switch (code) {
    case SIPMACHINE_EVENT_REGISTER_OK://注册成功
        Serial.println("REGISTER OK");
        break;

    case SIPMACHINE_EVENT_BYE://对方挂断电话
        Serial.println(sipMachine.getTelNrIncomingCall() + String(" Bye Call"));
        break;

    case SIPMACHINE_EVENT_CANCEL://对方取消电话
        Serial.println(sipMachine.getTelNrIncomingCall() + String(" Cancel Call"));
        break;

    case SIPMACHINE_EVENT_INVITE_OK://对方接听电话
        Serial.println(sipMachine.getTelNrIncomingCall() + String(" Accept incoming Call"));
        break;

    case SIPMACHINE_EVENT_MESSAGE_OK://发送消息成功
        Serial.println( String("send Message to ") + sipMachine.getTelNrIncomingCall() + String(" Ok"));
        break;
    
    case SIPMACHINE_EVENT_NOT_EXIST://呼叫号码不存在
        Serial.println(sipMachine.getTelNrIncomingCall() + String(" Call Leg/Transaction Does Not Exist"));
        break;

    case SIPMACHINE_EVENT_BUSY_HERE://对方正忙
        Serial.println(sipMachine.getTelNrIncomingCall() + String(" Busy Here"));
        break;

    default:
        // Serial.println(String(code) + " does not Exists :-)");
        break;
    }
}

/**
 * 收到的pcm数据发送到喇叭
 */
void readSpeachPcmCb(int16_t pcm)
{
    int16_t sample[2];
    sample[0] = pcm;
    sample[1] = sample[0];
    i2s_write_bytes(I2S_NUM_0, (const char *)&sample, 4, 0xffff);
}

/**
 * 从i2s mic读取一个pcm数据，返回到数据包中
 */
int16_t writeSpeachPcmCb()
{
    int8_t pcm_buf[4];
    i2s_read_bytes(I2S_NUM_0, (char *)&pcm_buf, 4, 0xffff);
    return (int16_t)(pcm_buf[0] + pcm_buf[1] * 256);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    connectToNetwork();

    sipMachine.setup(WiFi.localIP().toString(), serverIp);
    sipMachine.setEvent(sipMachineEvent);

    I2S_Init(I2S_BITS_PER_SAMPLE_16BIT, I2S_NUM_TXRX, 8000);

    sipMachine.setReadSpeachPcmCallback(readSpeachPcmCb);
    sipMachine.setWriteSpeachPcmCallback(writeSpeachPcmCb);

}

unsigned long t2 = millis();
unsigned long t4 = micros();
unsigned long t5 = millis();

void loop()
{
    if (WiFi.status() == WL_CONNECTED) {

        sipMachine.loop(0);

        SipMachine::Status status = sipMachine.getStatus();

        switch (status)
        {
        //待机状态
        case SipMachine::idle:
            // if ((t2 + 5000 < millis()) & (t2 + 6000 > millis())) {
            //     //拨打电话
            //     sipMachine.makeCall("10012");
            // }
            break;

        //接到电话，响铃
        case SipMachine::ringIn:
            Serial.println(String("Ringing Call Nr. ") + sipMachine.getTelNrIncomingCall());

            delay(1000);

            Serial.println(String("Accept incoming Call ") + sipMachine.getTelNrIncomingCall());
            //接听电话
            sipMachine.acceptIncomingCall();
            break;

        //拨出电话，响铃
        case SipMachine::ringOut:
            break;

        //通话中
        case SipMachine::call:
            // 20s后挂断电话
            // if ((t5 + 20000) < millis())
            // {
            //     sipMachine.bye();
            //     Serial.printf("bye bye\n");
            // }
            break;

        //收到短信
        case SipMachine::messageIn:
            Serial.println(String("Received message Nr. ") + sipMachine.getTelNrIncomingCall());
            Serial.println(String("Message:\n") + sipMachine.getMessageInData());

            delay(1000);

            //回复消息
            sipMachine.sendMessage(sipMachine.getTelNrIncomingCall(), "Hello, I amnowsomethingis not,onewillcontact you.");
            break;

        default:
            ;    
            break;
        }
    }
}

