/*///////////////////////////////////////
// MotionPong: a motion controled pong game
// using onion omega oled-exp and ultrasonic
// sensors to draw and update pong game 
//
// by: Patrick Hadlaw
*/

#include "oled.h"
#include "ultrasonic.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>

enum Gamemode{
	PLAYER_VS_PLAYER,
	PLAYER_VS_CPU,
	CPU_VS_CPU
};

// Define P_VS_C in makefile to create player-vs-cpu game and C_VS_C for cpu-vs-cpu game otherwise default is player-vs-player
#ifdef P_VS_C
#define GAMEMODE PLAYER_VS_CPU
#elif defined(C_VS_C)
#define GAMEMODE CPU_VS_CPU
#else
#define GAMEMODE PLAYER_VS_PLAYER
#endif

// Dimensions of pong paddles
const vec2i PADDLE_DIM = vec2i((int)(16*OLED::SCREEN_WIDTH)/100, (int)(8*OLED::SCREEN_HEIGHT)/100);
// Ball dimensions
const vec2i BALL_DIM = vec2i(4, 4);
// Range in initial ball velocities
const vec2i BALL_RANGE = vec2i(40, 40);

// PongPaddle class contains all state for each pong paddle
struct PongPaddle{
	PongPaddle(){
		lastTime = 0.0;
		currentTime = (float)clock()/CLOCKS_PER_SEC;
		position.x = (OLED::SCREEN_WIDTH/2) - PADDLE_DIM.x/2;
		position.y = 0;
		runningAverage = 0;
		for(int i = 0; i < 5; i++){
			previousDistances[i] = 0;
			previousDistances[i] = 0;
		}
		speed = 0.0;
	}
	Ultrasonic::Sensor sensor;		// Ultrasonic sensor for paddle
	vec2f position;					// Position vector of paddle
	float lastRunningAverage;		// Last frame's running average
	float lastTime;					// Last frame's time
	float currentTime;				// Current frame's time
	float runningAverage;			// Current running average of sensor distance
	float previousDistances[5];		// Previous distances used to calculate stars in running-average function
	float speed;					// Horizontal speed of paddle

	// pushDistance: adds distance to previousDistances used to calculate the deviation from past distances
	void pushDistance(float distance){
		for(int i = 0; i < 4; i++){
			previousDistances[i] = previousDistances[i + 1];
		}
		previousDistances[4] = distance;
	}

	// updateRunningAverage: averages distance into running average and reduces distance
	// if it is greater than one standard deviation from average of dataset
	void updateRunningAverage(float distance){
		if(distance == distance){
			pushDistance(distance);
			lastRunningAverage = runningAverage;
			lastTime = currentTime;
			currentTime = (float)clock()/CLOCKS_PER_SEC;
			float average = Stats::average<float>(previousDistances, 5);
			float stddev = Stats::sampleStandardDeviation<float>(previousDistances, 5);
			if(distance - average > stddev){
				previousDistances[4] = (average + stddev);
				runningAverage = runningAverage + previousDistances[4]/5 - runningAverage/5;
			}
			else if(average - distance > stddev){
				previousDistances[4] = (average - stddev);
				runningAverage = runningAverage + previousDistances[4]/5 - runningAverage/5;
			}
			else{
				runningAverage = runningAverage + distance/5 - runningAverage/5;
			}
			speed = (runningAverage - lastRunningAverage)/(lastTime - currentTime);
		}
	}
};

class MotionPong{
public:
	MotionPong(){
		mGameMode = PLAYER_VS_PLAYER;
		mBallPosition = vec2f(OLED::SCREEN_WIDTH/2, OLED::SCREEN_HEIGHT/2);
		mBallVelocity.x = 0;
		while(abs(mBallVelocity.x) < BALL_RANGE.x/2){
			mBallVelocity.x = (rand() % BALL_RANGE.x*2) - BALL_RANGE.x;
		}
		mBallVelocity.y = 0;
		while(abs(mBallVelocity.y) < BALL_RANGE.y/2){
			mBallVelocity.y = (rand() % BALL_RANGE.y*2) - BALL_RANGE.y;
		}
		mBallInitialVelocity = mBallVelocity;
		mPaddle1.position.y = 0;
		mPaddle2.position.y = (OLED::SCREEN_HEIGHT - 1) - PADDLE_DIM.y;
		mP1Score = 0;
		mP2Score = 0;
		mShouldClose = false;
	}
	MotionPong(Gamemode mode) : MotionPong() {
		mGameMode = mode;
	}
	~MotionPong(){
		mPaddle1.sensor.free();
		mPaddle2.sensor.free();
	}
	
