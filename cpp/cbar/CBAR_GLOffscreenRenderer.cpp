//
//  CBAR_GLOffscreenRenderer.cpp
//  Cambrian
//
//  Created by Joel Teply on 8/28/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_GLOffscreenRenderer.hpp"

#include <cbar/CBAR_Common.hpp>
#include <utility/ShaderUtilities.h>
#include <utility/Diagnostics.h>
#include <utility/CommonUtility.h>

namespace cbar {
    
    const GLchar *vertSrc = STRINGIZE
    (
     attribute vec4 position;
     attribute vec4 texturecoordinate;
     varying vec2 coordinate;
     
     void main() {
         gl_Position = position;
         coordinate = texturecoordinate.xy;
     }
     );
    
    const GLchar *fragRGBASrc = STRINGIZE
    (
     precision mediump float;
     varying vec2 coordinate;
     uniform sampler2D rgbaSrc;
     
     void main() {
         gl_FragColor = texture2D(rgbaSrc, coordinate);
     }
     );
    
    const GLchar *fragYUVSrc = STRINGIZE
    (
     precision mediump float;
     varying vec2 coordinate;
     uniform sampler2D lumSrc;
     uniform sampler2D chromaSrc;
     
     void main() {
         mediump vec3 yuv;
         yuv.x = texture2D(lumSrc, coordinate).r;
         yuv.yz = texture2D(chromaSrc, coordinate).rg - vec2(0.5, 0.5);
         
         gl_FragColor = vec4(mat3(1,1,1,
                                  0, -.21482, 2.12798,
                                  1.28033, -.38059,       0) * yuv, 1.0);
     }
     );
    
    enum {
        ATTRIB_VERTEX,
        ATTRIB_TEXTUREPOSITON,
        NUM_ATTRIBUTES
    };
    
    static const GLfloat squareVertices[] = {
        -1.0f, -1.0f, // bottom left
        1.0f, -1.0f, // bottom right
        -1.0f,  1.0f, // top left
        1.0f,  1.0f, // top right
    };
    
    static const float textureVertices[] = {
        0.0f, 0.0f, // bottom left
        1.0f, 0.0f, // bottom right
        0.0f,  1.0f, // top left
        1.0f,  1.0f, // top right
    };
    
    struct CBAR_GLOffscreenRenderer::Impl
    {
        Impl(CBAR_GLOffscreenRenderer *parent) : m_parent(parent)
        {
            
        }
        ~Impl() {
            destroy();
        }
        
        CBAR_GLOffscreenRenderer *m_parent;
        
        GLuint m_offscreenFramebufferHandle = 0;
        GLuint m_offscreenRenderbufferHandle = 0;
        GLuint m_offscreenRGBATextureHandle = 0;
        GLuint m_program = 0;
        GLuint m_frameSampler0 = 0;
        GLuint m_frameSampler1 = 0;
        GLuint m_target;
        GLuint m_width = 0, m_height = 0;
        
        bool m_isYUV = false;
        
        static const int VB_COUNT = NUM_ATTRIBUTES;
        GLuint m_vbo[VB_COUNT];
        
        GLuint m_vbState = 0;
        
        static const int TARGET_OES = 0x8D65;
        
        GLuint create(GLuint width, GLuint height, GLuint srcTarget, bool isYUV) {
            
            m_width = width;
            m_height = height;
            m_target = srcTarget;
            m_isYUV = isYUV;
            
            for (int i=0; i<VB_COUNT; i++) {
                m_vbo[i] = 0;
            }
            
            createShaderProgram();
            
            return createFBO();
        }
        
