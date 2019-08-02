//
//  Diagnostics.cpp
//  Cambrian
//
//  Created by Joel Teply on 11/19/12.
//
//

#include "Diagnostics.h"
#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <imaging/Imaging.h>
#include <imaging/ImageProcessing.h>
#include <cbar/pipeline/rendering/rendering.h>
#include "CommonUtility.h"
#include "Directory.h"

#ifdef __ANDROID__
#   include <android/log.h>
#   include <fastcv/fastcv.h>
#elif __APPLE__
#   import <os/log.h>
#endif

std::string Diagnostics::directoryPath = "/sdcard/vpdata";
bool Diagnostics::isInitialized = false;

static std::string _getFileName(std::string path)
{
    std::string filename;
    
    //bool isObjC = full_method.find(".m") != std::string::npos;
    
    if (path.length()) {
        //Get the filename
        size_t fileStartPos = path.find_last_of("/\\");
        if (fileStartPos == std::string::npos) fileStartPos = 0;
        else fileStartPos ++;
        
        //printf("FOUND AT %li in %s\n", fileStartPos, path.c_str());
        
        size_t fileEndPos = path.find_last_of(".");
        if (fileEndPos == std::string::npos) fileEndPos = path.length();
        
        filename.assign(path.begin() + fileStartPos, path.begin() + fileEndPos);
    }
    
    return filename;
}

static std::string _getMethodCall(std::string full_method)
{
    bool isObjC = full_method.find("[") != std::string::npos;
    
    //Get the method
    size_t methodStartPos = 0;
    size_t methodEndPos = 0;// full_method.length() - 1;
    
    if (isObjC) {
        size_t objcMethodStartPos = full_method.find(" ");
        methodStartPos = objcMethodStartPos + 1;
        methodEndPos = full_method.find_first_of(":");
        if (methodEndPos == std::string::npos) {
            methodEndPos = full_method.find_first_of("]");
        }
    }
    else {
        //CPP
        methodEndPos = full_method.find_first_of("(");
        if (methodEndPos != std::string::npos) {
            size_t cppMethodStartPos = full_method.find_last_of("::", methodEndPos);
            methodStartPos = cppMethodStartPos + 1;
        } else {
            return full_method;
        }
    }
    
    std::string methodAbbrev;
    if (methodStartPos && methodEndPos) {
        methodAbbrev.assign(full_method.begin() + methodStartPos, full_method.begin() + methodEndPos);
    }
    
    return methodAbbrev;
}

void Diagnostics::Initialize(std::string path)
{
    isInitialized = true;
	directoryPath = path;
}

void Diagnostics::logPointVector(const std::vector<cv::Point>&points) {
    printf("std::vector<cv::Point> logged_points = {");
    for (const auto &point : points) {
        printf("cv::Point(%d, %d),", point.x, point.y);
    }
    printf("};\n");
}

#define SPRINTF_TRACE_BUFSIZE 4096

int limited_sprintf( char* dest, const char* fmt, ... )
{
    /* in threaded code use malloc(3) instead */
    static char trace_buf[SPRINTF_TRACE_BUFSIZE];
    
    va_list va;
    va_start( va, fmt );
    int rc = vsnprintf( trace_buf, SPRINTF_TRACE_BUFSIZE, fmt, va );
    
    assert( rc != -1 && rc < SPRINTF_TRACE_BUFSIZE );
    
    memcpy( dest, trace_buf, rc + 1 ); /* +1 for \0 terminator */
    return rc;
}

void Diagnostics::DeviceLog(CB_LOG_LEVEL level, std::string fmt, std::string path, std::string method, int line, ...)
{
    //Get time
    std::string dateTime = Diagnostics::DateTimeLabel(":", true);
    
    std::string filename = _getFileName(path);
    
    std::string methodAbbrev = _getMethodCall(method);
    
    //prevent buffer overrun
    char label [fmt.length() + 1024];

#if __ANDROID__
    snprintf(label, sizeof label, "[%s::%s:%i] %s\n", filename.c_str(), methodAbbrev.c_str(), line, fmt.c_str());
#else
    snprintf(label, sizeof label, "[%s %s::%s:%i] %s\n", dateTime.c_str(), filename.c_str(), methodAbbrev.c_str(), line, fmt.c_str());
#endif
    
    char* result =  new char[fmt.length() + 1024];
    
    va_list arglist;
    va_start( arglist, line );
    vsprintf(result, label, arglist);
    va_end( arglist );
    fflush(stdout);
    
#ifdef __ANDROID__
    __android_log_print(level, "cbar", result);
#elif __APPLE__
//    if (level >= CB_LOG_ERROR) {
//        os_log_fault(OS_LOG_DEFAULT, "%s", result);
//    }
//    else if (level >= CB_LOG_INFO
//        || (level == CB_LOG_DEBUG && os_log_type_enabled(OS_LOG_DEFAULT, OS_LOG_TYPE_DEBUG))) {
//        std::cout << result;
//    }
    
    std::cout << result;
#else
    std::cout << result;
#endif
    
    delete [] result;
    
}

