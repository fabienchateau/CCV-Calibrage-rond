all: unicap--

unicap--: UnicapDevice.cpp UnicapDataBuffer.cpp
	g++ -o $@ $^ `pkg-config libunicap --cflags --libs`


SmartPtrTest: SmartPtrTest.cpp
	g++ -o $@ $^ `pkg-config libunicap --cflags --libs`

clean: 
	rm -f *.o
	rm -f unicap--