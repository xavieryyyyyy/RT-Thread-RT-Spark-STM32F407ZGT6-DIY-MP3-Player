
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : DIY MP3 Player - BCA143 Final Exam
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* USER CODE BEGIN Includes */
#include "drv_es8388.h"
#include <math.h>
#include "drv_lcd.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
#define AUDIO_BUFFER_SIZE  512

#define NOTE_C3  130.81f
#define NOTE_CS3 138.59f
#define NOTE_D3  146.83f
#define NOTE_DS3 155.56f
#define NOTE_E3  164.81f
#define NOTE_F3  174.61f
#define NOTE_FS3 185.00f
#define NOTE_G3  196.00f
#define NOTE_GS3 207.65f
#define NOTE_A3  220.00f
#define NOTE_AS3 233.08f
#define NOTE_B3  246.94f

#define NOTE_C4  261.63f
#define NOTE_CS4 277.18f
#define NOTE_D4  293.66f
#define NOTE_DS4 311.13f
#define NOTE_E4  329.63f
#define NOTE_F4  349.23f
#define NOTE_FS4 369.99f
#define NOTE_G4  392.00f
#define NOTE_GS4 415.30f
#define NOTE_A4  440.00f
#define NOTE_AS4 466.16f
#define NOTE_B4  493.88f

#define NOTE_C5  523.25f
#define NOTE_CS5 554.37f
#define NOTE_D5  587.33f
#define NOTE_DS5 622.25f
#define NOTE_E5  659.25f
#define NOTE_F5  698.46f
#define NOTE_FS5 739.99f
#define NOTE_G5  783.99f
#define NOTE_GS5 830.61f
#define NOTE_A5  880.00f
#define NOTE_AS5 932.33f
#define NOTE_B5  987.77f

#define REST     0.0f

typedef struct {
    float note;
    float duration;
} MusicalNote;

typedef struct {
    const char* name;
    const MusicalNote* notes;
    uint16_t length;
    uint16_t bpm;
} Song;
/* USER CODE END PD */

/* USER CODE BEGIN PM */
/* USER CODE END PM */

I2C_HandleTypeDef hi2c2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;
SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
uint16_t debug_audio_buffer[AUDIO_BUFFER_SIZE];
void Switch_Song(const Song* new_song);

const MusicalNote bohemian_track[] = {
    {NOTE_B4,1.0f},{NOTE_A4,1.0f},{NOTE_G4,1.0f},{NOTE_A4,1.0f},
    {NOTE_B4,2.0f},{NOTE_B4,1.0f},{NOTE_A4,1.0f},
    {NOTE_G4,1.0f},{NOTE_A4,1.0f},{NOTE_B4,2.0f},
    {NOTE_E4,1.0f},{NOTE_G4,1.0f},{NOTE_A4,1.0f},{NOTE_B4,2.0f},
    {REST,2.0f}
};
const Song song_bohemian = {
    .name = "Bohemian Rhapsody",
    .notes = bohemian_track,
    .length = sizeof(bohemian_track)/sizeof(bohemian_track[0]),
    .bpm = 72
};

const MusicalNote tiger_track[] = {
    {NOTE_G3,0.5f},{NOTE_G3,0.5f},{NOTE_G3,0.5f},{NOTE_E3,0.75f},{NOTE_G3,0.25f},
    {NOTE_A3,2.0f},{REST,0.5f},
    {NOTE_G3,0.5f},{NOTE_G3,0.5f},{NOTE_E3,0.75f},{NOTE_G3,0.25f},
    {NOTE_AS3,2.0f},{REST,0.5f},
    {NOTE_G3,0.5f},{NOTE_G3,0.5f},{NOTE_E3,0.75f},{NOTE_G3,0.25f},
    {NOTE_A3,1.0f},{NOTE_G3,1.0f},{NOTE_E3,2.0f},{REST,1.0f}
};
const Song song_tiger = {
    .name = "Eye Of The Tiger",
    .notes = tiger_track,
    .length = sizeof(tiger_track)/sizeof(tiger_track[0]),
    .bpm = 108
};

const MusicalNote interstellar_track[] = {
    {NOTE_E4,0.5f},{NOTE_A4,0.5f},{NOTE_B4,1.0f},
    {NOTE_E4,0.5f},{NOTE_A4,0.5f},{NOTE_C5,1.0f},
    {NOTE_E4,0.5f},{NOTE_A4,0.5f},{NOTE_D5,1.0f},
    {NOTE_E4,0.5f},{NOTE_A4,0.5f},{NOTE_B4,2.0f},
    {NOTE_E4,0.5f},{NOTE_A4,0.5f},{NOTE_E5,1.0f},
    {NOTE_D5,2.0f},{NOTE_C5,1.0f},{NOTE_B4,1.0f},
    {NOTE_A4,2.0f},{REST,1.0f}
};
const Song song_interstellar = {
    .name = "Interstellar",
    .notes = interstellar_track,
    .length = sizeof(interstellar_track)/sizeof(interstellar_track[0]),
    .bpm = 96
};

