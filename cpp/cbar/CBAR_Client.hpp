//
//  CBAR_Client.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/30/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_Client_hpp
#define CBAR_Client_hpp

#include <stdio.h>
#include <cambrian.h>
#include <cbar/pipeline/CBP_Types.hpp>

namespace cbar {
    class DLL_PUBLIC CBAR_Client {
    public:
        virtual ToolMode getToolMode() = 0;
        virtual void setToolMode(ToolMode toolMode) = 0;
        
        virtual double getBrushRadius() = 0;
        virtual void setBrushRadius(double brushRadius) = 0;
        
        virtual double getMinBrushRadius() { return 5.0; };
        virtual double getMaxBrushRadius() { return 50.0; };
    };
}

#endif /* CBAR_Client_hpp */
