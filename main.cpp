#include <gst/gst.h>
#include <iostream>
#include <string>
#include <map>

/**
 * GStreamer Pipeline Manager for Debix Model B (i.MX8M Plus)
 * Swappable input and output pipelines using hardware acceleration
 */

class GStreamerPipelineManager {
private:
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;
    
    // Pipeline component strings
    std::map<std::string, std::string> inputs;
    std::map<std::string, std::string> outputs;

public:
    GStreamerPipelineManager() : pipeline(nullptr), bus(nullptr), msg(nullptr) {
        // Initialize GStreamer
        gst_init(nullptr, nullptr);
        
        // Define input pipelines
        initializeInputs();
        
        // Define output pipelines
        initializeOutputs();
    }
    
    ~GStreamerPipelineManager() {
        cleanup();
    }
    
    void initializeInputs() {
        // 1. Video test source
        inputs["test_1080p"] = 
            "videotestsrc pattern=0 ! "
            "video/x-raw,width=1920,height=1080,framerate=30/1,format=I420 ! "
            "imxvideoconvert_g2d ! video/x-raw,format=NV12";
        
        inputs["test_4k"] = 
            "videotestsrc pattern=0 ! "
            "video/x-raw,width=3840,height=2160,framerate=30/1,format=I420 ! "
            "imxvideoconvert_g2d ! video/x-raw,format=NV12";
        
        // 2. Arducam IMX477
        inputs["imx477_1080p"] = 
            "v4l2src device=/dev/video0 ! "
            "video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12";
        
        inputs["imx477_4k"] = 
            "v4l2src device=/dev/video0 ! "
            "video/x-raw,width=3840,height=2160,framerate=30/1,format=NV12";
        
        inputs["imx477_1080p_60fps"] = 
            "v4l2src device=/dev/video0 ! "
            "video/x-raw,width=1920,height=1080,framerate=60/1,format=NV12";
        
        // 3. Generic camera
        inputs["generic_camera"] = 
            "v4l2src device=/dev/video0 ! "
            "video/x-raw,width=1920,height=1080,framerate=30/1 ! "
            "imxvideoconvert_g2d ! video/x-raw,format=NV12";
        
        // 4. File input - H.264
        inputs["file_h264"] = 
            "filesrc location=input.mp4 ! "
            "qtdemux ! h264parse ! vpudec_h264 ! "
            "imxvideoconvert_g2d ! video/x-raw,format=NV12";
        
        // 4. File input - H.265
        inputs["file_h265"] = 
            "filesrc location=input.mp4 ! "
            "qtdemux ! h265parse ! vpudec_h265 ! "
            "imxvideoconvert_g2d ! video/x-raw,format=NV12";
    }
    
    void initializeOutputs() {
        // 1. File output with VPU encoding - H.264
        outputs["file_h264_mp4"] = 
            "vpuenc_h264 bitrate=5000 gop-size=30 ! "
            "h264parse ! qtmux ! "
            "filesink location=output.mp4";
        
        outputs["file_h264_mkv"] = 
            "vpuenc_h264 bitrate=5000 gop-size=30 ! "
            "h264parse ! matroskamux ! "
            "filesink location=output.mkv";
        
        // 1. File output with VPU encoding - H.265
        outputs["file_h265_mp4"] = 
            "vpuenc_h265 bitrate=4000 gop-size=30 ! "
            "h265parse ! qtmux ! "
            "filesink location=output.mp4";
        
        outputs["file_h265_mkv"] = 
            "vpuenc_h265 bitrate=4000 gop-size=30 ! "
            "h265parse ! matroskamux ! "
            "filesink location=output.mkv";
        
        // 2. UDP streaming - H.264 RTP
        outputs["udp_h264_rtp"] = 
            "vpuenc_h264 bitrate=3000 gop-size=30 ! "
            "h264parse ! rtph264pay config-interval=1 pt=96 ! "
            "udpsink host=192.168.1.100 port=5000";
        
        // 2. UDP streaming - H.265 RTP
        outputs["udp_h265_rtp"] = 
            "vpuenc_h265 bitrate=2500 gop-size=30 ! "
            "h265parse ! rtph265pay config-interval=1 pt=96 ! "
            "udpsink host=192.168.1.100 port=5000";
        
        // 2. UDP streaming - MPEG-TS
        outputs["udp_mpegts"] = 
            "vpuenc_h264 bitrate=3000 gop-size=30 ! "
            "h264parse ! mpegtsmux ! "
            "udpsink host=192.168.1.100 port=5000";
        
        // 3. Display to screen - KMS (preferred)
        outputs["display_kms"] = 
            "kmssink";
        
        outputs["display_kms_fullscreen"] = 
            "kmssink fullscreen=true";
        
        // 3. Display to screen - Framebuffer (fallback)
        outputs["display_fb"] = 
            "imxvideoconvert_g2d ! video/x-raw,format=RGB16 ! "
            "fbdevsink device=/dev/fb0";
    }
    
