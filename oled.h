/*///////////////////////////////////////
// oled.h: This file contains methods for
// efficiently updating the oled-exp's
// screen to do this we use a double 
// image (buffer) technique to increase 
// efficiency
// 
*/

#ifndef OLED_H
#define OLED_H

#include "log.h"

// class vec2: generic 2 dimensional vector type for mathematical calculations
template<typename VecType>
class vec2{
public:
	vec2(){
		this->x = 0;
		this->y = 0;
	}
	vec2(VecType x, VecType y){
		this->x = x;
		this->y = y;
	}
	
	VecType x;	// x component
	VecType y;	// y component
};

// dotProduct: dot product of two similar typed vec2's
template<typename VecType>
VecType dotProduct(VecType u, VecType v){
	return u.x*v.x + u.y*v.y;
}

// normalize: unit vector of a vec2
template<typename VecType>
void normalize(vec2<VecType>& vec){
	VecType norm = sqrt(vec.x*vec.x + vec.y*vec.y);
	vec.x = vec.x/norm;
	vec.y = vec.y/norm;
}

// stretch: stretches vec2 by factor of (VecType: factor)
template<typename VecType>
void stretch(VecType& vec, VecType factor){
	vec.x = vec.x * factor;
	vec.y = vec.y * factor;
}

// stretchTo: stretch's to a magnitude of (VecType: length)
template<typename VecType>
void stretchTo(VecType& vec, VecType length){
	normalize<VecType>(vec);
	stretch<VecType>(vec, length);
}

typedef vec2<float> vec2f;
typedef vec2<int> vec2i;

namespace OLED{
	
	// Number of byte rows on oled expansion
	const int NUM_ROWS = 8;
	// Screen width of oled
	const int SCREEN_WIDTH = OLED_EXP_WIDTH;
	// Screen height of oled
	const int SCREEN_HEIGHT = OLED_EXP_HEIGHT;
	
	class DrawContext;

	// Image class defines an array of bytes (uint8_t) represeting all pixels on the oled-exp
	// also defines various methods for manipulating the image
	class Image{
		friend class OLED::DrawContext;
	public:
		Image(){ // Default constructor fills image buffer with zeros
			buffer = new uint8_t[SCREEN_WIDTH*NUM_ROWS];
			for(int i = 0; i < SCREEN_WIDTH*NUM_ROWS; i++){
				buffer[i] = 0;
			}
		}
		~Image(){
			if(buffer != NULL){
				delete[] buffer;
			}
		}
		
		// Bitwise XOR operator -> XOR's each byte with that of another image
		Image operator^(Image& other){ 
			
			Image xored = Image();
			if(this->buffer == NULL || other.buffer == NULL){
				LOG::warning("invalid use of operator ^ on class Image: cannot perform bitwise XOR on null buffer");
				return xored;
			}
			for(int i = 0; i < NUM_ROWS; i++){
				for(int j = 0; j < SCREEN_WIDTH; j++){
					xored.buffer[i*SCREEN_WIDTH + j] = this->buffer[i*SCREEN_WIDTH + j]^other.buffer[i*SCREEN_WIDTH + j];
				}
			}
			return xored;
		}
		
		// Bitwise AND operator -> AND's each byte with that of another image
		Image operator&(Image& other){ 
			
			Image anded = Image();
			
			if(this->buffer == NULL || other.buffer == NULL){
				LOG::warning("invalid use of operator & on class Image: cannot perform bitwise AND on null buffer");
				return anded;
			}
			
			for(int i = 0; i < NUM_ROWS; i++){
				for(int j = 0; j < SCREEN_WIDTH; j++){
					anded.buffer[i*SCREEN_WIDTH + j] = this->buffer[i*SCREEN_WIDTH + j]&other.buffer[i*SCREEN_WIDTH + j];
				}
			}
			
			return anded;
		}
		
