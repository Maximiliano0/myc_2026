/* Includes ------------------------------*/
#include "my_lib.h"

/* Macros --------------------------------*/

/* Global Variable -----------------------*/
TIM_HandleTypeDef hbasetim; // TIM3 Handler Variable
TIM_HandleTypeDef hpwm; // TIM2 Handler Variable
TIM_HandleTypeDef hictim; // TIM4 Handler Variable
volatile uint8_t debounce = 0, sampling = 0;
volatile IC_State ic_state = End_Capture;
volatile uint16_t rval = 0, fval = 0, duration = 0;

/* Public Function Definitions ------------*/

/* Hardware Configuration */
uint8_t Hw_Init(void){

	/* HAL Initialization	*/
	HAL_Init(); // --> Init Software

	/* CLK Configuration */
	SystemClock_Config(); // --> Init Hardware

	/* TIM4 Configuration (Input Capture) */
	TIM4_Config();

	/* TIM3 Configuration (Base Timer) */
	TIM3_Config();

	/* TIM2 Configuration (PWM Timer) */
	TIM2_Config();

	/* GPIO Configuration */
	GPIO_Config(); // --> Init Hardware

	return(1);
}


/* CLK Configuration */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  return;
}

/* GPIO Configuration */
void GPIO_Config(void){

	GPIO_InitTypeDef LED_Pin_Conf = {0};
	GPIO_InitTypeDef SWITCH_Pin_Conf = {0};
	GPIO_InitTypeDef PWM_Pin_Conf = {0};
	GPIO_InitTypeDef TRIG_Pin_Conf = {0}, ECHO_Pin_Conf = {0};

	/* GPIO Port Hardware Enable */
	// --> Hardware
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* LED Pin Features Load */
	// --> Software
	LED_Pin_Conf.Pin = LED_PIN;
	LED_Pin_Conf.Mode = GPIO_MODE_OUTPUT_PP;
	LED_Pin_Conf.Pull = GPIO_NOPULL;
	LED_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;

	/* LED Pin Hardware Configuration */
	// --> Hardware
	HAL_GPIO_Init(LED_PORT, &LED_Pin_Conf);

	/* SWITCH Pin Features Load */
	// --> Software
	SWITCH_Pin_Conf.Pin = SWITCH_PIN;
	SWITCH_Pin_Conf.Mode = GPIO_MODE_INPUT;
	SWITCH_Pin_Conf.Pull = GPIO_NOPULL;
	SWITCH_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;

	/* SWITCH Pin Hardware Configuration */
	// --> Hardware
	HAL_GPIO_Init(SWITCH_PORT, &SWITCH_Pin_Conf);

	/* PWM Pin Features Load */
	// --> Software
	PWM_Pin_Conf.Pin = PWM_PIN;
	PWM_Pin_Conf.Mode = GPIO_MODE_AF_PP;
	PWM_Pin_Conf.Pull = GPIO_NOPULL;
	PWM_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;
	PWM_Pin_Conf.Alternate = GPIO_AF1_TIM2;

	/* PWM Pin Hardware Configuration */
	// --> Hardware
	HAL_GPIO_Init(PWM_PORT, &PWM_Pin_Conf);

	/* HC-SR04 Echo Features Load */
	// --> Software
	ECHO_Pin_Conf.Pin = ECHO_PIN;
	ECHO_Pin_Conf.Mode = GPIO_MODE_AF_PP;
	ECHO_Pin_Conf.Pull = GPIO_NOPULL;
	ECHO_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;
	ECHO_Pin_Conf.Alternate = GPIO_AF2_TIM4;

	/* HC-SR04 Echo */
	// --> Hardware
    HAL_GPIO_Init(ECHO_PORT, &ECHO_Pin_Conf);

	/* HC-SR04 Trigger Features Load */
	// --> Software
	TRIG_Pin_Conf.Pin = TRG_PIN;
	TRIG_Pin_Conf.Mode = GPIO_MODE_OUTPUT_PP;
	TRIG_Pin_Conf.Pull = GPIO_NOPULL;
	TRIG_Pin_Conf.Speed = GPIO_SPEED_FREQ_LOW;

	/* HC-SR04 Trigger */
	// --> Hardware
	HAL_GPIO_Init(TRG_PORT, &TRIG_Pin_Conf);

	// Initial State LOW
	HAL_GPIO_WritePin(TRG_PORT, TRG_PIN, GPIO_PIN_RESET);

	return;
}

/* TIM3 Configuration */
void TIM3_Config(void){

	TIM_ClockConfigTypeDef CLK_Source = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/*  TIM Features Load */
	// --> Software
	hbasetim.Instance = BASE_TIMER;
	hbasetim.Init.Prescaler = BASE_PRESCALER-1;
	hbasetim.Init.CounterMode = TIM_COUNTERMODE_UP;
	hbasetim.Init.Period = _BasePeriod(1); // 1 [ms]
	hbasetim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hbasetim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	/* TIM Hardware Configuration */
	// --> Hardware
	HAL_TIM_Base_Init(&hbasetim);

	/* CLK Source Selection */
	CLK_Source.ClockSource = TIM_CLOCKSOURCE_INTERNAL; // --> Software
	HAL_TIM_ConfigClockSource(&hbasetim, &CLK_Source); // --> Hardware

	/* TIM OVF Interrupt Config */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // --> Software
	HAL_TIMEx_MasterConfigSynchronization(&hbasetim, &sMasterConfig); // --> Hardware

	/* TIMER RUN (Interruption)	*/
	HAL_TIM_Base_Start_IT(&hbasetim); // --> Hardware

	return;
}

