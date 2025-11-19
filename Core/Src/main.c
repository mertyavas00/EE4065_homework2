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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "image.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t my_histogram[256]; // Histogram sonuçlarını tutacak dizi
uint32_t equalized_histogram[256];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Calculate_Histogram(const uint8_t* image, uint32_t size, uint32_t* histogram) {
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }
    for (uint32_t i = 0; i < size; i++) {
        uint8_t pixel_val = image[i];
        histogram[pixel_val]++;
    }
}

void Histogram_Equalization(uint8_t* image, uint32_t size) {
    uint32_t histogram[256];
    uint32_t cdf[256];
    uint32_t map[256];
    for(int i=0; i<256; i++) histogram[i] = 0;

    for(uint32_t i=0; i<size; i++) {
        histogram[image[i]]++;
    }
    cdf[0] = histogram[0];
    for(int i=1; i<256; i++) {
        cdf[i] = cdf[i-1] + histogram[i];
    }

    for(int i=0; i<256; i++) {
        float val = ((float)cdf[i] / (float)size) * 255.0f;
        map[i] = (uint8_t)val;
    }

    for(uint32_t i=0; i<size; i++) {
        uint8_t old_pixel = image[i];
        image[i] = map[old_pixel];
    }
}

uint8_t output_image[IMG_SIZE];

void Apply_Convolution(const uint8_t* input, uint8_t* output, int width, int height, const int8_t* kernel, int kernel_sum, int offset) {

    int k_size = 3;
    int half_k = k_size / 2;

    for (int y = half_k; y < height - half_k; y++) {
        for (int x = half_k; x < width - half_k; x++) {

            int32_t sum = 0;

            for (int ky = -half_k; ky <= half_k; ky++) {
                for (int kx = -half_k; kx <= half_k; kx++) {
                    uint8_t pixel = input[(y + ky) * width + (x + kx)];
                    int8_t k_val = kernel[(ky + half_k) * 3 + (kx + half_k)];

                    sum += pixel * k_val;
                }
            }
            if (kernel_sum != 0) {
                sum /= kernel_sum;
            }

            sum += offset;
            if (sum < 0) sum = 0;
            if (sum > 255) sum = 255;

            output[y * width + x] = (uint8_t)sum;
        }
    }
}

void Bubble_Sort(uint8_t* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint8_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void Apply_Median_Filter(const uint8_t* input, uint8_t* output, int width, int height) {
    int k_size = 3;
    int half_k = k_size / 2;
    uint8_t window[9];

    for (int y = half_k; y < height - half_k; y++) {
        for (int x = half_k; x < width - half_k; x++) {

            int count = 0;
            for (int ky = -half_k; ky <= half_k; ky++) {
                for (int kx = -half_k; kx <= half_k; kx++) {
                    window[count++] = input[(y + ky) * width + (x + kx)];
                }
            }
            Bubble_Sort(window, 9);

            output[y * width + x] = window[4];
        }
    }
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
Calculate_Histogram(raw_image, IMG_SIZE, my_histogram);

Histogram_Equalization((uint8_t*)raw_image, IMG_SIZE);

Calculate_Histogram(raw_image, IMG_SIZE, equalized_histogram);

const int8_t low_pass_kernel[9] = {
      1, 1, 1,
      1, 1, 1,
      1, 1, 1
  };

Apply_Convolution(raw_image, output_image, IMG_WIDTH, IMG_HEIGHT, low_pass_kernel, 9, 0);

const int8_t high_pass_kernel[9] = {
      -1, -1, -1,
      -1,  8, -1,
      -1, -1, -1
  };

Apply_Convolution(raw_image, output_image, IMG_WIDTH, IMG_HEIGHT, high_pass_kernel, 1, 0);

Apply_Median_Filter(raw_image, output_image, IMG_WIDTH, IMG_HEIGHT);
/* USER CODE END 2 */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
