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

#ifndef ADC_PERNO_H__
#define ADC_PERNO_H__

#include <stdint.h>

/**@brief Function for initializing the battery voltage module.
 */
void ADC_Perno_init(nrf_saadc_input_t Pin_sensor,nrf_saadc_gain_t Gain);


/**@brief Function for reading the battery voltage.
 *
 * If battery ADC is not already initialised, this function
 * initialises battery reading automatically. 
 *
 * @returns battery voltage in millivolts.
 */
uint16_t Get_ADC_Perno(nrf_saadc_input_t Pin_sensor);

#endif