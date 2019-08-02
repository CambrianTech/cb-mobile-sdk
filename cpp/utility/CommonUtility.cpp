//
//  CommonUtility.cpp
//  Cambrian
//
//  Created by Joel Teply on 11/3/13.
//
//

#include "CommonUtility.h"
#include <time.h>
#include <sys/time.h>
#include <algorithm>
#include <random>
#include <chrono>
#include "Threads.h"

#define MILLION 1000000

std::string
time_to_string(time_t rawtime, std::string format)
{
    struct tm * timeinfo;
    char buffer[80];
    
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(buffer,80, format.c_str(), timeinfo);
    return buffer;
}

int64_t
sys_usec_time()
{
    timeval st_t;
    gettimeofday(&st_t, 0);
    return int64_t(st_t.tv_sec) * MILLION + st_t.tv_usec;

}

float seconds_elapsed(int64_t startUsec, int64_t endUsec) {
    return usec_to_seconds(endUsec - startUsec);
}

float usec_to_seconds(int64_t usec) {
    return float(float(usec) / float(MILLION));
}

int64_t seconds_to_usec(float seconds) {
    return int64_t(seconds * float(MILLION));
}

std::string trim_copy(std::string const & str) {
    auto s = str;
    return ltrim(rtrim(s));
}

std::string & trim(std::string & str)
{
    return ltrim(rtrim(str));
}

std::string & ltrim(std::string & str)
{
    auto it2 =  std::find_if( str.begin() , str.end() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( str.begin() , it2);
    return str;
}

std::string & rtrim(std::string & str)
{
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
}

union uuid
{
    unsigned char bytes[16];
    unsigned int quads[4];
    struct
    {
        unsigned int a;
        unsigned short int b;
        unsigned short int c;
        unsigned short int d;
        unsigned short int e0;
        unsigned int e1;
    } prettyprint;
} a_uuid;

static std::default_random_engine m_uuidGenerator;
static bool m_uuidGeneratorSeeded = false;
static CBMutex m_uuidGeneratorSeedMutex;

std::string newUUID() {
    m_uuidGeneratorSeedMutex.lock();
    if (!m_uuidGeneratorSeeded) {
        m_uuidGenerator.seed(std::chrono::system_clock::now().time_since_epoch().count());
        m_uuidGeneratorSeeded = true;
    }
    m_uuidGeneratorSeedMutex.unlock();
    
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<unsigned int>::max());
    
    a_uuid.quads[0] = distribution(m_uuidGenerator);
    a_uuid.quads[1] = distribution(m_uuidGenerator);
    a_uuid.quads[2] = distribution(m_uuidGenerator);
    a_uuid.quads[3] = distribution(m_uuidGenerator);
    
    std::string uuidStr;
    
    uuidStr = string_sprintf("%08X-%04hX-%04hX-%04hX-%04hX%08X",
            a_uuid.prettyprint.a, a_uuid.prettyprint.b, a_uuid.prettyprint.c,
            a_uuid.prettyprint.d, a_uuid.prettyprint.e0, a_uuid.prettyprint.e1);
    
    //printf("uuid=%s\n", uuidStr.c_str());
    
    return uuidStr;
}

void strReplaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    
    size_t start_pos = 0;
    
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

const char * touchStepToString(TouchStep step) {
    switch (step) {
        case TouchStepBegan:
            return "TouchStepBegan";
        case TouchStepEnded:
            return "TouchStepEnded";
        case TouchStepMoved:
            return "TouchStepMoved";
        case TouchStepCancelled:
            return "TouchStepCancelled";
        case TouchStepTapped:
            return "TouchStepTapped";
    }
}
