# Simple GStreamer Pipelines for i.MX8M Plus

Simple, swappable input and output pipelines for the Debix Model B.

## Quick Start

```bash
# 1. Build
make

# 2. Create a test video file
chmod +x create_test_video.sh
./create_test_video.sh

# 3. Run with test file: ./gst_pipeline_manager <input> <output> [input_path] [output_path]
./gst_pipeline_manager file_vpu display test.mp4
./gst_pipeline_manager file_vpu file_vpu test.mp4 output.mp4

# Or with camera (when attached)
./gst_pipeline_manager imx477 file_vpu "" output.mp4
./gst_pipeline_manager webcam display
```

## Available Pipelines

See [PIPELINES.md](PIPELINES.md) for all pipeline components.

**Inputs:**
- `file_vpu` - File with VPU decode (H.264)
- `imx477` - IMX477 MIPI camera
- `file_generic` - Generic file decoder
- `webcam` - Generic webcam

**Outputs:**
- `file_vpu` - File with VPU encode (H.264)
- `network_udp` - UDP/RTP stream (hardware)
- `file_generic` - Generic file (software encode)
- `display` - Wayland/KMS display
- `display_fb` - Framebuffer display

## Files

- `PIPELINES.md` - Pipeline components and examples
- `input.cpp` - C++ pipeline manager
- `Makefile` - Build configuration
- `test_pipelines.sh` - Test hardware availability
- `create_test_video.sh` - Create a test video file
