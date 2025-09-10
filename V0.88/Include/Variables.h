#ifndef __VARIABLES_H
#define __VARIABLES_H

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_rtc.h"

#define APP_BLE_CONN_CFG_TAG 1 /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME          "SmartW" /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE                                                                      \
    BLE_UUID_TYPE_VENDOR_BEGIN /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO                                                                      \
    3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_ADV_INTERVAL                                                                           \
    200                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). antes 64*/  //antes 90

#define MIN_CONN_INTERVAL                                                                          \
    MSEC_TO_UNITS(10, UNIT_1_25_MS) /**< Minimum acceptable connection interval (20 ms),           \
                                       Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL                                                                          \
    MSEC_TO_UNITS(75, UNIT_1_25_MS) /**< Maximum acceptable connection interval (75 ms),           \
                                       Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY 0             /**< Slave latency. */
#define CONN_SUP_TIMEOUT                                                                           \
    MSEC_TO_UNITS(4000, UNIT_10_MS) /**< Connection supervisory timeout (4 seconds), Supervision   \
                                       Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY                                                             \
    APP_TIMER_TICKS(5000) /**< Time from initiating event (connect or start of notification) to    \
                             first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY                                                              \
    APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the   \
                              first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT                                                               \
    3 /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                                                                                  \
    0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location \
                  on stack unwind. */

#define UART_TX_BUF_SIZE  256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE  256 /**< UART RX buffer size. */

#define Largo_Advertising 0x18 // Largo_Advertising  10 son 16 y 18 son 24

#define COMPARE_COUNTERTIME                                                                        \
    (2UL) // 3UL   este dato es la cantidad de segundos de seteo del comparador RTC.

#define LEDBUTTON_BUTTON_PIN                                                                       \
    BSP_BUTTON_0 /**< Button that will write to the LED characteristic of the peer */
#define BUTTON_DETECTION_DELAY                                                                     \
    APP_TIMER_TICKS(50) /**< Delay from a GPIOTE event until a button is reported as pushed (in    \
                           number of timer ticks). */

#define PA_TX_MODE           0
#define PA_RX_MODE           1
#define PA_OFF_MODE          2

#define APP_BLE_CONN_CFG_TAG 1

#define SIZE_FLASH           8000 // debe guardar de multiplos de 32 bytes

#define Size_Memory_History  250

// **********************************************Fstorage

uint8_t  m_beacon_info[Largo_Advertising];
uint32_t contador = 0x00;
int8_t   Potencia_antenna =
    -20; // 4                        // Radio transmit power in dBm (accepted values are -40, -30,
         // -20, -16, -12, -8, -4, 0, 3, and 4 dBm). 4 maximo para nrf52832
int16_t Max_Time_Unconfident =
    400; // Tiempo maximo de conexion establecida sin asegurar fuente confiable, 40 segundos.

bool     device_sleep         = false;
uint32_t sleep_in_time_ticker = 0x6000; /** 430 son 43 segundos 0x4646A*/
// 0x69780 son 12 horas
// 0x4646A son 8 horas menos 15 segundos

uint32_t APP_ADV_DURATION = 500; /**< 1500 son 15 segundos The advertising duration (180 seconds) in
                                    units of 10 milliseconds.  era 800 */

uint32_t            Counter_sleep_ticker;

bool                Wakeup_by_Timer = false;
const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); // Declaring an instance of nrf_drv_rtc for RTC0.
uint16_t            loop_timer;

bool                Device_on        = true;
bool                Wakeup_by_button = false;
uint16_t            loop_button;

bool                Device_BLE_connected = false;
bool                connection_confident = false;
int16_t             Counter_to_disconnect;

bool                Sensor_connected = false;
bool                Boton_Presionado = false;
int16_t             Time_button_pressed;
int16_t             MAX_Time_Button_pressed = 20;

bool                Transmiting_Ble         = false;
uint16_t            Counter_Time_blink;

int8_t              Blink_led_Time_on  = 1;
int8_t              Blink_led_Time_off = 7;
bool                Led_encendido      = false;

float               Lenght_Cut;       // lARGO DEL CORTE.
float               Step_of_resistor; //  2 millimiter each resistors
float               Resistor_Cuted;

uint16_t            Battery_level = 0;

int32_t             temp;

uint8_t             data_flash[SIZE_FLASH] = {0};
uint32_t            len                    = SIZE_FLASH; // Cantidad de Datos Guardado en la Flash

bool                Write_Flash            = false;
bool                Write_DATE             = false;
uint16_t            Timer_write_Date       = 600; // cada 30 segundos graba la hora y fecha en flash
uint16_t            Counter_Timer_write_Date;     // cada 30 segundos graba la hora y fecha en flash

uint8_t             Tipo_dispositivo       = 0x10; // 10 Dispositivo desagaste Placas
uint8_t             Tipo_Configuracion_ohm = 0x00; // 00 para 6200 ohm  01 para 6800 ohm
uint8_t             Offset_desgaste =
    0x00; // valor que corrige la distancia entre el perno y el inicio de la placa.
