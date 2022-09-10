#include <Arduino.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <avr/sleep.h>
#include <avr/power.h>

#define PB_INVALID (PB5 + 1)

/**
 * nRF24L01 instance
 *                              +-\/-+
 *                NC      PB5  1|o   |8  Vcc --- nRF24L01  VCC, pin2 --- LED --- 5V
 * nRF24L01  CE, pin3 -x- PB3  2|    |7  PB2 --- nRF24L01  SCK, pin5
 * nRF24L01 CSN, pin4 --- PB4  3|    |6  PB1 --- nRF24L01 MOSI, pin6
 * nRF24L01 GND, pin1 --- GND  4|    |5  PB0 --- nRF24L01 MISO, pin7
 *                              +----+
 * 
 * How to transmit:
 *  1. Use the same CRC configuration
 *  2. Set the PRIM_RX bit to 0
 *  3. Set the Auto Retransmit Count to 0 to disable the auto retransmit functionally
 *  4. Use the same address width
 *  5. Use the same frequency channel
 *  6. Select data rate 1Mbps or 250Kbps
 *  7. Set PWR_UP high
 *  8. Clock in a payload that has the same length
 *  9. Pulse CE to transmit the packet
 *  
 * How to receive:
 *  1. Use the same CRC configuration
 *  2. Set the PWR_UP and PRIM_RX bit to 1
 *  3. Disable auto acknowledgement on the data pipe that is addressed
 *  4. Use the same address width
 *  5. Use the same frequency channel
 *  6. Select data rate 1Mbps or 250Kbps
 *  7. Set correct payload width on the data pipe that is addressed
 *  8. Set CE high
 */
RF24 radio(
  PB3, // PB_INVALID, // CE_PIN，使用无效pin脚
  PB4  // CSN_PIN
);

/**
 * 发送地址：com.cobox.iot.cobell.source(CBSRC)/com.cobox.iot.cobell.sink(CBSNK)
 * 地址长度：40bits, 5bytes
 */
uint8_t targetAddress[][6] = {
  {"CBSRC"}, // com.cobox.iot.cobell.source -> CoBellSouRCe -> CBSRC
  {"CBSNK"}  // com.cobox.iot.cobell.sink -> CoBellSiNK -> CBSNK
};

uint8_t alertMessage[] = {
  "com.cobox.iot.cobell.source,alert" // 消息格式："[发送者],[指令]"
};

enum TargetAddress {
  SOURCE = 0, SINK = 1
};

/**
 * 启动后闪灯：1长3短
 */
static void blinkForLaunched() {
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);

  delay(2000);
}

static void sendAlartEvent() {
  // 开始在SPI总线上初始化
  if (!radio.begin()) {
    printf("Radio device connect failed, shutdown!");
    radio.powerDown();
    return;
  }

  radio.setCRCLength(RF24_CRC_16); // 1. 所有设备使用相同的CRC配置
  radio.stopListening();           // 2. 关闭接受通道
  radio.setRetries(                // 3. 配置重传机制
    5, // 重试间隔：1500us = (5 * 250us + 250us)
    3  // 重试次数：3 times
  );
  radio.setAutoAck(true);          // 4. 开启自动回执
  radio.setAddressWidth(5);        // 5. 配置相同的地址宽度：5bytes
  radio.setChannel(0);             // 6. 配置相同的通信频率：(2400 + 0)MHz
  radio.setDataRate(RF24_1MBPS);   // 7. 配置相同的通信速率：1Mbps
  radio.setPALevel(RF24_PA_MAX);   // 8. 配置发送功率：0 dBm
  radio.powerUp();                 // 9. 无线发射开机

  // 设置发送负载大小，负载设置为发送内容的大小会提高性能
  radio.setPayloadSize(sizeof(alertMessage));
  
  // 开启发送管道
  radio.openWritingPipe(targetAddress[SINK]);

  // 发送消息
  radio.write(alertMessage, sizeof(alertMessage));

  // 无线发射关机
  radio.powerDown();
}

static void systemDuringSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // 设置休眠模式：掉电模式
  cli(); // 关闭全局中断
  sleep_enable(); // 允许休眠
  sei(); // 开启全局中断

  sleep_cpu(); // 开始休眠，并等待系统被中断唤醒

  sleep_disable(); // 唤醒后及时禁用休眠
}

void setup() {
  // 调试：执行开机闪灯
  blinkForLaunched();
}

void loop() {
  // 发送门铃信息
  sendAlartEvent();

  // 关机
  systemDuringSleep();  
}