		// Overloaded assignment immediantly copies buffer data to image
		void operator=(const Image& image){ 
			if(this->buffer == NULL){
				Image();
			}
			for(int i = 0; i < this->size(); i++){
				this->buffer[i] = image.buffer[i]; 
			}
		}
		
		// writePixel: writes pixel to image at (x, y)
		bool writePixel(unsigned x, unsigned y){ 
			if(buffer == NULL){
				LOG::warning("failed to write pixel to image: buffer is deleted");
				return false;
			}
			if(x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT){
				LOG::warning("failed to write pixel to image: coordinates out of bounds");
				return false;
			}
			unsigned row = y / NUM_ROWS;
			uint8_t byte = 1 << (y % NUM_ROWS);
			buffer[row*SCREEN_WIDTH + x] |= byte;
			return true;
		}

		// writeByte: writes a given byte to image at a given row and column
		bool writeByte(unsigned row, unsigned column, uint8_t byte){ 
			if(buffer == NULL){
				LOG::warning("failed to write pixel to image: buffer is deleted");
				return false;
			}
			if(column >= SCREEN_WIDTH || row >= NUM_ROWS){
				LOG::warning("failed to write byte to image: position out of bounds");
				return false;
			}
			buffer[row*SCREEN_WIDTH + column] |= byte;
			return true;
		}
		
		// writeRect: writes a rectangle to image starting at (x, y) with dimensions width and height
		bool writeRect(unsigned width, unsigned height, unsigned x, unsigned y){ 
				if(x + width >= SCREEN_WIDTH || y + height >= SCREEN_HEIGHT || buffer == NULL){
				LOG::warning(std::string("failed to write rect to image with coordinates: (") + std::to_string(x) + ", " + std::to_string(y) + ") and dimensions: " + std::to_string(width) + "x" + std::to_string(height));
				return false;
			}
			for(int i = y; i < y + height; i++){
				for(int j = x; j < x + width; j++){
					unsigned row = i/NUM_ROWS;
					uint8_t byte = 1 << (i%NUM_ROWS);
					buffer[row*SCREEN_WIDTH + j] |= byte;
				}
			}

			return true;
		}
		
		// clear: fills buffer with zeros
		bool clear(){ 
			if(buffer == NULL){
				LOG::warning("failed to write pixel to image: buffer was deleted");
				return false;
			}
			for(int i = 0; i < this->size(); i++){
				buffer[i] = 0;
			}
			return true;
		}
		
		// clearExclusiveBytes: clears pixels of oled excluding nextImages pixels
		bool clearExclusiveBytes(Image* nextImage){ 
			if(buffer == NULL){
				LOG::warning("failed to undraw image: buffer was deleted");
				return false;
			}
			for(int i = 0; i < NUM_ROWS; i++){
				for(int j = 0; j < SCREEN_WIDTH; j++){
					uint8_t byte = buffer[i*SCREEN_WIDTH + j];
					if(byte > 0){
						byte = ~byte;
						byte = byte & nextImage->buffer[i*SCREEN_WIDTH + j];
						oledSetCursorByPixel(i, j);
						oledWriteByte(byte);
					}
				}
			}
			return true;
		}
		
		// size: returns size of buffer
		int size(){
			if(buffer == NULL){
				return 0;
			}
			else{
				return SCREEN_WIDTH*NUM_ROWS;
			}
		}
		
		// drawImage: draws entire image using oledDraw -> this is very slow
		bool drawImage(){
			int status = oledDraw(this->buffer, this->size());
			if(status == EXIT_FAILURE){
				LOG::error("failed to draw image to oled");
				return false;
			}
			return true;
		}

