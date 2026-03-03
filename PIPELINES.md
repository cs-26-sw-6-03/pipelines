# Simple GStreamer Pipelines for i.MX8M Plus

Mix and match: `[INPUT] ! [OUTPUT]`

---

## INPUTS

### 1. File Input (VPU Decoded) - H.264
```bash
filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! video/x-raw
```

### 2. Camera IMX477 (MIPI)
```bash
v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12
```

### 3. Generic File Input
```bash
filesrc location=input.mp4 ! decodebin ! videoconvert ! video/x-raw
```

### 4. Generic Webcam
```bash
v4l2src device=/dev/video0 ! videoconvert ! video/x-raw
```

---

## OUTPUTS

### 1. File Output (VPU Encoded) - H.264
```bash
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=output.mp4
```

### 2. Network UDP/RTP (Hardware)
```bash
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! vpuenc_h264 bitrate=3000 gop-size=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.1.100 port=5000
```

### 3. Generic File Output
```bash
videoconvert ! x264enc bitrate=5000 ! h264parse ! qtmux ! filesink location=output.mp4
```

### 4. Display Output (Wayland/KMS)
```bash
kmssink
```

### 4b. Display Output (Framebuffer - fallback)
```bash
videoconvert ! video/x-raw,format=RGB16 ! fbdevsink device=/dev/fb0
```

---

## EXAMPLES

### File → Display
```bash
gst-launch-1.0 \
    filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! \
    kmssink
```

### Camera → File
```bash
gst-launch-1.0 \
    v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    imxvideoconvert_g2d ! vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=output.mp4
```

### File → Network
```bash
gst-launch-1.0 \
    filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! vpuenc_h264 bitrate=3000 gop-size=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.1.100 port=5000
```

### Camera → Display
```bash
gst-launch-1.0 \
    v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    kmssink
```

---

## CREATE TEST FILE

```bash
# Quick way: run the script
chmod +x create_test_video.sh
./create_test_video.sh

# Manual: 10-second test video
gst-launch-1.0 videotestsrc num-buffers=300 ! video/x-raw,width=1920,height=1080,framerate=30/1 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=test.mp4
```

---

## RECEIVE NETWORK STREAM

```bash
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=H264,clock-rate=90000,payload=96" ! \
    rtph264depay ! h264parse ! vpudec ! kmssink
```
