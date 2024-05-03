#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
 
 
#include "driver/uart.h"
 
#include "sdkconfig.h"
#include "esp_log.h"
#include "cJSON.h"
#include "esp_adc_cal.h"
 
#include "driver/twai.h"
 
#include "esp_timer.h"
 
#define MAX_FIFO_SIZE 10
#define Ignition 14
#define Reverse 12
#define Break 15
#define ModeL 18
#define ModeR 13
#define SideStand 16
 
#define temp5 23 // adc
#define temp6 25 // adc
#define temp7 26 // adc
#define temp8 27 // adc
 
int ingi =0;
int modeL = 0;
int modeR = 0;
 
int brake = 0;
int reve = 0;
int sidestand = 0 ;
 
// int temp1 = 0;
// int temp2 =0;
// int temp3 = 0;
// int temp4 = 0;
// int temp5 = 0;
// int temp6 = 0;
// int temp7 = 0;
// int temp8 = 0;
//char mode[] = "";
//char arr[3][10] = {"ingi","modeL","modeR", "brake", "reve"};
 
 
//#include <ds18b20.h> // temperature sensor
 
/* --------------------- Definitions and static variables ------------------ */
// Example Configuration
#define PING_PERIOD_MS 250
#define NO_OF_DATA_MSGS 50
#define NO_OF_ITERS 3
#define ITER_DELAY_MS 500
#define RX_TASK_PRIO 9
#define TX_TASK_PRIO 11
#define CTRL_TSK_PRIO 8
#define TX_GPIO_NUM CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM CONFIG_EXAMPLE_RX_GPIO_NUM
#define EXAMPLE_TAG "TWAI Master"
 
#define ID_MASTER_STOP_CMD 0x0A0
#define ID_MASTER_START_CMD 0x0A1
#define ID_MASTER_PING 0x0A2
#define ID_SLAVE_STOP_RESP 0x0B0
#define ID_SLAVE_DATA 0x0B1
#define ID_SLAVE_PING_RESP 0x0B2
 
#define ID_BATTERY_3_VI 0x1725
#define ID_BATTERY_2_VI 0x1bc5
#define ID_BATTERY_1_VI 0x1f05
 
#define ID_BATTERY_3_SOC 0x1727
#define ID_BATTERY_2_SOC 0x1bc7
#define ID_BATTERY_1_SOC 0x1f07
 
#define ID_LX_BATTERY_VI 0x6
#define ID_LX_BATTERY_T 0xa
#define ID_LX_BATTERY_SOC 0x8
#define ID_LX_BATTERY_PROT 0x9
 
 
 
#define ID_MOTOR_RPM 0x230
#define ID_MOTOR_TEMP 0x233
#define ID_MOTOR_CURR_VOLT 0x32
//#define ADC_WIDTH_BIT_DEFAULT (ADC_WIDTH_BIT_9 - 1)
 
 
static float Voltage_1 = 0;
static float Current_1 = 0;
static float SOC_1 = 0;
static float SOH_1 = 0;
 
static int Voltage_2 = 0;
static int Current_2 = 0;
static float SOC_2 = 0;
static float SOH_2 = 0;
 
static float Voltage_3 = 0;
static float Current_3 = 0;
static int SOC_3 = 0;
static int SOH_3 = 0;
 
static uint16_t iDs[ 3 ] = {0x1f00,0x1bc0,0x1720};
 
//static uint16_t iDs[2] = {0x1f00, 0x1bc0};
 
static uint8_t state = 0;
static int M_CONT_TEMP = 0;
static int M_MOT_TEMP = 0;
static int M_THROTTLE = 0;
 
static int M_AC_CURRENT = 0;
static int M_AC_VOLTAGE = 0;
static int M_DC_CURRENT = 0;
static int M_DC_VOLATGE = 0;
 
static int S_DC_CURRENT = 0;
static int S_AC_CURRENT1 = 0;
static int S_AC_CURRENT2 = 0;
 
static float TRIP_1 = 0;
static float TRIP1 = 0;
 
static int t_stamp = 0;
int adc_value = 0;
int adc_value1 = 0;
int adc_value2 = 0;
 
uint32_t RPM;
 
char motor_err[32];
 
char batt_err[32];
char batt_temp[32];
 
 
uint32_t THROTTLE;
uint32_t CONT_TEMP;
uint32_t MOT_TEMP;
 
uint32_t DC_CURRENT;
uint32_t AC_CURRENT;
uint32_t AC_VOLTAGE;
uint32_t DC_VOLTAGE;
 
