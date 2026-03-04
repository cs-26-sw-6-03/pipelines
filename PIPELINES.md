# Simple GStreamer Pipelines for i.MX8M Plus

Mix and match: `[INPUT] ! [OUTPUT]`

---

## INPUTS

### 1. File Input (VPU Decoded) - H.264
```bash
filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec
```

### 2. Camera IMX477 (MIPI)
```bash
v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12
```

### 3. Generic File Input
```bash
filesrc location=input.mp4 ! qtdemux ! h264parse ! avdec_h264 ! videoconvert
```

### 4. Generic Webcam
```bash
v4l2src device=/dev/video0 ! videoconvert
```

---

## OUTPUTS

### 1. File Output (VPU Encoded) - H.264
```bash
videoconvert ! vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=output.mp4
```

### 2. Network UDP/RTP (Hardware)
```bash
videoconvert ! vpuenc_h264 bitrate=3000 gop-size=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.1.100 port=5000
```

### 3. Generic File Output
```bash
videoconvert ! x264enc bitrate=5000 ! h264parse ! qtmux ! filesink location=output.mp4
```

### 4. Auto Display (auto-selects best sink)
```bash
videoconvert ! autovideosink
```

### 4b. Display Output (Wayland/KMS)
```bash
queue ! videoconvert ! kmssink connector-id=0
```

**Note:** Use `connector-id` to select display output. Find available connectors:
```bash
modetest -c  # or gst-inspect-1.0 kmssink
```

### 4c. Display Output (Wayland Surface)
```bash
queue ! videoconvert ! waylandsink display=wayland-0 fullscreen=false
```
Creates a window in your Wayland compositor (GNOME/Weston/etc.)

### 4b. Display Output (Framebuffer - fallback)
```bash
videoconvert ! video/x-raw,format=RGB16 ! fbdevsink device=/dev/fb0
```

---

## EXAMPLES

### File → Auto Display
```bash
gst-launch-1.0 \
    filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! \
    videoconvert ! autovideosink
```

### File → Display
```bash
gst-launch-1.0 \
    filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! \
    queue ! videoconvert ! kmssink connector-id=0
```

### File → Wayland
```bash
gst-launch-1.0 \
    filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! \
    queue ! videoconvert ! waylandsink display=wayland-0 fullscreen=false
```

### Camera → File
```bash
gst-launch-1.0 \
    v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=output.mp4
```

### File → Network
```bash
gst-launch-1.0 \
    filesrc location=input.mp4 ! qtdemux ! h264parse ! vpudec ! \
    videoconvert ! vpuenc_h264 bitrate=3000 gop-size=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.1.100 port=5000
```

### Camera → Display
```bash
gst-launch-1.0 \
    v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    videoconvert ! kmssink connector-id=0
```

### Camera → Wayland
```bash
gst-launch-1.0 \
    v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    videoconvert ! waylandsink display=wayland-0 fullscreen=false
```

---

## CREATE TEST FILE

```bash
# Quick way: run the script
chmod +x create_test_video.sh
./create_test_video.sh

# Manual: 10-second test video
gst-launch-1.0 videotestsrc num-buffers=300 ! video/x-raw,width=1920,height=1080,framerate=30/1 ! \
    videoconvert ! vpuenc_h264 bitrate=5000 gop-size=30 ! h264parse ! qtmux ! filesink location=test.mp4
```

---

## RECEIVE NETWORK STREAM

```bash
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=H264,clock-rate=90000,payload=96" ! \
    rtph264depay ! h264parse ! vpudec ! videoconvert ! waylandsink display=wayland-0 fullscreen=true
```
