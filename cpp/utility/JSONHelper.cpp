//
//  JSONHelper.cpp
//  Cambrian
//
//  Created by Joel Teply on 11/13/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//

#include "JSONHelper.h"

#include <fstream>
#include <json/json.h>
#include <sstream>
#include "Directory.h"

//https://github.com/open-source-parsers/jsoncpp/wiki

Json::Value JSONHelper::createScalarJSONValue(const cv::Scalar &scalar) {
    Json::Value scalarNode;
    for (int i=0; i<4; i++) {
        scalarNode.append(scalar[i]);
    }
    
    return scalarNode;
}

cv::Scalar JSONHelper::getScalarJSONValue(const Json::Value &scalarValue) {
    cv::Scalar scalar;
    for (int i=0; i<4; i++) {
        scalar[i] = scalarValue[i].asDouble();
    }
    return scalar;
}

Json::Value JSONHelper::createCVPointJSONValue(const cv::Point2f &point) {
    Json::Value scalarNode;
    scalarNode.append(point.x);
    scalarNode.append(point.y);
    return scalarNode;
}

cv::Point2f JSONHelper::getCVPointJSONValue(const Json::Value &pointValue)
{
    cv::Point2f point;
    point.x = pointValue[0].asDouble();
    point.y = pointValue[1].asDouble();
    return point;
}

bool JSONHelper::parseJSONFile(const std::string &path, Json::Value &rootNode, std::string *errors) {
    std::ifstream documentStream(path, std::ifstream::binary);
    
    Json::CharReaderBuilder rbuilder;
    return Json::parseFromStream(rbuilder, documentStream, &rootNode, errors);
}

bool JSONHelper::parseJSONString(const std::string &jsonString, Json::Value &rootNode, std::string *errors) {
    
    std::stringstream stream;
    stream << jsonString;
    
    Json::CharReaderBuilder rbuilder;
    return Json::parseFromStream(rbuilder, stream, &rootNode, errors);
}

std::string JSONHelper::jsonToString(const Json::Value &node) {
    
    std::ostringstream stream;
    stream << node;
    return stream.str();
}

void JSONHelper::fillJSONArray(Json::Value &arrayNode, const float *data, float length) {
    for (int i=0; i<length; i++) {
        arrayNode.append(data[i]);
    }
}

void JSONHelper::parseJSONArray(const Json::Value &arrayNode, float *data) {
    for (int i=0; i<arrayNode.size(); i++) {
        data[i] = arrayNode[i].asFloat();
    }
}
