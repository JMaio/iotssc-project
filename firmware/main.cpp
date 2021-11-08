// ----------------------------------------------------------------------------
// Copyright 2016-2020 ARM Ltd.
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
#include <cstdint>
#include <ctime>
#ifndef MBED_TEST_MODE
#include "mbed-cloud-client/MbedCloudClient.h" // Required for new MbedCloudClient()
#include "factory_configurator_client.h"       // Required for fcc_* functions and FCC_* defines
#include "m2mresource.h"                       // Required for M2MResource
#include "key_config_manager.h"                // Required for kcm_factory_reset

#include "mbed.h"
#include "DeviceKey.h"
#include "kv_config.h"
#include "mbed-trace/mbed_trace.h"             // Required for mbed_trace_*

// https://os.mbed.com/teams/ST/code/HelloWorld_ST_Sensors//file/55795f30572e/main.cpp/
// sensors
#include "LSM6DSLSensor.h"

#include <deque> 
using namespace std; 

// use a double-ended queue to store readings
#define window_size 5
// #define polling_freq 1s / 5 // 5 Hz
#define polling_freq 1s / 1 // 1 Hz
// delay between sensor buffer submissions
// #define sensor_upload_delay window_size*polling_freq

// queue must have size: window_size * [3 * sizeof(int8_t)]
#define queue_size window_size * 3 * sizeof(int32_t)
// deque <int32_t>  acc_queue(queue_size, 0);
deque <int32_t>  acc_mag_queue(window_size * sizeof(int32_t), 0);
// deque <int32_t> gyro_queue(queue_size, 0);

static DevI2C devI2c(PB_11,PB_10);
static LSM6DSLSensor acc_gyro(&devI2c,LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW,PD_11); // low address

void init_sensors() {
    // uint8_t id;

    /* Init sensors with default params */
    printf("Init sensors... ");

    acc_gyro.init(NULL);

    /* Enable sensors */
    acc_gyro.enable_x();
    acc_gyro.enable_g();
    printf("Done!\n");

    // printf("\033[2J\033[20A");
    // printf ("\r\n--- Starting new run ---\r\n\r\n");

    // acc_gyro.read_id(&id);
    // printf("LSM6DSL accelerometer & gyroscope = 0x%X\r\n", id);
}




// ========================= BUILT-IN MBED STUFF =========================

// Pointers to the resources that will be created in main_application().
static MbedCloudClient *cloud_client;
static bool cloud_client_running = true;
static NetworkInterface *network = NULL;
static int error_count = 0;

// Fake entropy needed for non-TRNG boards. Suitable only for demo devices.
const uint8_t MBED_CLOUD_DEV_ENTROPY[] = { 0xf6, 0xd6, 0xc0, 0x09, 0x9e, 0x6e, 0xf2, 0x37, 0xdc, 0x29, 0x88, 0xf1, 0x57, 0x32, 0x7d, 0xde, 0xac, 0xb3, 0x99, 0x8c, 0xb9, 0x11, 0x35, 0x18, 0xeb, 0x48, 0x29, 0x03, 0x6a, 0x94, 0x6d, 0xe8, 0x40, 0xc0, 0x28, 0xcc, 0xe4, 0x04, 0xc3, 0x1f, 0x4b, 0xc2, 0xe0, 0x68, 0xa0, 0x93, 0xe6, 0x3a };
const int MAX_ERROR_COUNT = 5;

// dynamic resource which holds buffered data
static M2MResource* m2m_acc_buff_res;
static M2MObjectInstance* m2m_acc_res;
static M2MResource* m2m_acc_x_res;
static M2MResource* m2m_acc_y_res;
static M2MResource* m2m_acc_z_res;
static M2MResource* m2m_acc_mag_res;

static M2MResource* m2m_gyro_x_res;
static M2MResource* m2m_gyro_y_res;
static M2MResource* m2m_gyro_z_res;

static M2MResource* m2m_get_res;
static M2MResource* m2m_put_res;
static M2MResource* m2m_post_res;
static M2MResource* m2m_deregister_res;
static M2MResource* m2m_factory_reset_res;
static SocketAddress sa;

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;
Mutex value_increment_mutex;