int8_t Offset_sensor = 0x00; // valor que corrige cuando se corta un sensor, es decir se corta para
                             // adaptarlo a un perno.

uint8_t  Reset_Index = 0;    // posicion de la matrix del reset.
uint8_t  Reset_Line  = 0x00; // posicion de la matrix del reset.

bool     Rtc_waiting = false;
uint8_t  rtc_time;

bool     Another_Value = false;
uint8_t  History_Day_Counter = 0; // Contador de días para historial cada 2 días

uint16_t Valor_1       = 0;
uint16_t Valor_2       = 0;

uint16_t loop_send_med;
int8_t   Tipo_Envio       = 0;
int8_t   Configuration    = 1;
int8_t   History          = 2;
int8_t   Last_History     = 3;
int8_t   History_By_Index = 4;
int8_t   Values           = 5;
bool     Next_Sending     = false;

bool     impresion_log    = false;
bool     History_Buffer_Full = false;  // Indica si el buffer de historial está lleno

int8_t   Sensibilidad_Res = 0;

typedef struct
{
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint32_t Contador;
    uint16_t V1;
    uint16_t V2;
    uint16_t V3;
    uint16_t V4;
    uint16_t V5;
    uint16_t V6;
    uint16_t V7;
    uint16_t V8;
    uint8_t  temp;
    uint8_t  battery;
    uint8_t  antpwr;
} store_History;

uint16_t      History_Position = 0;
uint16_t      Sending_Position = 0;
store_History History_value;

typedef struct
{
    uint8_t       company[2];
    uint8_t       total_reset[4];
    uint8_t       total_adv[4];
    bool          Enable_Custom_mac;
    uint8_t       mac_custom[6];
    uint8_t       sleep_time[3];
    uint8_t       adv_time[2];
    uint8_t       mac_original[6];
    uint8_t       Type_resistor;
    uint8_t       Type_sensor;
    uint8_t       Type_battery;
    uint8_t       offset_plate_bolt; // distancia desde sensor al
    uint8_t       Version[3];
    uint8_t       Offset_sensor_cut;
    uint8_t       Offset_sensor_bolt;
    uint8_t       Offset_sensor_measure;
    uint8_t       Sending_Position;
    uint8_t       type_advertising;
    uint8_t       Sensibility;
    uint8_t       fecha[6];
    uint8_t       hora[6];
    uint8_t       reset[14];
    uint8_t       second;
    uint8_t       minute;
    uint8_t       hour;
    uint8_t       date;
    uint8_t       month;
    uint16_t      year;
    uint16_t      last_history;
    store_History history[Size_Memory_History];
} store_flash;

store_flash Flash_array;
/*
uint8_t Flash_storage_datos_init[SIZE_FLASH]=
{0x33,0x22,                     // Company                        Comienza en 0
0x0,0x0,0x0,0x0,                // Counter Global Restart         Comienza en 2
0x0,0x0,0x0,0x0,                // Counter Global ADV             Comienza en 6
0x00,                           // New mac? 1                     Comienza en 10
0x00,0x00,0x00,0x00,0x00,0x00,  // Mac                            Comienza en 11
0x4,0x64,0x6A,                  // Sleep Time  in mSecond         Comienza en 17          4646A son
8 horas 0x5,0xdc,                       // ADV Time in mSecond            Comienza en 20
0x0,0x0,0x0,0x0,0x0,0x0,        // MAC de fabrica                 Comienza en 22
0x00,                           // Tipo 00 6.2Kohm 01 6.8Kohm     Comienza en 28          Para
corregir el error. 0x10,                           // Tipo de Sensor                 Comienza en 29
// 10 Perno     de 120 mm Espaciamiento 2 mm Total 22 divisiones
// 11 Esparrago de 250 mm Espaciamiento 6 mm total 32 Divisiones
// 12 Esparrago de 200 mm Espaciamiento 6 mm total 23 Divisiones�
// 13 Esparrago de 70.2 mm Espaciamiento 1.3 mm total 54 Divisiones
0x1,                            // 00 Bateria de 3.6 V 1200 mAh   Comienza en 30
// 01 Bateria de 3.7 v 500 mAh


0x80,                            // creacion de offset de desgaste Comienza en 31      el cero es
128 de valor
// Mayor a 128 significa que medicion esta hacia afuera de la placa
// Menor a 128 significa que medicion esta dentro de la placa.

0x1,0x0,0x1,                     // Version de la app  32,33,34
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};  // comienza en 35
*/
store_flash Flash_Factory;

// Función para resetear el buffer de historial manualmente (opcional)
static inline void Reset_History_Buffer(void)
{
    History_Buffer_Full = false;
    History_Position = 0;
    Flash_array.last_history = 0;
    NRF_LOG_RAW_INFO("Buffer de historial reseteado manualmente\r\n");
    NRF_LOG_FLUSH();
}

#endif // __VARIABLES_H
