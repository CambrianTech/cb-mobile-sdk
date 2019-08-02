/*
 Copyright (C) 2016 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 Shader compiler and linker utilities
 */


#ifndef RosyWriter_ShaderUtilities_h
#define RosyWriter_ShaderUtilities_h
    
#include <cbcommon/CB_Types.h>
#include <cambrian.h>

GLint glueCompileShader(GLenum target, GLsizei count, const GLchar **sources, GLuint *shader);
GLint glueLinkProgram(GLuint program);
GLint glueValidateProgram(GLuint program);
GLint glueGetUniformLocation(GLuint program, const GLchar *name);

GLint glueCreateProgram(GLuint target, const GLchar *vertSource, const GLchar *fragSource,
                        GLsizei attribNameCt, const GLchar **attribNames, 
                        const GLint *attribLocations,
                        GLsizei uniformNameCt, const GLchar **uniformNames,
                        GLint *uniformLocations,
                        GLuint *program);

void checkGlError(const char* stmt, const char* fname, int line);

#ifdef DEBUG
#define GL_CHECK(stmt) do { \
stmt; \
checkGlError(#stmt, __FILE__, __LINE__); \
} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#endif
