# GStreamer Pipelines for Debix Model B (i.MX8M Plus)

## Hardware-Accelerated Plugins for i.MX8M Plus
- `vpuenc_h264`, `vpuenc_h265` - VPU hardware H.264/H.265 encoders
- `vpudec_h264`, `vpudec_h265` - VPU hardware H.264/H.265 decoders
- `imxvideoconvert_g2d` - G2D hardware-accelerated format conversion
- `v4l2src` - Video4Linux2 camera source
- `kmssink` - Direct display output using DRM/KMS (framebuffer)

---

## INPUT PIPELINES

### 1. Video Test Source (for testing)
```bash
# Color bars test pattern
gst-launch-1.0 videotestsrc pattern=0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=I420 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    [OUTPUT_PIPELINE]

# SMPTE color bars
gst-launch-1.0 videotestsrc pattern=0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1 ! \
    [OUTPUT_PIPELINE]
```

### 2. Arducam IMX477 Camera Input
```bash
# Arducam IMX477 - 1080p @ 30fps
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    [OUTPUT_PIPELINE]

# Arducam IMX477 - 4K @ 30fps
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=3840,height=2160,framerate=30/1,format=NV12 ! \
    [OUTPUT_PIPELINE]

# Arducam IMX477 - 1080p @ 60fps
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=60/1,format=NV12 ! \
    [OUTPUT_PIPELINE]
```

### 3. Generic Camera Input (V4L2)
```bash
# Generic USB or MIPI camera
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    [OUTPUT_PIPELINE]

# Auto-detect camera capabilities
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    [OUTPUT_PIPELINE]
```

### 4. File Input (Video File)
```bash
# H.264 file with hardware decoding
gst-launch-1.0 filesrc location=/path/to/video.mp4 ! \
    qtdemux ! h264parse ! vpudec_h264 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    [OUTPUT_PIPELINE]

# H.265 file with hardware decoding
gst-launch-1.0 filesrc location=/path/to/video.mp4 ! \
    qtdemux ! h265parse ! vpudec_h265 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    [OUTPUT_PIPELINE]

# Raw video file
gst-launch-1.0 filesrc location=/path/to/video.yuv ! \
    videoparse width=1920 height=1080 format=nv12 framerate=30/1 ! \
    [OUTPUT_PIPELINE]
```

---

## OUTPUT PIPELINES

### 1. Hardware Encode and Write to File (VPU Encoder)

#### H.264 Encoding
```bash
# H.264 High Profile with VPU encoder - MP4 container
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! \
    h264parse ! qtmux ! \
    filesink location=/path/to/output.mp4

# H.264 with MKV container
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! \
    h264parse ! matroskamux ! \
    filesink location=/path/to/output.mkv

# H.264 raw elementary stream
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! \
    filesink location=/path/to/output.h264
```

#### H.265 Encoding
```bash
# H.265 with VPU encoder - MP4 container
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h265 bitrate=4000 gop-size=30 ! \
    h265parse ! qtmux ! \
    filesink location=/path/to/output.mp4

# H.265 with MKV container
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h265 bitrate=4000 gop-size=30 ! \
    h265parse ! matroskamux ! \
    filesink location=/path/to/output.mkv
```

### 2. Package and Send over UDP

#### H.264 over RTP/UDP
```bash
# H.264 RTP stream over UDP
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! rtph264pay config-interval=1 pt=96 ! \
    udpsink host=192.168.1.100 port=5000

# H.264 with multicast
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! rtph264pay config-interval=1 pt=96 ! \
    udpsink host=224.1.1.1 port=5000 auto-multicast=true
```

#### H.265 over RTP/UDP
```bash
# H.265 RTP stream over UDP
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h265 bitrate=2500 gop-size=30 ! \
    h265parse ! rtph265pay config-interval=1 pt=96 ! \
    udpsink host=192.168.1.100 port=5000
```

