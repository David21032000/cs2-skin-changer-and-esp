CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++20 -O2 -s -fvisibility=hidden -shared -static-libgcc -static-libstdc++ \
           -m64 -Wall -Wno-unused-variable -Wno-unused-parameter -Wno-comment -Isrc
LDFLAGS = -ld3d11 -ldxgi -ld3dcompiler -lwinmm -lws2_32 -limm32 -lgdi32 -ldwmapi -lwinhttp

SRCDIR = src
OBJDIR = obj

MINHOOK_SRC = $(wildcard $(SRCDIR)/MinHook/*.c)
MINHOOK_HDE_SRC = $(wildcard $(SRCDIR)/MinHook/hde/*.c)
MINHOOK_OBJ = $(patsubst $(SRCDIR)/MinHook/%.c,$(OBJDIR)/MinHook/%.o,$(MINHOOK_SRC))
MINHOOK_HDE_OBJ = $(patsubst $(SRCDIR)/MinHook/hde/%.c,$(OBJDIR)/MinHook/hde/%.o,$(MINHOOK_HDE_SRC))

CHEAT_SRC = $(filter-out $(SRCDIR)/MinHook/%,$(wildcard $(SRCDIR)/*.cpp))
CHEAT_OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CHEAT_SRC))

IMGUI_SRC = $(wildcard $(SRCDIR)/imgui/*.cpp)
IMGUI_OBJ = $(patsubst $(SRCDIR)/imgui/%.cpp,$(OBJDIR)/imgui/%.o,$(IMGUI_SRC))

all: $(OBJDIR) $(OBJDIR)/MinHook $(OBJDIR)/imgui camus.dll

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/MinHook:
	mkdir -p $(OBJDIR)/MinHook
	mkdir -p $(OBJDIR)/MinHook/hde

$(OBJDIR)/imgui:
	mkdir -p $(OBJDIR)/imgui

$(OBJDIR)/MinHook/%.o: $(SRCDIR)/MinHook/%.c
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(OBJDIR)/MinHook/hde/%.o: $(SRCDIR)/MinHook/hde/%.c
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(OBJDIR)/imgui/%.o: $(SRCDIR)/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

camus.dll: $(MINHOOK_OBJ) $(MINHOOK_HDE_OBJ) $(IMGUI_OBJ) $(CHEAT_OBJ)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o "camus.dll"

clean:
	rm -rf $(OBJDIR) camus.dll

.PHONY: all clean
