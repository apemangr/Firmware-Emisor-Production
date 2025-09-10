#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "nrf_fstorage_nvmc.h"

extern store_flash Flash_array;
extern uint8_t     Reset_Index;

static void        fstorage_evt_handler(nrf_fstorage_evt_t *p_evt);

NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage) = {
    /* Set a handler for fstorage events. */
    .evt_handler = fstorage_evt_handler,

    /*
    These below are the boundaries of the flash space assigned to this instance of fstorage.
    * You must set these manually, even at runtime, before nrf_fstorage_init() is called.
    * The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
    * last page of flash available to write data. */
    .start_addr = 0x7C000,
    .end_addr   = 0x7FFFF,
};

//.start_addr = 0x7C000,
//.end_addr   = 0x7FFFF,

// LA MEMORIA FLASH tiene segmentos de 4096 bytes, y cuando se borra la memoeria se borra por
// segmentos

/**@brief   Sleep until an event is received. */
static void power_manage(void)
{
#ifdef SOFTDEVICE_PRESENT
    (void)sd_app_evt_wait();
#else
    __WFE();
#endif
}

static void print_flash_info(nrf_fstorage_t *p_fstorage)
{
    NRF_LOG_RAW_INFO("========| flash info |========\r\n");
    NRF_LOG_RAW_INFO("erase unit: \t%d bytes\r\n", p_fstorage->p_flash_info->erase_unit);
    NRF_LOG_RAW_INFO("program unit: \t%d bytes\r\n", p_fstorage->p_flash_info->program_unit);
    NRF_LOG_RAW_INFO("==============================\r\n");
    NRF_LOG_FLUSH();
}
static uint32_t nrf5_flash_end_addr_get()
{
    uint32_t const bootloader_addr = BOOTLOADER_ADDRESS;
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
    uint32_t const code_sz         = NRF_FICR->CODESIZE;

    return (bootloader_addr != 0xFFFFFFFF ? bootloader_addr : (code_sz * page_sz));
}

static void fstorage_evt_handler(nrf_fstorage_evt_t *p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        // NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
    case NRF_FSTORAGE_EVT_WRITE_RESULT: {
        // NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
        //              p_evt->len, p_evt->addr);
    }
    break;

    case NRF_FSTORAGE_EVT_ERASE_RESULT: {
        // NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
        //              p_evt->len, p_evt->addr);
    }
    break;

    default:
        break;
    }
}

void wait_for_flash_ready(nrf_fstorage_t const *p_fstorage)
{
    /* While fstorage is busy, sleep and wait for an event. */
    while (nrf_fstorage_is_busy(p_fstorage))
    {
        power_manage();
    }
}

void FsStorage_Init()
{
    ret_code_t rc;
    NRF_LOG_RAW_INFO("fstorage example started.\r\n");

    nrf_fstorage_api_t *p_fs_api;
    p_fs_api = &nrf_fstorage_sd;
    rc       = nrf_fstorage_init(&fstorage, p_fs_api, NULL);
    APP_ERROR_CHECK(rc);
    print_flash_info(&fstorage);
    (void)nrf5_flash_end_addr_get();
}

void Fstorage_Read_Data()
{
    ret_code_t rc;
    if (len > sizeof(Flash_array))
    {
        len = sizeof(Flash_array);
    }
    NRF_LOG_RAW_INFO("Datos de la Flash nueva : %is\n", sizeof(Flash_array));
    NRF_LOG_FLUSH();

    rc = nrf_fstorage_read(&fstorage, fstorage.start_addr, &Flash_array, sizeof(Flash_array));
    if (rc != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_fstorage_read() returned: %s\n", nrf_strerror_get(rc));
        NRF_LOG_FLUSH();
    }
}
void Fstorage_Erase_Writing()
{
    ret_code_t rc;

    rc = nrf_fstorage_erase(&fstorage, fstorage.start_addr, 2, NULL); // 0x3e

    if (rc != NRF_SUCCESS)
    {
        NRF_LOG_INFO("nrf_fstorage_erase() returned: %s\n", nrf_strerror_get(rc));
        NRF_LOG_FLUSH();
    }

    wait_for_flash_ready(&fstorage);
    rc =
        nrf_fstorage_write(&fstorage, fstorage.start_addr, &Flash_array, sizeof(Flash_array), NULL);

    APP_ERROR_CHECK(rc);
    wait_for_flash_ready(&fstorage);
}

void Show_reset_step()
{
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_FLUSH();
    NRF_LOG_RAW_INFO("Puntos de reset del programa  : \r\n");
    NRF_LOG_FLUSH();
    for (int i = 0; i < (0 + 10); i++)
    {
        NRF_LOG_RAW_INFO("%i,", Flash_array.reset[i]);
    }
    NRF_LOG_RAW_INFO("\r\n");
    NRF_LOG_FLUSH();
}

/**@brief Application main function.
 */
void Reset_Line_Step(int Line)
{

    Flash_array.reset[Reset_Index] = Line;
    /*
    data_flash[Reset_Index]=Line;

    */
    Fstorage_Erase_Writing();
}

