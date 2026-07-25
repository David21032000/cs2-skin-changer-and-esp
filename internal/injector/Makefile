CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++20 -O2 -s -static-libgcc -static-libstdc++ -m64 -mconsole
LDFLAGS = -lntdll -lpsapi

all: camus_injector.exe

camus_injector.exe: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp $(LDFLAGS) -o camus_injector.exe

clean:
	rm -f camus_injector.exe

.PHONY: all clean