const MusicalNote sweden_track[] = {
    {NOTE_D4,2.0f},{NOTE_E4,2.0f},{NOTE_G4,4.0f},
    {NOTE_D4,2.0f},{NOTE_E4,2.0f},{NOTE_G4,2.0f},{NOTE_A4,2.0f},
    {NOTE_B4,2.0f},{NOTE_A4,2.0f},{NOTE_G4,4.0f},
    {NOTE_D4,2.0f},{NOTE_E4,2.0f},{NOTE_G4,4.0f},{REST,4.0f}
};
const Song song_sweden = {
    .name = "Minecraft Sweden",
    .notes = sweden_track,
    .length = sizeof(sweden_track)/sizeof(sweden_track[0]),
    .bpm = 75
};

const MusicalNote nokia_track[] = {
    {NOTE_E5,0.5f},{NOTE_D5,0.5f},{NOTE_FS4,1.0f},{NOTE_GS4,1.0f},
    {NOTE_CS5,0.5f},{NOTE_B4,0.5f},{NOTE_D4,1.0f},{NOTE_E4,1.0f},
    {NOTE_B4,0.5f},{NOTE_A4,0.5f},{NOTE_CS4,1.0f},{NOTE_E4,1.0f},
    {NOTE_A4,2.0f},{REST,2.0f}
};
const Song song_nokia = {
    .name = "Nokia Ringtone",
    .notes = nokia_track,
    .length = sizeof(nokia_track)/sizeof(nokia_track[0]),
    .bpm = 180
};

const MusicalNote fur_elise_track[] = {
    {NOTE_E5,0.5f},{NOTE_DS5,0.5f},{NOTE_E5,0.5f},{NOTE_DS5,0.5f},
    {NOTE_E5,0.5f},{NOTE_B4,0.5f},{NOTE_D5,0.5f},{NOTE_C5,0.5f},
    {NOTE_A4,2.0f},{REST,0.5f},
    {NOTE_C4,0.5f},{NOTE_E4,0.5f},{NOTE_A4,0.5f},
    {NOTE_B4,2.0f},{REST,0.5f},
    {NOTE_E4,0.5f},{NOTE_GS4,0.5f},{NOTE_B4,0.5f},
    {NOTE_C5,2.0f},{REST,1.0f}
};
const Song song_fur_elise = {
    .name = "Fur Elise",
    .notes = fur_elise_track,
    .length = sizeof(fur_elise_track)/sizeof(fur_elise_track[0]),
    .bpm = 120
};

const MusicalNote happy_bday_track[] = {
    {NOTE_C4,0.75f},{NOTE_C4,0.25f},{NOTE_D4,1.0f},{NOTE_C4,1.0f},
    {NOTE_F4,1.0f},{NOTE_E4,2.0f},{REST,0.5f},
    {NOTE_C4,0.75f},{NOTE_C4,0.25f},{NOTE_D4,1.0f},{NOTE_C4,1.0f},
    {NOTE_G4,1.0f},{NOTE_F4,2.0f},{REST,0.5f},
    {NOTE_C4,0.75f},{NOTE_C4,0.25f},{NOTE_C5,1.0f},{NOTE_A4,1.0f},
    {NOTE_F4,1.0f},{NOTE_E4,1.0f},{NOTE_D4,2.0f},{REST,0.5f},
    {NOTE_AS4,0.75f},{NOTE_AS4,0.25f},{NOTE_A4,1.0f},{NOTE_F4,1.0f},
    {NOTE_G4,1.0f},{NOTE_F4,2.0f}
};
const Song song_happy_bday = {
    .name = "Happy Birthday",
    .notes = happy_bday_track,
    .length = sizeof(happy_bday_track)/sizeof(happy_bday_track[0]),
    .bpm = 100
};

const MusicalNote ode_to_joy_track[] = {
    {NOTE_E4,1.0f},{NOTE_E4,1.0f},{NOTE_F4,1.0f},{NOTE_G4,1.0f},
    {NOTE_G4,1.0f},{NOTE_F4,1.0f},{NOTE_E4,1.0f},{NOTE_D4,1.0f},
    {NOTE_C4,1.0f},{NOTE_C4,1.0f},{NOTE_D4,1.0f},{NOTE_E4,1.0f},
    {NOTE_E4,1.5f},{NOTE_D4,0.5f},{NOTE_D4,2.0f},{REST,1.0f}
};
const Song song_ode_to_joy = {
    .name = "Ode To Joy",
    .notes = ode_to_joy_track,
    .length = sizeof(ode_to_joy_track)/sizeof(ode_to_joy_track[0]),
    .bpm = 120
};

