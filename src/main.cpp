#include <Arduino.h>

#include "driver/gpio.h"
#include "driver/twai.h"
// main
// const gpio_num_t kCanTx = GPIO_NUM_6;
// const gpio_num_t kCanRx = GPIO_NUM_7;

// sub
const gpio_num_t kCanTx = GPIO_NUM_43;
const gpio_num_t kCanRx = GPIO_NUM_44;

HardwareSerial MySerial(1);

void setup() {
  MySerial.begin(115200, SERIAL_8N1, 34, 33);

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(kCanTx, kCanRx, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  pinMode(GPIO_NUM_45, OUTPUT);
  digitalWrite(GPIO_NUM_45, HIGH);

  // Install TWAI driver
  while (!twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK);

  /*
   //Install TWAI driver
   if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
       printf("Driver installed\n");
   } else if (twai_driver_install(&g_config, &t_config, &f_config) ==
   ESP_ERR_INVALID_ARG) { printf("ESP_ERR_INVALID_ARG:
   twai_driver_install()\n"); } else if (twai_driver_install(&g_config,
   &t_config, &f_config) == ESP_ERR_NO_MEM) { printf("ESP_ERR_MEM:
   twai_driver_install()\n"); } else if (twai_driver_install(&g_config,
   &t_config, &f_config) == ESP_ERR_INVALID_STATE){
       printf("EPS_ERR_INVALID_STATE: twai_driver_install()\n");
   } else {
       return;
   }

   //Start TWAI driver
   if (twai_start() == ESP_OK) {
       printf("TWAI started\n");
   } else if (twai_start() == ESP_ERR_INVALID_STATE) {
       printf("ESP_ERR_INVALID_STATE: twai_start()\n");
   } else {
       return;
   }
 */
}

void loop() {
  // Start TWAI driver
  while (!twai_start() == ESP_OK);

  // Configure message to transmit
  twai_message_t message_transmit;
  message_transmit.identifier = 0x000;
  message_transmit.extd = 0;
  message_transmit.data_length_code = 4;
  message_transmit.data[0] = 0;
  message_transmit.data[1] = 6;
  message_transmit.data[2] = 3;
  message_transmit.data[3] = 0;

  // Queue message for transmission
  if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_OK) {
    MySerial.printf("Message queued for transmission\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_INVALID_ARG) {
    MySerial.printf("ESP_ERR_INVALID_ARG: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_TIMEOUT) {
    MySerial.printf("ESP_ERR_INVALID_TIMEOUT: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_FAIL) {
    MySerial.printf("ESP_FAIL: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_INVALID_STATE) {
    MySerial.printf("ESP_ERR_INVALID_STATE: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_NOT_SUPPORTED) {
    MySerial.printf("ESP_ERR_NOT_SUPPORTED: twai_transmit()\n");
  } else {
    return;
  }

  uint32_t identifier = 0x1806E5F4;

  // Wait for message to be received
  twai_message_t message_receive;
  esp_err_t result = twai_receive(&message_receive, pdMS_TO_TICKS(3000));
  if (result == ESP_OK) {
    MySerial.printf("Message received\n");
  } else if (result == ESP_ERR_TIMEOUT) {
    MySerial.printf("ESP_ERR_TIMEOUT: twai_receive()\n");
  } else if (result == ESP_ERR_INVALID_ARG) {
    MySerial.printf("ESP_ERR_INVALID_ARG: twai_receive()\n");
  } else if (result == ESP_ERR_INVALID_STATE) {
    MySerial.printf("ESP_ERR_INVALID_STATE: twai_receive()\n");
  }
  MySerial.printf("--->\n");
  // Process received message
  if (message_receive.extd) {
    MySerial.printf("Message is in Extended Format\n");
    MySerial.printf("ID is %x\n", message_receive.identifier);
  } else {
    MySerial.printf("Message is in Standard Format\n");
    MySerial.printf("ID is %x\n", message_receive.identifier);
  }
  if (!(message_receive.rtr)) {
    for (int i = 0; i < message_receive.data_length_code; i++) {
      MySerial.printf("Data byte %d = %x\n", i, message_receive.data[i]);
    }
  }

  while (!(twai_stop() == ESP_OK));
  // Serial.printf("TWAI stopped...\n");
  /*
    // Uninstall the TWAI driver
    if (twai_driver_uninstall() == ESP_OK) {
      printf("Driver uninstalled\n");
    } else {
      printf("EPS_ERR_INVALID_STATE: twai_driver_uninstall\n");
    }
  */
}