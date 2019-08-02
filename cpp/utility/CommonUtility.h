//
//  CommonUtility.h
//  Cambrian
//
//  Created by Joel Teply on 11/3/13.
//
//

#ifndef __Cambrian__CommonUtility__
#define __Cambrian__CommonUtility__

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <cstdio>
#include <string>
#include <cassert>
#include <cambrian.h>

#include <cbcommon/CB_Types.h>

std::string time_to_string(time_t rawtime, std::string format=std::string("%a %b %d %Y"));

int64_t sys_usec_time();
float usec_to_seconds(int64_t usec);
int64_t seconds_to_usec(float seconds);
float seconds_elapsed(int64_t startUsec, int64_t endUsec=sys_usec_time());

std::string & trim(std::string & str);
std::string & ltrim(std::string & str);
std::string & rtrim(std::string & str);

std::string trim_copy(std::string const & str);

std::string newUUID();

template< typename... Args >
std::string string_sprintf( const char* format, Args... args ) {
    int length = snprintf( nullptr, 0, format, args... );
    assert( length >= 0 );
    
    char* buf = new char[length + 1];
    snprintf( buf, length + 1, format, args... );
    
    std::string str( buf );
    delete[] buf;
    return str;
}

void strReplaceAll(std::string& str, const std::string& from, const std::string& to);

const char * touchStepToString(TouchStep step);

#endif /* defined(__Cambrian__CommonUtility__) */