/* TIM2 Configuration */
void TIM2_Config(void){

	TIM_ClockConfigTypeDef CLK_Source = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef PWM_Config = {0};

	/*  TIM Features Load */
	// --> Software
	hpwm.Instance = PWM_TIMER;
	hpwm.Init.Prescaler = PWM_PRESCALER-1;
	hpwm.Init.CounterMode = TIM_COUNTERMODE_UP;
	hpwm.Init.Period = _PWMPeriod(100); // 100 [ms]
	hpwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hpwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	/* TIM Initialize */
	HAL_TIM_Base_Init(&hpwm); // --> Software

	/* CLK Source Selection */
	CLK_Source.ClockSource = TIM_CLOCKSOURCE_INTERNAL; // --> Software
	HAL_TIM_ConfigClockSource(&hpwm, &CLK_Source); // --> Hardware

	/* PWM Initialize */
	HAL_TIM_PWM_Init(&hpwm); // --> Software

	/* TIM OVF Interrupt Config */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // --> Software
	HAL_TIMEx_MasterConfigSynchronization(&hpwm, &sMasterConfig); // --> Hardware

	/* PWM Features Load */
	// --> Software
	PWM_Config.OCMode = TIM_OCMODE_PWM1;
	PWM_Config.Pulse = _PWMDutty(30, _PWMPeriod(100)); // 30%
	PWM_Config.OCPolarity = TIM_OCPOLARITY_HIGH;
	PWM_Config.OCFastMode = TIM_OCFAST_DISABLE;

	/* PWM Hardware Configuration */
	// --> Software
	HAL_TIM_PWM_ConfigChannel(&hpwm, &PWM_Config, TIM_CHANNEL_1);

	return;
}

/* TIM4 Configuration */
void TIM4_Config(void){

	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_IC_InitTypeDef sConfigIC = {0};

	/*  TIM Features Load */
	// --> Software
	hictim.Instance = IC_TIMER;
	hictim.Init.Prescaler = IC_PRESCALER-1;
	hictim.Init.CounterMode = TIM_COUNTERMODE_UP;
	hictim.Init.Period = 0xFFFF; // Free-running 16-bit @1us/tick (~65.5 ms range)
	hictim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	hictim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	/* TIM Initialize */
	HAL_TIM_Base_Init(&hictim); // --> Software

	/* TIM OVF Interrupt Config */
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // --> Software
	HAL_TIMEx_MasterConfigSynchronization(&hictim, &sMasterConfig); // --> Hardware

	/* IC Features Load */
	//--> Software
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;

	/* IC Hardware Configuration */
	// --> Software
	HAL_TIM_IC_ConfigChannel(&hictim, &sConfigIC, TIM_CHANNEL_1);

	/* IC Start with Interrupt */
	// --> Hardware
	HAL_TIM_IC_Start_IT(&hictim, TIM_CHANNEL_1);

	return;
}

/* Private Function Definitions ------------*/

/* Filtered Read Pin */
Click_State  CLICK_Detect(GPIO_TypeDef* GPIO_PORT, uint16_t GPIO_PIN){

	Button_State button_state = Non_Pressed;
	static uint8_t state = 0;

	/* Click Detection FSM */
	switch(state){
		case 0: // Non Pressed
			button_state = (HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN)==SWITCH_ON)?	Pressed:Non_Pressed;
			if(button_state == Pressed){
				debounce = DeBounce_Delay;
				state = 1;
			}
			break;
		case 1: // Pressed - esperando que termine el filtro anti-rebote
			if(debounce == 0){ // Termino el Delay
				button_state = (HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN)==SWITCH_ON)?	Pressed:Non_Pressed;
				if(button_state == Pressed)	state = 2; // Confirmado: esperar release
				else state = 0; // Era rebote/glitch: descartar
			}
			break;
		case 2: // Confirmado presionado - esperando que el usuario suelte
			button_state = (HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN)==SWITCH_ON)?	Pressed:Non_Pressed;
			if(button_state == Non_Pressed){ // Levantaron el dedo
				state = 0;
				return(Clicked);
			}
			break;
		default:
			state = 0;
			break;
	}

	return(Non_Clicked);
}

/* TIM3 IRQ Definition */
void TIM3_IRQHandler(void)
{

  /* DEBOUNCE */
  if(debounce > 0) debounce--;

	/* SAMPLING */
  if(sampling > 0) sampling--;
  else sampling = Sampling_Period;

  /* TIM2 IT CATCH */
  HAL_TIM_IRQHandler(&hbasetim); // --> Hardware

  return;
}

void TIM4_IRQHandler(void)
{	
  	/* TIM4 IT CATCH */
  	HAL_TIM_IRQHandler(&hictim);

	// NOT WELL DEFINED FSM
	// ic_state IS CHANGED FROM OUTSIDE
	switch(ic_state){
		case Waiting_REdge:
			// Read Actual State of TIM4 counts
			// After Rising Edge on TIM4 -> CHANNEL_1
			rval = HAL_TIM_ReadCapturedValue(&hictim, TIM_CHANNEL_1);
			ic_state = Waiting_FEdge;
			break;

		case Waiting_FEdge:
			// Read Actual State of TIM4 counts
			// After Falling Edge on TIM4 -> CHANNEL_1
			fval = HAL_TIM_ReadCapturedValue(&hictim, TIM_CHANNEL_1);

			// Not Overflowed
			if(fval >= rval) duration = fval - rval;
			else duration = fval + ((IC_TIMER->ARR) + 1 - rval);

			// Valid Data
			ic_state = End_Capture;
			break;

		case End_Capture:
			break;
	}

}