    bool createPipeline(const std::string& inputName, const std::string& outputName) {
        // Check if input and output exist
        if (inputs.find(inputName) == inputs.end()) {
            std::cerr << "Error: Input '" << inputName << "' not found" << std::endl;
            return false;
        }
        
        if (outputs.find(outputName) == outputs.end()) {
            std::cerr << "Error: Output '" << outputName << "' not found" << std::endl;
            return false;
        }
        
        // Build pipeline string
        std::string pipelineStr = inputs[inputName] + " ! " + outputs[outputName];
        
        std::cout << "Creating pipeline: " << pipelineStr << std::endl;
        
        // Create pipeline
        GError *error = nullptr;
        pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
        
        if (error) {
            std::cerr << "Failed to create pipeline: " << error->message << std::endl;
            g_error_free(error);
            return false;
        }
        
        return true;
    }
    
    bool createCustomPipeline(const std::string& inputName, const std::string& outputName,
                             const std::string& customInputPath, const std::string& customOutputPath) {
        // Check if input and output exist
        if (inputs.find(inputName) == inputs.end()) {
            std::cerr << "Error: Input '" << inputName << "' not found" << std::endl;
            return false;
        }
        
        if (outputs.find(outputName) == outputs.end()) {
            std::cerr << "Error: Output '" << outputName << "' not found" << std::endl;
            return false;
        }
        
        // Replace paths in pipeline strings
        std::string inputStr = inputs[inputName];
        std::string outputStr = outputs[outputName];
        
        if (!customInputPath.empty()) {
            // Replace default input path with custom
            size_t pos = inputStr.find("location=");
            if (pos != std::string::npos) {
                size_t endPos = inputStr.find(" ", pos);
                inputStr.replace(pos + 9, endPos - (pos + 9), customInputPath);
            }
        }
        
        if (!customOutputPath.empty()) {
            // Replace default output path with custom
            size_t pos = outputStr.find("location=");
            if (pos != std::string::npos) {
                size_t endPos = outputStr.find(" ", pos);
                if (endPos == std::string::npos) endPos = outputStr.length();
                outputStr.replace(pos + 9, endPos - (pos + 9), customOutputPath);
            }
            
            // Replace default host/port for UDP
            pos = outputStr.find("host=");
            if (pos != std::string::npos && customOutputPath.find(":") != std::string::npos) {
                // Assume format "host:port"
                size_t colonPos = customOutputPath.find(":");
                std::string host = customOutputPath.substr(0, colonPos);
                std::string port = customOutputPath.substr(colonPos + 1);
                
                size_t endPos = outputStr.find(" ", pos);
                outputStr.replace(pos + 5, endPos - (pos + 5), host);
                
                pos = outputStr.find("port=");
                if (pos != std::string::npos) {
                    endPos = outputStr.find(" ", pos);
                    if (endPos == std::string::npos) endPos = outputStr.length();
                    outputStr.replace(pos + 5, endPos - (pos + 5), port);
                }
            }
        }
        
        // Build pipeline string
        std::string pipelineStr = inputStr + " ! " + outputStr;
        
        std::cout << "Creating custom pipeline: " << pipelineStr << std::endl;
        
        // Create pipeline
        GError *error = nullptr;
        pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
        
        if (error) {
            std::cerr << "Failed to create pipeline: " << error->message << std::endl;
            g_error_free(error);
            return false;
        }
        
        return true;
    }
    
