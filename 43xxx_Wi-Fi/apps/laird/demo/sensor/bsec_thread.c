/**********************************************************************************************************************/
/* header files */
/**********************************************************************************************************************/
#include "wiced.h"

#include "sd_sterling_demo_dct.h"

#include "bsec_integration.h"
#include "bsec_thread.h"

/******************************************************************************
* Constants
******************************************************************************/

#define I2C_XFER_RETRY_COUNT    (3)
#define I2C_DMA_POLICY          WICED_FALSE

/******************************************************************************
* Global variables
******************************************************************************/

/* Global variable holding thread data */
BSEC_DATA_T bsec_thread_data;

/******************************************************************************
* Local functions
******************************************************************************/

/* I2C device definition for the BME680 */
static const wiced_i2c_device_t BME680_DEV =
{
    .port = WICED_I2C_2,
    .address = BME680_I2C_ADDR_PRIMARY,
    .address_width = I2C_ADDRESS_WIDTH_7BIT,
    .flags = 0,
    .speed_mode = I2C_HIGH_SPEED_MODE
};

/*!
 * @brief           Write operation in either I2C or SPI
 *
 * param[in]        dev_addr        I2C or SPI device address
 * param[in]        reg_addr        register address
 * param[in]        reg_data_ptr    pointer to the data to be written
 * param[in]        data_len        number of bytes to be written
 *
 * @return          result of the bus communication function
 */
static int8_t bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data_ptr, uint16_t data_len)
{
    wiced_result_t result;
    wiced_i2c_message_t msg;
    uint8_t buf[64];

    /* Make sure that we have space in our buffer */
    if (data_len > (sizeof(buf) - 1))
    {
        return WICED_ERROR;
    }

    /* Fill in the buffer */
    buf[0] = reg_addr;
    memcpy (buf + 1, reg_data_ptr, data_len);

    /* Create the TX message */
    result = wiced_i2c_init_tx_message (
        &msg,
        buf, data_len + 1,
        I2C_XFER_RETRY_COUNT, I2C_DMA_POLICY);

    /* Send the message */
    if (result == WICED_SUCCESS)
    {
        result = wiced_i2c_transfer(&BME680_DEV, &msg, 1); // 1 message transfer
        /* TODO: change this to transfer 2 messages to avoid buffer? */
    }

    return result;
}

/*!
 * @brief           Read operation in either I2C or SPI
 *
 * param[in]        dev_addr        I2C or SPI device address
 * param[in]        reg_addr        register address
 * param[out]       reg_data_ptr    pointer to the memory to be used to store the read data
 * param[in]        data_len        number of bytes to be read
 *
 * @return          result of the bus communication function
 */
static int8_t bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data_ptr, uint16_t data_len)
{
    wiced_result_t result;
    wiced_i2c_message_t msg;

    // Initialize the i2c message params for the address write then multi-byte read
    result = wiced_i2c_init_combined_message (&msg,
        &reg_addr, reg_data_ptr, sizeof(reg_addr), data_len,
        I2C_XFER_RETRY_COUNT, I2C_DMA_POLICY);
    if (result == WICED_SUCCESS)
    {
        result = wiced_i2c_transfer(&BME680_DEV, &msg, 1);
    }

    return result;
}

/*!
 * @brief           System specific implementation of sleep function
 *
 * @param[in]       t_ms    time in milliseconds
 *
 * @return          none
 */
static void sleep(uint32_t t_ms)
{
    wiced_rtos_delay_milliseconds(t_ms);
}

/*!
 * @brief           Capture the system time in microseconds
 *
 * @return          system_current_time    current system timestamp in microseconds
 */
static int64_t get_timestamp_us(void)
{
    wiced_time_t t;
    int64_t retval;

    wiced_time_get_time(&t);
    retval = t;
    retval *= 1000;

    return retval;
}

/*!
 * @brief           Handling of the ready outputs
 *
 * @param[in]       timestamp       time in nanoseconds
 * @param[in]       iaq             IAQ signal
 * @param[in]       iaq_accuracy    accuracy of IAQ signal
 * @param[in]       temperature     temperature signal
 * @param[in]       humidity        humidity signal
 * @param[in]       pressure        pressure signal
 * @param[in]       raw_temperature raw temperature signal
 * @param[in]       raw_humidity    raw humidity signal
 * @param[in]       gas             raw gas sensor signal
 * @param[in]       bsec_status     value returned by the bsec_do_steps() call
 *
 * @return          none
 */
