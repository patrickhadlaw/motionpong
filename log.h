#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>

class String{
public:
	String(){
		
	}
	String(const char* str){
		int i = 0;
		for(; str[i] != '\0'; i++){
			
		}
		if(string != NULL){
			delete[] string;
		}
		string = new char[i];
		for(i = 0; str[i] != '\0'; i++){
			string[i] = str[i];
		}
	}
	String(const String& str){
		*this = str;
	}
	String(const char str[]){
		
	}
	~String(){
		if(string != NULL){
			delete[] string;
		}
	}
	
	void operator=(String str){
		if(string != NULL){
			delete[] string;
		}
		string = new char[str.length() - 1];
		for(int i = 0; i < str.length() - 1; i++){
			string[i] = str[i];
		}
	}
	void operator=(const char* str){
		int i = 0;
		for(; str[i] != '\0'; i++){
			
		}
		if(string != NULL){
			delete[] string;
		}
		string = new char[i];
		for(i = 0; str[i] != '\0'; i++){
			string[i] = str[i];
		}
	}
	
	char operator[](const unsigned& index){
		for(int i = 0; i <= index; i++){
			if(!string[i]){
				return '\0';
			}
		}
		return string[index];
	}
	
	String operator+(String left, String right){
		String added;
		int newSize = (left.length() - 2) + (right.length() - 2) + 1;
		added.string = new char[newSize];
		for(int i = 0; i < left.length() - 2; i++){
			added.string[i] = left.string[i];
		}
		for(int i = 0; i < right.length() - 2; i++){
			added.string[(left.length() - 2) + i] = right.string[i];
		}
		return added;
	}
	
	unsigned length(){
		if(string == NULL){
			return 0;
		}
		unsigned i = 0
		for(; string[i] != '\0'; i++){}
		return i+1;
	}
	
	bool empty(){
		if(string == NULL){
			return true;
		}
		return false;
	}
	
	const char* data(){
		return (const char*)string;
	}
	
private:
	char* string = NULL;
};

namespace LOG{
	const char file[] = "runtime.log";
	void writeLine(String line){
		std::ofstream outfile;
		outfile.open(LOG::file, ios::ate);
		if(!outfile.is_open()){
			std::cerr << "Warning: failed to write line: \n\n" << line.data() << "\nTo log file: " << file << std::endl;
			return;
		}
		else{
			String out = line + String("\n");
			outfile << out.data();
		}
		outfile.close();
	}
	
	void message(String msg){
		std::cout << "[LOG]: " << msg.data() << std::endl;
		String finalMsg = String("[LOG]Message: ") + msg;
		writeLine(finalMsg);
	}
	
	void error(String err){
		std::cerr << "[LOG][ERROR]: " << err.data() << std::endl;
		String finalErr = String("[LOG][ERROR]: ") + err;
		writeLine(finalErr);
	}
	
	void warning(String warn){
		std::cerr << "[LOG][WARNING]: " << err.data() << std::endl;
		String finalWarn = String("[LOG][ERROR]: ") + warn;
		writeLine(finalWarn);
	}
}


#endif // LOG_H