static int lastSecond = -1;
static int lastMS = 0;

std::string Diagnostics::DateTimeLabel(const char * separator, bool includeMilliseconds)
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char dateTime [50];
    sprintf(dateTime, "%.02i%s%.02i%s%.02i",
            ltm->tm_hour, separator,  ltm->tm_min, separator, ltm->tm_sec);
    
    if (!includeMilliseconds) return std::string(dateTime);
    
    //TODO: Need to use the actual date, these are offset by some amount but time_t only returns seconds
    if (lastSecond == ltm->tm_sec) {
        lastMS ++;
        if (lastMS > 999) {
            lastMS = 0;
        }
    }
    
    char dateMilliTime [50];
    sprintf(dateMilliTime, "%s%s%.03i", dateTime, separator, lastMS);
    
    lastSecond = ltm->tm_sec;
    
    return std::string(dateMilliTime);
}

std::string Diagnostics::DateTimeFileLabel()
{
    std::istringstream test;
    test.ignore();
    return Diagnostics::DateTimeLabel(".", true);
}

std::string Diagnostics::SaveDiagnosticMask(const cv::Mat &mask, const cv::Mat &background, std::string format, ...)
{
    if (!mask.cols) return "";
    
    cv::Mat maskDisplay = cv::Mat::zeros(mask.rows, mask.cols, CV_8UC1);
    maskDisplay.setTo(cv::Scalar(255), mask);
    
    char filename[1024];
    va_list args;
    va_start (args, format);
    vsprintf(filename, format.c_str(), args);
    va_end (args);
    
    cv::Mat debug = background.clone();
    imaging::ImageProcessing::overlayMaskOntoRGB(maskDisplay, debug);
    
    return SaveDiagnosticImage(false, debug, filename);
}

void Diagnostics::showImage(const std::string &name, const cv::Mat &image) 
{  
    SaveDiagnosticImage(false, image, name.c_str());
}

std::string Diagnostics::getDiagnosticsPath(std::string format, ...)
{
    char filename[1024];
    va_list args;
    va_start (args, format);
    vsprintf(filename, format.c_str(), args);
    va_end (args);
    
    char enhancedFileName [1024];
    const char* suffix = "";
    
    sprintf(enhancedFileName, "%s/%s_%s%s", directoryPath.c_str(),
            Diagnostics::DateTimeFileLabel().c_str(), filename, suffix);
    
    return std::string(enhancedFileName);
}

std::map<std::string, int64_t>m_uploadTimes;

std::string Diagnostics::SaveDiagnosticImage(bool isBGR, const cv::Mat &image, std::string format, ...)
{
    
#if DISABLE_ALL_LOGGING
    return "";
#endif
    
    auto lastUploadTime = m_uploadTimes[format];
    auto timeElapsed = seconds_elapsed(lastUploadTime);
    
    if (timeElapsed < IMAGE_LOG_FREQUENCY) return "";
    
    m_uploadTimes[format] = sys_usec_time();
    
    char filename[1024];
    va_list args;
    va_start (args, format);
    vsprintf(filename, format.c_str(), args);
    va_end (args);
    
    char enhancedFileName [1024];
    const char* suffix = "";
    if (!strstr(filename, ".png") && !strstr(filename, ".jpg")) {
        suffix = ".png";
    }
    
    sprintf(enhancedFileName, "%s/%s_%s%s", directoryPath.c_str(),
            Diagnostics::DateTimeFileLabel().c_str(), filename, suffix);
    
    if (image.empty()) {
        CBWrite("Image %s was empty\n", enhancedFileName);
        return "";
    }
    
    Directory::ensure_parent_dir_exists(enhancedFileName);
    
    cv::Mat _image = image.clone();

    std::thread([isBGR, enhancedFileName, _image](){
        if (!isBGR && _image.channels() == 3) {
            cv::cvtColor(_image, _image, CV_RGB2BGR);
        } else if (!isBGR && _image.channels() == 4) {
            cv::cvtColor(_image, _image, CV_RGBA2BGRA);
        }
        
        CBWrite("Writing image (%dx%d) to %s\n", _image.cols, _image.rows, enhancedFileName);
        
        bool success = cv::imwrite(enhancedFileName, _image);
        
        if (!success) {
            CBError("Could not write image.");
            return;
        }
        
#if UPLOAD_LOGGED_IMAGES
        if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
            renderer->getCallback()->uploadDiagnosticImageAtPath(enhancedFileName);
        }
#endif
        
    }).detach();
    
    return enhancedFileName;
}

