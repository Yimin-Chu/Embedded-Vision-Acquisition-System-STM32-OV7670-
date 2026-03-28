/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"

#include "config.h"
#include "ov7670.h"

#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart3;
extern DCMI_HandleTypeDef hdcmi;

/* Snapshot buffer: IMG_ROWS x IMG_COLS pixels, each 16-bit (YCbCr) */
uint16_t snapshot_buff[IMG_ROWS * IMG_COLS];

/* Set to 1 by the DMA ISR when one full frame has been transferred.
   Volatile cast is used when reading to prevent compiler register caching. */
uint8_t dma_flag = 0;

void print_buf(void);


int main(void)
{
  /* Reset of all peripherals */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_I2C2_Init();
  MX_TIM1_Init();
  MX_TIM6_Init();

  /* Provide the 8 MHz XCLK to the camera via TIM1 CH1 PWM */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  /* -------------------------------------------------------------------
   * Section 2.1 - Serial port test
   * Send a synthetic black frame then a white frame to verify the serial
   * link and serial_monitor are working before using the real camera.
   * ------------------------------------------------------------------- */

  /* Black frame */
  print_msg("\r\nPREAMBLE!\r\n");
  for (int i = 0; i < IMG_ROWS * IMG_COLS; i++) {
    uint8_t px = 0x00;
    HAL_UART_Transmit(&huart3, &px, 1, 10000);
  }
  HAL_Delay(500);

  /* White frame */
  print_msg("\r\nPREAMBLE!\r\n");
  for (int i = 0; i < IMG_ROWS * IMG_COLS; i++) {
    uint8_t px = 0xFF;
    HAL_UART_Transmit(&huart3, &px, 1, 10000);
  }
  HAL_Delay(500);

  /* -------------------------------------------------------------------
   * Section 3 - Camera initialisation
   * ------------------------------------------------------------------- */
  ov7670_init();
  //while (1)
  //{
     // if (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin)) {
      /*HAL_Delay(100);  /* debounce */

     // print_msg("Snap!\r\n");

      /* Start DMA snapshot capture */
     // dma_flag = 0;
     // ov7670_snapshot(snapshot_buff);

      /* Wait for DMA to finish.
         HAL_Delay(1) is required - without it the DMA interrupt
         can silently fail to fire (known STM HAL quirk). */
     // while (!*(volatile uint8_t *)&dma_flag) {
      //  HAL_Delay(1);
     // }

      /* Send the captured frame through serial port */
     // print_buf();
    //}
  ///}

  /* -------------------------------------------------------------------
   * Section 4 - Continuous video stream
   *
   * Tearing prevention strategy (Section 4.3):
   *   1. Start DMA once in CONTINUOUS mode.
   *   2. Each time dma_flag is set:
   *        a. Suspend DCMI - camera stops overwriting the buffer.
   *        b. Transmit the completed frame over serial.
   *        c. Resume DCMI - next frame starts capturing immediately.
   *        d. Clear dma_flag and loop back to waiting.
   *
   * NOTE: HAL_Delay(1) inside the wait loop is mandatory.  Without it
   * the DMA interrupt can silently fail to fire (known STM HAL quirk).
   * ------------------------------------------------------------------- */

    dma_flag = 0;
    ov7670_capture(snapshot_buff);   /* start continuous DMA capture */

    while (1)
    {
      /* Wait for DMA to signal one complete frame is ready */
      while (!*(volatile uint8_t *)&dma_flag) {
        HAL_Delay(1);
      }

      /* Suspend DCMI to lock the buffer while we transmit */
      HAL_DCMI_Suspend(&hdcmi);

      /* Send the grayscale frame through serial port */
      print_buf();

      /* Resume DCMI so the next frame starts filling the buffer */
      HAL_DCMI_Resume(&hdcmi);

      /* Clear flag - ready for next frame */
      dma_flag = 0;
    }
}


/* ---------------------------------------------------------------------------
 * print_buf
 *
 * Transmit one grayscale frame through USART3:
 *
 *   "\r\nPREAMBLE!\r\n"         -- sync header expected by serial_monitor
 *   IMG_ROWS * IMG_COLS bytes    -- one Y (luma) byte per pixel
 *
 * Camera YCbCr memory layout (viewed as uint8_t*):
 *   index 0: Cb0  index 1: Y0  index 2: Cr0  index 3: Y1  ...
 *
 * Y is always at ODD byte indices.
 * Sending EVEN bytes (Cb/Cr) gives a nearly-grey image -- that is wrong.
 * --------------------------------------------------------------------------- */
void print_buf(void) {
  const uint8_t *raw = (const uint8_t *)snapshot_buff;

  print_msg("\r\nPREAMBLE!\r\n");

  for (int i = 0; i < IMG_ROWS * IMG_COLS; i++) {
    uint8_t y = raw[2 * i + 1];    /* odd byte index = Y (luma) */
    HAL_UART_Transmit(&huart3, &y, 1, 10000);
  }
}