    void start() {
        if (!pipeline) {
            std::cerr << "Error: No pipeline created" << std::endl;
            return;
        }
        
        std::cout << "Starting pipeline..." << std::endl;
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
    }
    
    void run() {
        if (!pipeline) {
            std::cerr << "Error: No pipeline created" << std::endl;
            return;
        }
        
        bus = gst_element_get_bus(pipeline);
        
        std::cout << "Pipeline running. Press Ctrl+C to stop." << std::endl;
        
        // Wait for EOS or error
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                         static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        
        // Parse message
        if (msg != nullptr) {
            GError *err;
            gchar *debug_info;
            
            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    std::cerr << "Error: " << err->message << std::endl;
                    std::cerr << "Debug: " << debug_info << std::endl;
                    g_clear_error(&err);
                    g_free(debug_info);
                    break;
                case GST_MESSAGE_EOS:
                    std::cout << "End of stream" << std::endl;
                    break;
                default:
                    // Should not reach here
                    std::cerr << "Unexpected message" << std::endl;
                    break;
            }
            gst_message_unref(msg);
        }
    }
    
    void stop() {
        if (pipeline) {
            std::cout << "Stopping pipeline..." << std::endl;
            gst_element_set_state(pipeline, GST_STATE_NULL);
        }
    }
    
    void cleanup() {
        if (pipeline) {
            gst_object_unref(pipeline);
            pipeline = nullptr;
        }
        if (bus) {
            gst_object_unref(bus);
            bus = nullptr;
        }
    }
    
    void listInputs() {
        std::cout << "\nAvailable Input Pipelines:" << std::endl;
        for (const auto& pair : inputs) {
            std::cout << "  - " << pair.first << std::endl;
        }
    }
    
    void listOutputs() {
        std::cout << "\nAvailable Output Pipelines:" << std::endl;
        for (const auto& pair : outputs) {
            std::cout << "  - " << pair.first << std::endl;
        }
    }
};

// Example usage
int main(int argc, char *argv[]) {
    GStreamerPipelineManager manager;
    
    if (argc < 3) {
        std::cout << "GStreamer Pipeline Manager for Debix Model B (i.MX8M Plus)" << std::endl;
        std::cout << "\nUsage: " << argv[0] << " <input> <output> [input_path] [output_path]" << std::endl;
        
        manager.listInputs();
        manager.listOutputs();
        
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  " << argv[0] << " test_1080p display_kms" << std::endl;
        std::cout << "  " << argv[0] << " imx477_1080p file_h264_mp4" << std::endl;
        std::cout << "  " << argv[0] << " imx477_4k udp_h264_rtp" << std::endl;
        std::cout << "  " << argv[0] << " file_h264 display_kms input.mp4" << std::endl;
        std::cout << "  " << argv[0] << " generic_camera file_h264_mp4 \"\" output.mp4" << std::endl;
        std::cout << "  " << argv[0] << " imx477_1080p udp_h264_rtp \"\" 192.168.1.100:5000" << std::endl;
        
        return 1;
    }
    
    std::string inputName = argv[1];
    std::string outputName = argv[2];
    std::string inputPath = (argc > 3) ? argv[3] : "";
    std::string outputPath = (argc > 4) ? argv[4] : "";
    
    bool success;
    if (inputPath.empty() && outputPath.empty()) {
        success = manager.createPipeline(inputName, outputName);
    } else {
        success = manager.createCustomPipeline(inputName, outputName, inputPath, outputPath);
    }
    
    if (!success) {
        return 1;
    }
    
    manager.start();
    manager.run();
    manager.stop();
    
    return 0;
}