	// init: initialises all hardware and sets initial state of game.
	bool init(){
		switch(mGameMode){
		case PLAYER_VS_PLAYER:
			LOG::message("initializing MotionPong with game-mode: player-vs-player");
			break;
		case PLAYER_VS_CPU:
			LOG::message("initializing MotionPong with game-mode: player-vs-cpu");
			break;
		case CPU_VS_CPU:
			LOG::message("initializing MotionPong with game-mode: cpu-vs-cpu");
			break;
		}

		// Initialising OLED expansion
		bool good = OLED::init();
		if(!good){
			throw std::runtime_error(LOG::error("failed to initialize oled expansion"));
			return false;
		}
		bool err = false;

		// Initialising sensors next
		mPaddle1.sensor = Ultrasonic::Sensor(err, US_ONE_TRIGGER, US_ONE_ECHO);
		if(err){
			throw std::runtime_error(LOG::error("failed to initialize ultrasonic sensor 1"));
			return false;
		}
		mPaddle2.sensor = Ultrasonic::Sensor(err, US_TWO_TRIGGER, US_TWO_ECHO);
		if(err){
			throw std::runtime_error(LOG::error("failed to initialize ultrasonic sensor 2"));
			return false;
		}

		sleep(1); // Lets the ultrasonic sensors settle
		

		mPreviousTime = clock()/CLOCKS_PER_SEC;
		return true;
	}

	// update: calculates and updates ball position taking into account collisions
	bool update(){
		float deltaTime = (float)clock()/CLOCKS_PER_SEC - mPreviousTime;
		mPreviousTime = (float)clock()/CLOCKS_PER_SEC;
		vec2f newBallPos;
		newBallPos.x = mBallPosition.x + mBallVelocity.x*deltaTime;
		newBallPos.y = mBallPosition.y + mBallVelocity.y*deltaTime;
		
		if(newBallPos.x < 0){
			newBallPos.x = fabs(newBallPos.x);
			mBallVelocity.x = -mBallVelocity.x;
		}
		else if(newBallPos.x >= OLED::SCREEN_WIDTH - BALL_DIM.x - 1){
			newBallPos.x = 2*(OLED::SCREEN_WIDTH - BALL_DIM.x - 1) - newBallPos.x;
			mBallVelocity.x = - mBallVelocity.x;
		}

		if(newBallPos.y < PADDLE_DIM.y && newBallPos.x > mPaddle1.position.x - BALL_DIM.x && newBallPos.x < mPaddle1.position.x + PADDLE_DIM.x){
			newBallPos.y = 2*PADDLE_DIM.y - newBallPos.y;
			mBallVelocity.y = -mBallVelocity.y*1.2;
			mBallVelocity.x += mPaddle1.speed*deltaTime;
		}
		else if(newBallPos.y <= 0){
			mP2Score++;
			return reset();
		}
		else if(newBallPos.y > OLED::SCREEN_HEIGHT - (PADDLE_DIM.y + BALL_DIM.y) && newBallPos.x > mPaddle2.position.x - BALL_DIM.x && newBallPos.x < mPaddle2.position.x + PADDLE_DIM.x){
			newBallPos.y = 2*(OLED::SCREEN_HEIGHT - BALL_DIM.y - PADDLE_DIM.y) - newBallPos.y;
			mBallVelocity.y = -mBallVelocity.y*1.2;
			mBallVelocity.x += mPaddle2.speed*deltaTime;
		}
		else if(newBallPos.y >= OLED::SCREEN_HEIGHT - BALL_DIM.y){
			mP1Score++;
			return reset();
		}

		if(mBallVelocity.y > OLED::SCREEN_HEIGHT/2){
			mBallVelocity.y = OLED::SCREEN_HEIGHT/2;
		}

		mBallPosition = newBallPos;
	}
	
