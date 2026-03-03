#!/bin/bash
# Create a simple test video file

echo "Creating test.mp4 (10 seconds, 1080p)..."

gst-launch-1.0 videotestsrc pattern=0 num-buffers=300 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1 ! \
    videoconvert ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! \
    h264parse ! qtmux ! \
    filesink location=test.mp4

if [ $? -eq 0 ]; then
    echo "✓ test.mp4 created successfully"
    ls -lh test.mp4
else
    echo "✗ Failed to create test.mp4"
fi
