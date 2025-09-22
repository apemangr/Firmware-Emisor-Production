#include "app_util.h"
#include "boards.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_soc.h"

#include "app_timer.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "ble_nus.h"
#include "bsp_btn_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_delay.h"
#include "nrf_drv_saadc.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_temp.h"

#include "Temperature_chip.h"

uint16_t Requested_History_Index = 0xFFFF;

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT); /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                         /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                           /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);               /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static uint16_t m_ble_nus_max_data_len =
    BLE_GATT_ATT_MTU_DEFAULT - 3; /**< Maximum length of data (in bytes) that can be transmitted to
                                     the peer by the Nordic UART service module. */

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may
 * need to inform the application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went
 * wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void disabled_uart(void)
{
    uint32_t err_code                = app_uart_close();
    NRF_UARTE0->TASKS_STOPTX         = 1;
    NRF_UARTE0->TASKS_STOPRX         = 1;
    NRF_UARTE0->ENABLE               = 0;
    *(volatile uint32_t *)0x40002FFC = 0; /* Power down UARTE0 */
    *(volatile uint32_t *)0x40002FFC;
    *(volatile uint32_t *)0x40002FFC = 1; /* Power on UARTE0 so it is ready for next time */
}

void pa_assist(uint32_t gpio_pa_pin, uint32_t gpio_lna_pin)
{
    ret_code_t            err_code;
    static const uint32_t gpio_toggle_ch = 0;
    static const uint32_t ppi_set_ch     = 0;
    static const uint32_t ppi_clr_ch     = 1;
    // Configure SoftDevice PA assist
    ble_opt_t opt;
    memset(&opt, 0, sizeof(ble_opt_t));
    // Common PA config
    opt.common_opt.pa_lna.gpiote_ch_id  = gpio_toggle_ch; // GPIOTE channel
    opt.common_opt.pa_lna.ppi_ch_id_clr = ppi_set_ch;     // PPI channel for pin clearing
    opt.common_opt.pa_lna.ppi_ch_id_set = ppi_clr_ch;     // PPI channel for pin setting
    // PA config
    opt.common_opt.pa_lna.pa_cfg.active_high  = 1;           // Set the pin to be active high
    opt.common_opt.pa_lna.pa_cfg.enable       = 1;           // Enable toggling
    opt.common_opt.pa_lna.pa_cfg.gpio_pin     = gpio_pa_pin; // The GPIO pin to toggle
    opt.common_opt.pa_lna.lna_cfg.active_high = 1;
    opt.common_opt.pa_lna.lna_cfg.enable      = 1;
    opt.common_opt.pa_lna.lna_cfg.gpio_pin    = gpio_lna_pin;
    err_code                                  = sd_ble_opt_set(BLE_COMMON_OPT_PA_LNA, &opt);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access
 * Profile) parameters of the device. It also sets the permissions and
 * appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;

    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code =
        sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code                          = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

void send_data_Nus(uint16_t length, uint8_t data_array[BLE_NUS_MAX_DATA_LEN])
{
    uint32_t err_code;
    err_code = ble_nus_data_send(&m_nus, data_array, &length, m_conn_handle);
    if ((err_code != NRF_SUCCESS))
    {
        loop_send_med--;
        Next_Sending = false;
    }
}

