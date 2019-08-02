/*
 Copyright (C) 2016 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 Shader compiler and linker utilities
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "ShaderUtilities.h"
#include "Diagnostics.h"
#include "CommonUtility.h"

/* Compile a shader from the provided source(s) */
GLint glueCompileShader(GLenum target, GLsizei count, const GLchar **sources, GLuint *shader)
{
    GLint status;
    
    *shader = glCreateShader(target);
    glShaderSource(*shader, count, sources, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength = 0;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        CBLog("Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        int i;
        
        CBError("Failed to compile shader:\n");
        for (i = 0; i < count; i++)
            CBLog("%s", sources[i]);
    }
    
    return status;
}


/* Link a program with all currently attached shaders */
GLint glueLinkProgram(GLuint program)
{
    GLint status;
    
    glLinkProgram(program);
    
#if defined(DEBUG)
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(program, logLength, &logLength, log);
        CBLog("Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0)
        CBError("Failed to link program %d", program);
    
    return status;
}


/* Validate a program (for i.e. inconsistent samplers) */
GLint glueValidateProgram(GLuint program)
{
    GLint status;
    
    glValidateProgram(program);
    
#if defined(DEBUG)
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(program, logLength, &logLength, log);
        CBLog("Program validate log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        CBError("Failed to validate program %d", program);
    
    return status;
}


/* Return named uniform location after linking */
GLint glueGetUniformLocation(GLuint program, const GLchar *uniformName)
{
    GLint loc;
    
    loc = glGetUniformLocation(program, uniformName);
    
    return loc;
}

static const int TARGET_OES = 0x8D65;

/* Convenience wrapper that compiles, links, enumerates uniforms and attribs */
GLint glueCreateProgram(GLuint target,
                        const GLchar *vertSource, const GLchar *fragSource,
                        GLsizei attribNameCt, const GLchar **attribNames,
                        const GLint *attribLocations,
                        GLsizei uniformNameCt, const GLchar **uniformNames,
                        GLint *uniformLocations,
                        GLuint *program)
{
    GLuint vertShader = 0, fragShader = 0, prog = 0, status = 1, i;
    
    string fragmentShader(fragSource);
    
    if (target == TARGET_OES) {
        string newSrcHeader = "#extension GL_OES_EGL_image_external : require\n";
        string newSrcReplacementOld = "uniform sampler2D ";
        string newSrcReplacementNew = "uniform samplerExternalOES ";
        
        // replace
        string oldSrc(fragSource);
        strReplaceAll(oldSrc, newSrcReplacementOld, newSrcReplacementNew);
        
        // prepend header
        fragmentShader = string(newSrcHeader);
        fragmentShader.append(oldSrc);
    }
    
    const GLchar *fragShaderConverted = (GLchar *) fragmentShader.c_str();
    
    // Create shader program
    prog = glCreateProgram();
    
    // Create and compile vertex shader
    status *= glueCompileShader(GL_VERTEX_SHADER, 1, &vertSource, &vertShader);
    
    // Create and compile fragment shader
    status *= glueCompileShader(GL_FRAGMENT_SHADER, 1, &fragShaderConverted, &fragShader);
    
    // Attach vertex shader to program
    glAttachShader(prog, vertShader);
    
    // Attach fragment shader to program
    glAttachShader(prog, fragShader);
    
    // Bind attribute locations
    // This needs to be done prior to linking
    for (i = 0; i < attribNameCt; i++)
    {
        if(strlen(attribNames[i]))
            glBindAttribLocation(prog, attribLocations[i], attribNames[i]);
    }
    
    // Link program
    status *= glueLinkProgram(prog);
    
    // Get locations of uniforms
    if (status)
    {
        for(i = 0; i < uniformNameCt; i++)
        {
            if(strlen(uniformNames[i]))
                uniformLocations[i] = glueGetUniformLocation(prog, uniformNames[i]);
        }
        *program = prog;
    }
    
    // Release vertex and fragment shaders
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    
    return status;
}

void checkGlError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        CBError("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

