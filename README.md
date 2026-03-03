# Simple GStreamer Pipelines for i.MX8M Plus

Simple, swappable input and output pipelines for the Debix Model B.

## Quick Start

```bash
# Build
make

# Run: ./gst_pipeline_manager <input> <output>
./gst_pipeline_manager file_vpu display
./gst_pipeline_manager imx477 file_vpu
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
