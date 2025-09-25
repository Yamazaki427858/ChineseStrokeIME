# Makefile for Enhanced Chinese Stroke IME with OptimizedUI
# 使用 MinGW-w64 編譯

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -mwindows -DUNICODE -D_UNICODE
LDFLAGS = -mwindows -static-libgcc -static-libstdc++ -lgdi32 -luser32 -lkernel32 -lshell32 -lcomctl32

# 目標執行檔
TARGET = ChineseStrokeIME.exe

# 原始檔案
SOURCES = main.cpp \
          ime_core.cpp \
          input_handler.cpp \
          dictionary.cpp \
          buffer_manager.cpp \
          window_manager.cpp \
          config_loader.cpp \
          screen_manager.cpp \
          position_manager.cpp \
          tray_manager.cpp

# 目標檔案
OBJECTS = $(SOURCES:.cpp=.o)

# 標頭檔案
HEADERS = ime_core.h \
          input_handler.h \
          dictionary.h \
          buffer_manager.h \
          window_manager.h \
          config_loader.h \
          screen_manager.h \
          position_manager.h \
          tray_manager.h

# 版本信息
VERSION = 2.0.0

# 預設目標
all: $(TARGET)

# 連結執行檔
$(TARGET): $(OBJECTS)
	@echo 編譯 $(TARGET) v$(VERSION) OptimizedUI版本...
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo 編譯完成！

# 編譯規則
%.o: %.cpp $(HEADERS)
	@echo 編譯 $<...
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	@echo 清理項目文件...
	del /F $(OBJECTS) $(TARGET)

# 重新編譯
rebuild: clean all

# 執行程式
run: $(TARGET)
	@echo 執行 $(TARGET)...
	$(TARGET)

.PHONY: all clean rebuild run