void Send_Confirmation_OK()
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 2;
    data_array[0]        = 0x01;
    data_array[1]        = 0x02;
    send_data_Nus(index, data_array);
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART
 * BLE Service and send it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t *p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

        NRF_LOG_RAW_INFO("Reciviendo Datos\r\n");
        NRF_LOG_FLUSH();
        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }
        NRF_LOG_RAW_INFO("Datos recepcionados :  ");
        NRF_LOG_FLUSH();

        for (uint32_t l = 0; l < (p_evt->params.rx_data.length); l++)
        {
            NRF_LOG_RAW_INFO("%x,", (p_evt->params.rx_data.p_data[l] - 0x30));
        }
        NRF_LOG_FLUSH();

        // 01 Carga contador de reinicios del emisor
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '1'))
        {

            contador = (uint32_t)(((p_evt->params.rx_data.p_data[2] - 0x30) * 10000) +
                                  ((p_evt->params.rx_data.p_data[3] - 0x30) * 1000) +
                                  ((p_evt->params.rx_data.p_data[4] - 0x30) * 100) +
                                  ((p_evt->params.rx_data.p_data[5] - 0x30) * 10) +
                                  ((p_evt->params.rx_data.p_data[6] - 0x30) * 1));
            NRF_LOG_RAW_INFO("Veces reiniciado (01) en %i\r\n", contador);
            NRF_LOG_FLUSH();
            Flash_array.total_reset[0] = (contador >> 24) & 0xFF;
            Flash_array.total_reset[1] = (contador >> 16) & 0xFF;
            Flash_array.total_reset[2] = (contador >> 8) & 0xFF;
            Flash_array.total_reset[3] = (contador & 0xFF);
            Write_Flash                = true;
            Send_Confirmation_OK();
        }

        // 02 modifica tiempo de dormido tiempo 02xxxxx x en segundos
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '2'))
        {

            sleep_in_time_ticker = (uint32_t)(((p_evt->params.rx_data.p_data[2] - 0x30) * 10000) +
                                              ((p_evt->params.rx_data.p_data[3] - 0x30) * 1000) +
                                              ((p_evt->params.rx_data.p_data[4] - 0x30) * 100) +
                                              ((p_evt->params.rx_data.p_data[5] - 0x30) * 10) +
                                              ((p_evt->params.rx_data.p_data[6] - 0x30) * 1)) *
                                   10; // de segundos se pasa a ticker de 0.1 segundos, se
                                       // multiplica por 10
            NRF_LOG_RAW_INFO("Tiempo de dormido (02) a %i\r\n", sleep_in_time_ticker);
            NRF_LOG_FLUSH();
            if (sleep_in_time_ticker >= 20 &&
                sleep_in_time_ticker <= 144000) // Permite dormido entre 1 segundo y 4 horas
            {
                Flash_array.sleep_time[0] = (sleep_in_time_ticker >> 16) & 0xFF;
                Flash_array.sleep_time[1] = (sleep_in_time_ticker >> 8) & 0xFF;
                Flash_array.sleep_time[2] = sleep_in_time_ticker & 0xFF;
                Write_Flash               = true;
            }
            Send_Confirmation_OK();
        }
        // 03 modifica tiempo de advertising 03xxxxx  x en segundos
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '3'))
        {
            NRF_LOG_RAW_INFO("Modificando tiempo de ADV (03) \r\n");
            APP_ADV_DURATION = (uint32_t)(((p_evt->params.rx_data.p_data[2] - 0x30) * 10000) +
                                          ((p_evt->params.rx_data.p_data[3] - 0x30) * 1000) +
                                          ((p_evt->params.rx_data.p_data[4] - 0x30) * 100) +
                                          ((p_evt->params.rx_data.p_data[5] - 0x30) * 10) +
                                          ((p_evt->params.rx_data.p_data[6] - 0x30) * 1)) *
                               100; // de segundos se pasa a ticker de 0.1 segundos, se multiplica
                                    // por 10
            NRF_LOG_RAW_INFO("Comando 03 recepcionado : Modificando tiempo de adv a %i\r\n",
                             APP_ADV_DURATION);
            NRF_LOG_FLUSH();

            if (APP_ADV_DURATION >= 100 &&
                APP_ADV_DURATION <= 6000) // Permite adverting entre 1 segundo y 60 segundos
            {
                Flash_array.adv_time[0] = (APP_ADV_DURATION >> 8) & 0xFF;
                Flash_array.adv_time[1] = APP_ADV_DURATION & 0xFF;
                Write_Flash             = true;
            }
            Send_Confirmation_OK();
        }
        // 04 Carga contador de Advertising 04xxxxx
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '4'))
        {

            contador = (uint32_t)(((p_evt->params.rx_data.p_data[2] - 0x30) * 10000) +
                                  ((p_evt->params.rx_data.p_data[3] - 0x30) * 1000) +
                                  ((p_evt->params.rx_data.p_data[4] - 0x30) * 100) +
                                  ((p_evt->params.rx_data.p_data[5] - 0x30) * 10) +
                                  ((p_evt->params.rx_data.p_data[6] - 0x30) *
                                   1)); // de segundos se pasa a ticker de 0.1
                                        // segundos, se multiplica por 10
            NRF_LOG_RAW_INFO("Contador de ADV (04) en %i\r\n", contador);
            NRF_LOG_FLUSH();
            Flash_array.total_adv[0] = (contador >> 24) & 0xFF;
            Flash_array.total_adv[1] = (contador >> 16) & 0xFF;
            Flash_array.total_adv[2] = (contador >> 8) & 0xFF;
            Flash_array.total_adv[3] = (contador & 0xFF);
            Write_Flash              = true;
            Send_Confirmation_OK();
        }

        // 05 Resetea El nodo
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '5'))
        {
            NRF_LOG_RAW_INFO("Comando 05 recepcionado : Reseteando nodo\r\n");
            NRF_LOG_FLUSH();
            Reset_Line_Step(0);
            NVIC_SystemReset();
        }

        // 06  Escribe HORA en NoDO  ********
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '6'))
        {
            NRF_LOG_INFO(" Cargando Hora 06");
            NRF_LOG_FLUSH();
            //"060YYYY.MM.DD HH.MM.SS"
            t.year = (int)((p_evt->params.rx_data.p_data[3] - 0x30) * 1000) +
                     ((p_evt->params.rx_data.p_data[4] - 0x30) * 100) +
                     ((p_evt->params.rx_data.p_data[5] - 0x30) * 10) +
                     ((p_evt->params.rx_data.p_data[6] - 0x30));
            t.month            = (int)((p_evt->params.rx_data.p_data[8] - 0x30) * 10 +
                            (p_evt->params.rx_data.p_data[9] -
                             0x30)); //*10 + ((int) p_evt->params.rx_data.p_data[7]));
            t.date             = (int)((p_evt->params.rx_data.p_data[11] - 0x30) * 10 +
                           (p_evt->params.rx_data.p_data[12] - 0x30));
            t.hour             = (uint8_t)((p_evt->params.rx_data.p_data[14] - 0x30) * 10 +
                               (p_evt->params.rx_data.p_data[15] - 0x30));
            t.minute           = (uint8_t)((p_evt->params.rx_data.p_data[17] - 0x30) * 10 +
                                 (p_evt->params.rx_data.p_data[18] - 0x30));
            t.second           = (uint8_t)((p_evt->params.rx_data.p_data[20] - 0x30) * 10 +
                                 (p_evt->params.rx_data.p_data[21] - 0x30));

            Flash_array.second = t.second;
            Flash_array.minute = t.minute;
            Flash_array.hour   = t.hour;
            Flash_array.date   = t.date;
            Flash_array.month  = t.month;
            Flash_array.year   = t.year;

            NRF_LOG_RAW_INFO("%02i/%02i/%02i   ", Flash_array.year, Flash_array.month,
                             Flash_array.date);
            NRF_LOG_RAW_INFO("%02i:%02i:%02i:%02i  \r\n", Flash_array.hour, Flash_array.minute,
                             Flash_array.second);

            NRF_LOG_FLUSH();

            // Write_Flash=true;
            Send_Confirmation_OK();
        }
        // 07 Lee hora del nodo
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '7'))
        {
            NRF_LOG_INFO("Enviando Hora 07");
            NRF_LOG_FLUSH();
            unsigned char data_arr[20];
            data_arr[0]  = (char)(t.year / 1000 + 0x30);
            data_arr[0]  = (char)(t.date / 10 + 0x30);
            data_arr[1]  = (char)((t.date - ((int)(t.date / 10)) * 10) + 0x30);
            data_arr[2]  = '/';
            data_arr[3]  = (char)(t.month / 10 + 0x30);
            data_arr[4]  = (char)((t.month - ((int)(t.month / 10)) * 10) + 0x30);
            data_arr[5]  = '/';
            data_arr[6]  = (char)((t.year / 1000) + 0x30);
            data_arr[7]  = (char)(((t.year / 100) - (int)((t.year / 1000) * 10)) + 0x30);
            data_arr[8]  = (char)(((t.year / 10) - (int)((t.year / 100) * 10)) + 0x30);
            data_arr[9]  = (char)(((t.year) - (int)((t.year / 10) * 10)) + 0x30);
            data_arr[10] = (char)' ';
            data_arr[11] = (char)(t.hour / 10 + 0x30);
            data_arr[12] = (char)((t.hour - ((int)(t.hour / 10)) * 10) + 0x30);
            data_arr[13] = ':';
            data_arr[14] = (char)(t.minute / 10 + 0x30);
            data_arr[15] = (char)((t.minute - ((int)(t.minute / 10)) * 10) + 0x30);
            data_arr[16] = ':';
            data_arr[17] = (char)(t.second / 10 + 0x30);
            data_arr[18] = (char)((t.second - ((int)(t.second / 10)) * 10) + 0x30);
            //      snprintf(data_arr, sizeof data_arr, "%s/%s/%s %s:%s:%s", t.date,
            //      t.month, t.year,t.hour,t.minute,(char t.second));
            uint16_t length = 19;
            err_code        = ble_nus_data_send(&m_nus, data_arr, &length, m_conn_handle);
            if ((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_BUSY) &&
                (err_code != NRF_ERROR_NOT_FOUND))
            {
                APP_ERROR_CHECK(err_code);
            }
        }

        // Comando 08 - Envía el ultimo historial
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '8'))
        {
            NRF_LOG_RAW_INFO("Comando 08 recibido: Enviando último historial.\r\n");
            NRF_LOG_FLUSH();
            Tipo_Envio   = Last_History; // Activa el envío del último historial
            Next_Sending = true;         // Indica que hay datos para enviar
            Next_Transmition();          // Llama a la función para enviar el

            // Next_Transmition();        // Llama a la función para enviar el
            // historial
        }

        // Comando 09 - Envia un historial segun el indice
        if ((p_evt->params.rx_data.p_data[0] == '0') && (p_evt->params.rx_data.p_data[1] == '9'))
        {
            // Extrae los dígitos después de '09'
            uint16_t idx = 0;
            uint8_t *ptr = &p_evt->params.rx_data.p_data[2];
            uint8_t  len = p_evt->params.rx_data.length;
            // Calcula cuántos dígitos hay (desde el tercer byte hasta el final)
            for (uint8_t i = 2; i < len; i++)
            {
                if (ptr[i - 2] >= '0' && ptr[i - 2] <= '9')
                {
                    idx = idx * 10 + (ptr[i - 2] - '0');
                }
                else
                {
                    break; // Deja de leer si encuentra un carácter no numérico
                }
            }
            Requested_History_Index = idx;
            NRF_LOG_RAW_INFO("Comando 09 recibido: Enviando historial #%d.\r\n",
                             Requested_History_Index);
            NRF_LOG_FLUSH();
            Tipo_Envio   = History_By_Index;
            Next_Sending = true;
            Next_Transmition();
        }

        // 20 modifica la MAC 20AABBCCDDEEFF
        if ((p_evt->params.rx_data.p_data[0] == '2') && (p_evt->params.rx_data.p_data[1] == '0'))
        {
            NRF_LOG_RAW_INFO("Cargando MAC custom (20) \r\n");
            NRF_LOG_FLUSH();
            char str1[6];
            for (uint32_t i = 0; i < 6; i++)
            {
                str1[i] = p_evt->params.rx_data.p_data[i + 2];
            }
            char str2[6];
            for (uint32_t i = 6; i < 12; i++)
            {
                str2[i - 6] = p_evt->params.rx_data.p_data[i + 2];
            }

            char   *remaining1;
            char   *remaining2;
            int64_t answer1, answer2;

            answer1 = strtol(str1, &remaining1, 16);
            answer2 = strtol(str2, &remaining2, 16);
            NRF_LOG_RAW_INFO("The converted hexadecimal is %x  %x\r\n  ", answer1, answer2);
            NRF_LOG_FLUSH();
            Flash_array.mac_custom[0] = (answer1 >> 16) & 0xFF;
            Flash_array.mac_custom[1] = (answer1 >> 8) & 0xFF;
            Flash_array.mac_custom[2] = answer1 & 0xFF;
            Flash_array.mac_custom[3] = (answer2 >> 16) & 0xFF;
            Flash_array.mac_custom[4] = (answer2 >> 8) & 0xFF;
            Flash_array.mac_custom[5] = answer2 & 0xFF;

            Write_Flash               = true;
            Send_Confirmation_OK();
        }
        // 21 USA mac de fabrica
        if ((p_evt->params.rx_data.p_data[0] == '2') && (p_evt->params.rx_data.p_data[1] == '1'))
        {

            NRF_LOG_RAW_INFO("Usando mac fabrica (21) \r\n");
            NRF_LOG_FLUSH();

            Flash_array.Enable_Custom_mac = 0x0;
            Write_Flash                   = true;
            Send_Confirmation_OK();
        }
        // 22 USA mac cargada
        if ((p_evt->params.rx_data.p_data[0] == '2') && (p_evt->params.rx_data.p_data[1] == '2'))
        {

            NRF_LOG_RAW_INFO("Usando mac custom (22) \r\n");
            NRF_LOG_FLUSH();
            Flash_array.Enable_Custom_mac = 0x1;
            Write_Flash                   = true;
            Send_Confirmation_OK();
        }

        // 30 Modifica Tipo de Sensor 30xx
        if ((p_evt->params.rx_data.p_data[0] == '3') && (p_evt->params.rx_data.p_data[1] == '0'))
        {
            NRF_LOG_RAW_INFO("Modificando Tipo de Emisor (30)\r\n");
            NRF_LOG_FLUSH();
            char str1[2];
            str1[0] = p_evt->params.rx_data.p_data[2];
            str1[1] = p_evt->params.rx_data.p_data[3];
            char   *remaining1;
            int64_t answer1;
            answer1 = strtol(str1, &remaining1, 16);
            NRF_LOG_RAW_INFO("The converted hexadecimal is %x \r\n ", answer1);
            NRF_LOG_FLUSH();
            Flash_array.Type_sensor = (answer1) & 0xFF;
            Write_Flash             = true;
            Send_Confirmation_OK();
        }
        // 31 Modifica Tipo de resistencia  31xx
        if ((p_evt->params.rx_data.p_data[0] == '3') && (p_evt->params.rx_data.p_data[1] == '1'))
        {
            NRF_LOG_RAW_INFO("Modificando Tipo de resistencia (31)\r\n");
            NRF_LOG_FLUSH();
            char str1[2];
            str1[0] = p_evt->params.rx_data.p_data[2];
            str1[1] = p_evt->params.rx_data.p_data[3];
            char   *remaining1;
            int64_t answer1;
            answer1 = strtol(str1, &remaining1, 16);
            NRF_LOG_RAW_INFO("The converted hexadecimal is %x \r\n  ", answer1);
            NRF_LOG_FLUSH();

            Flash_array.Type_resistor = (answer1) & 0xFF;

            Write_Flash               = true;
            Send_Confirmation_OK();
            if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
            {
                while (app_uart_put('\n') == NRF_ERROR_BUSY)
                    ;
            }
        }
        // 32 Modifica Tipo de Sencibilidad para guardar historia 32xx
        if ((p_evt->params.rx_data.p_data[0] == '3') && (p_evt->params.rx_data.p_data[1] == '2'))
        {
            NRF_LOG_RAW_INFO("Modificando sensibilidad (32)\r\n");
            NRF_LOG_FLUSH();
            char str1[2];
            str1[0] = p_evt->params.rx_data.p_data[2];
            str1[1] = p_evt->params.rx_data.p_data[3];
            char   *remaining1;
            int64_t answer1;
            answer1 = strtol(str1, &remaining1, 16);
            NRF_LOG_RAW_INFO("The converted hexadecimal is %x \r\n  ", answer1);
            NRF_LOG_FLUSH();
            Flash_array.Sensibility = (answer1) & 0xFF;
            Sensibilidad_Res        = Flash_array.Sensibility;
            Write_Flash             = true;
            Send_Confirmation_OK();
            if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
            {
                while (app_uart_put('\n') == NRF_ERROR_BUSY)
                    ;
            }
        }

        // 40 Modifica Tipo de bateria
        if ((p_evt->params.rx_data.p_data[0] == '4') && (p_evt->params.rx_data.p_data[1] == '0'))
        {
            NRF_LOG_RAW_INFO("Modificando Tipo de Bateria (40) \r\n");
            NRF_LOG_FLUSH();
            char str1[2];
            str1[0] = p_evt->params.rx_data.p_data[2];
            str1[1] = p_evt->params.rx_data.p_data[3];

            char   *remaining1;
            int64_t answer1;

            answer1 = strtol(str1, &remaining1, 16);
            NRF_LOG_RAW_INFO("The converted hexadecimal is %x \r\n", answer1);
            NRF_LOG_FLUSH();

            Flash_array.Type_battery = (answer1) & 0xFF;
            Write_Flash              = true;
            Send_Confirmation_OK();
            if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
            {
                while (app_uart_put('\n') == NRF_ERROR_BUSY)
                    ;
            }
        }
        // 50 Modifica offset del perno con respecto a la placa
        if ((p_evt->params.rx_data.p_data[0] == '5') && (p_evt->params.rx_data.p_data[1] == '0'))
        {
            NRF_LOG_RAW_INFO("Modificando Offset (50) \r\n");
            NRF_LOG_FLUSH();

            int8_t answer1;
            answer1 = ((p_evt->params.rx_data.p_data[2] - 0x30) * 100) +
                      (p_evt->params.rx_data.p_data[3] - 0x30) * 10 +
                      (p_evt->params.rx_data.p_data[4] - 0x30);
            NRF_LOG_RAW_INFO("The converted hexadecimal is %x  , dec %d \n  ", answer1, answer1);
            NRF_LOG_FLUSH();
            Flash_array.offset_plate_bolt = (answer1) & 0xFF;

            Write_Flash                   = true;

            Send_Confirmation_OK();

            if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
            {
                while (app_uart_put('\n') == NRF_ERROR_BUSY)
                    ;
            }
        }

        // 51 Modifica offset del sensor cuando se adapta al largo del perno
        if ((p_evt->params.rx_data.p_data[0] == '5') && (p_evt->params.rx_data.p_data[1] == '1'))
        {
            NRF_LOG_RAW_INFO("Modificando Offset sensor (51) \r\n");
            NRF_LOG_FLUSH();

            int8_t answer1;

            answer1 = ((p_evt->params.rx_data.p_data[2] - 0x30) * 100) +
                      (p_evt->params.rx_data.p_data[3] - 0x30) * 10 +
                      (p_evt->params.rx_data.p_data[4] - 0x30);
            NRF_LOG_RAW_INFO("cantidad de sensores cortados es %x  , dec %d \n  ", answer1,
                             answer1);
            NRF_LOG_FLUSH();
            Flash_array.Offset_sensor_cut = (answer1) & 0xFF;

            Write_Flash                   = true;
            Send_Confirmation_OK();

            if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
            {
                while (app_uart_put('\n') == NRF_ERROR_BUSY)
                    ;
            }
        }

        if ((p_evt->params.rx_data.p_data[0] == '5') && (p_evt->params.rx_data.p_data[1] == '2'))
        {
            NRF_LOG_RAW_INFO("Reset de bloqueo de programa (52)\r\n");
            NRF_LOG_FLUSH();
            for (int a = 0; a <= 13; a++)
            {
                Flash_array.reset[a] = 0x00;
            }
            Write_Flash = true;
            Send_Confirmation_OK();
            if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
            {
                while (app_uart_put('\n') == NRF_ERROR_BUSY)
                    ;
            }
        }

        // 96 Solicita los valores de los ADC's y contador
        if ((p_evt->params.rx_data.p_data[0] == '9') && (p_evt->params.rx_data.p_data[1] == '6'))
        {
            NRF_LOG_RAW_INFO("\n\nEnviando valores de los ADC's y contador (96)");
            NRF_LOG_FLUSH();
            Tipo_Envio    = Values;
            Next_Sending  = true;
            loop_send_med = 0;
            Next_Transmition();
        }

        // 97 Limpia historia
        if ((p_evt->params.rx_data.p_data[0] == '9') && (p_evt->params.rx_data.p_data[1] == '7'))
        {

            memset(&Flash_array.history, 0, sizeof(Flash_array.history));
            /*
            for (int loop_med = 0; loop_med < Size_Memory_History; loop_med++) {

              Flash_array.history[loop_med].Contador = 0;
              Flash_array.history[loop_med].V1 = 0;
              Flash_array.history[loop_med].V2 = 0;
              Flash_array.history[loop_med].battery = 0;
            }
            */

            History_Position             = 0;
            Flash_array.last_history     = History_Position;
            Flash_array.Sending_Position = 0;
            Write_Flash                  = true;
            Send_Confirmation_OK();
        }
        // 98 envio de historia
        if ((p_evt->params.rx_data.p_data[0] == '9') && (p_evt->params.rx_data.p_data[1] == '8'))
        {
            Tipo_Envio    = History;
            Next_Sending  = true;
            loop_send_med = 0;
        }
        // 99 ENVIO DE CONFIGURACION
        if ((p_evt->params.rx_data.p_data[0] == '9') && (p_evt->params.rx_data.p_data[1] == '9'))
        {
            Tipo_Envio    = Configuration;
            Next_Sending  = true;
            loop_send_med = 0;
        }
    }
    if (p_evt->type == BLE_NUS_EVT_TX_RDY)
    {
        Next_Sending = true;
    }
}

