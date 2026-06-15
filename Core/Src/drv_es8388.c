/*
 * File: es8388.c
 * Description: Bare-metal driver implementation for ES8388 audio codec.
 * Targeted for: 16-bit data, 16-bit frame, 44.1kHz Philips I2S.
 */

#include "drv_es8388.h"

/* ========================================================================== */
/* Macro Configurations & Local State                                         */
/* ========================================================================== */
#define ES8388_I2C_ADDR   (0x10 << 1)  // Shifted 7-bit address for STM32 HAL (0x20)
#define I2C_TIMEOUT       100          // 100ms hardware block timeout

// Direct, local handle to track the assigned physical STM32 I2C peripheral
static I2C_HandleTypeDef *es_hi2c = NULL;
static uint8_t current_volume = 50;

/* ========================================================================== */
/* Static Low-Level Hardware Access Functions                                 */
/* ========================================================================== */

/**
 * @brief  Writes a byte to a specific register on the ES8388 via standard I2C.
 */
static void reg_write(uint8_t addr, uint8_t val)
{
    if (es_hi2c == NULL) return;

    uint8_t buffer[2];
    buffer[0] = addr;
    buffer[1] = val;

    // Direct bare-metal sequential write blocking call
    HAL_I2C_Master_Transmit(es_hi2c, ES8388_I2C_ADDR, buffer, 2, I2C_TIMEOUT);
}

/**
 * @brief  Reads a byte back from a specific register on the ES8388.
 */
static uint8_t reg_read(uint8_t addr)
{
    if (es_hi2c == NULL) return 0xFF;

    uint8_t val = 0xFF;

    // Send the address pointer we wish to read to the device
    if (HAL_I2C_Master_Transmit(es_hi2c, ES8388_I2C_ADDR, &addr, 1, I2C_TIMEOUT) == HAL_OK)
    {
        // Read the resulting register value byte back
        HAL_I2C_Master_Receive(es_hi2c, ES8388_I2C_ADDR, &val, 1, I2C_TIMEOUT);
    }

    return val;
}

/**
 * @brief  Helper utility to format and commit analog/digital gains inside the chip.
 */
static void es8388_set_adc_dac_volume(es8388_mode_t mode, int volume, int dot)
{
    if (volume < -96 || volume > 0)
    {
        volume = (volume < -96) ? -96 : 0;
    }

    dot = (dot >= 5 ? 1 : 0);
    volume = (-volume << 1) + dot;

    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_ADCCONTROL8, volume);
        reg_write(ES8388_ADCCONTROL9, volume);
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_DACCONTROL5, volume);
        reg_write(ES8388_DACCONTROL4, volume);
    }
}

/**
 * @brief  Toggles absolute hardware muting on the DAC output stream internally.
 */
static void es8388_set_voice_mute(uint8_t enable)
{
    uint8_t reg = reg_read(ES8388_DACCONTROL3);
    reg &= 0xFB; // Strip bit 2
    reg_write(ES8388_DACCONTROL3, reg | ((enable & 0x01) << 2));
}

/* ========================================================================== */
/* Public API Implementations                                                 */
/* ========================================================================== */

HAL_StatusTypeDef es8388_init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    // Bind the hardware pointer
    es_hi2c = hi2c;

    // Initial mute statement to protect speakers during register sequencing
    reg_write(ES8388_DACCONTROL3, 0x04);

    /* Chip Control and Power Management Configuration */
    reg_write(ES8388_CONTROL2,   0x50);
    reg_write(ES8388_CHIPPOWER,  0x00); // Normal operation mode / Power up all modules
    reg_write(ES8388_MASTERMODE, 0x00); // CODEC CONFIGURED IN I2S SLAVE MODE

    /* DAC Initialization Path */
    reg_write(ES8388_DACPOWER,     0xC0); // Temporarily disable DAC stages
    reg_write(ES8388_CONTROL1,     0x12); // Master Play & Record Mode enabled

    // HARDCODED TIMING FIX FOR 16-BIT DATA/FRAME PHILIPS I2S:
    // 0x18 explicitly forces 16-bit word lengths & Philips alignment standard (FMT = 00)
    reg_write(ES8388_DACCONTROL1,  0x18);
    reg_write(ES8388_DACCONTROL2,  0x02); // DACFsMode = SINGLE SPEED; DACFsRatio = 256
    reg_write(ES8388_DACCONTROL16, 0x00); // Route audio signals straight through LIN1 & RIN1
    reg_write(ES8388_DACCONTROL17, 0x9C); // Left DAC to Left Mixer enabled at 0dB
    reg_write(ES8388_DACCONTROL20, 0x9C); // Right DAC to Right Mixer enabled at 0dB
    reg_write(ES8388_DACCONTROL21, 0x80); // Share the same internal LRCK clock (Locked to ADC clock source)
    reg_write(ES8388_DACCONTROL23, 0x00); // VROI = 0

    es8388_set_adc_dac_volume(ES_MODE_DAC, 0, 0); // Ground default digital volume to 0dB
    reg_write(ES8388_DACPOWER,     0x3C); // Re-enable DAC paths along with Lout1/Rout1/Lout2/Rout2

    /* ADC Initialization Path */
    reg_write(ES8388_ADCPOWER,     0xFF); // Reset power flags
    reg_write(ES8388_ADCCONTROL1,  0xBB); // Configure Microphone PGA Gains
    reg_write(ES8388_ADCCONTROL2,  0x00); // Set LIN1/RIN1 as direct ADC Inputs
    reg_write(ES8388_ADCCONTROL3,  0x02);

    // 0x00 = I2S Philips Standard format, 16-bit length (FMT = 00, WL = 00)
    reg_write(ES8388_ADCCONTROL4,  0x00);
    reg_write(ES8388_ADCCONTROL5,  0x02); // ADCFsMode = Single Speed, Clock Ratio = 256

    es8388_set_adc_dac_volume(ES_MODE_ADC, 0, 0); // Set input record volume to 0dB
    reg_write(ES8388_ADCPOWER,     0x09); // Activate ADC modules, bring down MICBIAS

    /* Default Hardware Line Volume Step Settings */
    reg_write(ES8388_DACCONTROL24, 0x1E); // LOUT1 Volume balance step
    reg_write(ES8388_DACCONTROL25, 0x1E); // ROUT1 Volume balance step

    // Establish a default safe state
    es8388_volume_set(60);

    return HAL_OK;
}

