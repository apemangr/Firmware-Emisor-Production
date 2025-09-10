#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h" 
#include "boards.h" 
#include "app_util.h"

#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "nrf_fstorage_nvmc.h"

#include "nrf_drv_saadc.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_temp.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "History.h" 
#include "Variables.h"
#include "Battery.h"
#include "calendar.h"
#include "ADC_Perno.h"
#include "Sensor_desgaste.h"
#include "FSstorage.h"
#include "Led_signal.h"
#include "Antena.h"
#include "button.h"
#include "Common.h"


extern void cli_init(void);
extern void cli_start(void);
extern void cli_process(void);

int main(void)
{
  ret_code_t err_code; 
  log_init();
  FsStorage_Init();
  Fstorage_Read_Data();
  Flash_factory();

  Flash_reset_check();          Reset_Line_Step(1); 
  NRF_LOG_RAW_INFO("**** INICIO DEL PROGRAMA **************************** 04 **  \r\n");
  NRF_LOG_RAW_INFO("**** STEP DEL RESET %i \r\n", (Reset_Index));
  
  NRF_LOG_FLUSH();              Reset_Line_Step(2);


  
  nrf_temp_init();              Reset_Line_Step(3);
  timers_init();                Reset_Line_Step(4);
  buttons_init();               Reset_Line_Step(5);
  leds_init();                  Reset_Line_Step(6);
  err_code = app_button_enable();
  APP_ERROR_CHECK(err_code);
  
  rtc_config();                 Reset_Line_Step(9);
  
  Led_intro();                  Reset_Line_Step(7);
  
  Rtc_waiting=true;
  rtc_time=20;
  
  NRF_LOG_FLUSH();              Reset_Line_Step(8); 

  nrf_delay_ms(200);            Reset_Line_Step(10);
  ble_stack_init();             Reset_Line_Step(11);
  pa_assist(TX_PA,RX_PA);       Reset_Line_Step(12);
  
  Reset_Line = 8;
  err_code = Flash_is_empty();
  APP_ERROR_CHECK(err_code);
  
  Tipo_dispositivo = Flash_array.Type_sensor;
  Offset_desgaste  = Flash_array.offset_plate_bolt;
  Offset_sensor    = Flash_array.Offset_sensor_cut;
  Sensibilidad_Res = Flash_array.Sensibility;


    if (impresion_log) { 
  for (int loop_med= 0 ; loop_med<=10; loop_med++)
  {
    
    NRF_LOG_RAW_INFO("fecha %d/%d/%d ",Flash_array.history[loop_med].day,Flash_array.history[loop_med].month,Flash_array.history[loop_med].year);    
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("%d:%d:%d ",Flash_array.history[loop_med].hour,Flash_array.history[loop_med].minute,Flash_array.history[loop_med].second);    
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("historia %d:- %d/%d/%d/%d \r\n",loop_med,Flash_array.history[loop_med].Contador,Flash_array.history[loop_med].V1,Flash_array.history[loop_med].V2,Flash_array.history[loop_med].battery);
    NRF_LOG_FLUSH();
    nrf_delay_ms(20);
  }
    }
  if (Tipo_dispositivo==0x10) {Step_of_resistor=2;}
  if (Tipo_dispositivo==0x11) {Step_of_resistor=6;}
  if (Tipo_dispositivo==0x12) {Step_of_resistor=6;}
  if (Tipo_dispositivo==0x13) {Step_of_resistor=1.3000f;}  
  if (Tipo_dispositivo==0x14) {Step_of_resistor=1.3000f;}   // Con alerta interior Perno.
  if (Tipo_dispositivo==0x15) {Step_of_resistor=1.5000f;}  // Sensor 100 divisiones de 1.5mm
  if (Tipo_dispositivo==0x16) {Step_of_resistor=4.0000f;}  // Sensor 100 divisiones de 4  mm
 
  
  History_Position       = Flash_array.last_history;
  
  // Verificar si el buffer de historial ya está lleno
  if (History_Position >= (Size_Memory_History - 1))
  {
    History_Buffer_Full = true;
    NRF_LOG_RAW_INFO("Buffer de historial detectado como lleno al inicializar\r\n");
    NRF_LOG_FLUSH();
  }
  
  History_value.Contador = Flash_array.history[History_Position].Contador;
  History_value.V1       = Flash_array.history[History_Position].V1;
  History_value.V2       = Flash_array.history[History_Position].V2;
  History_value.battery  = Flash_array.history[History_Position].battery;
  
  t.second=Flash_array.second;
  t.minute=Flash_array.minute ;
  t.hour=Flash_array.hour;
  t.date=Flash_array.date;
  t.month=Flash_array.month ;
  t.year=Flash_array.year ;
  
  NRF_LOG_RAW_INFO("mm step resistor recien configurado " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(Step_of_resistor));  
  Counter_restart();
  Fstorage_Erase_Writing();
  Fstorage_Read_Data(); 


    if (Flash_array.Enable_Custom_mac==0x01 && Flash_array.mac_custom[0]!=0x00)                             // Si hay que ocupar la mac custom y el dato no es cero.
  {
    NRF_LOG_RAW_INFO("Setting Custom Mac Address.... \r\n");
    NRF_LOG_FLUSH();
    set_addr();
  }

  nrf_delay_ms(100);            Reset_Line_Step(13);    // carga condensadores
  gap_params_init();            Reset_Line_Step(14);
  gatt_init();                  Reset_Line_Step(15);
  services_init();              Reset_Line_Step(16);
  advertising_init();           Reset_Line_Step(17);
  conn_params_init();           Reset_Line_Step(18);
  nrf_delay_ms(150);                                    // carga condensadores
  advertising_start();          Reset_Line_Step(19);
  
  for (;;)
  {
    if (Wakeup_by_Timer) 
    {
      Reset_Line_Step(20);
      Counter_sleep_ticker=0;
      loop_timer++;
      Device_on = true;
      Wakeup_by_Timer = false;
      NRF_LOG_RAW_INFO("Starting by RTC No. %d y contador %d  ",loop_timer,contador);
      NRF_LOG_RAW_INFO("%02i/%02i/%02i - ",t.year,t.month,t.date);
      NRF_LOG_RAW_INFO("%02i:%02i:%02i:%02i \r\n",t.hour,t.minute,t.second,t.msecond);
      
      NRF_LOG_FLUSH();
      advertising_init();
      conn_params_init();
      Reset_Line_Step(21);
      advertising_start();
    }
    
    if (Wakeup_by_button) 
    {
      Reset_Line_Step(22);
      Counter_sleep_ticker=0;
      loop_button++;
      Wakeup_by_button = false;
      Device_on = true;
      NRF_LOG_RAW_INFO("Starting by button No. %d y contador %d  ",loop_button,contador);
      NRF_LOG_RAW_INFO("%02i/%02i/%02i - ",t.year,t.month,t.date);
      NRF_LOG_RAW_INFO("%02i:%02i:%02i:%02i \r\n",t.hour,t.minute,t.second,t.msecond);
      NRF_LOG_FLUSH();
      advertising_init();
      conn_params_init();
      Reset_Line_Step(23);
      advertising_start();
    }
    if (Device_BLE_connected)
    {
      if ((Tipo_Envio > 0) && (Next_Sending))
      {
         Next_Transmition();
      }
      if (Counter_to_disconnect>=Max_Time_Unconfident)
      {
        Counter_to_disconnect =0;
        if(m_conn_handle != BLE_CONN_HANDLE_INVALID) {
          NRF_LOG_RAW_INFO("DESCONECCION POR FUENTE NO CONFIRMADA\r\n");
          NRF_LOG_FLUSH();
          sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);      
        }
      }
    }
    if (Boton_Presionado)
    {
      if (Time_button_pressed>=MAX_Time_Button_pressed)
      {
        NRF_LOG_RAW_INFO("DETECCION BOTON PRESIONADO \r\n");
        NRF_LOG_FLUSH();
        Time_button_pressed=0;
        if (device_sleep)
        {
          Tipo_dispositivo=0xff;                          // Modo configuracion
          Wakeup_by_button=true;
          device_sleep=false; 
          NRF_LOG_RAW_INFO("BOTON PRESIONADO y equipo despertado ");
          NRF_LOG_RAW_INFO("%02i/%02i/%02i - ",t.year,t.month,t.date);
          NRF_LOG_RAW_INFO("%02i:%02i:%02i:%02i \r\n",t.hour,t.minute,t.second,t.msecond);
          NRF_LOG_FLUSH();   
        }
      }
    }
    
    if (Transmiting_Ble && !Device_BLE_connected)
    {
      if (Led_encendido && Counter_Time_blink>=Blink_led_Time_on)
      {
        Counter_Time_blink=0;
        Led_encendido=false;
        bsp_board_led_off(0);
        bsp_board_led_off(4);
        if (Sensor_connected) 
        {bsp_board_led_off(1);
        bsp_board_led_off(4);
        }
        else
          bsp_board_led_off(2);      
      }
      if (!Led_encendido && Counter_Time_blink>=Blink_led_Time_off)
      {
        Counter_Time_blink=0;
        
        Led_encendido=true;
        bsp_board_led_on(0);
        bsp_board_led_on(4);
        if (Sensor_connected) 
          bsp_board_led_on(1);
        else
          bsp_board_led_on(2);      
      }     
    }
    
    if (Transmiting_Ble && Device_BLE_connected)
    {
      bsp_board_led_on(0);
      bsp_board_led_on(4);
      bsp_board_led_off(1);
      bsp_board_led_off(2);
      
    }
    
    if (Write_Flash && !Device_BLE_connected) 
    { 
      Write_Flash=false;
      Fstorage_Erase_Writing();
      Fstorage_Read_Data();
      Reset_Line_Step(0);
      NVIC_SystemReset(); 
    } 
    if (Write_DATE && !Device_BLE_connected) 
    { 
      Flash_array.fecha[0]=(t.year-2000);
      Flash_array.fecha[1]=(t.month);
      Flash_array.fecha[2]=(t.date);
      Flash_array.hora[0]=t.hour;
      Flash_array.hora[1]=t.minute;
      Flash_array.hora[2]=t.second;
      
      Write_DATE=false;
      Fstorage_Erase_Writing();
      Fstorage_Read_Data();
      Reset_Line_Step(0);
    } 
    
    idle_state_handle();
  }
}

/**
* @}
*/
