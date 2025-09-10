/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "nrf_drv_saadc.h"
#include "ADC_PERNO.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define REVERSE_PROT_VOLT_DROP_MILLIVOLTS        1 
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS            600  //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_RES_10BIT                            1024 //!< Maximum digital value for 10-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION             6    //!< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) *ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

static nrf_saadc_value_t adc_buf_Perno;           //!< Buffer used for storing ADC values.
// static uint8_t Perno_is_init = 0;

/**@brief Function handling events from 'nrf_drv_saadc.c'.
 *
 * @param[in] p_evt SAADC event.
 */
static void saadc_event_handler_adc_perno(nrf_drv_saadc_evt_t const * p_evt)
{

    if (p_evt->type == NRF_DRV_SAADC_EVT_DONE)
    {
      NRF_LOG_DEBUG("ADC done \r\n");
    }
}

void ADC_Perno_init(nrf_saadc_input_t Pin_sensor, nrf_saadc_gain_t Gain)
{
    nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
    ret_code_t err_code = nrf_drv_saadc_init(&saadc_config, saadc_event_handler_adc_perno);
    APP_ERROR_CHECK(err_code);

    nrf_saadc_channel_config_t config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(Pin_sensor);  //NRF_SAADC_INPUT_VDD
    config.gain = Gain;
    config.acq_time   = NRF_SAADC_ACQTIME_15US;  //NRF_SAADC_ACQTIME_20US
      
    err_code = nrf_drv_saadc_channel_init(0, &config);
    APP_ERROR_CHECK(err_code);
}


uint16_t Get_ADC_Perno(nrf_saadc_input_t Pin_sensor)
{
    ret_code_t err_code;
    if (!nrf_drv_saadc_is_busy())
    {
        err_code = nrf_drv_saadc_sample_convert(0, &adc_buf_Perno);
        APP_ERROR_CHECK(err_code);
    }
    
    return adc_buf_Perno;
}