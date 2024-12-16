/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdint>
#include <cstdio>
#include "mbed.h"

#include "mbed_version.h"

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Application helpers
#include "DummySensor.h"
#include "trace_helper.h"
#include "lora_radio_helper.h"
#include "brightness.h"
#include "soil.h"
#include "GPS.h"
#include "temperatur.h"
#include "color.h"
#include "Accelerometer.h"
#include "RGB.h"


using namespace events;
using namespace std::chrono_literals;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        10s

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
 * Dummy pin for dummy sensor
 */
#define PC_9                            0


#define DEF_LATITUDE 52.5200
#define DEF_LONGITUDE 13.4050
#define DEF_ALTITUDE 0

/**
 * Dummy sensor class object
 */
 // Gemeinsame I2C-Instanz für alle Sensoren
I2C i2c(PB_7, PB_6);  // Dieselben I2C-Pins für alle Sensoren
DS1820  ds1820(PC_9);
GPS gps(PA_9, PA_10, PA_12);
Brightness light_sensor;
SoilSensor soilmoisture;
TemperatureSensor tempSensor(i2c);
ColorSensor colorSensor(i2c);
Accelerometer accel(i2c);
RGB rgb;


// globale Variablen
// GPS
int satelliteCount = 0; 
float latitude = 0, longitude = 0, altitude = 0;

// Temp humd
float temperature = 0, humidity = 0;

// Light sensor
float brightness = 0;

// soil moisture
float soil_moisture = 0;

// Color Sensor
uint16_t clear = 0, red = 0, green = 0, blue = 0;

// Accelerometer
float x_Axis = 0, y_Axis = 0, z_Axis = 0;


/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;

/**
 * Default and configured device EUI, application EUI and application key
 */
static const uint8_t DEFAULT_DEV_EUI[] = {0x40, 0x39, 0x32, 0x35, 0x59, 0x37, 0x91, 0x94};
//static uint8_t DEV_EUI[] = {0x7c, 0x39, 0x32, 0x35, 0x59, 0x37, 0x91, 0x94};
//static uint8_t APP_EUI[] = {0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0xfc, 0x4d};
//static uint8_t APP_KEY[] = {0xf3, 0x1c, 0x2e, 0x8b, 0xc6, 0x71, 0x28, 0x1d,
//                            0x51, 0x16, 0xf0, 0x8f, 0xf0, 0xb7, 0x92, 0x8f};

static uint8_t DEV_EUI[] = {0x7c, 0x39, 0x32, 0x35, 0x59, 0x37, 0x91, 0x94};
static uint8_t APP_EUI[] = {0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0xac, 0x4a};
static uint8_t APP_KEY[] = {0xf3, 0x1c, 0x2e, 0x8b, 0xc6, 0x71, 0x28, 0x1d,
                            0x51, 0x16, 0xf0, 0x8f, 0xf0, 0xb7, 0x92, 0x8f};


// Define packed struct for GPS data
struct __attribute__((packed)) sensor_data {
    float lat;
    float lon;
    int16_t alt;

    int16_t temp;
    int16_t humid;

    uint16_t soil;
    uint16_t light;

    uint16_t red;
    uint16_t green;
    uint16_t blue;

    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
};

struct sensor_data mySensor_data;

void get_all_sesnor_data()
{
    // GPS
    gps.readAndProcessGPSData();
    satelliteCount = gps.getNumSatellites();
    latitude = gps.getLatitude();
    longitude = gps.getLongitude();
    altitude = gps.getAltitude();

    // Temp and Humid
    temperature = tempSensor.readTemperature();
    humidity = tempSensor.readHumidity();

    // Light and Soil Mositure
    brightness = light_sensor.read();
    soil_moisture = soilmoisture.readMoisture();
    
    // Color Sensor
    colorSensor.init();
    colorSensor.readColorData(clear, red, green, blue);
    red = colorSensor.getRed();
    green = colorSensor.getGreen();
    blue = colorSensor.getBlue();

    // Accelerometer
    accel.initialize();
    x_Axis = accel.getAccX();
    y_Axis = accel.getAccY();
    z_Axis = accel.getAccZ();
}

void update_data_in_struct()
{
  // Default if no signal
  if(latitude == 0.0)
  {
      mySensor_data.lat = DEF_LATITUDE;
  }  
  else mySensor_data.lat = latitude;

  if(longitude == 0.0)
  {
      mySensor_data.lon = DEF_LONGITUDE;
  }
  else mySensor_data.lon = longitude;

  if(altitude == 0.0)
  {
      mySensor_data.alt = DEF_ALTITUDE;
  }
  else mySensor_data.alt = (int16_t)altitude;


  // Data to struct with numbers after ","
  mySensor_data.temp = (int16_t)(temperature * 100.0f);
  mySensor_data.humid = (int16_t)(humidity * 100.0f);

  mySensor_data.soil = (int16_t)(soil_moisture * 100.0f);
  mySensor_data.light = (int16_t)(brightness * 100.0f);

  mySensor_data.red = red;
  mySensor_data.green = green;
  mySensor_data.blue = blue;

  mySensor_data.acc_x = (int16_t)(x_Axis * 100.0f);
  mySensor_data.acc_y = (int16_t)(y_Axis * 100.0f);
  mySensor_data.acc_z = (int16_t)(z_Axis * 100.0f);
}

