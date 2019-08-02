//
//  JSONHelper.h
//  Cambrian
//
//  Created by Joel Teply on 11/13/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//

#ifndef __Cambrian__JSONHelper__
#define __Cambrian__JSONHelper__

#include <stdio.h>

#include <cambrian.h>
#include <opencv2/core/core.hpp>
#include <imaging/LineProcessing.h>
#include <json/json.h>

#if NDEBUG
#   define FORMAT_OUTPUT 0
#else
#   define FORMAT_OUTPUT 1
#endif

class DLL_PUBLIC JSONHelper {
public:
    
    static Json::Value createScalarJSONValue(const cv::Scalar &scalar);
    static cv::Scalar getScalarJSONValue(const Json::Value &scalarValue);

    static Json::Value createCVPointJSONValue(const cv::Point2f &point);
    static cv::Point2f getCVPointJSONValue(const Json::Value &pointValue);
    
    static void fillJSONArray(Json::Value &arrayNode, const float *data, float length);
    static void parseJSONArray(const Json::Value &arrayNode, float *data);
    
    static bool parseJSONFile(const std::string &path, Json::Value &rootNode, std::string *errors=nullptr);
    static bool parseJSONString(const std::string &jsonString, Json::Value &rootNode, std::string *errors=nullptr);
    static std::string jsonToString(const Json::Value &node);
    
};

#endif /* defined(__Cambrian__JSONHelper__) */
