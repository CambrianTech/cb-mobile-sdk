//
//  CBAR_GLOffscreenRenderer.hpp
//  Cambrian
//
//  Created by Joel Teply on 8/28/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_GLOffscreenRenderer_hpp
#define CBAR_GLOffscreenRenderer_hpp

#include <memory>
#include <stdio.h>
#include <cambrian.h>
#include <cbar/CBAR_Common.hpp>

namespace cbar {
    
    class DLL_PUBLIC CBAR_GLOffscreenRenderer {
        
    public:
        CBAR_GLOffscreenRenderer();
        ~CBAR_GLOffscreenRenderer();
        
        GLuint create(GLuint width, GLuint height, GLuint srcTarget=GL_TEXTURE_2D, bool isYUV=false);
        void destroy();
        
        void render(GLuint srcTexture0, GLuint srcTexture1=0);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};

#endif /* CBAR_GLOffscreenRenderer_hpp */