        bool createShaderProgram() {
            
            if (m_program) {
                destroyShaderProgram();
            }
            
            GLint attribLocation[NUM_ATTRIBUTES] = {
                ATTRIB_VERTEX, ATTRIB_TEXTUREPOSITON,
            };
            
            GLchar *attribName[NUM_ATTRIBUTES] = {
                (GLchar*)"position", (GLchar*)"texturecoordinate",
            };
            
            glueCreateProgram(m_target, vertSrc, m_isYUV ? fragYUVSrc : fragRGBASrc,
                              NUM_ATTRIBUTES, (const GLchar **)&attribName[0], attribLocation,
                              0, 0, 0,
                              &m_program);
            
            if (!m_program) {
                CBError("Problem initializing the program.");
                return false;
            }
            
            if (m_isYUV) {
                m_frameSampler0 = glueGetUniformLocation( m_program, "lumSrc" );
                m_frameSampler1 = glueGetUniformLocation( m_program, "chromaSrc" );
            } else {
                m_frameSampler0 = glueGetUniformLocation( m_program, "rgbaSrc" );
            }
            
            //create vbo's
            GL_CHECK(glGenBuffers(VB_COUNT, m_vbo));
            
            //triangle positions
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[ATTRIB_VERTEX]));
            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), &squareVertices, GL_STATIC_DRAW));
            
            //texture coordinates
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[ATTRIB_TEXTUREPOSITON]));
            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(textureVertices), &textureVertices, GL_STATIC_DRAW));
            
            //use efficient vertex arrays
            GL_CHECK(glGenVertexArrays(1, &m_vbState));
            GL_CHECK(glBindVertexArray(m_vbState));
            
            // Pass in the position information
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[ATTRIB_VERTEX]));
            GL_CHECK(glEnableVertexAttribArray(ATTRIB_VERTEX));
            GL_CHECK(glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, false, 0, 0));
            
            // Pass in the texture information
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo[ATTRIB_TEXTUREPOSITON]));
            GL_CHECK(glEnableVertexAttribArray(ATTRIB_TEXTUREPOSITON));
            GL_CHECK(glVertexAttribPointer(ATTRIB_TEXTUREPOSITON, 2, GL_FLOAT, false, 0, 0));
            
            //unbind
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
            GL_CHECK(glBindVertexArray( 0 ));
            
            return true;
        }
        
        void destroy() {
            destroyFBO();
            destroyShaderProgram();
        }
        
        void destroyShaderProgram() {
            
            if (m_program ) {
                glDeleteBuffers(VB_COUNT, m_vbo);
                glDeleteVertexArrays(1, &m_vbState);
                
                glDeleteProgram(m_program );
                m_program = 0;
            }
        }
        
        GLuint createFBO() {
            
            //get existing fb/rb
            GLint saveFramebuffer;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saveFramebuffer);
            GLint saveRenderbuffer;
            glGetIntegerv(GL_RENDERBUFFER_BINDING, &saveRenderbuffer);
            GLint saveTexName;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &saveTexName);
            
            if (m_offscreenFramebufferHandle) {
                destroyFBO();
            }
            
            //Create render buffers and frame buffers
            GL_CHECK(glGenFramebuffers( 1, &m_offscreenFramebufferHandle ));
            GL_CHECK(glBindFramebuffer( GL_FRAMEBUFFER, m_offscreenFramebufferHandle ));
            
            GL_CHECK(glGenRenderbuffers(1, &m_offscreenRenderbufferHandle));
            GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, m_offscreenRenderbufferHandle));
            GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height));
            GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_offscreenRenderbufferHandle));
            
            //create offscreen texture to render to
            GL_CHECK(glGenTextures(1, &m_offscreenRGBATextureHandle));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_offscreenRGBATextureHandle));
            
            GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
            GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_offscreenRGBATextureHandle, 0));
            
            if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ) {
                CBError("Failure with framebuffer generation" );
            }
            
            //restore fb/rb
            GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, saveFramebuffer));
            GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, saveRenderbuffer));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, saveTexName));
            
            return m_offscreenRGBATextureHandle;
        }
        
        void destroyFBO() {
            if (m_offscreenFramebufferHandle) {
                glDeleteFramebuffers(1, &m_offscreenFramebufferHandle);
                m_offscreenFramebufferHandle = 0;
            }
            
            if (m_offscreenRenderbufferHandle) {
                glDeleteRenderbuffers(1, &m_offscreenRenderbufferHandle);
                m_offscreenRenderbufferHandle = 0;
            }
            
            if (m_offscreenRGBATextureHandle) {
                glDeleteTextures(1, &m_offscreenRGBATextureHandle);
                m_offscreenRGBATextureHandle = 0;
            }
        }
        
        void render(GLuint srcTexture0, GLuint srcTexture1) {
            // Set the view port to the entire view
            GL_CHECK(glBindFramebuffer( GL_FRAMEBUFFER, m_offscreenFramebufferHandle ));
            GL_CHECK(glViewport( 0, 0, m_width, m_height ));
            GL_CHECK(glUseProgram( m_program ));
            GL_CHECK(glActiveTexture( GL_TEXTURE0 ));
            
            GL_CHECK(glBindTexture(m_target, srcTexture0));
            GL_CHECK(glUniform1i( m_frameSampler0, 0 ));
            
            // Set texture parameters
            GL_CHECK(glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));
            GL_CHECK(glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
            GL_CHECK(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ));
            GL_CHECK(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ));
            
            if (m_isYUV) {
                GL_CHECK(glActiveTexture( GL_TEXTURE1 ));
                
                GL_CHECK(glBindTexture(m_target, srcTexture1));
                GL_CHECK(glUniform1i( m_frameSampler1, 1 ));
                
                // Set texture parameters
                GL_CHECK(glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));
                GL_CHECK(glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
                GL_CHECK(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ));
                GL_CHECK(glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ));
            }
            
            // Bind attributes. This is all that's needed with a vertex array
            GL_CHECK(glBindVertexArray(m_vbState));
            
            // Draw
            GL_CHECK(glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ));
            
            // Unbind
            GL_CHECK(glBindTexture( m_target, 0 ));
            GL_CHECK(glBindTexture( GL_TEXTURE_2D, 0 ));
            GL_CHECK(glBindFramebuffer( GL_FRAMEBUFFER, 0 ));
            GL_CHECK(glBindVertexArray(0));
            
            //GL_CHECK(glFlush());
        }
    };
    
    CBAR_GLOffscreenRenderer::CBAR_GLOffscreenRenderer() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBAR_GLOffscreenRenderer::~CBAR_GLOffscreenRenderer() {
        
    }
    
    GLuint CBAR_GLOffscreenRenderer::create(GLuint width, GLuint height, GLuint srcTarget, bool isYUV) {
        return m_pImpl->create(width, height, srcTarget, isYUV);
    }
    
    void CBAR_GLOffscreenRenderer::destroy() {
        m_pImpl->destroy();
    }
    
    void CBAR_GLOffscreenRenderer::render(GLuint srcTexture0, GLuint srcTexture1) {
        m_pImpl->render(srcTexture0, srcTexture1);
    }
};