const Song* const playlist[] = {
    &song_bohemian,
    &song_tiger,
    &song_interstellar,
    &song_sweden,
    &song_nokia,
    &song_fur_elise,
    &song_happy_bday,
    &song_ode_to_joy
};
#define PLAYLIST_SIZE (sizeof(playlist)/sizeof(playlist[0]))

const Song* volatile current_song = &song_bohemian;
volatile int8_t  current_song_idx = 0;
volatile uint8_t is_paused = 0;
uint32_t current_note_idx = 0;
uint32_t samples_elapsed = 0;
volatile float current_freq = 0.0f;
float current_phase = 0.0f;
volatile uint8_t updateLCD = 0;
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2S3_Init(void);
static void MX_FSMC_Init(void);

/* USER CODE BEGIN 0 */
void Switch_Song(const Song* new_song)
{
    __disable_irq();
    current_song = new_song;
    current_note_idx = 0;
    samples_elapsed = 0;
    current_freq = new_song->notes[0].note;
    __enable_irq();
}

void Fill_Audio_Buffer(uint16_t offset, uint16_t length)
{
    float samples_per_beat = (44100.0f * 60.0f) / (float)current_song->bpm;
    uint32_t gap_samples = 1323;
    float phase_inc = (2.0f * 3.1415926535f * current_freq) / 44100.0f;

    for (uint16_t i = 0; i < length; i += 2)
    {
        if (is_paused) {
            debug_audio_buffer[offset + i]     = 0;
            debug_audio_buffer[offset + i + 1] = 0;
            continue;
        }

        samples_elapsed++;
        MusicalNote active_note = current_song->notes[current_note_idx];
        uint32_t target_duration = (uint32_t)(active_note.duration * samples_per_beat);

        if (samples_elapsed >= (target_duration - gap_samples)) {
            current_freq = REST;
            phase_inc = 0.0f;
        }

        if (samples_elapsed >= target_duration) {
            samples_elapsed = 0;
            current_note_idx++;
            if (current_note_idx >= current_song->length) current_note_idx = 0;
            current_freq = current_song->notes[current_note_idx].note;
            phase_inc = (2.0f * 3.1415926535f * current_freq) / 44100.0f;
        }

        int16_t sample = 0;
        if (current_freq > 0.0f) {
            sample = (current_phase < 3.1415926535f) ? 4096 : -4096;
            current_phase += phase_inc;
            if (current_phase >= 2.0f * 3.1415926535f)
                current_phase -= 2.0f * 3.1415926535f;
        } else {
            current_phase = 0.0f;
        }

        debug_audio_buffer[offset + i]     = (uint16_t)sample;
        debug_audio_buffer[offset + i + 1] = (uint16_t)sample;
    }
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
    Fill_Audio_Buffer(0, AUDIO_BUFFER_SIZE / 2);
}
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
    Fill_Audio_Buffer(AUDIO_BUFFER_SIZE / 2, AUDIO_BUFFER_SIZE / 2);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_tick_up    = 0;
    static uint32_t last_tick_left  = 0;
    static uint32_t last_tick_right = 0;
    uint32_t now = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_5) {
        // UP button = Play/Pause
        if ((now - last_tick_up) < 200) return;   // debounce 200 ms
        last_tick_up = now;
        is_paused = !is_paused;
        updateLCD = 1;
    }
    else if (GPIO_Pin == GPIO_PIN_0) {
        // LEFT button = Previous Track
        if ((now - last_tick_left) < 200) return;  // debounce 200 ms
        last_tick_left = now;
        current_song_idx--;
        if (current_song_idx < 0) current_song_idx = PLAYLIST_SIZE - 1;
        Switch_Song(playlist[current_song_idx]);
        is_paused = 0;
        updateLCD = 1;
    }
    else if (GPIO_Pin == GPIO_PIN_4) {
        // RIGHT button = Next Track
        if ((now - last_tick_right) < 200) return; // debounce 200 ms
        last_tick_right = now;
        current_song_idx++;
        if (current_song_idx >= PLAYLIST_SIZE) current_song_idx = 0;
        Switch_Song(playlist[current_song_idx]);
        is_paused = 0;
        updateLCD = 1;
    }
}