static void output_ready(int64_t timestamp, float iaq, uint8_t iaq_accuracy, float temperature, float humidity,
     float pressure, float raw_temperature, float raw_humidity, float gas, bsec_library_return_t bsec_status,
     float static_iaq, float co2_equivalent, float breath_voc_equivalent)
{
    wiced_rtos_lock_mutex(&(bsec_thread_data.mutex));
    wiced_time_get_utc_time_ms(&(bsec_thread_data.timestamp));
    bsec_thread_data.temperature = temperature;
    bsec_thread_data.humidity = humidity;
    bsec_thread_data.pressure = pressure;
    bsec_thread_data.iaq = iaq;
    bsec_thread_data.iaq_accuracy = iaq_accuracy;
    wiced_rtos_unlock_mutex(&(bsec_thread_data.mutex));
}

/*!
 * @brief           Load previous library state from non-volatile memory
 *
 * @param[in,out]   state_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to state_buffer
 */
static uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer)
{
    sd_sterling_demo_dct_data_t* dct_app = NULL;

    /* Read device info data from user DCT */
    if (wiced_dct_read_lock(
            (void**) &dct_app, WICED_FALSE, DCT_APP_SECTION,
            0, sizeof(*dct_app)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("state_load: failed to read DCT\n"));
        return 0;
    }

    memcpy(state_buffer, dct_app->bsec_state, dct_app->bsec_state_len);

    wiced_dct_read_unlock (dct_app, WICED_FALSE);

    return dct_app->bsec_state_len;
}

/*!
 * @brief           Save library state to non-volatile memory
 *
 * @param[in]       state_buffer    buffer holding the state to be stored
 * @param[in]       length          length of the state string to be stored
 *
 * @return          none
 */
static void state_save(const uint8_t *state_buffer, uint32_t length)
{
    sd_sterling_demo_dct_data_t* dct_app = NULL;

    printf ("BME680 saving state\n");

    /* Read device info data from user DCT */
    if (wiced_dct_read_lock(
            (void**) &dct_app, WICED_TRUE, DCT_APP_SECTION,
            0, sizeof(*dct_app)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("state_load: failed to read DCT\n"));
        return;
    }

    memcpy(dct_app->bsec_state, state_buffer, length);
    dct_app->bsec_state_len = length;

    wiced_dct_write((const void*) dct_app, DCT_APP_SECTION,
            0, sizeof(sd_sterling_demo_dct_data_t));

    // release the lock
    wiced_dct_read_unlock( dct_app, WICED_TRUE );
}
 
/*!
 * @brief           Load library config from non-volatile memory
 *
 * @param[in,out]   config_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to config_buffer
 */
static uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer)
{
    // ...
    // Load a library config from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no config was available, 
    // otherwise return length of loaded config string.
    // ...
    return 0;
}

/******************************************************************************
* Global functions
******************************************************************************/

/*!
 * @brief       Main function which configures BSEC library and then reads and processes the data from sensor based
 *              on timer ticks
 *
 * @return      result of the processing
 */
void bsec_thread (wiced_thread_arg_t arg)
{
    return_values_init ret;
    
    wiced_i2c_init(&BME680_DEV);

    /* Call to the function which initializes the BSEC library 
     * Switch on low-power mode and provide no temperature offset */
    ret = bsec_iot_init(BSEC_SAMPLE_RATE_LP, 0.0f,
        bus_write, bus_read, sleep, state_load, config_load);
    if (ret.bme680_status)
    {
        /* Could not intialize BME680 */
        printf ("bsec_iot_init: could not init BME680: %d\n",
            (int)ret.bme680_status);
        return;
    }
    else if (ret.bsec_status)
    {
        /* Could not intialize BSEC library */
        printf ("bsec_iot_init: could not init library: %d\n",
            (int)ret.bsec_status);
        return;
    }
    
    /*
     * Call to endless loop function which reads and processes data based on
     * sensor settings. State is saved every 10000 samples, which means
     * every 10.000 * 3 secs = 500 minutes = 8.33 hours.
     */
    bsec_iot_loop(sleep, get_timestamp_us, output_ready, state_save, 10000);
}