void Next_Transmition()
{
    static uint8_t  data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t  index = 2;
    static uint16_t position;
    uint16_t        Total_sendig;

    if (Flash_array.Sending_Position == 0)
    {
        Total_sendig = History_Position;
    }
    else
    {
        Total_sendig = Size_Memory_History - 1;
    }

    if (Tipo_Envio == Configuration)
    {
        static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
        static uint8_t index = 2;

        switch (loop_send_med)
        {
        case 0: // Envio de RESET
            data_array[0] = 0x01;
            data_array[1] = Flash_array.total_reset[0];
            data_array[2] = Flash_array.total_reset[1];
            data_array[3] = Flash_array.total_reset[2];
            data_array[4] = Flash_array.total_reset[3];
            index         = 5;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 1: // Envio de tiempo dormido en deci-segundos
            data_array[0] = 0x02;
            data_array[1] = Flash_array.sleep_time[0];
            data_array[2] = Flash_array.sleep_time[1];
            data_array[3] = Flash_array.sleep_time[2];
            index         = 4;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 2: // Envio de tiempo del advertising
            data_array[0] = 0x03;
            data_array[1] = Flash_array.adv_time[0];
            data_array[2] = Flash_array.adv_time[1];
            index         = 3;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 3: // Envio de cantidad de advertising
            data_array[0] = 0x04;
            data_array[1] = Flash_array.total_adv[0];
            data_array[2] = Flash_array.total_adv[1];
            data_array[3] = Flash_array.total_adv[2];
            data_array[4] = Flash_array.total_adv[3];
            index         = 5;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 4: // Envio de mac CUstom
            data_array[0] = 0x20;
            data_array[1] = Flash_array.mac_custom[0];
            data_array[2] = Flash_array.mac_custom[1];
            data_array[3] = Flash_array.mac_custom[2];
            data_array[4] = Flash_array.mac_custom[3];
            data_array[5] = Flash_array.mac_custom[4];
            data_array[6] = Flash_array.mac_custom[5];
            index         = 7;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 5: // Envio de mac de Fabrica
            data_array[0] = 0x23;
            data_array[1] = Flash_array.mac_original[0];
            data_array[2] = Flash_array.mac_original[1];
            data_array[3] = Flash_array.mac_original[2];
            data_array[4] = Flash_array.mac_original[3];
            data_array[5] = Flash_array.mac_original[4];
            data_array[6] = Flash_array.mac_original[5];
            index         = 7;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 6: // Envio de que tipo de mac se esta usando
            data_array[0] = 0x24;
            data_array[1] = Flash_array.Enable_Custom_mac;
            index         = 2;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 7: // Envio de tipo de emisor
            data_array[0] = 0x30;
            data_array[1] = Flash_array.Type_sensor;
            index         = 2;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 8: // Envio de tipo de resistencia
            data_array[0] = 0x31;
            data_array[1] = Flash_array.Type_resistor;
            index         = 2;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 9: // Envio de tipo de Bateria instalada
            data_array[0] = 0x40;
            data_array[1] = Flash_array.Type_battery;
            index         = 2;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 10: // Envio de offset del perno con respecto de la placa
            data_array[0] = 0x50;
            data_array[1] = Flash_array.offset_plate_bolt;
            index         = 2;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 11: // Cantidad de mm sensor cortado
            data_array[0] = 0x51;
            data_array[1] = Flash_array.Offset_sensor_cut;
            index         = 2;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 12: // Envio de Cantidad de reset
            data_array[0] = 0x52;
            for (int a = 0; a < 10; a++)
                data_array[1 + a] = Flash_array.reset[a];
            index = 9;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            break;

        case 13: // Envio de version
            data_array[0] = 0x53;
            data_array[1] = Flash_array.Version[0];
            data_array[2] = Flash_array.Version[1];
            data_array[3] = Flash_array.Version[2];
            index         = 4;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
            // Tipo_Envio = 0;
            // Next_Sending = false;
            break;

        case 14: // Envio de Sensibilidad
            data_array[0] = 0x32;
            data_array[1] = Flash_array.Sensibility;
            index         = 2;
            send_data_Nus(index, data_array);
            Tipo_Envio   = 0;
            Next_Sending = false;
            break;
        }
    }

    if (Tipo_Envio == History)
    {
        if (loop_send_med < (Total_sendig))
        {

            position             = 0;
            data_array[position] = 0x98;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].day & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].month & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].year >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].year & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].hour & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].minute & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].second & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].Contador >> 24) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].Contador >> 16) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].Contador >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].Contador & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V1 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V1 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V2 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V2 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].battery & 0xFF);
            position++;
            data_array[position] = (Flash_array.mac_original[0]);
            position++;
            data_array[position] = (Flash_array.mac_original[1]);
            position++;
            data_array[position] = (Flash_array.mac_original[2]);
            position++;
            data_array[position] = (Flash_array.mac_original[3]);
            position++;
            data_array[position] = (Flash_array.mac_original[4]);
            position++;
            data_array[position] = (Flash_array.mac_original[5]);
            position++;
            data_array[position] = (Flash_array.mac_custom[0]);
            position++;
            data_array[position] = (Flash_array.mac_custom[1]);
            position++;
            data_array[position] = (Flash_array.mac_custom[2]);
            position++;
            data_array[position] = (Flash_array.mac_custom[3]);
            position++;
            data_array[position] = (Flash_array.mac_custom[4]);
            position++;
            data_array[position] = (Flash_array.mac_custom[5]);
            position++;

            // *** Nuevos Campos ****
            data_array[position] = (Flash_array.history[loop_send_med].V3 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V3 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V4 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V4 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V5 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V5 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V6 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V6 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V7 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V7 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V8 >> 8) & 0xFF;
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].V8 & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].temp & 0xFF);
            position++;
            data_array[position] = (Flash_array.history[loop_send_med].antpwr & 0xFF);
            position++;
            data_array[position] = 1;
            position++;
            data_array[position] = 2;
            position++;
            data_array[position] = 3;
            position++;
            data_array[position] = 4;
            position++;
            data_array[position] = ((loop_send_med + 1) >> 8) & 0xFF;
            position++;
            data_array[position] = ((loop_send_med + 1) & 0xFF);
            position++;
            data_array[position] = ((Total_sendig >> 8) & 0xFF);
            position++;
            data_array[position] = (Total_sendig & 0xFF);
            position++;
            // *** Nuevos Campos ****
            index = position;
            loop_send_med++;
            Next_Sending = true;
            send_data_Nus(index, data_array);
        }
        else
        {
            Tipo_Envio   = 0;
            Next_Sending = false;
        }
    }

    if (Tipo_Envio == Last_History)
    {
        uint16_t last_position;
        // if (Flash_array.last_history > 0)
        // {
        //   last_position = Flash_array.last_history - 1; // Obtiene la posición
        //   del último historial
        // }

        last_position = Flash_array.last_history - 1; // Obtiene la posición del último historial
        if (last_position < 0 || last_position >= Size_Memory_History)
        {
            NRF_LOG_RAW_INFO("\nEvitando overflow, se setea a cero\r\n");
            last_position = 0; // Asegura que no sea menor a 0
        }
        position               = 0;

        data_array[position++] = 0x08;
        data_array[position++] = Flash_array.history[last_position].day;
        data_array[position++] = Flash_array.history[last_position].month;
        data_array[position++] = (Flash_array.history[last_position].year >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].year & 0xFF;
        data_array[position++] = Flash_array.history[last_position].hour;
        data_array[position++] = Flash_array.history[last_position].minute;
        data_array[position++] = Flash_array.history[last_position].second;
        data_array[position++] = (Flash_array.history[last_position].Contador >> 24) & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].Contador >> 16) & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].Contador >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].Contador & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V1 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V1 & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V2 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V2 & 0xFF;
        data_array[position++] = Flash_array.history[last_position].battery;

        // MAC original (6 bytes)
        data_array[position++] = Flash_array.mac_original[0];
        data_array[position++] = Flash_array.mac_original[1];
        data_array[position++] = Flash_array.mac_original[2];
        data_array[position++] = Flash_array.mac_original[3];
        data_array[position++] = Flash_array.mac_original[4];
        data_array[position++] = Flash_array.mac_original[5];

        // MAC custom (6 bytes)
        data_array[position++] = Flash_array.mac_custom[0];
        data_array[position++] = Flash_array.mac_custom[1];
        data_array[position++] = Flash_array.mac_custom[2];
        data_array[position++] = Flash_array.mac_custom[3];
        data_array[position++] = Flash_array.mac_custom[4];
        data_array[position++] = Flash_array.mac_custom[5];

        // Valores V3 a V8 (2 bytes cada uno)
        data_array[position++] = (Flash_array.history[last_position].V3 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V3 & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V4 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V4 & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V5 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V5 & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V6 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V6 & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V7 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V7 & 0xFF;
        data_array[position++] = (Flash_array.history[last_position].V8 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_position].V8 & 0xFF;

        // Temperatura del modulo
        // Muestra la temperatura del último historial
        NRF_LOG_RAW_INFO("Temperatura del último historial: %d\r\n", temp);
        data_array[position++] = (temp & 0xFF);

        // Número de historial
        data_array[position++] = (last_position >> 8) & 0xFF;
        data_array[position++] = last_position & 0xFF;

        index                  = position; // Actualiza el índice con la posición final
        Next_Sending           = false;    // Finaliza el envío
        send_data_Nus(index, data_array);  // Envía los datos
        Tipo_Envio = 0;
    }

    if (Tipo_Envio == History_By_Index)
    {
        position = 0;
        if (Requested_History_Index < Size_Memory_History)
        {
            uint16_t idx           = Requested_History_Index;
            data_array[position++] = 0x98; // Código para identificar el envío por índice
            data_array[position++] = Flash_array.history[idx].day;
            data_array[position++] = Flash_array.history[idx].month;
            data_array[position++] = (Flash_array.history[idx].year >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].year & 0xFF;
            data_array[position++] = Flash_array.history[idx].hour;
            data_array[position++] = Flash_array.history[idx].minute;
            data_array[position++] = Flash_array.history[idx].second;
            data_array[position++] = (Flash_array.history[idx].Contador >> 24) & 0xFF;
            data_array[position++] = (Flash_array.history[idx].Contador >> 16) & 0xFF;
            data_array[position++] = (Flash_array.history[idx].Contador >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].Contador & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V1 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V1 & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V2 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V2 & 0xFF;
            data_array[position++] = Flash_array.history[idx].battery;

            // MAC original (6 bytes)
            for (int i = 0; i < 6; i++)
                data_array[position++] = Flash_array.mac_original[i];
            // MAC custom (6 bytes)
            for (int i = 0; i < 6; i++)
                data_array[position++] = Flash_array.mac_custom[i];

            // V3 a V8
            data_array[position++] = (Flash_array.history[idx].V3 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V3 & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V4 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V4 & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V5 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V5 & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V6 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V6 & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V7 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V7 & 0xFF;
            data_array[position++] = (Flash_array.history[idx].V8 >> 8) & 0xFF;
            data_array[position++] = Flash_array.history[idx].V8 & 0xFF;

            // Número de historial enviado
            data_array[position++] = (idx >> 8) & 0xFF;
            data_array[position++] = idx & 0xFF;

            index                  = position;
            Next_Sending           = false;
            send_data_Nus(index, data_array);
        }
        else
        {
            NRF_LOG_RAW_INFO("Índice de historial fuera de rango.\r\n");
            Next_Sending = false;
        }
        Tipo_Envio = 0;
    }

    if (Tipo_Envio == Values)
    {
        position          = 0;
        uint16_t last_pos = 0;
        if (Flash_array.last_history > 0)
            last_pos = Flash_array.last_history - 1;
        else
            last_pos = 0; // O puedes abortar el envío si no hay datos

        data_array[position++] = 0x96;
        data_array[position++] = (Flash_array.history[last_pos].V1 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_pos].V1 & 0xFF;
        data_array[position++] = (Flash_array.history[last_pos].V2 >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_pos].V2 & 0xFF;
        data_array[position++] = (Flash_array.history[last_pos].Contador >> 24) & 0xFF;
        data_array[position++] = (Flash_array.history[last_pos].Contador >> 16) & 0xFF;
        data_array[position++] = (Flash_array.history[last_pos].Contador >> 8) & 0xFF;
        data_array[position++] = Flash_array.history[last_pos].Contador & 0xFF;

        // Formatea todo lo que se va a enviar y muestralo por consola con NRF_LOG_RAW_INFO
        NRF_LOG_RAW_INFO("Enviando valores de los ADC's y contador:\r\n");
        NRF_LOG_RAW_INFO("V1: %d, V2: %d, Contador: %d\r\n", Flash_array.history[last_pos].V1,
                         Flash_array.history[last_pos].V2, Flash_array.history[last_pos].Contador);
        index        = position;
        Next_Sending = false;
        send_data_Nus(index, data_array);
        Tipo_Envio = 0;
    }
}