void print_client_ids(void)
{
    printf("Account ID: %s\n", cloud_client->endpoint_info()->account_id.c_str());
    printf("Endpoint name: %s\n", cloud_client->endpoint_info()->endpoint_name.c_str());
    printf("Device ID: %s\n\n", cloud_client->endpoint_info()->internal_endpoint_name.c_str());
}

void value_increment(void)
{
    value_increment_mutex.lock();
    // // post this value or something
    // m2m_post_res->set_value(m2m_get_res->get_value_int());
    m2m_get_res->set_value(m2m_get_res->get_value_int() + 1);
    printf("Counter %" PRIu64 "\n", m2m_get_res->get_value_int());
    value_increment_mutex.unlock();
}

// custom reading of sensors for pelion
// void read_sensors(int32_t acc[], int32_t gyro[]) {
void read_sensors() {
    // int32_t axes[3];
    int32_t axes[3];

    // hacky way to get the readings and place them in the queue
    acc_gyro.get_x_axes(axes);
    // // cycle the queue
    // acc_queue.pop_front();
    // // accelerometer data must be converted to uint somewhere before being added to the queue?
    // // no, do this only in the final data representation step
    // acc_queue.push_back(*axes);
    // // pelion updating logic from: pelion-example-disco-iot01
    // // float acc_x =  (double)axes[0] / 1000.0, acc_y  = (double)axes[1] / 1000.0, acc_z  = (double)axes[2] / 1000.0;
    float acc_x = (double)axes[0] / 1000.0, 
          acc_y = (double)axes[1] / 1000.0, 
          acc_z = (double)axes[2] / 1000.0;

    
    
    // printf("LSM6DSL [acc/ g]:      %f, %f, %f\r\n", acc_x, acc_y, acc_z);
    // m2m_acc_x_res->set_value(axes[0]);
    // m2m_acc_y_res->set_value(axes[1]);
    // m2m_acc_z_res->set_value(axes[2]);
    printf("LSM6DSL [acc/mg]:     %6d, %6d, %6d\r\n", axes[0], axes[1], axes[2]);
    // calculate the magnitude
    int32_t acc_mag_sq = 1000*(acc_x*acc_x + acc_y*acc_y + acc_z*acc_z);
    printf("LSM6DSL mag^2:    (%6d)\r\n", acc_mag_sq);
    m2m_acc_mag_res->set_value(acc_mag_sq);

    // printf("LSM6DSL [acc/mg]:     %.2f, %.2f, %.2f\r\n", acc_x, acc_y, acc_z);
    // printf("axes            :     ", axes, "\r\n");
    
    // acc_gyro.get_g_axes(axes);
    // // gyro_queue.push_front(*axes);
    // // float gyro_x = (double) axes[0] / 1000.0, 
    // //       gyro_y = (double) axes[1] / 1000.0, 
    // //       gyro_z = (double) axes[2] / 1000.0;
    
    // m2m_gyro_x_res->set_value(axes[0]);
    // m2m_gyro_y_res->set_value(axes[1]);
    // m2m_gyro_z_res->set_value(axes[2]);
    // printf("LSM6DSL [gyro/mdps]:  %6d, %6d, %6d\r\n", axes[0], axes[1], axes[2]);
    //     ThisThread::sleep_for(1000ms);
    // }
}

// String make_acc_window_string(int32_t* q) {
//     // hacky string interpolation
//     for (int dp = 0; dp < q.s; inc-expression) {
//     statements
//     }
//     return "";
// }

// custom value setting function. this will update the pelion resource with data 
// every so often, but it will only be transmitted when requested
void update_data_buffer() {
    // https://os.mbed.com/docs/mbed-os/v6.7/apis/time.html
    printf("Updated data buffer: %u\n", (unsigned int)time(NULL));
    // why does this work?
    // http://www.enseignement.polytechnique.fr/informatique/INF478/docs/Cpp/en/cpp/container/deque.html
    // begin, cbegin
    // returns an iterator to the beginning
    // (public member function)
    // uint8_t buffer[12] = "Opaque Data";
    // m2m_acc_buff_res->set_value(buffer, queue_size);

    // // https://stackoverflow.com/a/6502825/9184658
    // // convert the int32 data to uint8 to transmit to pelion
    // uint8_t *vp = (uint8_t *)&*acc_queue.begin();
    // // does sizeof(v) work??
    // // m2m_acc_buff_res->set_value_raw(vp, sizeof(vp));
    // m2m_acc_buff_res->set_value_raw(vp, 4*queue_size);

    // m2m_acc_buff_res->set_value(*acc_queue.begin());
    // m2m_acc_buff_res->update_value(&*acc_queue.begin(), queue_size);
    // m2m_acc_buff_res->update_value(&*acc_queue.begin(), queue_size);
    // value_increment_mutex.unlock();
}

