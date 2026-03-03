#!/bin/bash
# Test hardware availability for i.MX8M Plus

echo "========================================"
echo "i.MX8M Plus GStreamer Hardware Check"
echo "========================================"

# GStreamer installed?
if ! command -v gst-launch-1.0 &> /dev/null; then
    echo "✗ GStreamer not found"
    exit 1
fi
echo "✓ GStreamer installed"

# VPU encoder
if gst-inspect-1.0 vpuenc_h264 &> /dev/null; then
    echo "✓ VPU encoder (vpuenc_h264)"
else
    echo "✗ VPU encoder NOT found"
fi

# VPU decoder
if gst-inspect-1.0 vpudec &> /dev/null; then
    echo "✓ VPU decoder (vpudec)"
else
    echo "✗ VPU decoder NOT found"
fi

# G2D converter
if gst-inspect-1.0 imxvideoconvert_g2d &> /dev/null; then
    echo "✓ G2D converter (imxvideoconvert_g2d)"
else
    echo "✗ G2D converter NOT found"
fi

# Cameras
echo ""
echo "Cameras:"
if [ -e /dev/video0 ]; then
    v4l2-ctl --list-devices
else
    echo "  No cameras found"
fi

echo ""
echo "========================================"