	// draw: draws context while concurrently updates sensors
	bool draw(){
		if(mShouldClose){
			return true;
		}
		switch(mGameMode){
		case PLAYER_VS_PLAYER:
			mPaddle2.sensor.launchThreadedRead();
		case PLAYER_VS_CPU:
			mPaddle1.sensor.launchThreadedRead();
		case CPU_VS_CPU:
			break;
		}
		
		int status = oledSetCursor(3, 0);
		status = status | oledWriteChar(mP1Score + '0');
		status = status | oledSetCursor(3, 20);
		status = status | oledWriteChar(mP2Score + '0');
		
		mDrawContext.writeRect(PADDLE_DIM.x, PADDLE_DIM.y, static_cast<int>(mPaddle1.position.x), static_cast<int>(mPaddle1.position.y));
		mDrawContext.writeRect(PADDLE_DIM.x, PADDLE_DIM.y, static_cast<int>(mPaddle2.position.x), static_cast<int>(mPaddle2.position.y));
		mDrawContext.writeRect(BALL_DIM.x, BALL_DIM.y, static_cast<int>(mBallPosition.x), static_cast<int>(mBallPosition.y));
		mDrawContext.clear();
		mDrawContext.draw();
		mDrawContext.swapBuffers();
		if(mGameMode == PLAYER_VS_CPU || mGameMode == CPU_VS_CPU){
			float deltaTime = (float)clock()/CLOCKS_PER_SEC - mPreviousTime;
			if(mPaddle2.position.x + PADDLE_DIM.x/2 > mBallPosition.x){
				mPaddle2.position.x -= fabs(mBallInitialVelocity.x*3)*deltaTime*(1/(1 + rand()%3));
			}
			else if(mPaddle1.position.x + PADDLE_DIM.x/2 < mBallPosition.x){
				mPaddle2.position.x += fabs(mBallInitialVelocity.x*3)*deltaTime*(1/(1 + rand()%3));
			}

			if(mGameMode == CPU_VS_CPU){
				if(mPaddle1.position.x + PADDLE_DIM.x/2 > mBallPosition.x){
					mPaddle1.position.x -= fabs(mBallInitialVelocity.x*3)*deltaTime*(1/(1 + rand()%3));
				}
				else if(mPaddle1.position.x + PADDLE_DIM.x/2 < mBallPosition.x){
					mPaddle1.position.x += fabs(mBallInitialVelocity.x*3)*deltaTime*(1/(1 + rand()%3));
				}
			}
		}

		if(mGameMode == PLAYER_VS_PLAYER || mGameMode == PLAYER_VS_CPU){
			mPaddle1.updateRunningAverage(mPaddle1.sensor.joinThreadedRead());
			mPaddle1.position.x = Ultrasonic::convertToScreenXCoord(mPaddle1.runningAverage);
		}
		if(mGameMode == PLAYER_VS_PLAYER){
			mPaddle2.updateRunningAverage(mPaddle2.sensor.joinThreadedRead());
			mPaddle2.position.x = OLED::SCREEN_WIDTH - Ultrasonic::convertToScreenXCoord(mPaddle2.runningAverage);
		}
		if(mPaddle1.position.x < 0){
			mPaddle1.position.x = 0;
		}
		else if(mPaddle1.position.x >= OLED::SCREEN_WIDTH - PADDLE_DIM.x){
			mPaddle1.position.x = (OLED::SCREEN_WIDTH - PADDLE_DIM.x) - 1;
		}
		if(mPaddle2.position.x < 0){
			mPaddle2.position.x = 0;
		}
		else if(mPaddle2.position.x >= OLED::SCREEN_WIDTH - PADDLE_DIM.x){
			mPaddle2.position.x = (OLED::SCREEN_WIDTH - PADDLE_DIM.x) - 1;
		}
		
		if(status < 0){
			return false;
		}
		return true;
	}

	// playersAreReady: returns true if players hands are close to sensors signifying that player is ready for next round
	bool playersAreReady(){
		mPaddle1.sensor.launchThreadedRead();
		mPaddle2.sensor.launchThreadedRead();

		mPaddle1.updateRunningAverage(mPaddle1.sensor.joinThreadedRead());
		mPaddle1.position.x = Ultrasonic::convertToScreenXCoord(mPaddle1.runningAverage);
		if(mPaddle1.position.x < 0){
			mPaddle1.position.x = 0;
		}
		else if(mPaddle1.position.x >= OLED::SCREEN_WIDTH - PADDLE_DIM.x){
			mPaddle1.position.x = (OLED::SCREEN_WIDTH - PADDLE_DIM.x) - 1;
		}
		mPaddle2.updateRunningAverage(mPaddle2.sensor.joinThreadedRead());
		mPaddle2.position.x = OLED::SCREEN_WIDTH - Ultrasonic::convertToScreenXCoord(mPaddle2.runningAverage);
		if(mPaddle2.position.x < 0){
			mPaddle2.position.x = 0;
		}
		else if(mPaddle2.position.x >= OLED::SCREEN_WIDTH - PADDLE_DIM.x){
			mPaddle2.position.x = (OLED::SCREEN_WIDTH - PADDLE_DIM.x) - 1;
		}

		switch(mGameMode){
		case PLAYER_VS_PLAYER:
			if(mPaddle1.position.x < (3*OLED::SCREEN_WIDTH / 4) && mPaddle2.position.x > (OLED::SCREEN_WIDTH - PADDLE_DIM.x) - (3*OLED::SCREEN_WIDTH / 4)){
				return true;
			}
			else{
				return false;
			}
			break;
		case PLAYER_VS_CPU:
			if(mPaddle1.position.x < (3*OLED::SCREEN_WIDTH / 4)){
				return true;
			}
			else{
				return false;
			}
			break;
		case CPU_VS_CPU:
			return true;
			break;
		}
	}