void get_res_update(const char* /*object_name*/)
{
    printf("Counter resource set to %d\n", (int)m2m_get_res->get_value_int());
}

void put_res_update(const char* /*object_name*/)
{
    printf("PUT update %d\n", (int)m2m_put_res->get_value_int());
}

void execute_post(void* /*arguments*/)
{
    printf("POST executed\n");
}

void deregister_client(void)
{
    printf("Unregistering and disconnecting from the network.\n");
    cloud_client->close();
}

void deregister(void* /*arguments*/)
{
    printf("POST deregister executed\n");
    m2m_deregister_res->send_delayed_post_response();

    deregister_client();
}

void client_registered(void)
{
    printf("Client registered.\n");
    print_client_ids();
    error_count = 0;
}

void client_registration_updated(void)
{
    printf("Client registration updated.\n");
    error_count = 0;
}

void client_unregistered(void)
{
    printf("Client unregistered.\n");
    (void) network->disconnect();
    cloud_client_running = false;
}

void factory_reset(void*)
{
    printf("POST factory reset executed\n");
    m2m_factory_reset_res->send_delayed_post_response();

    kcm_factory_reset();
}

void client_error(int err)
{
    printf("client_error(%d) -> %s\n", err, cloud_client->error_description());
    if (err == MbedCloudClient::ConnectNetworkError ||
        err == MbedCloudClient::ConnectDnsResolvingFailed ||
        err == MbedCloudClient::ConnectSecureConnectionFailed) {
        if(++error_count == MAX_ERROR_COUNT) {
            printf("Max error count %d reached, rebooting.\n\n", MAX_ERROR_COUNT);
            ThisThread::sleep_for(1000ms);
            NVIC_SystemReset();
	}
    }
}

void update_progress(uint32_t progress, uint32_t total)
{
    uint8_t percent = (uint8_t)((uint64_t)progress * 100 / total);
    printf("Update progress = %" PRIu8 "%%\n", percent);
}

void flush_stdin_buffer(void)
{
    FileHandle *debug_console = mbed::mbed_file_handle(STDIN_FILENO);
    while(debug_console->readable()) {
        char buffer[1];
        debug_console->read(buffer, 1);
    }
}