void Update_LCD_Display(void)
{
    lcd_clear(BLACK);

    /* ── Header ─────────────────────────── */
    lcd_set_color(BLACK, CYAN);
    lcd_show_string(0, 0, 16, "~~~~~~~~~~~~~~~~");
    lcd_set_color(BLACK, WHITE);
    lcd_show_string(28, 16, 16, "DIY MP3 PLAYER");
    lcd_set_color(BLACK, CYAN);
    lcd_show_string(0, 32, 16, "~~~~~~~~~~~~~~~~");

    /* ── Status ─────────────────────────── */
    if (is_paused) {
        lcd_set_color(BLACK, RED);
        lcd_show_string(65, 50, 24, "PAUSED");
    } else {
        lcd_set_color(BLACK, GREEN);
        lcd_show_string(52, 50, 24, "PLAYING");
    }

    /* ── Track info ──────────────────────── */
    char track_num[20];
    snprintf(track_num, 20, "Track  %d  of  %d",
             current_song_idx + 1, (int)PLAYLIST_SIZE);
    lcd_set_color(BLACK, WHITE);
    lcd_show_string(4, 82, 16, track_num);

    lcd_set_color(BLACK, GRAY);
    lcd_show_string(0, 98, 16, "- - - - - - - - ");

    /* ── Song name (auto split) ──────────── */
    char name_buf[32];
    strncpy(name_buf, current_song->name, 31);
    name_buf[31] = '\0';

    int len = strlen(name_buf);
    lcd_set_color(BLACK, YELLOW);

    if (len <= 13) {
        /* center single line */
        int x = (240 - len * 12) / 2;
        if (x < 0) x = 0;
        lcd_show_string(x, 112, 24, name_buf);
    } else {
        /* split at last space before char 13 */
        char line1[17] = {0};
        char line2[17] = {0};
        int split = 12;
        for (int i = 12; i >= 0; i--) {
            if (name_buf[i] == ' ') { split = i; break; }
        }
        strncpy(line1, name_buf, split);
        strncpy(line2, name_buf + split + 1, 16);
        int x1 = (240 - (int)strlen(line1) * 12) / 2;
        int x2 = (240 - (int)strlen(line2) * 12) / 2;
        if (x1 < 0) x1 = 0;
        if (x2 < 0) x2 = 0;
        lcd_show_string(x1, 108, 24, line1);
        lcd_show_string(x2, 136, 24, line2);
    }

    /* ── Divider ─────────────────────────── */
    lcd_set_color(BLACK, GRAY);
    lcd_show_string(0, 166, 16, "~~~~~~~~~~~~~~~~");

    /* ── Button guide ────────────────────── */
    lcd_set_color(BLACK, CYAN);
    lcd_show_string(4, 182, 16, "UP  = Play / Pause");
    lcd_show_string(4, 198, 16, "LFT = Prev  Track");
    lcd_show_string(4, 214, 16, "RGT = Next  Track");
}
/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  MX_I2S3_Init();
  MX_FSMC_Init();

  /* USER CODE BEGIN 2 */
  drv_lcd_init();

  /* Splash screen */
  lcd_clear(BLACK);
  lcd_set_color(BLACK, CYAN);
  lcd_show_string(0,  70, 16, "~~~~~~~~~~~~~~~~");
  lcd_set_color(BLACK, WHITE);
  lcd_show_string(28, 88, 16, "DIY MP3 PLAYER");
  lcd_set_color(BLACK, CYAN);
  lcd_show_string(0, 104, 16, "~~~~~~~~~~~~~~~~");
  lcd_set_color(BLACK, YELLOW);
  lcd_show_string(48, 128, 16, "BCA143  Final");
  lcd_set_color(BLACK, WHITE);
  lcd_show_string(40, 150, 16, "Loading . . .");
  HAL_Delay(1500);

  es8388_init(&hi2c2);
  es8388_start(ES_MODE_DAC_ADC);
  es8388_volume_set(100);

  hdma_spi3_tx.Init.Mode        = DMA_CIRCULAR;
  hdma_spi3_tx.Init.FIFOMode    = DMA_FIFOMODE_DISABLE;
  hdma_spi3_tx.Init.MemBurst    = DMA_MBURST_SINGLE;
  hdma_spi3_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma_spi3_tx) != HAL_OK) Error_Handler();

  current_note_idx = 0;
  samples_elapsed  = 0;
  current_freq     = current_song->notes[0].note;

  HAL_I2S_Transmit_DMA(&hi2s3, debug_audio_buffer, AUDIO_BUFFER_SIZE);

  Update_LCD_Display();
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */
  /* USER CODE BEGIN 3 */
      if (updateLCD) {
          Update_LCD_Display();
          updateLCD = 0;
      }
  }
  /* USER CODE END 3 */
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PLL_PLLM_CONFIG(10);
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void MX_I2C2_Init(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK) Error_Handler();
}

static void MX_I2S3_Init(void)
{
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK) Error_Handler();
}

static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9|GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

static void MX_FSMC_Init(void)
{
  FSMC_NORSRAM_TimingTypeDef Timing = {0};
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK3;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK) Error_Handler();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