/**@snippet [Handling the data received over BLE] */

/**@brief Function for initializing services that will be used by the
 * application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code               = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code              = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart
 * module and append it to a string. The string will be be sent over BLE when
 * the last character received was a 'new line' '\n' (hex 0x0A) or if the string
 * has reached the maximum data length.
 */
/**@snippet [Handling the data received over UART] */

void uart_event_handle(app_uart_evt_t *p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
    case APP_UART_DATA_READY:
        UNUSED_VARIABLE(app_uart_get(&data_array[index]));
        index++;

        if ((data_array[index - 1] == '\n') || (data_array[index - 1] == '\r') ||
            (index >= m_ble_nus_max_data_len))
        {
            if (index > 1)
            {
                NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                NRF_LOG_HEXDUMP_DEBUG(data_array, index);

                do
                {
                    uint16_t length = (uint16_t)index;
                    err_code        = ble_nus_data_send(&m_nus, data_array, &length, m_conn_handle);
                    if ((err_code != NRF_ERROR_INVALID_STATE) &&
                        (err_code != NRF_ERROR_RESOURCES) && (err_code != NRF_ERROR_NOT_FOUND))
                    {
                        APP_ERROR_CHECK(err_code);
                    }
                } while (err_code == NRF_ERROR_RESOURCES);
            }

            index = 0;
        }
        break;

    case APP_UART_COMMUNICATION_ERROR:
        APP_ERROR_HANDLER(p_event->data.error_communication);
        break;

    case APP_UART_FIFO_ERROR:
        APP_ERROR_HANDLER(p_event->data.error_code);
        break;

    default:
        break;
    }
}

