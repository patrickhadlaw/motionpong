/*///////////////////////////////////////
// ultrasonic.h: This file contains methods for
// reading sensor data from the ultrasonic sensors
*/

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "oled.h"

#include <limits>
#include <thread>
#include <future>
#include <chrono>

#define US_ONE_TRIGGER 18
#define US_ONE_ECHO 19
#define US_TWO_TRIGGER 2
#define US_TWO_ECHO 3

namespace Stats{
	// Helper function for our generic quicksort
	template<typename Type>
	bool quicksortHelper(Type dataset[], const int size, int left, int right){
		if(right <= left){
			return true;
		}
		else
		{
			int l2 = left;
			int r2 = right - 1;
			Type partition = dataset[right];
			do{
				while(dataset[l2] <= partition && l2 < right){
					l2++;
				}
				while(dataset[r2] > partition && r2 > left){
					r2--;
				}
				if(l2 < r2){
					Type tmp = dataset[l2];
					dataset[l2] = dataset[r2];
					dataset[r2] = tmp;
				}
			}
			while(l2 < r2);
			Type t = dataset[l2];
			dataset[l2] = partition;
			dataset[right] = t;
			
			// l2 is pivot
			quicksortHelper<Type>(dataset, size, left, l2 - 1);
			quicksortHelper<Type>(dataset, size, l2, right);
		}
	}

	// A generic quicksort function
	template<typename Type>
	bool quicksort(Type dataset[], const int size){
		return quicksortHelper<Type>(dataset, size, 0, size - 1);
	}

	template<typename Type>
	float average(Type dataset[], const int size){
		if(size < 1){
			return std::numeric_limits<Type>::quiet_NaN();
		}
		float sum = 0.0f;
		for(int i = 0; i < size; i++){
			sum = sum + (float)dataset[i];
		}
		return sum/size;
	}

	// A generic sample standard deviation function
	template<typename Type>
	float sampleStandardDeviation(Type dataset[], const int size){
		if(size <= 1){
			return std::numeric_limits<Type>::quiet_NaN();
		}
		float avg = average<Type>(dataset, size);
		float sumOfDeviation = 0.0f;
		for(int i = 0; i < size; i++){
			sumOfDeviation += pow((dataset[i] - avg), 2);
		}
		return sqrt((1/((float)size - 1.0)) * sumOfDeviation);
	}

	// A generic population standard deviation function
	template<typename Type>
	float populationStandardDeviation(Type dataset[], const int size){
		if(size <= 1){
			return std::numeric_limits<Type>::quiet_NaN();
		}
		float avg = average<Type>(dataset, size);
		float sumOfDeviation = 0.0f;
		for(int i = 0; i < size; i++){
			sumOfDeviation += pow((dataset[i] - avg), 2);
		}
		return sqrt((1.0/(float)size) * sumOfDeviation);
	}
}

namespace Ultrasonic{
	
	const float MIN_DISTANCE = 0.04f; // 4cm min distance
	const float MAX_DISTANCE = 0.45f; // 45cm max distance
	const float SPEED_OF_SOUND = 343.0f; // 343 m/s

	const int INTERPOLATION_SAMPLES = 9;
	const float SENSOR_TIMEOUT = (2*(MAX_DISTANCE + 1.0))/SPEED_OF_SOUND;

	// Interpolation function for ultrasonic sensor using multiple sensor samples
	double interpolate(double data[INTERPOLATION_SAMPLES]){

		Stats::quicksort<double>(data, INTERPOLATION_SAMPLES);
		int medianIndex = (int)(INTERPOLATION_SAMPLES/2);
		return data[medianIndex];
	}

