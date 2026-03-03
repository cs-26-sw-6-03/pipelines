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
        // 1. File input with VPU decode (H.264)
        inputs["file_vpu"] = 
            "filesrc location=input.mp4 ! "
            "qtdemux ! h264parse ! vpudec ! video/x-raw";
        
        // 2. Camera IMX477 (MIPI)
        inputs["imx477"] = 
            "v4l2src device=/dev/video0 ! "
            "video/x-raw,width=1920,height=1080,framerate=30/1,format=NV12";
        
        // 3. Generic file input
        inputs["file_generic"] = 
            "filesrc location=input.mp4 ! "
            "decodebin ! videoconvert ! video/x-raw";
        
        // 4. Generic webcam
        inputs["webcam"] = 
            "v4l2src device=/dev/video0 ! "
            "videoconvert ! video/x-raw";
    }
    
    void initializeOutputs() {
        // 1. File output with VPU encoding (H.264)
        outputs["file_vpu"] = 
            "videoconvert ! "
            "vpuenc_h264 bitrate=5000 gop-size=30 ! "
            "h264parse ! qtmux ! "
            "filesink location=output.mp4";
        
        // 2. Network UDP/RTP with hardware encoding
        outputs["network_udp"] = 
            "videoconvert ! "
            "vpuenc_h264 bitrate=3000 gop-size=30 ! "
            "h264parse ! rtph264pay config-interval=1 pt=96 ! "
            "udpsink host=192.168.1.100 port=5000";
        
        // 3. Generic file output (software encode)
        outputs["file_generic"] = 
            "videoconvert ! x264enc bitrate=5000 ! "
            "h264parse ! qtmux ! "
            "filesink location=output.mp4";
        
        // 4. Display output (Wayland/KMS)
        outputs["display"] = 
            "kmssink";
        
        // 4b. Display output (framebuffer fallback)
        outputs["display_fb"] = 
            "videoconvert ! video/x-raw,format=RGB16 ! "
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
        std::cout << "  " << argv[0] << " file_vpu display        # File to display (VPU decode)" << std::endl;
        std::cout << "  " << argv[0] << " imx477 file_vpu        # Camera to file (VPU encode)" << std::endl;
        std::cout << "  " << argv[0] << " file_vpu network_udp   # File to network stream" << std::endl;
        std::cout << "  " << argv[0] << " webcam display         # Webcam to display" << std::endl;
        std::cout << "  " << argv[0] << " file_vpu file_vpu input.mp4 output.mp4  # Transcode" << std::endl;
        
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