void Counter_restart()
{
    uint32_t result = 0;
    result += (Flash_array.total_reset[3] << 0);
    result += (Flash_array.total_reset[2] << 8);
    result += (Flash_array.total_reset[1] << 16);
    result += (Flash_array.total_reset[0] << 24);
    result++;
    Flash_array.total_reset[0] = ((result >> 24) & 0xFF);
    Flash_array.total_reset[1] = ((result >> 16) & 0xFF);
    Flash_array.total_reset[2] = ((result >> 8) & 0XFF);
    Flash_array.total_reset[3] = (result & 0XFF);
}
void Counter_ADV()
{
    uint32_t result = 0;
    result += (Flash_array.total_adv[3] << 0);
    result += (Flash_array.total_adv[2] << 8);
    result += (Flash_array.total_adv[1] << 16);
    result += (Flash_array.total_adv[0] << 24);
    result++;
    Flash_array.total_adv[0] = ((result >> 24) & 0xFF);
    Flash_array.total_adv[1] = ((result >> 16) & 0xFF);
    Flash_array.total_adv[2] = ((result >> 8) & 0XFF);
    Flash_array.total_adv[3] = (result & 0XFF);
}
void Flash_factory()
{
    memset(&Flash_Factory, 0, sizeof(Flash_Factory));

    Flash_Factory.company[0]    = 0x33;
    Flash_Factory.company[1]    = 0x22;


    // 0x00 0x00 0x64 -> 10 segundos de dormido
    Flash_Factory.sleep_time[0] = 0x00;
    Flash_Factory.sleep_time[1] = 0x17; // es en base a decimas de segundo 1800 serian 180 segundos.
    Flash_Factory.sleep_time[2] = 0x70; // 1770 son 6000 lo que equivale a 600 segundos = 10 min.
    
    // 0x03 0xe8 -> 10 segundos de ADV
    Flash_Factory.adv_time[0] = 0x03;
    Flash_Factory.adv_time[1] = 0xe8;

    Flash_Factory.Type_sensor        = 0x10;
    Flash_Factory.offset_plate_bolt  = 0x80; // 77 son 119  que son 28 mm cortados
    Flash_Factory.Offset_sensor_bolt = 0x7d;
    Flash_Factory.Offset_sensor_cut  = 0x80; // por que 1C?=28   80 es 128
    Flash_Factory.Sensibility        = 15;
    Flash_Factory.last_history       = 0;
    Flash_Factory.Sending_Position   = 0;
    Flash_Factory.second             = 0;
    Flash_Factory.minute             = 0;
    Flash_Factory.hour               = 0;
    Flash_Factory.date               = 01;
    Flash_Factory.month              = 01;
    Flash_Factory.year               = 2024;
    Flash_Factory.Version[0]         = 0;
    Flash_Factory.Version[1]         = 8;
    Flash_Factory.Version[2]         = 8;
}

void Flash_reset_check()
{
    for (int i = Reset_Index; i < (Reset_Index + 10); i++)
    {
        if (Flash_array.reset[i] == 0x00 || Flash_array.reset[i] == 0xff)
        {
            Reset_Index = i;
            break;
        }
    }
}

uint32_t Flash_is_empty()
{
    uint32_t error_code = NRF_SUCCESS;

    if ((Flash_array.company[0] == 0xff) && (Flash_array.company[1] == 0xff))
    {
        NRF_LOG_RAW_INFO("Grabacion Flash Standard.... tama�o %d \r\n", sizeof(Flash_array));
        NRF_LOG_FLUSH();
        memset(&Flash_array, 0x00, sizeof(Flash_array));
        Flash_array = Flash_Factory;
        Fstorage_Erase_Writing();
        Fstorage_Read_Data();
        ble_gap_addr_t my_addr;

        error_code = sd_ble_gap_addr_get(&my_addr);

        if (error_code != NRF_SUCCESS)
        {
            return error_code;
        }

        Flash_array.mac_original[0] = my_addr.addr[5];
        Flash_array.mac_original[1] = my_addr.addr[4];
        Flash_array.mac_original[2] = my_addr.addr[3];
        Flash_array.mac_original[3] = my_addr.addr[2];
        Flash_array.mac_original[4] = my_addr.addr[1];
        Flash_array.mac_original[5] = my_addr.addr[0];
    }
    else
    {
        // Retoma el contador que llevaba
        contador += (Flash_array.total_adv[3] << 0);
        contador += (Flash_array.total_adv[2] << 8);
        contador += (Flash_array.total_adv[1] << 16);
        contador += (Flash_array.total_adv[0] << 24);

        // Carga Tipo de Sensor

        Tipo_Configuracion_ohm = Flash_array.Type_resistor;

        t.year                 = Flash_array.fecha[0] + 2000;
        t.month                = Flash_array.fecha[1];
        t.date                 = Flash_array.fecha[2];
        t.hour                 = Flash_array.hora[0];
        t.minute               = Flash_array.hora[1];
        t.second               = Flash_array.hora[2];
    }

    // Carga Tiempo de dormido
    sleep_in_time_ticker = (Flash_array.sleep_time[0] << 16);
    sleep_in_time_ticker += (Flash_array.sleep_time[1] << 8);
    sleep_in_time_ticker += (Flash_array.sleep_time[2] << 0);

    NRF_LOG_RAW_INFO("time sleep %i.... \r\n", sleep_in_time_ticker);
    NRF_LOG_FLUSH();

    // Carga Tiempo de Advertising programado
    APP_ADV_DURATION = (Flash_array.adv_time[0] << 8);
    APP_ADV_DURATION += (Flash_array.adv_time[1] << 0);

    NRF_LOG_RAW_INFO(" time adv %i....\r\n", APP_ADV_DURATION);
    NRF_LOG_FLUSH();

    return error_code;
}