int main(void)
{
    // don't forget to initialize the sensors
    init_sensors();

    int status;

    status = mbed_trace_init();
    if (status != 0) {
        printf("mbed_trace_init() failed with %d\n", status);
        return -1;
    }

    // Mount default kvstore
    printf("Application ready\n");
    status = kv_init_storage_config();
    if (status != MBED_SUCCESS) {
        printf("kv_init_storage_config() - failed, status %d\n", status);
        return -1;
    }

#if MBED_MAJOR_VERSION > 5
    // Initialize root of trust
    DeviceKey &devkey = DeviceKey::get_instance();
    devkey.generate_root_of_trust();
#endif

    // Connect with NetworkInterface
    printf("Connect to network\n");
    network = NetworkInterface::get_default_instance();
    if (network == NULL) {
        printf("Failed to get default NetworkInterface\n");
        return -1;
    }
    status = network->connect();
    if (status != NSAPI_ERROR_OK) {
        printf("NetworkInterface failed to connect with %d\n", status);
        return -1;
    }
    status = network->get_ip_address(&sa);
    if (status!=0) {
        printf("get_ip_address failed with %d\n", status);
        return -2;
    }
    printf("Network initialized, connected with IP %s\n\n", sa.get_ip_address());

    // Run developer flow
    printf("Start developer flow\n");
    status = fcc_init();
    if (status != FCC_STATUS_SUCCESS) {
        printf("fcc_init() failed with %d\n", status);
        return -1;
    }

    // Inject hardcoded entropy for the device. Suitable only for demo devices.
    (void) fcc_entropy_set(MBED_CLOUD_DEV_ENTROPY, sizeof(MBED_CLOUD_DEV_ENTROPY));
    status = fcc_developer_flow();
    if (status != FCC_STATUS_SUCCESS && status != FCC_STATUS_KCM_FILE_EXIST_ERROR && status != FCC_STATUS_CA_ERROR) {
        printf("fcc_developer_flow() failed with %d\n", status);
        return -1;
    }

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
    cloud_client = new MbedCloudClient(client_registered, client_unregistered, client_error, NULL, update_progress);
#else
    cloud_client = new MbedCloudClient(client_registered, client_unregistered, client_error);
#endif // MBED_CLOUD_CLIENT_SUPPORT_UPDATE

    // Initialize client
    cloud_client->init();

    printf("Create resources\n");
    M2MObjectList m2m_obj_list;

    // ================================== create resources ==================================

    // res_accelerometer_x = client.create_resource("3313/0/5702", "Accelerometer X");
    // res_accelerometer_x->set_value(0);
    // res_accelerometer_x->methods(M2MMethod::GET);
    // res_accelerometer_x->observable(true);
 
    // res_accelerometer_y = client.create_resource("3313/0/5703", "Accelerometer Y");
    // res_accelerometer_y->set_value(0);
    // res_accelerometer_y->methods(M2MMethod::GET);
    // res_accelerometer_y->observable(true);
 
    // res_accelerometer_z = client.create_resource("3313/0/5704", "Accelerometer Z");
    // res_accelerometer_z->set_value(0);
    // res_accelerometer_z->methods(M2MMethod::GET);
    // res_accelerometer_z->observable(true);

    // res_gyroscope_x = client.create_resource("3334/0/5702", "Gyroscope X");
    // res_gyroscope_x->set_value(0);
    // res_gyroscope_x->methods(M2MMethod::GET);
    // res_gyroscope_x->observable(true);
 
    // res_gyroscope_y = client.create_resource("3334/0/5703", "Gyroscope Y");
    // res_gyroscope_y->set_value(0);
    // res_gyroscope_y->methods(M2MMethod::GET);
    // res_gyroscope_y->observable(true);
 
    // res_gyroscope_z = client.create_resource("3334/0/5704", "Gyroscope Z");
    // res_gyroscope_z->set_value(0);
    // res_gyroscope_z->methods(M2MMethod::GET);
    // res_gyroscope_z->observable(true);

    // // create a dynamic instance to buffer accelerometer data
    // // Opaque (from the device application):
    // // https://developer.pelion.com/docs/device-management/current/connecting/handle-resources.html
    // // M2MObject* object = M2MInterfaceFactory::create_object("3313");
    // M2MObjectInstance* m2m_acc_res = M2MInterfaceFactory::create_object("3313")->create_object_instance();
    // m2m_acc_res->set_observable(true);

    // does it need to explicitly be set to zero? "res_accelerometer_x->set_value(0);"
    // NOTE: values kept as integers since Mbed doesn't seem happy passing floats to Pelion (show as "%f")
    m2m_acc_x_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3313, 0, 5702, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);
    m2m_acc_y_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3313, 0, 5703, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);
    m2m_acc_z_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3313, 0, 5704, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);

    // create a virtual accelerometer device which broadcasts the magnitude on a single (X) axis
    m2m_acc_mag_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3313, 1, 5702, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);
    // https://developer.pelion.com/docs/device-management/current/connecting/collecting-resources.html
    // For a dynamic Resource or Resource Instance, you can set the observable mode when creating the Resource or Resource Instance. 
    // You can change it later if you need to:
    // m2m_acc_x_res->set_value(0);
    // m2m_acc_y_res->set_value(0);
    // m2m_acc_z_res->set_value(0);
    // m2m_acc_x_res->set_observable(true);
    // m2m_acc_y_res->set_observable(true);
    // m2m_acc_z_res->set_observable(true);

    // // make the parent instance of the resources observable
    // m2m_acc_res = &m2m_acc_x_res->get_parent_object_instance();
    // m2m_acc_res->set_observable(true);

    m2m_acc_mag_res->set_observable(true);


    // m2m_acc_res  = M2MInterfaceFactory::create_resource(m2m_obj_list, 3313, 0, );



    

    // m2m_acc_buff_res = object_instance->create_dynamic_resource("5700", "Accelerometer Buffer", M2MResourceInstance::OPAQUE, true);
    // m2m_acc_buff_res->set_observable(true);


    // m2m_acc_buff_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3313, 1, 5700, M2MResourceInstance::OPAQUE, M2MBase::GET_ALLOWED);
    // // M2MObjectInstance* object_instance = object->create_object_instance(1);
    // // m2m_acc_buff_res = object_instance->create_dynamic_resource("5700", "Accelerometer Buffer", M2MResourceInstance::OPAQUE, true);
    // m2m_acc_buff_res->set_observable(true);


    m2m_gyro_x_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3334, 0, 5702, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);
    m2m_gyro_y_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3334, 0, 5703, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);
    m2m_gyro_z_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3334, 0, 5704, M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED);
    // m2m_gyro_x_res->set_value(0);
    // m2m_gyro_y_res->set_value(0);
    // m2m_gyro_z_res->set_value(0);
    m2m_gyro_x_res->set_observable(true);
    m2m_gyro_y_res->set_observable(true);
    m2m_gyro_z_res->set_observable(true);

    // GET resource 3200/0/5501
    // PUT also allowed for resetting the resource
    m2m_get_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3200, 0, 5501, M2MResourceInstance::INTEGER, M2MBase::GET_PUT_ALLOWED);
    if (m2m_get_res->set_value(0) != true) {
        printf("m2m_get_res->set_value() failed\n");
        return -1;
    }
    if (m2m_get_res->set_value_updated_function(get_res_update) != true) {
        printf("m2m_get_res->set_value_updated_function() failed\n");
        return -1;
    }

    // PUT resource 3201/0/5853
    m2m_put_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3201, 0, 5853, M2MResourceInstance::INTEGER, M2MBase::GET_PUT_ALLOWED);
    if (m2m_put_res->set_value(0) != true) {
        printf("m2m_put_res->set_value() failed\n");
        return -1;
    }
    if (m2m_put_res->set_value_updated_function(put_res_update) != true) {
        printf("m2m_put_res->set_value_updated_function() failed\n");
        return -1;
    }

    // POST resource 3201/0/5850
    m2m_post_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 3201, 0, 5850, M2MResourceInstance::INTEGER, M2MBase::POST_ALLOWED);
    if (m2m_post_res->set_execute_function(execute_post) != true) {
        printf("m2m_post_res->set_execute_function() failed\n");
        return -1;
    }

    // POST resource 5000/0/1 to trigger deregister.
    m2m_deregister_res = M2MInterfaceFactory::create_resource(m2m_obj_list, 5000, 0, 1, M2MResourceInstance::INTEGER, M2MBase::POST_ALLOWED);

    // Use delayed response
    m2m_deregister_res->set_delayed_response(true);

    if (m2m_deregister_res->set_execute_function(deregister) != true) {
        printf("m2m_deregister_res->set_execute_function() failed\n");
        return -1;
    }

    // optional Device resource for running factory reset for the device. Path of this resource will be: 3/0/5.
    m2m_factory_reset_res = M2MInterfaceFactory::create_device()->create_resource(M2MDevice::FactoryReset);
    if (m2m_factory_reset_res) {
        m2m_factory_reset_res->set_execute_function(factory_reset);
    }

    printf("Register Pelion Device Management Client\n\n");

    cloud_client->on_registration_updated(client_registration_updated);

    cloud_client->add_objects(m2m_obj_list);
    cloud_client->setup(network);

    t.start(callback(&queue, &EventQueue::dispatch_forever));
    // queue.call_every(3000ms, read_sensors);
    queue.call_every(polling_freq, read_sensors);
    // let's assume that pelion can take care of buffering the data
    // queue.call_every(sensor_upload_delay, update_data_buffer);

    // Flush the stdin buffer before reading from it
    flush_stdin_buffer();

    while(cloud_client_running) {
        int in_char = getchar();
        if (in_char == 'i') {
            print_client_ids(); // When 'i' is pressed, print endpoint info
            continue;
        } else if (in_char == 'r') {
            (void) fcc_storage_delete(); // When 'r' is pressed, erase storage and reboot the board.
            printf("Storage erased, rebooting the device.\n\n");
            ThisThread::sleep_for(1000ms);
            NVIC_SystemReset();
        } else if (in_char > 0 && in_char != 0x03) { // Ctrl+C is 0x03 in Mbed OS and Linux returns negative number
            value_increment(); // Simulate button press
            continue;
        }
        deregister_client();
        break;
    }
    return 0;
}

#endif /* MBED_TEST_MODE */