HAL_StatusTypeDef es8388_start(es8388_mode_t mode)
{
    uint8_t prev_data = reg_read(ES8388_DACCONTROL21);

    if (mode == ES_MODE_LINE)
    {
        reg_write(ES8388_DACCONTROL16, 0x09); // Divert pathing to LIN2/RIN2
        reg_write(ES8388_DACCONTROL17, 0x50); // Cross mixer gains
        reg_write(ES8388_DACCONTROL20, 0x50);
        reg_write(ES8388_DACCONTROL21, 0xC0); // Power on ADC clock mappings
    }
    else
    {
        reg_write(ES8388_DACCONTROL21, 0x80); // Maintain normal DAC operations
    }

    uint8_t data = reg_read(ES8388_DACCONTROL21);

    if (prev_data != data)
    {
        // Toggle power engine registers to safely latch clock routing configuration changes
        reg_write(ES8388_CHIPPOWER, 0xF0);
        reg_write(ES8388_CHIPPOWER, 0x00);
    }

    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC || mode == ES_MODE_LINE)
    {
        reg_write(ES8388_ADCPOWER, 0x00); // Fully clear ADC shutdown flags
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC || mode == ES_MODE_LINE)
    {
        reg_write(ES8388_DACPOWER, 0x3C); // Activate output analog stages
        es8388_set_voice_mute(0);          // Unmute stream output
    }

    return HAL_OK;
}

HAL_StatusTypeDef es8388_stop(es8388_mode_t mode)
{
    if (mode == ES_MODE_LINE)
    {
        reg_write(ES8388_DACCONTROL21, 0x80);
        reg_write(ES8388_DACCONTROL16, 0x00);
        reg_write(ES8388_DACCONTROL17, 0x90);
        reg_write(ES8388_DACCONTROL20, 0x90);
        return HAL_OK;
    }
    if (mode == ES_MODE_DAC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_DACPOWER, 0x00); // Pull down DAC analog pipelines entirely
        es8388_set_voice_mute(1);          // Apply immediate hardware mute clamping
    }
    if (mode == ES_MODE_ADC || mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_ADCPOWER, 0xFF); // Pull up ADC block power cuts
    }
    if (mode == ES_MODE_DAC_ADC)
    {
        reg_write(ES8388_DACCONTROL21, 0x9C); // Kill internal clock structures
    }

    return HAL_OK;
}

void es8388_volume_set(uint8_t volume)
{
    uint32_t real_vol = 0;

    if (volume > 100) volume = 100;
    current_volume = volume;

    // Invert scale: 0 is loudest on ES8388 registers, 100 is quietest
    volume = 100 - volume;
    real_vol = (192 * volume) / 100;

    reg_write(ES8388_DACCONTROL4, (uint8_t)real_vol);  // Commit Left DAC attenuation
    reg_write(ES8388_DACCONTROL5, (uint8_t)real_vol);  // Commit Right DAC attenuation
}

uint8_t es8388_volume_get(void)
{
    return current_volume;
}

HAL_StatusTypeDef es8388_enable_mic_bypass(void)
{
    if (es_hi2c == NULL) return HAL_ERROR;

    // 1. Power on everything: ADC, DAC, and Mixer internal circuits
    reg_write(ES8388_CHIPPOWER,  0x00); // Power up all internal state machines
    reg_write(ES8388_ADCPOWER,   0x00); // Power up ADC and input lines
    reg_write(ES8388_DACPOWER,   0x3C); // Power up DAC and output lines (LOUT1/ROUT1)

    // 2. Select Input Channels
    // 0x00 connects LIN1/RIN1 to the internal PGA (Onboard Mic or Line In)
    reg_write(ES8388_ADCCONTROL2, 0x00);

    // 3. Set Input Pre-Amplifier (PGA) Gain
    // 0x88 gives a solid +24dB gain boost so you can easily hear the microphone
    reg_write(ES8388_ADCCONTROL1, 0x88);

    // 4. THE CRUCIAL BYPASS ROUTING: Connect input to the Output Mixers
    // 0x1B routes the Lin1/Rin1 signals directly to Left/Right Output Mixers
    reg_write(ES8388_DACCONTROL16, 0x1B);

    // 5. Enable the Mixer bypass paths and set to 0dB gain
    reg_write(ES8388_DACCONTROL17, 0x40); // Connect Left Input to Left Mixer (0dB)
    reg_write(ES8388_DACCONTROL20, 0x40); // Connect Right Input to Right Mixer (0dB)

    // 6. Set physical line output volume sliders to a safe, clear level
    reg_write(ES8388_DACCONTROL24, 0x21); // LOUT1 Volume
    reg_write(ES8388_DACCONTROL25, 0x21); // ROUT1 Volume

    // 7. Make absolutely sure the Master Mute is turned off
    uint8_t dac_ctrl3 = reg_read(ES8388_DACCONTROL3);
    reg_write(ES8388_DACCONTROL3, dac_ctrl3 & 0xFB); // Force clear the mute bit

    return HAL_OK;
}
