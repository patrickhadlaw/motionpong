#ifndef LCD_H
#define LCD_H

#include "log.h"

#define	BLACK 0x0000
#define	BLUE 0x001F
#define	RED 0xF800
#define	GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

namespace LCD{
	
	const int SCREEN_WIDTH = 480;
	const int SCREEN_HEIGHT = 320;
	
	bool drawPixel(uint16_t x, uint16_t y, uint16_t color){
		std::cout << "Draw pixel: (" << x << ", " << y << ")";
		return true;
	}
	
	bool drawRect(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint16_t color){
		
	}
	
	bool drawRectCenter(uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint16_t color){
		return drawRect(width, height, x - width/2, y - height/2, color);
	}
	
	bool sendDataToArduino(){
		return true;
	}
}

#endif // LCD_H