static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params = {.rx_pin_no    = RX_PIN_NUMBER,
                                                .tx_pin_no    = TX_PIN_NUMBER,
                                                .rts_pin_no   = RTS_PIN_NUMBER,
                                                .cts_pin_no   = CTS_PIN_NUMBER,
                                                .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
                                                .use_parity   = false,
#if defined(UART_PRESENT)
                                                .baud_rate = NRF_UART_BAUDRATE_115200
#else
                                                .baud_rate = NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST, err_code);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection
 * Parameters Module which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by
 * simply setting the disconnect_on_fail config parameter, but instead we use
 * the event handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went
 * wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code                               = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed
 * to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
    case BLE_ADV_EVT_FAST:
        err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
        APP_ERROR_CHECK(err_code);
        break;
    case BLE_ADV_EVT_IDLE:
        sleep_mode_enter();
        break;
    default:
        break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        NRF_LOG_RAW_INFO("Connected\r\n");
        uart_init();
        NRF_LOG_RAW_INFO("UART enabled\r\n");
        Device_BLE_connected  = true;
        Counter_to_disconnect = 0;
        err_code              = bsp_indication_set(BSP_INDICATE_CONNECTED);
        APP_ERROR_CHECK(err_code);
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code      = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        NRF_LOG_RAW_INFO("Disconnected\r\n");
        disabled_uart();
        NRF_LOG_RAW_INFO("UART Disabled\r\n");
        Device_BLE_connected = false;
        bsp_board_led_off(0); //
        bsp_board_led_off(4);
        Counter_to_disconnect = 0;
        // LED indication will be changed when advertising starts.
        //
        //
        //

        ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE);
        (void)sd_ble_gap_adv_stop(m_advertising.adv_handle);

        // Counter_to_disconnect = 0;
        Transmiting_Ble = false;

        m_conn_handle   = BLE_CONN_HANDLE_INVALID;
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
        NRF_LOG_DEBUG("PHY update request.");
        ble_gap_phys_t const phys = {
            .rx_phys = BLE_GAP_PHY_AUTO,
            .tx_phys = BLE_GAP_PHY_AUTO,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    }
    break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        // Pairing not supported
        err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                               NULL, NULL);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        // No system attributes have been stored.
        err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event
 * interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code           = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) &&
        (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_RAW_INFO("Data len is set to 0x%X(%d)\r\n", m_ble_nus_max_data_len,
                         m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central, p_gatt->att_mtu_desired_periph);
}