#### MPEG-TS over UDP
```bash
# MPEG-TS with H.264 over UDP
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! mpegtsmux ! \
    udpsink host=192.168.1.100 port=5000
```

### 3. Display to Screen (Framebuffer/KMS)

```bash
# Display using kmssink (DRM/KMS - preferred for i.MX8M Plus)
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    kmssink

# Display with specific plane and fullscreen
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    kmssink plane-id=0 fullscreen=true

# Display with overlay support
imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    kmssink connector-id=0 plane-id=0

# Fallback: Display using fbdevsink (older framebuffer interface)
imxvideoconvert_g2d ! video/x-raw,format=RGB16 ! \
    fbdevsink device=/dev/fb0
```

---

## COMPLETE PIPELINE EXAMPLES

### Example 1: Arducam IMX477 → File (H.264)
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! \
    h264parse ! qtmux ! \
    filesink location=output.mp4
```

### Example 2: Arducam IMX477 → UDP Stream
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! rtph264pay config-interval=1 pt=96 ! \
    udpsink host=192.168.1.100 port=5000
```

### Example 3: Arducam IMX477 → Display
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    kmssink
```

### Example 4: Test Source → File + Display (tee)
```bash
gst-launch-1.0 videotestsrc pattern=0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    tee name=t \
    t. ! queue ! vpuenc_h264 bitrate=5000 ! h264parse ! qtmux ! filesink location=output.mp4 \
    t. ! queue ! kmssink
```

### Example 5: File Input → UDP Stream
```bash
gst-launch-1.0 filesrc location=input.mp4 ! \
    qtdemux ! h264parse ! vpudec_h264 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! rtph264pay config-interval=1 pt=96 ! \
    udpsink host=192.168.1.100 port=5000
```

### Example 6: Camera → Multiple Outputs
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    tee name=t \
    t. ! queue ! vpuenc_h264 bitrate=5000 ! h264parse ! qtmux ! filesink location=output.mp4 \
    t. ! queue ! vpuenc_h264 bitrate=3000 ! h264parse ! rtph264pay pt=96 ! udpsink host=192.168.1.100 port=5000 \
    t. ! queue ! kmssink
```

---

## RECEIVING UDP STREAMS

### Receive H.264 RTP Stream
```bash
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=H264,clock-rate=90000,payload=96" ! \
    rtph264depay ! h264parse ! vpudec_h264 ! \
    imxvideoconvert_g2d ! kmssink
```

### Receive H.265 RTP Stream
```bash
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=H265,clock-rate=90000,payload=96" ! \
    rtph265depay ! h265parse ! vpudec_h265 ! \
    imxvideoconvert_g2d ! kmssink
```

---

## USEFUL COMMANDS

### List available cameras
```bash
v4l2-ctl --list-devices
```

### List camera formats and resolutions
```bash
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

### Check GStreamer plugins
```bash
gst-inspect-1.0 | grep -i vpu
gst-inspect-1.0 | grep -i imx
```

### Test display output
```bash
gst-launch-1.0 videotestsrc pattern=0 ! kmssink
```

---

## NOTES

1. **VPU Encoder Settings:**
   - Bitrate: Adjust based on quality needs (1000-10000 kbps typical)
   - GOP size: 30 is common (1 keyframe per second at 30fps)
   - Lower bitrate = smaller file/stream but lower quality

2. **Format Conversion:**
   - Use `imxvideoconvert_g2d` for hardware-accelerated conversion
   - NV12 is the preferred format for VPU encoders

3. **Camera Device:**
   - IMX477 typically appears as `/dev/video0` or `/dev/video1`
   - Check with `v4l2-ctl --list-devices`

4. **Display Output:**
   - `kmssink` is preferred for i.MX8M Plus (modern DRM/KMS)
   - `fbdevsink` is fallback for older framebuffer interface

5. **Network Streaming:**
   - Adjust bitrate based on network bandwidth
   - Use `config-interval=1` for robust RTP streaming
   - For wireless, consider bitrate ≤ 3000 kbps
