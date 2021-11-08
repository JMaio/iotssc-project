// ----------------------------------------------------------------------------
// Copyright 2016-2018 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
// from "pelion-example-disco-iot01"
#ifndef MBED_TEST_MODE

#include "mbed.h"
#include "mbed-cloud-client/MbedCloudClient.h"
#include "LittleFileSystem.h"


// Default block device available on the target board
BlockDevice* bd = BlockDevice::get_default_instance();
SlicingBlockDevice sd(bd, 0, 2*1024*1024);

#if COMPONENT_SD || COMPONENT_NUSD
// Use FATFileSystem for SD card type blockdevices
FATFileSystem fs("fs");
#else
// Use LittleFileSystem for non-SD block devices to enable wear leveling and other functions
LittleFileSystem fs("fs");
#endif

// Default User button for GET example and for resetting the storage
InterruptIn button(BUTTON1);
// Default LED to use for PUT/POST example
DigitalOut led(LED1, 1);

// How often to fetch sensor data (in seconds)
#define SENSORS_POLL_INTERVAL 3.0


// Sensors related includes and initialization
// #include "stm32l475e_iot01_accelero.h"

static DevI2C devI2c(PB_11,PB_10);
static LSM6DSLSensor sen_acc_gyro(&devI2c,LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW,PD_11); // low address
static DigitalOut shutdown_pin(PC_6);

// Declaring pointers for access to Pelion Client resources outside of main()
M2MResource *res_button;
M2MResource *res_led;

// Additional resources for sensor readings
M2MResource *res_accelerometer_x;
M2MResource *res_accelerometer_y;
M2MResource *res_accelerometer_z;
M2MResource *res_gyroscope_x;
M2MResource *res_gyroscope_y;
M2MResource *res_gyroscope_z;

// An event queue is a very useful structure to debounce information between contexts (e.g. ISR and normal threads)
// This is great because things such as network operations are illegal in ISR, so updating a resource in a button's fall() function is not allowed
EventQueue eventQueue;

// When the device is registered, this variable will be used to access various useful information, like device ID etc.
static const ConnectorClientEndpointInfo* endpointInfo;

/**
 * PUT handler
 * @param resource The resource that triggered the callback
 * @param newValue Updated value for the resource
 */
void put_callback(M2MResource *resource, m2m::String newValue) {
    printf("*** PUT received, new value: %s                             \n", newValue.c_str());
    led = atoi(newValue.c_str());
}

/**
 * POST handler
 * @param resource The resource that triggered the callback
 * @param buffer If a body was passed to the POST function, this contains the data.
 *               Note that the buffer is deallocated after leaving this function, so copy it if you need it longer.
 * @param size Size of the body
 */
void post_callback(M2MResource *resource, const uint8_t *buffer, uint16_t size) {
    printf("*** POST received (length %u). Payload: ", size);
    for (size_t ix = 0; ix < size; ix++) {
        printf("%02x ", buffer[ix]);
    }
    printf("\n");
}

/**
 * Button function triggered by the physical button press.
 */
void button_press() {
    int v = res_button->get_value_int() + 1;
    res_button->set_value(v);
    printf("*** Button clicked %d times                                 \n", v);
}

// /**
//  * Notification callback handler
//  * @param resource The resource that triggered the callback
//  * @param status The delivery status of the notification
//  */
// void button_callback(M2MResource *resource, const NoticationDeliveryStatus status) {
//     printf("*** Button notification, status %s (%d)                     \n", M2MResource::delivery_status_to_string(status), status);
// }

/**
 * Registration callback handler
 * @param endpoint Information about the registered endpoint such as the name (so you can find it back in portal)
 */
