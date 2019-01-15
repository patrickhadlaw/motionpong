#include "lcd.h"
#include "ultrasonic.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>

template<typename VecType>
class vec2{
public:
	vec2(VecType x, VecType y){
		this->x = x;
		this->y = y;
	}
	
	VecType x;
	VecType y;
};

template<typename VecType>
VecType dotProduct(VecType u, VecType v){
	return u.x*v.x + u.y*v.y;
}

template<typename VecType>
void normalize(vec2<VecType>& vec){
	VecType norm = sqrt(vec.x*vec.x + vec.y*vec.y);
	vec.x = vec.x/norm;
	vec.y = vec.y/norm;
}

template<typename VecType>
void stretch(VecType& vec, VecType factor){
	vec.x = vec.x * factor;
	vec.y = vec.y * factor;
}

template<typename VecType>
void stretchTo(VecType& vec, VecType length){
	normalize<VecType>(vec);
	stretch<VecType>(vec, length);
}

typedef vec2f vec2<float>;

enum class Gamemode{
	PLAYER_VS_PLAYER,
	PLAYER_VS_CPU,
	CPU_VS_CPU
}

// Time step - 30Hz
#define TIMESTEP 1/30
// Seconds to milliseconds
#define TO_MS(x) x*1000

const vec2f PADDLE_DIM = vec2f(LCD::SCREEN_WIDTH/10, LCD::SCREEN_HEIGHT/50);

class MotionPong{
public:
	MotionPong(){
		gameMode = PLAYER_VS_PLAYER;
		ballPosition = vec2f(LCD::SCREEN_WIDTH/2, LCD::SCREEN_HEIGHT/2);
		ballVelocity.x = rand() % 10; // TODO: check if velocity is proper
		ballVelocity.y = rand() % 10;
		paddleOnePosition = LCD::SCREEN_WIDTH/2;
		paddleTwoPosition = LCD::SCREEN_WIDTH/2;
		p1Score = 0;
		p2Score = 0;
		this->houldClose = false;
	}
	MotionPong(Gamemode mode){
		gameMode = mode;
	}
	~MotionPong(){
		
	}
	
	bool init(){
		bool good = Ultrasonic::init();
		if(!good){
			LOG::error("failed to initialize ultrasonic sensors");
			return false;
		}
		good = LCD::init();
		if(!good){
			LOG::error("failed to initialize arduino connection and display")
			return false;
		}
	}
	
	bool update(){
		switch(gameMode){
		case PLAYER_VS_PLAYER:
			float newPosition = Ultrasonic::convertToScreenXCoord(Ultrasonic::readDistance_us1());
			if(newPosition > LCD::SCREEN_WIDTH - PADDLE_DIM.x || newPosition < 0){
				
			}
			else{
				paddleOnePosition = newPosition;
			}
			newPosition = Ultrasonic::convertToScreenXCoord(Ultrasonic::readDistance_us2());
			if(newPosition > LCD::SCREEN_WIDTH - PADDLE_DIM.x || newPosition < 0){
				
			}
			else{
				paddleTwoPosition = newPosition;
			}
			
			vec2f newBallPos;
			newBallPos.x = ballPosition.x + ballVelocity.x*TIMESTEP;
			newBallPos.y = ballPosition.y + ballVelocity.y*TIMESTEP;
			vec2f newBallVelocity = ballVelocity;
			
			if(newBallPos.x < 0){
				newBallPos.x = fabs(newBallPos.x);
				newBallVelocity.x = - newBallVelocity.x;
			}
			else if(newBallPos.x > LCD::SCREEN_WIDTH){
				newBallPos.x = LCD::SCREEN_WIDTH - (newBallPos.x - LCD::SCREEN_WIDTH);
				newBallVelocity.x = - newBallVelocity.x;
			}
			
			if(newBallPos.y <= 0){
				p2Score++;
			}
			else if(newBallPos.y >= LCD::SCREEN_HEIGHT){
				p1Score++;
			}
			else if(newBallPos.y < PADDLE_DIM.y){
				if(newBallPos.x > paddleOnePosition && newBallPos.x < paddleOnePosition + PADDLE_DIM.x){
					newBallPos.y = fabs(newBallPos.y - PADDLE_DIM.y);
					
				}
			}
			else if(newBallPos.y > LCD::SCREEN_HEIGHT - PADDLE_DIM.y){
				if(newBallPos.x > paddleOnePosition && newBallPos.x < paddleOnePosition + PADDLE_DIM.x){
					newBallPos
				}
			}
			
			break;
		default:
			return false;
		}
	}
	
	bool draw(){
		if(paddleOneSpeed < 1.0){
			
		}
		else{
			
		}
		
	}
	
	bool shouldClose(){
		return this->shouldClose;
	}
	
private:
	Gamemode gameMode;
	
	bool shouldClose;
	
	vec2f ballPosition;
	vec2f ballVelocity;
	int paddleOnePosition;
	float paddleOneSpeed;
	int paddleTwoPosition;
	float paddleTwoSpeed;
	
	int p1Score;
	int p2Score;
	
};


int main(){
	LOG::writeLine("\n");
	LOG::message("MotionPong starting...");
	
	MotionPong pongGame;
	if(!pongGame.init()){
		return -1;
	}
	
	while(!pongGame.shouldClose()){
		bool good = pongGame.update();
		if(!good){
			LOG::warning("failed to update");
		}
		
		good = pongGame.draw();
		if(!good){
			LOG::warning("failed to draw pong game");
		}
	}
	
	return 0;
}