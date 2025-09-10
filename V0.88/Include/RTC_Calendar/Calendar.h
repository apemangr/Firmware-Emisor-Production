/*
 * 2019 Crea-lab Spa
 */

#ifndef CALENDAR_H
#define CALENDAR_H

#ifdef __cplusplus
extern "C"
{
#endif

    static bool not_leap(void);
    static void init(void);

    typedef struct
    {

        uint8_t  msecond;
        uint8_t  second;
        uint8_t  minute;
        uint8_t  hour;
        uint8_t  date;
        uint8_t  month;
        uint16_t year;

    } time;

    time t;

    void Calendar_tick_second(void)
    {
        // t.msecond++;
        if (++t.msecond >= 10) // 100
        {
            t.msecond = 0x00;
            // t.second++;

            //Another_Value = true;

            if (++t.second >= 0x3c) // 60
            {
                t.second = 0x00;
                // t.minute++;
                if (++t.minute >= 0x3c) // 60
                {
                    t.minute = 0x00;
                    // Activar guardado de historial cada 1 hora
                    //Another_Value = true;
                    
                    // t.hour++;
                    if (++t.hour == 0x18) // 24
                    {
                        t.hour = 0x00;
                        // Lógica para guardar historial cada 2 días

                        // History_Day_Counter++;
                        // if (History_Day_Counter >= 2)
                        // {
                        //     Another_Value = true;  // Activar guardado de historial cada 2 días
                        //     History_Day_Counter = 0; // Resetear contador
                        // }


                        Another_Value = true;

                        // t.date++;
                        if (++t.date == 0x20) // 31
                        {
                            t.month++;
                            t.date = 1;
                        }
                        else if (t.date == 31)
                        {
                            if ((t.month == 4) || (t.month == 6) || (t.month == 9) ||
                                (t.month == 11))
                            {
                                t.month++;
                                t.date = 0x01;
                            }
                        }
                        else if (t.date == 30)
                        {
                            if (t.month == 2)
                            {
                                t.month++;
                                t.date = 1;
                            }
                        }
                        else if (t.date == 29)
                        {
                            if ((t.month == 2) && (not_leap()))
                            {
                                t.month++;
                                t.date = 1;
                            }
                        }
                        if (t.month == 13)
                        {
                            t.month = 1;
                            t.year++;
                        }
                    }
                }
            }
        }
        // PORTB=~(((t.second&0x01)|t.minute<<1)|t.hour<<7);
    }

    bool not_leap(void) // check for leap year
    {
        if (!(t.year % 4))
        {
            return (false); //(uint8_t)(t.year%400);
        }

        else

        {
            return (true);
        }
    }

    static void rtc_handler_calendar(nrf_drv_rtc_int_type_t int_type)
    {
        Calendar_tick_second();
        if (device_sleep)
        {
            Counter_sleep_ticker++;
            if (Counter_sleep_ticker >= sleep_in_time_ticker)
            {
                Counter_sleep_ticker = 0;
                device_sleep         = false;
                NRF_LOG_FLUSH();
                NRF_LOG_RAW_INFO("%02i/%02i/%02i   ", t.year, t.month, t.date);
                NRF_LOG_RAW_INFO("%02i:%02i:%02i:%02i\r\n", t.hour, t.minute, t.second, t.msecond);
                NRF_LOG_FLUSH();
                Wakeup_by_Timer = true;
            }
        }
        if (Device_BLE_connected)
        {
            if (!connection_confident)
                Counter_to_disconnect++;
        }
        if (Boton_Presionado)
        {
            Time_button_pressed++;
        }
        if (Transmiting_Ble)
        {
            Counter_Time_blink++;
        }
        if (Rtc_waiting)
        {
            rtc_time = rtc_time - 1;
            if (rtc_time <= 0)
            {
                Rtc_waiting = false;
            }
        }

        if (!Write_DATE)
        {
            if (Counter_Timer_write_Date >= Timer_write_Date)
            {
                Counter_Timer_write_Date = 0;

                NRF_LOG_RAW_INFO("Graba hora\r\n");

                NRF_LOG_RAW_INFO("%02i/%02i/%02i   ", t.year, t.month, t.date);
                NRF_LOG_RAW_INFO("%02i:%02i:%02i:%02i\r\n", t.hour, t.minute, t.second, t.msecond);
                NRF_LOG_FLUSH();
                Write_DATE = true;
            }
            Counter_Timer_write_Date++;
        }
    }

    static void rtc_config(void)
    {
        uint32_t             err_code;
        nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
        config.prescaler            = 3276;
        err_code                    = nrf_drv_rtc_init(&rtc, &config, rtc_handler_calendar);
        APP_ERROR_CHECK(err_code);
        nrf_drv_rtc_tick_enable(&rtc, true);
        err_code = nrf_drv_rtc_cc_set(&rtc, 0, COMPARE_COUNTERTIME * 16, true);
        APP_ERROR_CHECK(err_code);
        nrf_drv_rtc_enable(&rtc);
    }

    /**@brief Function for initializing the timer module.
     */
    static void timers_init(void)
    {
        ret_code_t err_code = app_timer_init();
        APP_ERROR_CHECK(err_code);
    }

#ifdef __cplusplus
}
#endif
#endif
