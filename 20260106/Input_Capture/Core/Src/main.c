/* Includes ---------*/
#include "main.h"

/* Global variables ----*/
extern volatile IC_State ic_state;
extern volatile uint16_t duration;


/* Main Function ----*/
int main(void)
{
  HCSR04_State hc_state = Origin;
  uint32_t soft_delay = 0;
  float distance = 0;

  // Hardware Initialize
  Hw_Init();

  // Infinite Loop
  while (1)
  {
	  switch(hc_state){
	  	  case Origin:
	  		  // Trigger 0 -> 1
	  		  HAL_GPIO_WritePin(TRG_PORT, TRG_PIN, GPIO_PIN_SET);
	  		  // Non Blocking Delay
	  		  soft_delay = IC_TIMER->CNT;
	  		  // Go Waiting
	  		  hc_state = Triggering;
	  		  break;

	  	  case Triggering:
	  		  if((IC_TIMER->CNT - soft_delay) >= 10){ // 10 us
	  			  // Trigger 1 -> 0
		  		  HAL_GPIO_WritePin(TRG_PORT, TRG_PIN, GPIO_PIN_RESET);
		  		  // Go Capturing
		  		  hc_state = Capturing;
		  		  // Start "Capturing"
		  		  ic_state = Waiting_REdge;
	  			  //Reset Soft Delay
	  			  soft_delay = IC_TIMER->CNT;
	  		  }
	  		  break;

	  	  case Capturing:
	  		  // End_Capture: echo terminó normalmente
	  		  if(ic_state == End_Capture){
	  			  distance = _GetDistance(duration);
	  			  // Clamp: si excede 40 cm
	  			  if(distance > 40) distance = 40;
	  			  // Start Again
	  			  hc_state = Origin;
	  			  // Blocking Delay Only for Debugg !!!!
	  			  HAL_Delay(70); // 70 ms
	  		  }
	  		  // Time-out 38 ms: HC-SR04 nunca bajó echo (sin obstáculo)
	  		  else if((IC_TIMER->CNT - soft_delay) >= 38000){
	  			  distance = 40;
	  			  // Start Again
	  			  hc_state = Origin;
	  			  // Blocking Delay Only for Debugg !!!!
	  			  HAL_Delay(70); // 70 ms
	  		  }
	  		  break;

	  	  default:
	  		  distance = 0;
	  		  break;
	  	}

	 }

}


