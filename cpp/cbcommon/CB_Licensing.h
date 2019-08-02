//
//  CB_Licensing.h
//  Cambrian
//
//  Created by Joel Teply on 11/3/13.
//
//

#ifndef __Cambrian__CB_Licensing__
#define __Cambrian__CB_Licensing__

#include <iostream>
#include <vector>

namespace cb {
    class CB_Licensing {
    public:
        
        enum license_status {
            license_status_full = 0,
            license_status_trial_current = 1,
            license_status_trial_expired = 10,
            license_status_invalid_key = 11,
            license_status_undefined_key = 12,
            license_status_invalid_bundle = 13
        };
        
        struct license_info {
            std::string name;
            std::string key;
            std::vector<std::string> bundles;
            time_t end_time;
            license_status status;
            
            license_info() : status(license_status_undefined_key) {}
            license_info(std::string nm, std::string ky, std::string b1, time_t dt)
                : name(nm), key(ky), end_time(dt), status(license_status_undefined_key)
            {
                bundles.push_back(b1);
            }
            license_info(std::string nm, std::string ky, std::string b1, std::string b2, time_t dt)
                : name(nm), key(ky), end_time(dt), status(license_status_undefined_key)
            {
                bundles.push_back(b1);
                bundles.push_back(b2);
            }
            license_info(std::string nm, std::string ky, std::string b1, std::string b2, std::string b3, time_t dt)
                : name(nm), key(ky), end_time(dt), status(license_status_undefined_key)
            {
                bundles.push_back(b1);
                bundles.push_back(b2);
                bundles.push_back(b3);
            }
            license_info(std::string nm, std::string ky, std::vector<std::string>bs, time_t dt)
                : name(nm), key(ky), bundles(bs), end_time(dt), status(license_status_undefined_key) {}
        };
        
        static CB_Licensing::license_status enableWithKey(std::string key, std::string bundleID);
        static bool isEnabled();
        static bool isTrialExpired();
        static CB_Licensing::license_info getCurrentLicenseInfo();
    };
    
}

#endif /* defined(__Cambrian__CB_Licensing__) */
