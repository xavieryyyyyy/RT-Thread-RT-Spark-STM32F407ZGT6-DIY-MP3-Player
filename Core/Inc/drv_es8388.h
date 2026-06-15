/*
 * File: es8388.h
 * Description: Bare-metal driver header for ES8388 audio codec.
 * Targeted for: 16-bit data, 16-bit frame, 44.1kHz Philips I2S.
 */

#ifndef __DRV_ES8388_H__
#define __DRV_ES8388_H__

#include "stm32f4xx_hal.h"

/* ========================================================================== */
/* ES8388 Register Space                                                      */
/* ========================================================================== */
#define ES8388_CONTROL1         0x00
#define ES8388_CONTROL2         0x01
#define ES8388_CHIPPOWER        0x02
#define ES8388_ADCPOWER         0x03
#define ES8388_DACPOWER         0x04
#define ES8388_CHIPLOPOW1       0x05
#define ES8388_CHIPLOPOW2       0x06
#define ES8388_ANAVOLMANAG      0x07
#define ES8388_MASTERMODE       0x08
#define ES8388_ADCCONTROL1      0x09
#define ES8388_ADCCONTROL2      0x0a
#define ES8388_ADCCONTROL3      0x0b
#define ES8388_ADCCONTROL4      0x0c
#define ES8388_ADCCONTROL5      0x0d
#define ES8388_ADCCONTROL6      0x0e
#define ES8388_ADCCONTROL7      0x0f
#define ES8388_ADCCONTROL8      0x10
#define ES8388_ADCCONTROL9      0x11
#define ES8388_ADCCONTROL10     0x12
#define ES8388_ADCCONTROL11     0x13
#define ES8388_ADCCONTROL12     0x14
#define ES8388_ADCCONTROL13     0x15
#define ES8388_ADCCONTROL14     0x16

#define ES8388_DACCONTROL1      0x17
#define ES8388_DACCONTROL2      0x18
#define ES8388_DACCONTROL3      0x19
#define ES8388_DACCONTROL4      0x1a
#define ES8388_DACCONTROL5      0x1b
#define ES8388_DACCONTROL6      0x1c
#define ES8388_DACCONTROL7      0x1d
#define ES8388_DACCONTROL8      0x1e
#define ES8388_DACCONTROL9      0x1f
#define ES8388_DACCONTROL10     0x20
#define ES8388_DACCONTROL11     0x21
#define ES8388_DACCONTROL12     0x22
#define ES8388_DACCONTROL13     0x23
#define ES8388_DACCONTROL14     0x24
#define ES8388_DACCONTROL15     0x25
#define ES8388_DACCONTROL16     0x26
#define ES8388_DACCONTROL17     0x27
#define ES8388_DACCONTROL18     0x28
#define ES8388_DACCONTROL19     0x29
#define ES8388_DACCONTROL20     0x2a
#define ES8388_DACCONTROL21     0x2b
#define ES8388_DACCONTROL22     0x2c
#define ES8388_DACCONTROL23     0x2d
#define ES8388_DACCONTROL24     0x2e
#define ES8388_DACCONTROL25     0x2f
#define ES8388_DACCONTROL26     0x30
#define ES8388_DACCONTROL27     0x31
#define ES8388_DACCONTROL28     0x32
#define ES8388_DACCONTROL29     0x33
#define ES8388_DACCONTROL30     0x34

/* ========================================================================== */
/* Driver Enums                                                               */
/* ========================================================================== */
typedef enum
{
    ES_MODE_NONE    = 0x00,
    ES_MODE_DAC     = 0x01,
    ES_MODE_ADC     = 0x02,
    ES_MODE_DAC_ADC = 0x03,
    ES_MODE_LINE    = 0x04
} es8388_mode_t;

/* ========================================================================== */
/* Bare-Metal Driver API                                                      */
/* ========================================================================== */

/**
 * @brief  Initializes the ES8388 codec over direct HAL I2C.
 * Automatically targets 16-bit, 44.1kHz Philips I2S audio profiles.
 * @param  hi2c: Handle to the configured STM32 I2C peripheral (e.g., &hi2c2)
 * @retval HAL_StatusTypeDef: Returns HAL_OK on success, or HAL_ERROR on bus failure.
 */
HAL_StatusTypeDef es8388_init(I2C_HandleTypeDef *hi2c);

/**
 * @brief  Starts the specified audio pathway (DAC, ADC, or Both)
 * @param  mode: The target operational mode selection.
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef es8388_start(es8388_mode_t mode);

/**
 * @brief  Power down or stop the specified audio pathway safely.
 * @param  mode: The pathway to disable.
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef es8388_stop(es8388_mode_t mode);

/**
 * @brief  Sets the speaker output volume.
 * @param  volume: Target percentage value from 0 (Muted) to 100 (Max Volume).
 */
void es8388_volume_set(uint8_t volume);

/**
 * @brief  Reads the current global volume step back from the driver state.
 * @retval Volume range from 0 to 100.
 */
uint8_t es8388_volume_get(void);

/**
 * @brief  Configures the ES8388 to route MIC inputs directly to Line Outputs.
 * This bypasses the digital I2S/DMA entirely for purely analog testing.
 */
HAL_StatusTypeDef es8388_enable_mic_bypass(void);
#endif /* __DRV_ES8388_H__ */
