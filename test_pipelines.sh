#!/bin/bash
# Test script for GStreamer pipelines on Debix Model B (i.MX8M Plus)

echo "========================================"
echo "GStreamer Pipeline Test Suite"
echo "Debix Model B (i.MX8M Plus)"
echo "========================================"

# Check if GStreamer is installed
if ! command -v gst-launch-1.0 &> /dev/null; then
    echo "ERROR: GStreamer not found. Please install GStreamer."
    exit 1
fi

# Function to run a test
run_test() {
    local test_name=$1
    local pipeline=$2
    local duration=${3:-5}
    
    echo ""
    echo "Test: $test_name"
    echo "Pipeline: $pipeline"
    echo "Running for $duration seconds..."
    
    timeout ${duration}s gst-launch-1.0 $pipeline 2>&1 | tail -n 5
    
    if [ $? -eq 124 ]; then
        echo "✓ Test completed successfully"
    elif [ $? -eq 0 ]; then
        echo "✓ Pipeline finished normally"
    else
        echo "✗ Test failed"
    fi
}

# Test 1: Check available cameras
echo ""
echo "========================================"
echo "1. Checking available cameras..."
echo "========================================"
v4l2-ctl --list-devices

# Test 2: Test pattern to display
echo ""
echo "========================================"
echo "2. Testing video test source to display"
echo "========================================"
run_test "Test Pattern to KMS Display" \
    "videotestsrc pattern=0 ! video/x-raw,width=1920,height=1080,framerate=30/1 ! kmssink" \
    5

# Test 3: Test pattern to file
echo ""
echo "========================================"
echo "3. Testing video encode to file (H.264)"
echo "========================================"
run_test "Test Pattern to H.264 File" \
    "videotestsrc pattern=0 num-buffers=150 ! video/x-raw,width=1920,height=1080,framerate=30/1 ! \
     imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
     vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=test_output.mp4" \
    10

# Test 4: Camera to display (if camera available)
if [ -e /dev/video0 ]; then
    echo ""
    echo "========================================"
    echo "4. Testing camera to display"
    echo "========================================"
    run_test "Camera to KMS Display" \
        "v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1 ! kmssink" \
        5
else
    echo ""
    echo "========================================"
    echo "4. Skipping camera test (no camera found)"
    echo "========================================"
fi

# Test 5: File playback (if test file exists)
if [ -f test_output.mp4 ]; then
    echo ""
    echo "========================================"
    echo "5. Testing file playback to display"
    echo "========================================"
    run_test "File Playback to Display" \
        "filesrc location=test_output.mp4 ! qtdemux ! h264parse ! vpudec_h264 ! \
         imxvideoconvert_g2d ! kmssink" \
        10
else
    echo ""
    echo "========================================"
    echo "5. Skipping file playback test (no test file)"
    echo "========================================"
fi

# Test 6: Check VPU plugins
echo ""
echo "========================================"
echo "6. Checking VPU plugins"
echo "========================================"
echo "VPU Encoder plugins:"
gst-inspect-1.0 | grep -i "vpuenc"
echo ""
echo "VPU Decoder plugins:"
gst-inspect-1.0 | grep -i "vpudec"
echo ""
echo "i.MX G2D plugins:"
gst-inspect-1.0 | grep -i "imx"

echo ""
echo "========================================"
echo "Test suite completed!"
echo "========================================"