void registered(const ConnectorClientEndpointInfo *endpoint) {
    printf("Registered to Pelion Device Management. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
    endpointInfo = endpoint;
}

/**
 * Initialize sensors
 */
void sensors_init() {
    uint8_t id1;

    printf ("\nSensors configuration:\n");
    // Initialize sensors
    sen_acc_gyro.init(NULL);

    /// Call sensors enable routines
    sen_acc_gyro.enable_x();
    sen_acc_gyro.enable_g();

    sen_acc_gyro.read_id(&id1);

    printf("LSM6DSL accelerometer & gyroscope = 0x%X\n", id1);

    printf("\n"); ;
}

/**
 * Update sensors and report their values.
 * This function is called periodically.
 */
void sensors_update() {
    int32_t a_axes[3], g_axes[3];

    sen_acc_gyro.get_x_axes(a_axes);
    sen_acc_gyro.get_g_axes(g_axes);

    float acc_x =  (double)a_axes[0] / 1000.0, acc_y  = (double)a_axes[1] / 1000.0, acc_z  = (double)a_axes[2] / 1000.0;
    float gyro_x = (double)g_axes[0] / 1000.0, gyro_y = (double)g_axes[1] / 1000.0, gyro_z = (double)g_axes[2] / 1000.0;

    printf("                                                             \n");
    printf("LSM6DSL acc:  %7.3f x, %7.3f y, %7.3f z [g]          \n", acc_x, acc_y, acc_z);

    printf("\r\033[8A");

    if (endpointInfo) {
        res_accelerometer_x->set_value(acc_x);
        res_accelerometer_y->set_value(acc_y);
        res_accelerometer_z->set_value(acc_z);
        res_gyroscope_x->set_value(gyro_x);
        res_gyroscope_y->set_value(gyro_y);
        res_gyroscope_z->set_value(gyro_z);
    }
}


int do_the_sensor_thing(MbedCloudClient *client) {
    printf("\nInitializing sensors...\n");

    sensors_init();

    // SimpleMbedCloudClient handles registering over LwM2M to Pelion DM
    // SimpleMbedCloudClient client(net, bd, &fs);
    // int client_status = client.init();
    // if (client_status != 0) {
    //     printf("ERROR: Pelion Client initialization failed (%d)\n", client_status);
    //     return -1;
    // }

    // Creating resources, which can be written or read from the cloud
    res_button = client.create_resource("3200/0/5501", "Button Count");
    res_button->set_value(0);
    res_button->methods(M2MMethod::GET);
    res_button->observable(true);
    res_button->attach_notification_callback(button_callback);

    res_led = client.create_resource("3201/0/5853", "LED State");
    res_led->set_value(1);
    res_led->methods(M2MMethod::GET | M2MMethod::PUT);
    res_led->attach_put_callback(put_callback);


    res_accelerometer_x = client.create_resource("3313/0/5702", "Accelerometer X");
    res_accelerometer_x->set_value(0);
    res_accelerometer_x->methods(M2MMethod::GET);
    res_accelerometer_x->observable(true);

    res_accelerometer_y = client.create_resource("3313/0/5703", "Accelerometer Y");
    res_accelerometer_y->set_value(0);
    res_accelerometer_y->methods(M2MMethod::GET);
    res_accelerometer_y->observable(true);

    res_accelerometer_z = client.create_resource("3313/0/5704", "Accelerometer Z");
    res_accelerometer_z->set_value(0);
    res_accelerometer_z->methods(M2MMethod::GET);
    res_accelerometer_z->observable(true);

   
    res_gyroscope_x = client.create_resource("3334/0/5702", "Gyroscope X");
    res_gyroscope_x->set_value(0);
    res_gyroscope_x->methods(M2MMethod::GET);
    res_gyroscope_x->observable(true);

    res_gyroscope_y = client.create_resource("3334/0/5703", "Gyroscope Y");
    res_gyroscope_y->set_value(0);
    res_gyroscope_y->methods(M2MMethod::GET);
    res_gyroscope_y->observable(true);

    res_gyroscope_z = client.create_resource("3334/0/5704", "Gyroscope Z");
    res_gyroscope_z->set_value(0);
    res_gyroscope_z->methods(M2MMethod::GET);
    res_gyroscope_z->observable(true);

    printf("Initialized Pelion Client. Registering...\n");

    // Callback that fires when registering is complete
    client.on_registered(&registered);

    // Register with Pelion DM
    client.register_and_connect();

    int i = 600; // wait up 60 seconds before attaching sensors and button events
    while (i-- > 0 && !client.is_client_registered()) {
        wait_ms(100);
    }

    button.fall(eventQueue.event(&button_press));

    // The timer fires on an interrupt context, but debounces it to the eventqueue, so it's safe to do network operations
    Ticker timer;
    timer.attach(eventQueue.event(&sensors_update), SENSORS_POLL_INTERVAL);

    // You can easily run the eventQueue in a separate thread if required
    eventQueue.dispatch_forever();
}

#endif