	// Game calls reset when player scores, sets should close to true if a player wins (gets 4 points)
	bool reset(){ 
		mDrawContext.clear();
		OLED::quickClear();
		
		clock_t begin;
		bool ready = false;
		bool counting = true;
		int countDown = 0;

		if(mP1Score >= 4){
			oledWrite((char*)"Player 1 wins!");
			mShouldClose = true;
			return true;
		}
		else if(mP2Score >= 4){
			oledWrite((char*)"Player 2 wins!");
			mShouldClose = true;
			return true;
		}
		if(mGameMode != CPU_VS_CPU){
			oledSetCursor(3, 0);
			oledWrite((char*)"Place your hands near the sensors!");

			while(counting){
				if(playersAreReady()){
					if(ready){
						if((clock() - begin)/CLOCKS_PER_SEC > 1)
						{
							counting = false;
						}
					}
					else{
						ready = true;
						begin = clock();
					}
				}
				else{
					ready = false;
				}
			}
		}

		OLED::quickClear();
		oledSetCursor(3, 0);
		oledWriteChar('3');

		sleep(1);

		oledSetCursor(3, 0);
		oledWriteChar('2');

		sleep(1);

		oledSetCursor(3, 0);
		oledWriteChar('1');

		sleep(1);

		mBallPosition = vec2f(OLED::SCREEN_WIDTH/2, OLED::SCREEN_HEIGHT/2);
		mBallVelocity.x = 0;
		while(abs(mBallVelocity.x) < BALL_RANGE.x/2){
			mBallVelocity.x = (rand() % BALL_RANGE.x*2) - BALL_RANGE.x;
		}
		mBallVelocity.y = 0;
		while(abs(mBallVelocity.y) < BALL_RANGE.y/2){
			mBallVelocity.y = (rand() % BALL_RANGE.y*2) - BALL_RANGE.y;
		}
		if(mGameMode == PLAYER_VS_CPU || mGameMode == CPU_VS_CPU){
			mPaddle2.position.x = (OLED::SCREEN_WIDTH/2) - PADDLE_DIM.x/2;
			if(mGameMode == CPU_VS_CPU){
				mPaddle1.position.x = (OLED::SCREEN_WIDTH/2) - PADDLE_DIM.x/2;
			}
		}
		mBallInitialVelocity = mBallVelocity;
		mShouldClose = false;
		mPreviousTime = (float)clock()/CLOCKS_PER_SEC;
		mDrawContext.dumpBuffer();
		return true;
	}
	
	// Returns should close
	bool shouldClose(){
		return this->mShouldClose;
	}
	
private:
	Gamemode mGameMode;					// Current game-mode

	float mPreviousTime;				// Previous frame's time
	
	OLED::DrawContext mDrawContext;		// DrawContext: used to update oled screen, defined in oled.h
	PongPaddle mPaddle1;				// Player 1's paddle
	PongPaddle mPaddle2;				// Player 2's paddle

	bool mShouldClose;					// Close state of program
	
	vec2f mBallPosition;				// Ball's position vector
	vec2f mBallVelocity;				// Ball's velocity vector
	vec2f mBallInitialVelocity;			// Ball's initial velocity vector
	
	int mP1Score;						// Player one's score
	int mP2Score;						// Player two's score
	
};


int main(){
	try{
		LOG::writeLine("\n", false);
		LOG::message("MotionPong starting...");
		
		MotionPong pongGame(GAMEMODE);
		pongGame.init();
		pongGame.reset();
		
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
	}
	catch(std::runtime_error& err){
		std::cout << "[ABORT]: \n\t" << err.what() << "\n";
		return -1;
	}
	
	LOG::message("MotionPong exiting, goodbye...");
	
	return 0;
}
