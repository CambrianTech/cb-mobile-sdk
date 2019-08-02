//
//  Diagnostics.h
//  Cambrian
//
//  Created by Joel Teply on 11/19/12.
//
//

#ifndef __Cambrian__Diagnostics__
#define __Cambrian__Diagnostics__

#include <stdint.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cambrian.h>

//matches android_LogPriority
typedef enum CB_LOG_LEVEL
{
    CB_LOG_UNKNOWN = 0,
    CB_LOG_DEFAULT,    /* only for SetMinPriority() */
    CB_LOG_VERBOSE,
    CB_LOG_DEBUG,
    CB_LOG_INFO,
    CB_LOG_WARN,
    CB_LOG_ERROR,
    CB_LOG_FATAL,
    CB_LOG_SILENT,     /* only for SetMinPriority(); must be last */
} CB_LOG_LEVEL;

class DLL_PUBLIC Diagnostics
{
public:
    static bool isInitialized;
	static void Initialize(std::string path);
    
    static void DeviceLog(CB_LOG_LEVEL level,
                          std::string fmt,
                          std::string full_path,
                          std::string full_method,
                          int line, ...);
    
    static std::string DateTimeLabel(const char * separator, bool includeMilliseconds);
    static std::string DateTimeFileLabel();
    
    static void logPointVector(const std::vector<cv::Point>&pts);
    /**
     * Saves a diagnostic image to the given filename, in the directory specified in Initialize().
     */
    static std::string SaveDiagnosticMask(const cv::Mat &mask, const cv::Mat &image, std::string format, ...);
    static std::string SaveDiagnosticImage(bool isBGR, const cv::Mat &image, std::string format, ...);
    static void SaveDiagnosticScreenshot(std::string format="screenshot", ...);
    
    static std::string getDiagnosticsDirectory() { return directoryPath; }
    
    static std::string getDiagnosticsPath(std::string format, ...);
    
    static void showImage(const std::string &name, const cv::Mat &image);

#ifdef __ANDROID__
    static bool fastCVStatus(int error, const std::string &message, bool showSuccess=false);
#endif
    
private:
    static std::string directoryPath;
};

class DiagnosticsTimer {

public:
    DiagnosticsTimer(std::string name);
    ~DiagnosticsTimer();
    
    void initEvent(std::string eventName);
    void printTimeTaken(std::string eventName="");
private:
    int64_t _startTime;
    std::string _name;
    std::map<std::string, int64_t> _events;
};

class FPSTimer {
    
public:
    FPSTimer(float logSeconds=3.0) { m_logSeconds = logSeconds; }
    ~FPSTimer() {}
    
    void tick();
    bool logFPS();
    
    float getFPS() { return m_fps; };
    int64_t getTicks() { return m_fpsAttempts; };
private:
    float m_logSeconds;
    int64_t m_fpsLastFrameTime = 0;
    int64_t m_fpsAttempts = 0;
    int64_t m_fpsTotalDuration = 0;
    float m_fps = 0;
    int64_t m_fpsLastLogTime = 0;
};

#define ERROR_BEGIN ""
#define ERROR_END ""

#if VERBOSE
#undef DO_LOGGING
#define DO_LOGGING 1
#undef DO_WRITING
#define DO_WRITING 1
#endif

#define imshow(name, image) Diagnostics::showImage(name, image);
#define waitKey(time) {}
#define destroyWindow(name) {}

#if IMAGE_LOGGING

#define LOG_IMAGE(image, name, ...) Diagnostics::SaveDiagnosticImage(false, image, name, __VA_ARGS__);

#define LOG_MASK(mask, image, name) Diagnostics::SaveDiagnosticMask(mask, image, name);

#else

#define LOG_IMAGE(image, name, ...)
#define LOG_MASK(image, name)

#endif

#define CBAssert(condition, fmt, ...) \
if (!(condition)) { \
    Diagnostics::DeviceLog(CB_LOG_ERROR, \
    fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, \
    ##__VA_ARGS__); \
    assert(0); \
}

#define CBError(fmt, ...) Diagnostics::DeviceLog(CB_LOG_ERROR, \
    fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, \
    ##__VA_ARGS__)

#if DO_WRITING || DO_LOGGING
#   define CBWrite(fmt, ...) Diagnostics::DeviceLog(CB_LOG_INFO, \
        fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, \
        ##__VA_ARGS__)
#else
#   define CBWrite(fmt, ...) {}
#endif //DO_WRITING || DO_LOGGING


//TIMING
#if DO_LOGGING

#   define CBLog(fmt, ...) Diagnostics::DeviceLog(CB_LOG_DEBUG, \
        fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, \
        ##__VA_ARGS__)

#   define CBTimer() DiagnosticsTimer timer(__PRETTY_FUNCTION__)
#   define CBTimerPoint(name) timer.printTimeTaken(name)

#   define CBAvgTimeStart() size_t start = sys_usec_time();
#   define CBAvgTimePrint(name) { \
        int64_t duration = sys_usec_time() - start; \
        static int attempts = 0; \
        static size_t total = 0; \
        total += duration; \
        attempts ++; \
        float timeTaken = usec_to_seconds(total / attempts); \
        CBLog("%s import took %f seconds or %d fps", name, timeTaken, int(1.0f / timeTaken)); }
#else

#   define CBLog(fmt, ...) {}
#   define CBTimer() {}
#   define CBTimerPoint(name) {}

#   define CBAvgTimeStart()
#   define CBAvgTimePrint(name)

#endif //DO_LOGGING

#if VERBOSE

#   define CBVerbose(fmt, ...) Diagnostics::DeviceLog(CB_LOG_VERBOSE, \
        fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, \
        ##__VA_ARGS__)
#else

#   define CBVerbose(fmt, ...)

#endif

#endif /* defined(__Cambrian__Diagnostics__) */

