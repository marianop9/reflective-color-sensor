/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"

#include "queue.h"
#include "stream_buffer.h"

#include "cs_usb_comms.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// default priority for tasks (matches the priority used by the default task)
#define CS_DEFAULT_PRIORITY (osPriorityNormal)
// USB StreamBuffer capacity. Data received via USB is copied into this buffer
#define SB_USB_BUFFER_CAPACITY 64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* USER CODE BEGIN PV */
TaskHandle_t task_handle_usb_receiver;
TaskHandle_t task_handle_usb_sender;

StreamBufferHandle_t sb_usb_recv_buffer;
QueueHandle_t q_usb_send_queue;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void usb_recv_ISR(uint8_t *buf, uint32_t len);

void task_usb_receiver(void *pargs);
void task_usb_sender(void *pargs);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void toggle_led()
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Init scheduler */
    osKernelInitialize();

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    q_usb_send_queue = xQueueCreate(2, sizeof(cs_response_msg));
    sb_usb_recv_buffer = xStreamBufferCreate(SB_USB_BUFFER_CAPACITY, 1);
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of defaultTask */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    BaseType_t created;
    created = xTaskCreate(task_usb_sender,
                          "senderTask",
                          configMINIMAL_STACK_SIZE * 2,
                          NULL,
                          CS_DEFAULT_PRIORITY,
                          &task_handle_usb_sender);

    configASSERT(pdPASS == created);

    created = xTaskCreate(task_usb_receiver,
                          "recvTask",
                          configMINIMAL_STACK_SIZE * 2,
                          NULL,
                          CS_DEFAULT_PRIORITY,
                          &task_handle_usb_receiver);

    configASSERT(pdPASS == created);
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void usb_recv_ISR(uint8_t *buf, uint32_t len)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xStreamBufferSendFromISR(sb_usb_recv_buffer,
                             buf,
                             len,
                             &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void task_usb_receiver(void *arg)
{
    size_t available_bytes = 0;
    size_t received_bytes = 0;
    cs_command_id cmd = CMD_ERR;
    for (;;)
    {
        available_bytes = cs_get_free_cmd_buffer_len();
        if (available_bytes == 0)
        {
            /* If the buffer is full (no free bytes), no bytes will be written in the next
            xStreamBufferReceive call. After this, no further updates will be triggered. 
            The only way this can happen is if the host sends garbage, or sends multiple commands without ending in a line-feed (\n). The buffer is cleared to avoid this. */
            cs_clear_cmd_buffer();
            available_bytes = cs_get_free_cmd_buffer_len();
        }

        received_bytes = xStreamBufferReceive(sb_usb_recv_buffer,
                                 (void *)cs_get_free_cmd_buffer(),
                                 available_bytes,
                                 portMAX_DELAY);

        if (received_bytes == 0)
        {
            continue;
        }

        cs_updated_cmd_buffer(received_bytes);

        if (!cs_check_for_command(&cmd))
        {
            continue;
        }

        cs_response_msg resp;
        switch (cmd)
        {
        case CMD_TOGGLE_LED:
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            resp.id = RESP_TEXT;
            strcpy(resp.payload.text, "OK\n");
            break;
        case CMD_PING:
            resp.id = RESP_TEXT;
            strcpy(resp.payload.text, "PONG\n");
            break;
        case CMD_MEM:
            UBaseType_t receiver_stack_free_words = uxTaskGetStackHighWaterMark(NULL);
            UBaseType_t sender_stack_free_words = uxTaskGetStackHighWaterMark(task_handle_usb_sender);

            resp.id = RESP_TEXT;
            snprintf(resp.payload.text,
                     sizeof(resp.payload.text),
                     "Rcvr:%3lu, Sndr:%3lu\n", receiver_stack_free_words, sender_stack_free_words);
            break;
        case CMD_ERR:
        default:
            resp.id = RESP_TEXT;
            strcpy(resp.payload.text, "unknown cmd\n");
            break;
        }

        xQueueSend(q_usb_send_queue, &resp, portMAX_DELAY);
    }
}

void task_usb_sender(void *arg)
{
    for (;;)
    {
        cs_response_msg resp = {0};

        if (pdPASS != xQueueReceive(q_usb_send_queue, &resp, portMAX_DELAY))
        {
            continue;
        }

        switch (resp.id)
        {
        default:
            // use text buffer by default
            size_t len = strnlen(resp.payload.text, sizeof(resp.payload.text));
            CDC_Transmit_FS((uint8_t *)resp.payload.text, len);
            break;
        }
    }
}

// FreeRTOS hooks/assert hanndler
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

void vAssertCalled(const char *file, int line)
{
    (void)file;
    (void)line;

    taskDISABLE_INTERRUPTS();
    __BKPT(0); // breakpoint. Lets gdb inspect file and line
    for (;;)
    {
    }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN 5 */
    // UBaseType_t shwm;
    /* Infinite loop */
    for (;;)
    {
        osDelay(pdMS_TO_TICKS(1000));
        // shwm = uxTaskGetStackHighWaterMark(NULL);
        // (void) shwm;
    }
    /* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM2 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM2)
    {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