	// class Sensor: defines methods for the HC-SR04 ultrasonic sensors.
	class Sensor
	{
	public:
		Sensor(){
			mTriggerPin = -1;
			mEchoPin = -1;
		}
		// Initialise ultrasonic sensors with trigger pin: trigpin and echo pin: echopin, sets err to true if fails
		Sensor(bool& err, uint8_t trigpin, uint8_t echopin){
			this->mTriggerPin = trigpin;
			this->mEchoPin = echopin;
			LOG::message(std::string("initializing ultrasonic sensor with trigger pin: ") + std::to_string(mTriggerPin) + " and echo pin: " + std::to_string(mEchoPin));
			gpio_free(mTriggerPin);
			gpio_free(mEchoPin);
			int request = gpio_is_requested(mTriggerPin);
			if(request < 0){
				LOG::error(std::string("ultrasonic trigger gpio:") + std::to_string(mTriggerPin) + " already requested: ");
				err = true;
				return;
			}
			else{
				request = gpio_request(mTriggerPin, NULL);
				if(request < 0){
					LOG::error(std::string("failed to request ultrasonic trigger gpio: ") + std::to_string(mTriggerPin));
					err = true;
					return;
				}
			}
			request = gpio_is_requested(mEchoPin);
			if(request < 0){
				LOG::error(std::string("ultrasonic echo gpio:") + std::to_string(mEchoPin) + " already requested: ");
				err = true;
				return;
			}
			else{
				request = gpio_request(mEchoPin, NULL);
				if(request < 0){
					LOG::error(std::string("failed to request ultrasonic echo gpio:") + std::to_string(mEchoPin));
					err = true;
					return;
				}
			}

			int status = gpio_direction_output(mTriggerPin, 0);
			if(status < 0){
				LOG::error(std::string("failed to set ultrasonic trigger gpio as output: ") + std::to_string(mTriggerPin));
				err = true;
				return;
			}

			status = gpio_direction_input(mEchoPin);
			if(status < 0){
				LOG::error(std::string("failed to set ultrasonic echo gpio as input: ") + std::to_string(mEchoPin));
				err = true;
				return;
			}

			err = false;
		}

		Sensor(const Sensor& sensor){
			this->mTriggerPin = sensor.mTriggerPin;
			this->mEchoPin = sensor.mEchoPin;
		}

		~Sensor(){
			
		}

		// Needed because std::future's assignment operator is deleted
		void operator=(const Sensor& sensor){
			this->mTriggerPin = sensor.mTriggerPin;
			this->mEchoPin = sensor.mEchoPin;
		}

		// free: rees sensors gpios
		void free(){
			if (gpio_free(mTriggerPin) < 0 || gpio_free(mEchoPin) < 0)
			{
				LOG::warning("failed to free sensor gpio's");
			}
		}

		// reading: takes sensor reading using chrono for timing to not be dependant on CPU clock
		double reading(){
			int status = gpio_direction_output(mTriggerPin, GPIOF_INIT_HIGH);
			std::this_thread::sleep_for(std::chrono::microseconds(10));
			status = status | gpio_direction_output(mTriggerPin, GPIOF_INIT_LOW);
			if(status < 0){
				LOG::warning(std::string("failed to trigger ultrasonic sensor with trigger pin: ") + std::to_string(mTriggerPin) + " and echo pin: " + std::to_string(mEchoPin) + " with status: " + std::to_string(status));
			}
			else{
				auto inital = std::chrono::high_resolution_clock::now();
				auto final = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed;
				while(gpio_get_value(mEchoPin) == 0){
					final = std::chrono::high_resolution_clock::now();
					elapsed = final - inital;
					if(elapsed.count() > SENSOR_TIMEOUT){
						return std::numeric_limits<double>::quiet_NaN();
					}
				}
				inital = std::chrono::system_clock::now();
				while(gpio_get_value(mEchoPin) == 1){
					final = std::chrono::high_resolution_clock::now();
					elapsed = final - inital;
					if(elapsed.count() > (2*MAX_DISTANCE)/SPEED_OF_SOUND){
						break;
					}
				}
				return (SPEED_OF_SOUND*elapsed.count())/2;
			}
			return std::numeric_limits<double>::quiet_NaN();
		}

		// readInterpolated: takes multiple readings and calls the interpolate function on the dataset
		double readInterpolated(){
			double distances[INTERPOLATION_SAMPLES];
			int i = 0;
			int numNANs = 0;
			while(i < INTERPOLATION_SAMPLES){
				if(numNANs > INTERPOLATION_SAMPLES/2){
					return 0.0f;
				}
				double read = reading();
				if(read == read){
					if(read > MAX_DISTANCE){
						read = MAX_DISTANCE;
					}
					distances[i] = read;
					i++;
				}
				else{
					numNANs++;
				}
			}
			return interpolate(distances);
		}

		// launchThreadedRead: launches sensor read on thread promising future distance to mSensorValueFuture
		void launchThreadedRead(){
			mSensorValueFuture = std::async(&Sensor::readInterpolated, this);
		}

		// joinThreadedRead: returns value from threaded function using the future
		double joinThreadedRead(){
			return mSensorValueFuture.get();
		}

	private:
		std::future<double> mSensorValueFuture;
		int mTriggerPin;
		int mEchoPin;
	};
	
	// convertToScreenXCoord: converts distance in metres to OLED X coordinate
	float convertToScreenXCoord(float distance){
		if(distance < MIN_DISTANCE){
			return 0;
		}
		return (distance - MIN_DISTANCE)*((float)OLED::SCREEN_WIDTH/(MAX_DISTANCE - MIN_DISTANCE));
	}
}

#endif // ULTRASONIC_H
