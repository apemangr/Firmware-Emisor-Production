#include "app_util.h"
#include "array_math.h"
#include "boards.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h"

void Chip_temperature(void)
{
    // NRF_TEMP->TASKS_START = 1; /** Start the temperature measurement. */
    uint32_t err_code;
    err_code = sd_temp_get(&temp);
    APP_ERROR_CHECK(err_code);
    temp = (temp / 4);
    NRF_LOG_RAW_INFO("Temperatura actual del chip en celcius: %d\r\n", (int)(temp));
    NRF_LOG_FLUSH();
}

void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
    Transmiting_Ble    = false;
    Counter_Time_blink = 0;
    bsp_board_led_off(0);
    bsp_board_led_off(1);
    bsp_board_led_off(2);
    bsp_board_led_off(4);
    device_sleep = true;
    Show_reset_step();
    NRF_LOG_RAW_INFO("sleep By RTC\r\n");
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("ZZZZZzzz....\r\n");
    NRF_LOG_FLUSH();
    Device_on = false;
}

uint16_t GET_MEASURE_SENSOR_1(nrf_saadc_input_t Pin_Sensor, nrf_saadc_gain_t Gain)
{

    uint16_t medicion    = 0;
    uint16_t mediana_med = 0;
    uint16_t loop_med    = 0;
    uint16_t med_max     = 0;
    uint16_t med_min     = 0;
    int      T_med       = 10;
    uint16_t med[20];
    nrf_delay_ms(30); /// IMPORTANTE NO BORRAR CON ESTE COMANDO SE GENERA PAUSA PARA MEDICIONES
                      /// CORRECTAS ********* <------------

    for (loop_med = 0; loop_med < T_med; loop_med++)
    {
        nrf_delay_ms(10); // Agregar *********************************** 0.84 por ver si esta
                          // saltandose la medicion **********
        if (!nrf_drv_saadc_is_busy())
        {
            medicion = 0;
            medicion = Get_ADC_Perno(Pin_Sensor);
            if (medicion >= 20000)
                medicion = 0; // filtra mediciones fuera de rango.
            if (med_max == 0)
                med_max = medicion;
            if (med_min == 0)
                med_min = medicion;
            if (medicion <= med_min)
                med_min = medicion;
            if (medicion >= med_max)
                med_max = medicion;
            if (medicion <= 0)
                medicion = 0;
        }
        med[loop_med] = medicion;
    }
    mediana_med = mediana(med, T_med);
    NRF_LOG_RAW_INFO("Total Mediciones : %d, Min:%d , Max:%d , DIF : %d , Mediana :%d \r\n", T_med,
                     med_min, med_max, med_max - med_min, mediana_med);
    NRF_LOG_FLUSH();

    return mediana_med;
}

uint16_t GET_MEASURE_SENSOR(nrf_saadc_input_t Pin_Sensor, nrf_saadc_gain_t Gain)
{
    uint16_t loop_med = 0;
    int T_med = 10;
    uint16_t med[10];
    uint16_t mediana_med = 0;
    // bsp_board_led_on(3);
    LED_Control(1, 1, 50);
    LED_Control(0, 1, 0);

    // nrf_delay_ms(500);
    ADC_Perno_init(Pin_Sensor, Gain);
    // Se saca la calibracion del LOOP
    uint32_t Status = nrf_drv_saadc_calibrate_offset();
    DWT->CYCCNT     = 0;
    while (nrf_drv_saadc_is_busy())
    {
        if (DWT->CYCCNT > 6418000)
        {
            NRF_LOG_RAW_INFO("QUIEBRE EN *************** CALIBRACION  **********\r\n");
            NRF_LOG_FLUSH();
            break;
        }
    }

    for (loop_med = 0; loop_med < T_med; loop_med++)
    {
        med[loop_med] = GET_MEASURE_SENSOR_1(Pin_Sensor, Gain);
    }
    mediana_med = mediana(med, T_med);
    nrf_delay_ms(10);
    nrf_drv_saadc_channel_uninit(0);
    nrf_drv_saadc_uninit();
    // bsp_board_led_off(3);
    // nrf_delay_ms(30);

    DWT->CYCCNT = 0;
    while (nrf_drv_saadc_is_busy())
    { // while evita que emisor se quede pegado en la instruccion
        if (DWT->CYCCNT > 6418000)
        {
            NRF_LOG_RAW_INFO("QUIEBRE EN *************** Cierre de medicion **********\r\n");
            NRF_LOG_FLUSH();
            break;
        }
    }
    return mediana_med;
}
