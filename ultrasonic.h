#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "lcd.h"

#include <ugpio/ugpio.h>

#define GPIO_US_1_VCC 1
#define GPIO_US_1_TRIGGER 2
#define GPIO_US_1_ECHO 3
#define GPIO_US_2_VCC 4
#define GPIO_US_2_TRIGGER 5
#define GPIO_US_2_ECHO 6

namespace Ultrasonic{
	
	const float minDistance = 2.0f; // 2cm min distance
	const float maxDistance = 30.0f; // 30cm max distance
	
	bool init(){
		int request = 0;
		request = gpio_is_requested(GPIO_US_1_VCC);
		if(request < 0){
			LOG::error(String("failed to request gpio: ") + String(std::to_string(GPIO_US_1_VCC)))
			return false;
		}
		
		request = gpio_is_requested(GPIO_US_1_TRIGGER);
		if(request < 0){
			LOG::error(String("failed to request gpio: ") + String(std::to_string(GPIO_US_1_TRIGGER)))
			return false;
		}
		
		request = gpio_is_requested(GPIO_US_1_ECHO);
		if(request < 0){
			LOG::error(String("failed to request gpio: ") + String(std::to_string(GPIO_US_1_ECHO)))
			return false;
		}
		
		request = gpio_is_requested(GPIO_US_2_VCC);
		if(request < 0){
			LOG::error(String("failed to request gpio: ") + String(std::to_string(GPIO_US_2_VCC)))
			return false;
		}
		
		request = gpio_is_requested(GPIO_US_2_TRIGGER);
		if(request < 0){
			LOG::error(String("failed to request gpio: ") + String(std::to_string(GPIO_US_2_TRIGGER)))
			return false;
		}
		
		request = gpio_is_requested(GPIO_US_2_ECHO);
		if(request < 0){
			LOG::error(String("failed to request gpio: ") + String(std::to_string(GPIO_US_2_ECHO)))
			return false;
		}
	}
	
	float readDistance_us1(){
		
	}
	
	float readDistance_us2(){
		
	}
	
	float convertToScreenXCoord(float distance){
		return (distance - minDistance)*((float)LCD::SCREEN_WIDTH/(maxDistance - minDistance));
	}
}

#endif // ULTRASONIC_H