/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

static void advertising_init(void)
{
    // 0x02 0A 04  12 FF 33221001CB0000000000000000000039FF  05 09 686F6C79
    uint32_t               err_code;
    float                  Value_Wear;
    ble_advertising_init_t init;
    memset(&m_beacon_info, 0, sizeof(m_beacon_info));

    bsp_board_led_on(3);
    // nrf_delay_ms(500);
    //  Battery_level = 0;
    //  Battery_level =
    //      GET_MEASURE_SENSOR(ADC_Battery, Gain_Battery); // Circuito_List[0]  NRF_SAADC_INPUT_AIN2
    //  NRF_LOG_RAW_INFO("valor circuito 3 bateria dato obtenido %d ", Battery_level);
    //  if (Battery_level > 0xf339)
    //      Battery_level = 0x0; // Corrige valores bajo cero
    //  if (Battery_level > 0x366)
    //      Battery_level = 0x366; // Corrige Valores Altos de Voltaje
    //  if (Battery_level < 0x166)
    //      Battery_level = 0x166; // Corrige Valores Altos de Voltaje
    //  if (Battery_level <= 0x366 && Battery_level >= 0x167)
    //  {
    //      Battery_level = (Battery_level - 0x166) / 2;
    //  } // El nivel de Bateria es de 0 a 255, el 255 es 4.25 Volt... se calcula el
    //    // nivel de bateria en la WEB.
    //  NRF_LOG_RAW_INFO(" -- dato a Enviar %d \r\n", Battery_level);
    //  NRF_LOG_FLUSH();

    uint16_t Pista1 = GET_MEASURE_SENSOR(Circuito_1, Gain_Circuit); // Circuito_List[0]
    if (Pista1 > 0xf339)
        Pista1 = 0x0;
    NRF_LOG_RAW_INFO("valor circuito 1 %d \r\n", Pista1);
    NRF_LOG_FLUSH();
    uint16_t Pista2 =
        GET_MEASURE_SENSOR(Circuito_2, Gain_Circuit); // Circuito_List[0]  NRF_SAADC_INPUT_AIN2
    if (Pista2 > 0xf339)
        Pista2 = 0x0;
    NRF_LOG_RAW_INFO("valor circuito 2 %d \r\n", Pista2);
    NRF_LOG_FLUSH();
    bsp_board_led_off(3);
    nrf_delay_ms(30);

    Value_Wear = (Sensor_Analisys(Pista1, Pista2));

    NRF_LOG_RAW_INFO("Tipo dispositivo %x\r\n", Tipo_dispositivo);
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("mm step resistor " NRF_LOG_FLOAT_MARKER "\r\n",
                     NRF_LOG_FLOAT(Step_of_resistor));
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("mm totales cortados del sensor " NRF_LOG_FLOAT_MARKER "\r\n",
                     NRF_LOG_FLOAT(Value_Wear));
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("offset: n° resistencias cortadas %d \r\n", Flash_array.Offset_sensor_cut);
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("offset: mm placa-perno %d \r\n", (Offset_desgaste - 128));
    NRF_LOG_FLUSH();

    if (History_Position == 0)
    {
        Flash_array.history[History_Position].day      = t.date;
        Flash_array.history[History_Position].month    = t.month;
        Flash_array.history[History_Position].year     = t.year;
        Flash_array.history[History_Position].hour     = t.hour;
        Flash_array.history[History_Position].minute   = t.minute;
        Flash_array.history[History_Position].second   = t.second;
        Flash_array.history[History_Position].Contador = contador;
        Flash_array.history[History_Position].V1       = Pista1;
        Flash_array.history[History_Position].V2       = Pista2;
        Flash_array.history[History_Position].V3       = 1;
        Flash_array.history[History_Position].V4       = 2;
        Flash_array.history[History_Position].V5       = 3;
        Flash_array.history[History_Position].V6       = 4;
        Flash_array.history[History_Position].V7       = 5;
        Flash_array.history[History_Position].V8       = 6;
        Flash_array.history[History_Position].battery  = Battery_level;
        History_Position++;

        Flash_array.last_history = History_Position;
    }

    if (History_Position != 0)
    {
        Valor_1 = Flash_array.history[History_Position - 1].V1;
        Valor_2 = Flash_array.history[History_Position - 1].V2;

        // Se elimina la logica de cambio de sensibilidad por una que grabe un dato
        // por dia.
        /*
            if ((Pista1 < (Valor_1 - Sensibilidad_Res)) || (Pista1 > (Valor_1 +
           Sensibilidad_Res))) { Another_Value = true;
            }
           if ((Pista2 < (Valor_2 - Sensibilidad_Res)) || (Pista2 > (Valor_2 +
           Sensibilidad_Res))) { Another_Value = true;
           }
        */

        // Another_Value = true; //
        // **************************************************************** BORRAR
        // OJO  *******

        if (Another_Value && !History_Buffer_Full)
        {
            NRF_LOG_RAW_INFO("GRABACION HISTORIA posision/total %d/%d \r\n", History_Position,
                             Size_Memory_History);
            NRF_LOG_FLUSH();
            Flash_array.history[History_Position].day      = t.date;
            Flash_array.history[History_Position].month    = t.month;
            Flash_array.history[History_Position].year     = t.year;
            Flash_array.history[History_Position].hour     = t.hour;
            Flash_array.history[History_Position].minute   = t.minute;
            Flash_array.history[History_Position].second   = t.second;
            Flash_array.history[History_Position].Contador = contador;
            Flash_array.history[History_Position].V1       = Pista1;
            Flash_array.history[History_Position].V2       = Pista2;
            Flash_array.history[History_Position].V3       = 1;
            Flash_array.history[History_Position].V4       = 2;
            Flash_array.history[History_Position].V5       = 3;
            Flash_array.history[History_Position].V6       = 4;
            Flash_array.history[History_Position].V7       = 5;
            Flash_array.history[History_Position].V8       = 6;
            Flash_array.history[History_Position].battery  = Battery_level;
            History_Position++;

            if (History_Position >= (Size_Memory_History - 1))
            {
                History_Buffer_Full = true; // Marcar el buffer como lleno
                NRF_LOG_RAW_INFO("BUFFER DE HISTORIAL LLENO - No se guardaran mas registros\r\n");
                NRF_LOG_FLUSH();
            }

            Flash_array.last_history = History_Position;
            Another_Value            = false;
        }
        else if (Another_Value && History_Buffer_Full)
        {
            NRF_LOG_RAW_INFO("Intento de escritura rechazado - Buffer de historial lleno\r\n");
            NRF_LOG_FLUSH();
            Another_Value = false; // Resetear para evitar intentos repetitivos
        }
    }

    if (impresion_log)
    {
        for (int loop_med = 0; loop_med <= (History_Position + 1); loop_med++)
        {
            NRF_LOG_RAW_INFO("historia %d: - fecha %d/%d/%d ", loop_med,
                             Flash_array.history[loop_med].day, Flash_array.history[loop_med].month,
                             Flash_array.history[loop_med].year);
            NRF_LOG_FLUSH();
            NRF_LOG_RAW_INFO("%d:%d:%d ", Flash_array.history[loop_med].hour,
                             Flash_array.history[loop_med].minute,
                             Flash_array.history[loop_med].second);
            NRF_LOG_FLUSH();
            NRF_LOG_RAW_INFO("Cont: %d :  %d/%d/%d \r\n", Flash_array.history[loop_med].Contador,
                             Flash_array.history[loop_med].V1, Flash_array.history[loop_med].V2,
                             Flash_array.history[loop_med].battery);
            NRF_LOG_FLUSH();
            nrf_delay_ms(20);
        }
    }

    if (Value_Wear < 0xFD)
    {
        float _precut = (float)Flash_array.Offset_sensor_cut * Step_of_resistor;
        if (_precut > 0.0)
            Value_Wear = Value_Wear - _precut;

        float _offset = (float)Offset_desgaste - 128;

        if (_offset >= 0.0)
        {
            if (Value_Wear <= _offset)
                Value_Wear = 0.0;
            else
                Value_Wear = round(Value_Wear - _offset);
        }

        if (_offset < 0.0)
        {
            _offset = _offset * -1.0;
            if (Value_Wear != 0.0)
                Value_Wear = round(Value_Wear + _offset);
        }
    }

    NRF_LOG_RAW_INFO("mm desgastados corregidos " NRF_LOG_FLOAT_MARKER "\r\n",
                     NRF_LOG_FLOAT(Value_Wear));

    NRF_LOG_FLUSH();
    Chip_temperature();
    m_beacon_info[0] = Tipo_dispositivo; // 0x0f Vibracion
    m_beacon_info[1] = MSB_16(contador); // contador superior
    m_beacon_info[2] = LSB_16(contador); // contador inferior
    // primer Sector
    m_beacon_info[3] = MSB_16(Pista1);
    m_beacon_info[4] = LSB_16(Pista1);
    // Segundo Sector
    m_beacon_info[5]  = MSB_16(Pista2);
    m_beacon_info[6]  = LSB_16(Pista2);

    m_beacon_info[7]  = 0x00;
    m_beacon_info[8]  = 0x00;
    m_beacon_info[9]  = 0x00;
    m_beacon_info[10] = 0x00;
    m_beacon_info[11] = 0x00;
    m_beacon_info[12] = Flash_array.total_reset[3]; // Imprimiremos momentaneamente la cantidad de
                                                    // reset del equipo

    // Largo del corte esta en mm, en el sensor la division es de 2 mm.
    // al cortarse todas las resistencias quedan 2 de seguridad, pero el sensor
    // indicará igual mente la misma medida si las ultimas dos resistencias se
    // cortan el sensor envia un 255

    m_beacon_info[13] = (uint8_t)(Value_Wear);
    m_beacon_info[14] = (uint8_t)(Battery_level);
    m_beacon_info[15] = (uint8_t)temp; // temperatura del NODO
    m_beacon_info[18] = Flash_array.mac_original[0];
    m_beacon_info[19] = Flash_array.mac_original[1];
    m_beacon_info[20] = Flash_array.mac_original[2];
    m_beacon_info[21] = Flash_array.mac_original[3];
    m_beacon_info[22] = Flash_array.mac_original[4];
    m_beacon_info[23] = Flash_array.mac_original[5];
    contador++;
    Counter_ADV();
    Fstorage_Erase_Writing();
    Fstorage_Read_Data();
    wait_for_flash_ready(&fstorage);
    NRF_LOG_RAW_INFO("asignacion de datos ADV OK");
    NRF_LOG_FLUSH();

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier =
        0x2233; // se escribe al revez ... numero de compañia 3322
    manuf_specific_data.data.p_data = (uint8_t *)m_beacon_info;
    manuf_specific_data.data.size   = sizeof(m_beacon_info);

    memset(&init, 0, sizeof(init));

    init.advdata.name_type                     = BLE_ADVDATA_NO_NAME; // BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance            = false;
    init.advdata.p_manuf_specific_data         = &manuf_specific_data;
    init.advdata.p_tx_power_level              = &Potencia_antenna;

    init.config.ble_adv_on_disconnect_disabled = true;
    init.config.ble_adv_fast_enabled           = true;
    init.config.ble_adv_fast_interval          = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout           = APP_ADV_DURATION;
    init.evt_handler                           = on_adv_evt;

    err_code                                   = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    Transmiting_Ble   = true;

    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

void set_addr(void)
{
    static ble_gap_addr_t m_central_addr;
    m_central_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;

    m_central_addr.addr[0]   = Flash_array.mac_custom[5];
    m_central_addr.addr[1]   = Flash_array.mac_custom[4];
    m_central_addr.addr[2]   = Flash_array.mac_custom[3];
    m_central_addr.addr[3]   = Flash_array.mac_custom[2];
    m_central_addr.addr[4]   = Flash_array.mac_custom[1];
    m_central_addr.addr[5]   = Flash_array.mac_custom[0];
    sd_ble_gap_addr_set(&m_central_addr);
}
