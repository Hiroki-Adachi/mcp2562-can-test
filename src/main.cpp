#include <Arduino.h>

#include "driver/gpio.h"
#include "driver/twai.h"
// main
// const gpio_num_t kCanTx = GPIO_NUM_6;
// const gpio_num_t kCanRx = GPIO_NUM_7;

// sub
const gpio_num_t kCanTx = GPIO_NUM_35;
const gpio_num_t kCanRx = GPIO_NUM_36;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(kCanTx, kCanRx, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

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
  message_transmit.identifier = 0x050;
  message_transmit.extd = 0;
  message_transmit.data_length_code = 5;
  message_transmit.data[0] = 1;
  message_transmit.data[1] = 7;
  message_transmit.data[2] = 1;
  message_transmit.data[3] = 4;
  message_transmit.data[4] = 0;

  // Queue message for transmission
  if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_OK) {
    printf("Message queued for transmission\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_INVALID_ARG) {
    printf("ESP_ERR_INVALID_ARG: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_TIMEOUT) {
    printf("ESP_ERR_INVALID_TIMEOUT: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_FAIL) {
    printf("ESP_FAIL: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_INVALID_STATE) {
    printf("ESP_ERR_INVALID_STATE: twai_transmit()\n");
  } else if (twai_transmit(&message_transmit, pdMS_TO_TICKS(1000)) == ESP_ERR_NOT_SUPPORTED) {
    printf("ESP_ERR_NOT_SUPPORTED: twai_transmit()\n");
  } else {
    return;
  }

  uint32_t identifier = 0x3FF;

  // Wait for message to be received
  twai_message_t message_receive;
  while (1) {
    if (twai_receive(&message_receive, pdMS_TO_TICKS(10000)) == ESP_OK) {
      printf("Message received\n");
      if (message_receive.identifier == identifier) break;
    } else if (twai_receive(&message_receive, pdMS_TO_TICKS(10000)) == ESP_ERR_TIMEOUT) {
      printf("ESP_ERR_TIMEOUT: twai_receive()\n");
    } else if (twai_receive(&message_receive, pdMS_TO_TICKS(10000)) == ESP_ERR_INVALID_ARG) {
      printf("ESP_ERR_INVALID_ARG: twai_receive()\n");
    } else if (twai_receive(&message_receive, pdMS_TO_TICKS(10000)) == ESP_ERR_INVALID_STATE) {
      printf("ESP_ERR_INVALID_STATE: twai_receive()\n");
    } else {
      return;
    }
  }

  // Process received message
  if (message_receive.extd) {
    printf("Message is in Extended Format\n");
    printf("ID is %x\n", message_receive.identifier);
  } else {
    printf("Message is in Standard Format\n");
    printf("ID is %x\n", message_receive.identifier);
  }
  if (!(message_receive.rtr)) {
    for (int i = 0; i < message_receive.data_length_code; i++) {
      printf("Data byte %d = %u\n", i, message_receive.data[i]);
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