uint32_t voltage1_hx;
uint32_t current1_hx;
 
uint32_t voltage2_hx;
uint32_t current2_hx;
 
uint32_t voltage3_hx;
uint32_t current3_hx;
 
uint32_t SOC1_hx;
uint32_t SOH1_hx;
 
uint32_t SOC2_hx;
uint32_t SOH2_hx;
 
uint32_t SOC3_hx;
uint32_t SOH3_hx;
 
uint32_t Motor_err;
 
char bat1[32];
char bat2[32];
char bat3[32];
char vol_avg[32];
char sOC_avg[32];
char cur_tot[32];
char CVoltage_1[32];
char CVoltage_3[32];
char CVoltage_2[32];
char CCurrent_1[32];
char CCurrent_2[32];
char CCurrent_3[32];
char CSOC_1[32];
char CSOC_2[32];
char CSOC_3[32];
char CSOH_1[32];
char CSOH_2[32];
char CSOH_3[32];
char T_stamp[32];
char sensor[32];
char rpm[64];
char speed[64];
char throttle[64];
 
char motorTemp[64];
char contTmep[64];
char acCurrent[64];
char acVoltage[64];
char dcCurrent[64];
char dcVoltage[64];
char s_dcCurrent[64];
char s_acCurrent1[64];
char s_acCurrent2[64];
char trip1[64];
 
/////////////////////////////////////////////////////////////////////////////////
 
// #static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
// #static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
 
// static const twai_general_config_t g_config = {.mode = TWAI_MODE_NORMAL,
// .tx_io = GPIO_NUM_21,
// .rx_io = GPIO_NUM_22,
// .clkout_io = TWAI_IO_UNUSED,
// .bus_off_io = TWAI_IO_UNUSED,
// .tx_queue_len = 10,
// .rx_queue_len = 20,
// .alerts_enabled = TWAI_ALERT_ALL,
// .clkout_divider = 0};
 
// static QueueHandle_t tx_task_queue;
// static QueueHandle_t rx_task_queue;
// static SemaphoreHandle_t stop_ping_sem;
// static SemaphoreHandle_t cnt_Switch_start;
// static SemaphoreHandle_t done_sem;
// static SemaphoreHandle_t ctrl_task_Transmit;
// static SemaphoreHandle_t ctrl_task_receive;
// static SemaphoreHandle_t ctrl_task_send;
 
QueueHandle_t control_queue;
 
typedef struct {
    int modeR;
    int modeL;
    int brake;
    int ingi;
    int reve;
    int sidestand;
} ControlData;
 
 
/* --------------------------- Tasks and Functions -------------------------- */
// void decToBinary(int n)
// {
//    int binaryNum[32];
//    int i =0 ;
//    while(n>0){
//       binaryNum[i] = n % 2 ;
//       n = n/2 ;
//       i++ ;
 
//    }
 
//    for(int j= i -1 ; j>=0 ; j--)
//    {
//       printf("%d\n", binaryNum[j]);
//    }
// }
uint8_t arr[8];
uint8_t combinedValue = 0;
//int val_of_temp = 0;
int new_val ;
 
// esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
// esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
//     //Check type of calibration value used to characterize ADC
// if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
//     printf("eFuse Vref");
// } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
//     printf("Two Point");
// } else {
//     printf("Default");
// }
void switch_ip(void *arg) {
    ControlData control_data;
    while(1) {
        // Read GPIO inputs and update control_data struct
        control_data.modeR = !gpio_get_level(ModeR);
        control_data.modeL = !gpio_get_level(ModeL);
        control_data.brake = !gpio_get_level(Break);
        control_data.ingi = !gpio_get_level(Ignition);
        control_data.reve = !gpio_get_level(Reverse);
        control_data.sidestand = !gpio_get_level(SideStand);
 
        // Create a union to convert control_data struct to uint32_t
        union {
            ControlData data;
            uint32_t state;
        } u;
        u.data = control_data;
        uint32_t state = u.state;
 
        // Send the control data to the queue
        xQueueSend(control_queue, &state, portMAX_DELAY);
 
        vTaskDelay(pdMS_TO_TICKS(50)); // Adjust delay as needed
    }
}
 
