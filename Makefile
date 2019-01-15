# MotionPong Makefile
# REQUIRES C++11
# Generates three executables:
# motionPong -> player-vs-player
# motionPongPVC -> player-vs-cpu
# motionPongCVC -> cpu-vs-cpu

TARGET1 := motionPong

all: $(TARGET1)

$(TARGET1): 
	@echo "Compiling C++ program"
	$(CXX) $(CFLAGS) -L./lib/ $(TARGET1).cpp -o $(TARGET1) $(LDFLAGS) $(LIB)
	$(CXX) $(CFLAGS) -L./lib/ $(TARGET1).cpp -D P_VS_C -o $(TARGET1)PVC $(LDFLAGS) $(LIB)
	$(CXX) $(CFLAGS) -L./lib/ $(TARGET1).cpp -D C_VS_C -o $(TARGET1)CVC $(LDFLAGS) $(LIB)
clean:
	@rm -rf $(TARGET1) $(TARGET2)