		// drawInclusiveBytes: draws pixels of oled including argument: include's pixels
		bool drawInclusiveBytes(Image* include){ 
			if(buffer == NULL){
				LOG::warning("failed to draw image using raw bytes: buffer was deleted");
				return false;
			}
			for(int i = 0; i < NUM_ROWS; i++){
				for(int j = 0; j < SCREEN_WIDTH; j++){
					uint8_t byte = buffer[i*SCREEN_WIDTH + j];
					byte = byte | include->buffer[i*SCREEN_WIDTH + j];
					if(byte > 0){
						oledSetCursorByPixel(i, j);
						oledWriteByte(byte);
					}
				}
			}

			return true;
		}

	private:
		
		uint8_t* buffer;	// Array of bytes representing image for oled expansion each byte represents eight
								// vertical pixels in one column of one row of the oled expansion
	};

	// DrawContext class: defines two buffers: one for clearing previous image and one for drawing next image
	// Call order: write data to current, clear previous (excluding overlap), draw current (including overlap), then swap buffers
	class DrawContext{
	public:
		DrawContext(){
			mClearBuffer = OLED::Image();
			mCurrentBuffer = OLED::Image();
		}
		~DrawContext(){

		}

		// writeByte: writes byte to current buffer
		bool writeByte(unsigned row, unsigned column, uint8_t byte){
			return mCurrentBuffer.writeByte(row, column, byte);
		}

		// writeRect: writes rectangle to current buffer
		bool writeRect(unsigned width, unsigned height, unsigned x, unsigned y){
			return mCurrentBuffer.writeRect(width, height, x, y);
		}

		// clear: clears the screen: clearing only the non shared bytes of the clear buffer and current buffer
		bool clear(){ 
			// We xor the clear buffer with the current buffer than and it with the clear buffer this gives us a buffer of bits that need to be erased
			OLED::Image clearBytes = (mClearBuffer ^ mCurrentBuffer) & mClearBuffer;
			
			bool good = clearBytes.clearExclusiveBytes(&mCurrentBuffer);
			
			if(!good){
				LOG::warning("failed to clear draw context: unDraw on clear buffer failed");
				return false;
			}
			
			return true;
		}

		// dumpBuffer: clears both buffers
		bool dumpBuffer(){
			return mCurrentBuffer.clear() && mClearBuffer.clear();
		}

		// draw: draws the current buffer to the screen ignores bytes that were previously drawn
		bool draw(){ 
			// Removing bytes that were previously drawn 
			OLED::Image drawBytes = (mClearBuffer ^ mCurrentBuffer) & mCurrentBuffer;
			bool good = drawBytes.drawInclusiveBytes(&mCurrentBuffer);
			if(!good){
				LOG::warning("failed to draw context: drawBytes on current buffer failed");
				return false;
			}
			return true;
		}

		// swapBuffers: swaps the current buffer and the clear buffer than clears current buffer
		void swapBuffers(){
			uint8_t* tmp = mClearBuffer.buffer;
			mClearBuffer.buffer = mCurrentBuffer.buffer;
			mCurrentBuffer.buffer = tmp;
			for(int i = 0; i < mCurrentBuffer.size(); i++){
				mCurrentBuffer.buffer[i] = 0;
			}
		}

	private:
		OLED::Image mClearBuffer;		// Clear buffer stores pixel data of last frame is used for comparing to current frame to
											// determine which pixels need to cleared and which pixels need to be drawn
		OLED::Image mCurrentBuffer;		// Current buffer stores pixel data of current frame
	};

	// init: initialises oled expansion
	bool init(){
		int status = oledSetDisplayPower(1);
		if(status == EXIT_FAILURE){
			LOG::error("failed to power oled on");
			return false;
		}
		status = oledDriverInit();
		if(status == EXIT_FAILURE){
			LOG::error("failed to initialize oled driver");
			return false;
		}
		return true;
	}

	// quickClear: draws whitespace charecter to entire screen: is still far too slow to update the game at a reasonable refresh rate
	bool quickClear(){ 
		oledSetCursor(0, 0);
		for(int i = 0; i < 21*8; i++){
			oledWriteChar(' ');
		}
	}
}

#endif // OLED_H