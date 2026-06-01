/* Includes --------------------------*/
#include "stm32f4xx_hal.h"
#include "my_lib.h"

/* Variable Types --------------------*/
typedef enum{
	Origin = 0,
	Triggering = 1,
	Capturing = 2
}HCSR04_State;

/* Exported functions prototypes -----*/
void Error_Handler(void);
