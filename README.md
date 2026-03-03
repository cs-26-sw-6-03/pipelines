# GStreamer Pipelines for Debix Model B (i.MX8M Plus)

This repository contains optimized GStreamer pipelines for the Debix Model B board featuring the i.MX8M Plus processor with hardware-accelerated video encoding/decoding.

## Features

- **Hardware Acceleration**: Utilizes VPU (Video Processing Unit) for H.264/H.265 encoding and decoding
- **Swappable Pipelines**: Modular input and output pipeline design
- **Multiple Input Sources**: 
  - Video test patterns (for testing)
  - Arducam IMX477 camera
  - Generic V4L2 cameras
  - File input (H.264/H.265)
- **Multiple Output Targets**:
  - File output (MP4, MKV with VPU encoding)
  - UDP streaming (RTP, MPEG-TS)
  - Display output (KMS/DRM, framebuffer)

## Contents

1. **gstreamer_pipelines.md** - Complete pipeline reference documentation
2. **input.cpp** - C++ pipeline manager with swappable inputs/outputs
3. **Makefile** - Build configuration
4. **test_pipelines.sh** - Test suite for validating pipelines
5. **README.md** - This file

## Requirements

### Software Dependencies

```bash
# Core GStreamer
sudo apt-get install \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    libgstreamer1.0-dev

# i.MX8M Plus specific plugins (from NXP BSP)
# These should be included in your Yocto/BSP build:
# - gstreamer1.0-plugins-imx
# - imx-vpuwrap
# - imx-vpu-hantro
```

### Hardware

- Debix Model B (i.MX8M Plus)
- Optional: Arducam IMX477 camera module
- Display output (HDMI/MIPI-DSI)

## Quick Start

### 1. Test Your Setup

Make the test script executable and run it:

```bash
chmod +x test_pipelines.sh
./test_pipelines.sh
```

This will verify:
- GStreamer installation
- Available cameras
- VPU plugins
- Basic pipeline functionality

### 2. Build the C++ Pipeline Manager

```bash
make
```

### 3. Run Example Pipelines

```bash
# Test pattern to display
./gst_pipeline_manager test_1080p display_kms

# Camera to file (H.264)
./gst_pipeline_manager imx477_1080p file_h264_mp4

# Camera to UDP stream
./gst_pipeline_manager imx477_1080p udp_h264_rtp

# File playback to display
./gst_pipeline_manager file_h264 display_kms input.mp4

# Camera to file with custom output path
./gst_pipeline_manager generic_camera file_h264_mp4 "" output.mp4
```

## Command-Line Examples

### Camera to Display
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    kmssink
```

### Camera to File (H.264)
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    vpuenc_h264 bitrate=5000 gop-size=30 ! \
    h264parse ! qtmux ! \
    filesink location=output.mp4
```

### Camera to UDP Stream
```bash
gst-launch-1.0 v4l2src device=/dev/video0 ! \
    video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! rtph264pay config-interval=1 pt=96 ! \
    udpsink host=192.168.1.100 port=5000
```

### File to UDP Re-stream
```bash
gst-launch-1.0 filesrc location=input.mp4 ! \
    qtdemux ! h264parse ! vpudec_h264 ! \
    imxvideoconvert_g2d ! video/x-raw,format=NV12 ! \
    vpuenc_h264 bitrate=3000 gop-size=30 ! \
    h264parse ! rtph264pay config-interval=1 pt=96 ! \
    udpsink host=192.168.1.100 port=5000
```

### Receive UDP Stream
```bash
gst-launch-1.0 udpsrc port=5000 \
    caps="application/x-rtp,media=video,encoding-name=H264,clock-rate=90000,payload=96" ! \
    rtph264depay ! h264parse ! vpudec_h264 ! \
    imxvideoconvert_g2d ! kmssink
```

## Camera Configuration

### Find Your Camera
```bash
v4l2-ctl --list-devices
```

### Check Supported Formats
```bash
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

### Arducam IMX477 Settings

The IMX477 supports multiple resolutions:
- 1920x1080 @ 30/60 fps
- 3840x2160 @ 30 fps (4K)
- Various other modes

Ensure proper device tree configuration for MIPI CSI interface.

## Performance Tips

1. **Use NV12 Format**: Preferred by VPU encoders, reduces conversions
2. **Adjust Bitrate**: 
   - Local storage: 5000-8000 kbps
   - LAN streaming: 3000-5000 kbps
   - WiFi streaming: 2000-3000 kbps
3. **GOP Size**: 30 frames (1 keyframe/second @ 30fps) is a good default
4. **Use Hardware Acceleration**: Always use VPU encoders (`vpuenc_*`) instead of software encoders
5. **G2D Conversion**: Use `imxvideoconvert_g2d` for format conversion

## Troubleshooting

### Pipeline Fails to Start
- Check if camera device exists: `ls -l /dev/video*`
- Verify camera is not in use: `fuser /dev/video0`
- Check GStreamer version: `gst-launch-1.0 --version`

### VPU Not Working
- Verify VPU plugins: `gst-inspect-1.0 vpuenc_h264`
- Check kernel modules: `lsmod | grep hantro`
- Review BSP/Yocto build for VPU support

### Display Issues
- Test KMS sink: `gst-launch-1.0 videotestsrc ! kmssink`
- Check DRM devices: `ls -l /dev/dri/card*`
- Fallback to fbdevsink if KMS not available

### Network Streaming Issues
- Test network: `ping <destination_ip>`
- Check firewall: `sudo iptables -L`
- Monitor bandwidth: `iperf3 -c <server>`
- Reduce bitrate for wireless connections

## Project Structure

```
pipelines/
├── gstreamer_pipelines.md   # Complete pipeline documentation
├── input.cpp                 # C++ pipeline manager
├── Makefile                  # Build configuration
├── test_pipelines.sh         # Test suite
└── README.md                 # This file
```

## C++ API Usage

```cpp
#include "input.cpp"

int main() {
    GStreamerPipelineManager manager;
    
    // List available pipelines
    manager.listInputs();
    manager.listOutputs();
    
    // Create and run pipeline
    manager.createPipeline("imx477_1080p", "file_h264_mp4");
    manager.start();
    manager.run();
    manager.stop();
    
    return 0;
}
```

## Additional Resources

- [i.MX8M Plus Reference Manual](https://www.nxp.com/docs/en/reference-manual/IMX8MPRM.pdf)
- [GStreamer Documentation](https://gstreamer.freedesktop.org/documentation/)
- [NXP i.MX GStreamer Plugins](https://github.com/nxp-imx/imx-gstreamer)
- [Debix Documentation](https://www.debix.io/)

## License

This code is provided as-is for use with Debix Model B hardware.

## Contributing

Feel free to submit issues or pull requests for improvements or additional pipeline configurations.