/**
 * Entry point for application
 */
int main(void)
{
    printf("\r\n*** Sensor Networks @ ETSIST, UPM ***\r\n"
           "   Mbed (v%d.%d.%d) LoRaWAN example\r\n",
           MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    printf("\r\n DEV_EUI: ");
    for (int i = 0; i < sizeof(DEV_EUI); ++i) printf("%02x", DEV_EUI[i]);
    printf("\r\n APP_EUI: ");
    for (int i = 0; i < sizeof(APP_EUI); ++i) printf("%02x", APP_EUI[i]);
    printf("\r\n APP_KEY: ");
    for (int i = 0; i < sizeof(APP_KEY); ++i) printf("%02x", APP_KEY[i]);
    printf("\r\n");

    if (!memcmp(DEV_EUI, DEFAULT_DEV_EUI, sizeof(DEV_EUI))) {
        printf("\r\n *** You are using the default device EUI value!!! *** \r\n");
        printf("Please, change it to ensure that the device EUI is unique \r\n");
        return -1;
    }

    // setup tracing
    setup_trace();

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
            != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

    lorawan_connect_t connect_params;
    connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
    connect_params.connection_u.otaa.dev_eui = DEV_EUI;
    connect_params.connection_u.otaa.app_eui = APP_EUI;
    connect_params.connection_u.otaa.app_key = APP_KEY;
    connect_params.connection_u.otaa.nb_trials = 3;

    retcode = lorawan.connect(connect_params);

    if (retcode == LORAWAN_STATUS_OK ||
            retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    ev_queue.call_every(5s, get_all_sesnor_data);
    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}




/**
 * Sends a message to the Network Server
 */
static void send_message()
{   
    update_data_in_struct();

    uint16_t packet_len;
    int16_t retcode;
   

    // Send the struct directly as bytes
    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, (uint8_t*)&mySensor_data, sizeof(mySensor_data), MSG_UNCONFIRMED_FLAG);

    

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_UNCONFIRMED_FLAG);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
        : printf("\r\n send() - Error code %d \r\n", retcode);

        if (retcode == LORAWAN_STATUS_WOULD_BLOCK) {
            //retry in 3 seconds
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                ev_queue.call_in(3s, send_message);
            }
        }
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);


    /*printf("\r\n=== Sensor Data (%d bytes) ===\r\n", sizeof(sensor_data_t));
  printf("Location:\r\n");
  printf("  Latitude:  %.6f°\r\n", sensor_data.latitude);
  printf("  Longitude: %.6f°\r\n", sensor_data.longitude);
  printf("  Altitude:  %d m\r\n", sensor_data.altitude);

  printf("\r\nColor Sensor:\r\n");
  printf("  Red:   %d\r\n", sensor_data.red);
  printf("  Green: %d\r\n", sensor_data.green);
  printf("  Blue:  %d\r\n", sensor_data.blue);

  printf("\r\nAccelerometer:\r\n");
  printf("  X-Axis: %d\r\n", sensor_data.xAxis);
  printf("  Y-Axis: %d\r\n", sensor_data.yAxis);
  printf("  Z-Axis: %d\r\n", sensor_data.zAxis);

  printf("\r\nEnvironment:\r\n");
  printf("  Temperature: %d°C\r\n", sensor_data.temp);
  printf("  Humidity:    %d%%\r\n", sensor_data.hum);
  printf("  Soil Moisture:    %d%%\r\n", sensor_data.moisture);
  printf("  Brightness: %d%%\r\n", sensor_data.brightness);
  printf("=====================\r\n");
*/

    // Check and print GPS status
  if (satelliteCount == 0) 
  {
    printf("\r\n WARNING: No GPS Fix - Using Default/Last Known Location \r\n");
  }

    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++) {
        printf("%02x ", rx_buffer[i]);
    }
    printf("\r\n");
    

    if (strcmp((char *)rx_buffer, "Green") == 0)
    {
        rgb.set_green();
        printf("LED Green\r\n");
    }
    else if (strcmp((char *)rx_buffer, "Red") == 0) 
    {
        rgb.set_red();
        printf("LED Red \r\n");
    }
    else if (strcmp((char *)rx_buffer, "OFF") == 0) 
    {
        rgb.turn_off_led();
        printf("LEDs OFF\r\n");
    }
    else {
        printf("Wrong message received\r\n");
    }

    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            } else {
                ev_queue.call_every(TX_TIMER, send_message);
            }

            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