void Diagnostics::SaveDiagnosticScreenshot(std::string format, ...) {

    char filename[1024];
    va_list args;
    va_start (args, format);
    vsprintf(filename, format.c_str(), args);
    va_end (args);
    
    std::string path = Diagnostics::getDiagnosticsPath(filename);
    
    if (!strstr(path.c_str(), ".png")) {
        path += ".png";
    }
    
    //only PNG supported right now
    if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
        renderer->saveScreenshot(path, [renderer](bool success, const std::string &path) {
            if (success) {
                renderer->getCallback()->uploadDiagnosticImageAtPath(path);
            }
        });
    }
}

DiagnosticsTimer::DiagnosticsTimer(std::string name) {
    std::string method = _getMethodCall(name);
    _name = method.length() ? method : name;
    _startTime = sys_usec_time();
}

DiagnosticsTimer::~DiagnosticsTimer() {
    printTimeTaken();
}

void DiagnosticsTimer::initEvent(std::string eventName) {
    _events[eventName] = sys_usec_time();
}

void DiagnosticsTimer::printTimeTaken(std::string eventName)
{
#if DO_WRITING || DO_LOGGING
    int64_t currentTime = sys_usec_time();
    if (eventName.length()) {
        int64_t timeTaken = currentTime - _events[eventName] - _startTime;
        CBWrite("%s::%s has taken %.4f seconds\n", _name.c_str(), eventName.c_str(), usec_to_seconds(timeTaken));
    } else {
        int64_t timeTaken = currentTime - _startTime;
        CBWrite("%s completed after %.4f seconds\n", _name.c_str(), usec_to_seconds(timeTaken));
    }
#endif
}

#ifdef __ANDROID__

bool Diagnostics::fastCVStatus(int error, std::string message, bool showSuccess) {
    switch (error) {
        case FASTCV_SUCCESS:
            if (showSuccess) CBWrite("FASTCV SUCCESS::%s", message.c_str());
            return true;
            break;
        case FASTCV_EFAIL:
            CBWrite("FASTCV ERROR::%s General failure", message.c_str());
            break;
        case FASTCV_EUNALIGNPARAM:
            CBWrite("FASTCV ERROR::%s Unaligned pointer parameter", message.c_str());
            break;
        case FASTCV_EBADPARAM:
            CBWrite("FASTCV ERROR::%s Bad parameters", message.c_str());
            break;
        case FASTCV_EINVALSTATE:
            CBWrite("FASTCV ERROR::%s Called at invalid state", message.c_str());
            break;
        case FASTCV_ENORES:
            CBWrite("FASTCV ERROR::%s Insufficient resources, memory, thread...", message.c_str());
            break;
        case FASTCV_EUNSUPPORTED:
            CBWrite("FASTCV ERROR::%s Unsupported feature", message.c_str());
            break;
        case FASTCV_EHWQDSP:
            CBWrite("FASTCV ERROR::%s Hardware QDSP failed to respond", message.c_str());
            break;
        case FASTCV_EHWGPU:
            CBWrite("FASTCV ERROR::%s Hardware GPU failed to respond", message.c_str());
            break;
    }
    
    return false;
}

#endif

bool FPSTimer::logFPS() {
    if (usec_to_seconds(sys_usec_time() - m_fpsLastLogTime) > m_logSeconds) {
        m_fpsLastLogTime = sys_usec_time();
        return true;
    }
    return false;
}

void FPSTimer::tick() {

    if (m_fpsLastFrameTime) {
        int64_t duration = sys_usec_time() - m_fpsLastFrameTime;
        m_fpsTotalDuration += duration;
        m_fpsAttempts ++;
        float avgTime = usec_to_seconds(m_fpsTotalDuration / m_fpsAttempts);
        m_fps = 1.0f / avgTime;
        
        if (m_fpsAttempts >= 60) {
            m_fpsAttempts = 30;
            m_fpsTotalDuration = m_fpsAttempts * seconds_to_usec(avgTime);
        }
    }
    
    m_fpsLastFrameTime = sys_usec_time();
}
