#include <Arduino.h>
#include <pinout.h>

#include <SPI.h>
#include <nRF24L01.h>
#include "printf.h"
#include "RF24.h"


/**
 * nRF24L01 instance
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
  RF24_CE, // CE pin
  RF24_CSN // CS pin
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

static void listenAlertEvent() {
  // 开始在SPI总线上初始化
  if (!radio.begin()) {
    Serial.println("Radio device connect failed, shutdown!");
    radio.powerDown();
    return;
  } else {
    Serial.println("Radio device online");
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
  Serial.println("Radio device power up");
  
  // 开启接收管道
  radio.openReadingPipe(1, targetAddress[SINK]);
  Serial.printf("Radio device open reading pipline at %s\n", targetAddress[SINK]);

  // 开始接收信号
  radio.startListening();
  Serial.println("Radio device start listenning...");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Begin Serial at 115200");
  listenAlertEvent();
}

void loop() {
  if (!radio.isChipConnected()) {
    Serial.println("Radio offline");
    return;
  }
  if (!radio.available()) {
    return;
  }

  uint8_t buffer[64] = { NULL }; 
  memset(buffer, NULL, 64);
  radio.read(buffer, sizeof(buffer));
  Serial.printf("Radio received: %s", buffer);
}