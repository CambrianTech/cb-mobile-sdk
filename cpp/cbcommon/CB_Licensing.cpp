//
//  CB_Licensing.cpp
//  Cambrian
//
//  Created by Joel Teply on 11/3/13.
//
//
#include "CB_Licensing.h"
#include "CB_Config.h"

#include <time.h>
#include <utility/CommonUtility.h>
#include <utility/Diagnostics.h>

namespace cb {
    static bool _attemptedLicensing = false;
    static CB_Licensing::license_info _licenseInfo;
    
    static std::vector<CB_Licensing::license_info>getValidKeys() {
        static std::vector<CB_Licensing::license_info> keys;
        
        //unix get date 60 days in the future as timestamp: date -j -v+60d +%s
        //+60d adds 60 days
        //leave off +%s for human readable
        
        if (!keys.size()) {
            //full keys should start with f and trials with 0
            
            //Test expired key:
            keys.push_back(CB_Licensing::license_info("Expired Test", "581831c75bea4e778e4a829772508d26", "com.whatever", 1380906601));
            
            //Cambrian ALL
            keys.push_back(CB_Licensing::license_info("Cambrian", "f81b040d45bc43c688326e13b9877904", "com.cambriantech", "home-decorator", 0));
            
            //CLIENTS

            //Crown Paints:
            keys.push_back(CB_Licensing::license_info("Crown Paints", "f8debe7c3f2c4fd6958263c2306ed907",
                                                      "uk.co.crownpaints.myroompainter",
                                                      "uk.co.crownpaints.myroompainteramazon", 0));
            
            //Lowe's
            keys.push_back(CB_Licensing::license_info("Lowe's", "c81ff4999523421882672c2f5d468a75", "com.viewar.lowes", 0));
            
            //Arbiec - Commex, expires never
            keys.push_back(CB_Licensing::license_info("Comex Group", "571579bc32ed4761a84ae8375b12a977", "com.comex.colorlife", 0));
            
            //PPG, expires never
            keys.push_back(CB_Licensing::license_info("PPG Inovaworks", "65d561095e324d07bd1ce267d4472ee5",
                                                      "com.inovaworks.ppg.icolorv2", 0));
            
            //Home Depot, expires never
            keys.push_back(CB_Licensing::license_info("Home Depot", "581aa1c75aea4e77ae4a829772508a26",
                                                      "com.thehomedepot.coloryourworld", 0));
            
            //NEW CLIENT, expires Fri Jul 10 12:10:31 CDT 2015
            keys.push_back(CB_Licensing::license_info("NEW CLIENT", "76d5b1095e424d07bd1ce267d4472ee5",
                                                      "com.client-name",
                                                      "com.yourcompany.mypaintharmony", 1475249765));
        }

        return keys;
    }
    
    static CB_Licensing::license_status getLicenseStatus(CB_Licensing::license_info info) {
        if (info.key.length()) {
            if (info.key.length() == 32) {
                //check date
                if (info.end_time == 0) {
                    return CB_Licensing::license_status_full;
                }
                time_t current_time = time(0);
                if (current_time < info.end_time) {
                    return CB_Licensing::license_status_trial_current;
                } else {
                    printf("CB trial license key '%s' expired %s",  info.key.c_str(),
                          time_to_string(info.end_time).c_str());
                    return CB_Licensing::license_status_trial_expired;
                }
            }
            CBError("CB license key '%s' is invalid", info.key.c_str());
            return CB_Licensing::license_status_invalid_key;
        }
        CBError("CB license key was not supplied. Call method enableWithKey before all operations.");
        return CB_Licensing::license_status_undefined_key;
    }
    
    
    static bool isBundleIDValid(std::string bundle, std::string testBundleID) {
        if (testBundleID.length() < 1 || bundle.length() < 1) return false;
        
        if (testBundleID.find("com.cambriantech.") == 0) {
            return true;//starts with com.cambriantech
        }
        if (testBundleID.find("home-decorator") == 0) {
            return true;//starts with home-decorator
        }
        if (bundle.compare(testBundleID) == 0) {
            return true;
        }
        
        return false;
    }
    
    static CB_Licensing::license_info getLicenseInfo(std::string key, std::string bundle) {
        std::vector<CB_Licensing::license_info> validKeys = getValidKeys();
        
        for (int i=0; i<validKeys.size(); i++) {
            CB_Licensing::license_info test_key = validKeys[i];
            if (test_key.key.compare(key) == 0) {
                test_key.status = getLicenseStatus(test_key);
                
                //for licensed copies, check the bundle
                if (test_key.status == CB_Licensing::license_status_full) {
                    bool isValid = false;
                    for (std::string bundleID : test_key.bundles) {
                        if (isBundleIDValid(bundleID, bundle)) {
                            isValid = true;
                        }
                    }
                    if (!isValid) {
                        test_key.status = CB_Licensing::license_status_invalid_bundle;
                    }
                }
                
                return test_key;
            }
        }
        
        return CB_Licensing::license_info();
    }
    
    CB_Licensing::license_info
    CB_Licensing::getCurrentLicenseInfo() {
        return _licenseInfo;
    }
    
    CB_Licensing::license_status CB_Licensing::enableWithKey(std::string key, std::string bundleID) {
        _licenseInfo = getLicenseInfo(key, bundleID);
        
        _attemptedLicensing = true;
        
#if DEBUG
        CBWrite("DEBUG BUILD Compiled with OpenCV v%d.%d.%d", CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_VERSION_REVISION);
#endif
        
        if (_licenseInfo.status == license_status_full) {
            CBWrite("%s SDK version %.4f for %s", API_NAME, double(API_VERSION), _licenseInfo.name.c_str());
        } else if (_licenseInfo.status == license_status_trial_current) {
            CBWrite("%s SDK TRIAL LICENSE version %.4f for %s", API_NAME, double(API_VERSION), _licenseInfo.name.c_str());
        } else if (_licenseInfo.status == license_status_invalid_bundle) {
            CBWrite("Bundle %s is invalid for this license", bundleID.c_str());
        }
        
        return _licenseInfo.status;
    }
    
    bool
    CB_Licensing::isEnabled() {
        if (!_licenseInfo.key.length() && !_attemptedLicensing) {
            CBError("CB license key was not supplied. Call method enableWithKey before all operations.");
            return false;
        }
        if (_licenseInfo.status < 10) {
            return true;
//            if (feature == f_enabled) {
//                return true;
//            } else {
//                //check feature
//                return (_licenseInfo.features & feature) > 0;
//            }
        }
        return false;
    }
    
    bool
    CB_Licensing::isTrialExpired() {
        return _licenseInfo.status == license_status_trial_expired;
    }
}
