#include <Arduino.h>

#include <array>

#include "driver/gpio.h"
#include "driver/twai.h"
// main
// const gpio_num_t kCanTx = GPIO_NUM_6;
// const gpio_num_t kCanRx = GPIO_NUM_7;

// sub
const gpio_num_t kCanTx = GPIO_NUM_4;
const gpio_num_t kCanRx = GPIO_NUM_5;

namespace byte_0 {
constexpr auto kSocReset = 0b00000001;
constexpr auto kCharging = 0b00000100;
constexpr auto kShutdown = 0b00001000;
}  // namespace byte_0
namespace byte_1 {
namespace mode_operating {
constexpr auto kStop = 0b0000;
constexpr auto kDischarge = 0b0001;
constexpr auto kCharge = 0b0010;
constexpr auto kPlugin = 0b0011;
}  // namespace mode_operating
namespace flag_contactor {
constexpr auto kCharging = 0b00010000;
constexpr auto kDischarging = 0b00100000;
constexpr auto kEvRelay = 0b01000000;
}  // namespace flag_contactor
}  // namespace byte_1
namespace byte_3 {
constexpr auto kOffsetTemperature = 55;
}
namespace byte_4 {
constexpr auto kOffsetTemperature = 55;
}
void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(kCanTx, kCanRx, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
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
  message_transmit.identifier = 0x110;
  message_transmit.extd = false;
  message_transmit.rtr = false;
  message_transmit.data_length_code = 8;
  // message_transmit.data[0] = byte_0::kShutdown;
  message_transmit.data[0] = 0;
  // message_transmit.data[1] = (byte_1::mode_operating::kCharge | byte_1::flag_contactor::kCharging);
  message_transmit.data[1] = 0;
  message_transmit.data[2] = 50;                               // OCV_SOC
  message_transmit.data[3] = 30 + byte_3::kOffsetTemperature;  // Cell_Temp_Max
  message_transmit.data[4] = 20 + byte_4::kOffsetTemperature;  // Cell_Temp_Min
  message_transmit.data[5] = 0;
  message_transmit.data[6] = 0;
  message_transmit.data[7] = 0;

  // for (size_t i = 0; i < 8; i++) {
  //   printf("message[%d]:%u\n", i, message_transmit.data[i]);
  // }

  // Queue message for transmission

  esp_err_t result = twai_transmit(&message_transmit, pdMS_TO_TICKS(50));

  if (result == ESP_OK) {
    printf("Message queued for transmission\n");
  } else if (result == ESP_ERR_INVALID_ARG) {
    printf("ESP_ERR_INVALID_ARG: twai_transmit()\n");
  } else if (result == ESP_ERR_TIMEOUT) {
    printf("ESP_ERR_INVALID_TIMEOUT: twai_transmit()\n");
  } else if (result == ESP_FAIL) {
    printf("ESP_FAIL: twai_transmit()\n");
  } else if (result == ESP_ERR_INVALID_STATE) {
    printf("ESP_ERR_INVALID_STATE: twai_transmit()\n");
  } else if (result == ESP_ERR_NOT_SUPPORTED) {
    printf("ESP_ERR_NOT_SUPPORTED: twai_transmit()\n");
  } else {
    return;
  }

  // Wait for message to be received
  twai_message_t message_receive;
  // esp_err_t result = twai_receive(&message_receive, pdMS_TO_TICKS(1000));
  result = twai_receive(&message_receive, pdMS_TO_TICKS(500));
  if (result == ESP_OK) {
    printf("Message received\n");
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
  } else if (result == ESP_ERR_TIMEOUT) {
    printf("ESP_ERR_TIMEOUT: twai_receive()\n");
  } else if (result == ESP_ERR_INVALID_ARG) {
    printf("ESP_ERR_INVALID_ARG: twai_receive()\n");
  } else if (result == ESP_ERR_INVALID_STATE) {
    printf("ESP_ERR_INVALID_STATE: twai_receive()\n");
  } else {
    printf("ESP_ERR_INVALID_STATE: not define()\n");
  }

  if (message_receive.identifier == 0x100) {
    std::array<uint8_t, 2> bytes = {message_receive.data[2], message_receive.data[3]};
    float current_battery = static_cast<float>(((bytes[1]) << 8 | bytes[0]) / 100 - 100);
    bytes = {message_receive.data[4], message_receive.data[5]};
    float current_cherger = static_cast<float>(((bytes[1]) << 8 | bytes[0]) / 100);
    bytes = {message_receive.data[6], message_receive.data[7]};
    float current_load = static_cast<float>(((bytes[1]) << 8 | bytes[0]) / 100 - 100);
    if (current_load < 0) {
      current_load = 0;
    }
    // output .csv
    // Serial.printf("Time /ms, ChargerState, ChargerCurrent /A, BatterySate, BatteryCurrent /A, SOC /%%\n");
    printf("soh:%u, ", message_receive.data[0]);
    printf("soc:%u, ", message_receive.data[1]);
    printf("battery current:%f, ", current_battery);
    printf("charger current:%f, ", current_cherger);
    printf("load current:%f\n", current_load);
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