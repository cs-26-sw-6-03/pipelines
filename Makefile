# Makefile for GStreamer Pipeline Manager
# Debix Model B (i.MX8M Plus)

CXX = g++
CXXFLAGS = -Wall -std=c++11
GSTREAMER_FLAGS = $(shell pkg-config --cflags --libs gstreamer-1.0)

TARGET = gst_pipeline_manager
SRC = input.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(GSTREAMER_FLAGS)

clean:
	rm -f $(TARGET)

install:
	@echo "Installing dependencies..."
	@echo "Run: sudo apt-get install libgstreamer1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad"
	@echo "For i.MX8M Plus, install VPU plugins from NXP:"
	@echo "  gstreamer1.0-plugins-imx"

.PHONY: all clean install