int Dout ;
int Vmax = 100 ;
int Dmax = 4095 ;
int rpm_max= 4500;
static float batt_tmp = 0 ;
static void battery_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0); //gpio 32
    while(1)
    {
        int val_of_temp = adc1_get_raw(ADC1_CHANNEL_4);
        //uint32_t voltage = esp_adc_cal_raw_to_voltage(val_of_temp, adc_chars);
        //'float Vout = (float)3000 * (float)(Vmax / Dmax);
        //Vout = (float)((int)(Vout * 100.0)) / 100.0;
        batt_tmp = (float)val_of_temp * ((float)Vmax / (float)Dmax);
        //printf("the value shown %d \n", val_of_temp);//val_of_temp);
        // printf("The temperature of the battery is : %.4f \n", batt_tmp);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(50/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
 
// int Dout ;
// int Vmax = 80 ;
// int Dmax = 4095 ;
static float cnt_tmp = 0;
static void controller_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);
    while(1)
    {
        int val_temp_controller = adc1_get_raw(ADC1_CHANNEL_6);//gpio34
        //uint32_t voltage = esp_adc_cal_raw_to_voltage(val_of_temp, adc_chars);
        //'float Vout = (float)3000 * (float)(Vmax / Dmax);
        //Vout = (float)((int)(Vout * 100.0)) / 100.0;
        cnt_tmp = (float)val_temp_controller * ((float)Vmax / (float)Dmax);
        //printf("the value shown %d \n", val_temp_controller);//val_of_temp);
        // printf("MCU temperature is : %.4f \n", cnt_tmp);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(50/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
 
 
// int Dout ;
// int Vmax = 80 ;
// int Dmax = 4095 ;
static float pcb = 0 ;
static void pcb_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_0);
    while(1)
    {
        int val_temp_pcb = adc1_get_raw(ADC1_CHANNEL_7); //GPIO -35
       
        pcb = (float)val_temp_pcb * ((float)Vmax / (float)Dmax);
        //printf("the value shown %d \n", val_temp_pcb);//val_of_temp);
        // printf("PCB temperature is : %.4f \n", pcb);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(50/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
static float soc = 0 ;
static void soc_battery(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_0);
    while(1)
    {
        int soc_of_batt = adc1_get_raw(ADC1_CHANNEL_3); // GPIO - 39
        //uint32_t voltage = esp_adc_cal_raw_to_voltage(val_of_temp, adc_chars);
        //'float Vout = (float)3000 * (float)(Vmax / Dmax);
        //Vout = (float)((int)(Vout * 100.0)) / 100.0;
        soc = (float)soc_of_batt * ((float)Vmax / (float)Dmax);
        //printf("the value shown %d \n", soc_of_batt);//val_of_temp);
        printf("SoC of the battery is : %.4f  \n", soc);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(50/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
int Dout ;
int V_motor_max = 150 ;
int D_motor_max = 4095 ;
static float V_motor_out = 0;
static void motor_temp(void *arg)     //Conversion of analog to digital signal
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_0);
    while(1)
    {
        int val_temp_motor = adc1_get_raw(ADC1_CHANNEL_5); // gpio33
        //uint32_t voltage = esp_adc_cal_raw_to_voltage(val_of_temp, adc_chars);
        //'float Vout = (float)3000 * (float)(Vmax / Dmax);
        //Vout = (float)((int)(Vout * 100.0)) / 100.0;
        V_motor_out = (float)val_temp_motor * ((float)V_motor_max / (float)D_motor_max);
        //printf("the value shown %d \n", val_temp_motor);//val_of_temp);
        // printf("Temperature of the motor is : %.4f \n", V_motor_out);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(50/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
// int Dout ;
// int Vmax = 80 ;
// int Dmax = 4095 ;
static float thr_per = 0 ;
int thr_per_hex;
static char thr_per_hex_str[20];
                       //For storing the hexadecimal value
static void throttle_percentage(void *arg)     //Conversion of analog to digital signal
{
    int rpm;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);// gpio 36
    while(1)
    {
        int throttle_per = adc1_get_raw(ADC1_CHANNEL_0);//,ADC_WIDTH_BIT_12, NULL);
        printf("Throttle per Debug1(raw)---------->%d \n",throttle_per);
        //uint32_t voltage = esp_adc_cal_raw_to_voltage(val_of_temp, adc_chars);
        //'float Vout = (float)3000 * (float)(Vmax / Dmax);
        //Vout = (float)((int)(Vout * 100.0)) / 100.0;
        thr_per = (float)throttle_per * ((float)Vmax / 4095);

        //for converting to hexadecimal
        thr_per_hex= (int)thr_per;
        // Convert thr_per_hex to a hexadecimal string
        sprintf(thr_per_hex_str, "%x", thr_per_hex);

        //printf("the value shown %d \n", throttle_per);//val_of_temp);
        printf("The throttle percentage is -------------->: %.4f \n", thr_per);
        rpm= thr_per *(((float) rpm_max)/((float)Vmax)) ;
        printf("The Rpm Value is :%d \n", rpm);
        //ESP_LOGD(EXAMPLE_TAG, "Failed to queue message for transmission\n");
        vTaskDelay(50/portTICK_PERIOD_MS);
            //new_val = val_of_temp ;
    }
    vTaskDelay(NULL);
}
 
void twai_transmit_task(void *arg)
 {
    ControlData control_data;
    twai_message_t transmit_message;
    while(1)
    {
        if(xQueueReceive(control_queue, &control_data, portMAX_DELAY))
        {
            printf("Entered transmit");
            // Prepare message
            transmit_message.identifier = 0x18530902; // Replace with actual identifier
            transmit_message.data_length_code = 8;
            transmit_message.extd = 1;
            transmit_message.data[0] = 0x64; // Example data, replace with actual data
            transmit_message.data[1] = 0x03;
            transmit_message.data[2] = 0x00;
            transmit_message.data[3] = state;
            transmit_message.data[4] = 0x00;
            transmit_message.data[5] = 0x00;            
            transmit_message.data[6] = 0x00;
            transmit_message.data[7] = 0x00;
 
            // Transmit message
            if(twai_transmit(&transmit_message, 1000) == ESP_OK) {
                ESP_LOGI("TWAI", "Message queued for transmission");
            } else {
                ESP_LOGE("TWAI", "Failed to queue message for transmission");
            }
 
            transmit_message.identifier = 0x000000A; // Replace with actual identifier
            transmit_message.data_length_code = 8;
            transmit_message.extd = 1;
            transmit_message.data[0] = batt_tmp; // Example data, replace with actual data
            transmit_message.data[1] = batt_tmp;
            transmit_message.data[2] = batt_tmp;
            transmit_message.data[3] = batt_tmp;
            transmit_message.data[4] = batt_tmp;
            transmit_message.data[5] = batt_tmp;
            transmit_message.data[6] = batt_tmp;
            transmit_message.data[7] = batt_tmp;
 
            // Transmit message
            if(twai_transmit(&transmit_message, 1000) == ESP_OK) {
                ESP_LOGI("TWAI", "Message queued for transmission");
            } else {
                ESP_LOGE("TWAI", "Failed to queue message for transmission");
            }
 
            transmit_message.identifier = 0x18530903; // Replace with actual identifier
            transmit_message.data_length_code = 8;
            transmit_message.extd = 1;
            transmit_message.data[0] = cnt_tmp; // Example data, replace with actual data
            transmit_message.data[1] = V_motor_out;
            transmit_message.data[2] = V_motor_out;
            transmit_message.data[3] = 0x00;
            transmit_message.data[4] = 0x00;
            transmit_message.data[5] = 0x00;
            transmit_message.data[6] = 0x00;
            transmit_message.data[7] = 0x00;
 
            // Transmit message
            if(twai_transmit(&transmit_message, 10) == ESP_OK) {
                ESP_LOGI("TWAI", "Message queued for transmission");
            } else {
                ESP_LOGE("TWAI", "Failed to queue message for transmission");
            }
 
        }
    }
}
 
 
 
 
void app_main(void)
{
control_queue = xQueueCreate(MAX_FIFO_SIZE, sizeof(ControlData));
//xTaskCreatePinnedToCore(switch_ip, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
 
xTaskCreate(throttle_percentage , "throttle", 4096, NULL, 5, NULL);
xTaskCreate(pcb_temp , "pcb temp", 4096, NULL, 8, NULL);
xTaskCreate(motor_temp , "motor temp", 4096, NULL, 8, NULL);
xTaskCreate(battery_temp , "battery temperature", 4096, NULL, 8, NULL);
xTaskCreate(controller_temp , "motor controller temperature",4096 , NULL, 8, NULL);
xTaskCreate(soc_battery, "battery SoC", 4096, NULL, 8, NULL);
xTaskCreate(switch_ip, "Swicth_Tsk", 4096, NULL, 8, NULL);
//xTaskCreate(twai_transmit_task, "transmit_Tsk", 4096, NULL, 8, NULL);
xTaskCreate(twai_transmit_task, "Transmit_Tsk", 4096, NULL, 8, NULL);
//xSemaphoreGive(cnt_Switch_start); // Start control task
 
}