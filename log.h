/*///////////////////////////////////////
// log.h: This file contains methods for
// logging to the file: runtime.log
*/

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <ctime>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>

#include <oled-exp.h>
#include <ugpio/ugpio.h>

#ifdef DEBUG
#define DEBUG_POINT std::cout << "[DEBUG POINT] in function: " << __func__ << " in file: " << __FILE__ << " on line: " << __LINE__ << std::endl
#else
#define DEBUG_POINT
#endif

namespace LOG{
	const char file[] = "runtime.log";
	
	// writeLine: writes line to log file if timestamp is true a timestamp is prepended to line
	void writeLine(std::string line, bool timestamp = true){
		if(timestamp){
			char stamp[100];
			time_t t = time(0);
    		struct tm * now = localtime(&t);
    		strftime(stamp, 100, "%T", now);
			line = std::string("(") + stamp + ")" + line;
		}
		
		std::ofstream outfile;
		outfile.open(LOG::file, std::ios::ate | std::ios_base::app);
		if(!outfile.is_open()){
			std::cerr << "[Warning]: failed to write line: \n" << line.data() << "\nTo log file: " << file << std::endl;
			return;
		}
		else{
			outfile << line + "\n";
		}
		outfile.close();
	}
	
	// message: writes message to log file
	void message(std::string msg){
		std::cout << "[MESSAGE]: " << msg << std::endl;
		std::string finalMsg = std::string("[MESSAGE]: ") + msg;
		writeLine(finalMsg);
	}
	
	// error: writes error to log file
	std::string error(std::string err){ // returns error string
		std::string display = std::string("[FATAL ERROR]: ") + err;
		oledWrite((char*)display.data());
		std::cerr << "[LOG][ERROR]: " << err << std::endl;
		std::string finalErr = std::string("[LOG][ERROR]: ") + err;
		writeLine(finalErr);
		return display;
	}
	
	// warning: writes warning to log file
	void warning(std::string warn){
		std::cerr << "[LOG][WARNING]: " << warn << std::endl;
		std::string finalWarn = std::string("[LOG][ERROR]: ") + warn;
		writeLine(finalWarn);
	}
}
#endif // LOG_H
