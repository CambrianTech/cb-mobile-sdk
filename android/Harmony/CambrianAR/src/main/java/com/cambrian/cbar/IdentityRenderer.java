package com.cambrian.cbar;

import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import static com.cambrian.cbar.GlUtil.checkGlError;
import static com.cambrian.cbar.GlUtil.createProgram;

/**
 * Created by joelteply on 8/23/17.
 */

class IdentityRenderer {

    private final float[] VERTICES_DATA = {
            // X, Y, Z, U, V
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f
    };

    static final int BYTES_PER_FLOAT = 4;

    private final int VERTICES_DATA_POS_SIZE = 3;
    private final int VERTICES_DATA_UV_SIZE = 2;
    private final int VERTICES_DATA_STRIDE_BYTES = (VERTICES_DATA_POS_SIZE + VERTICES_DATA_UV_SIZE) * BYTES_PER_FLOAT;
    private final int VERTICES_DATA_POS_OFFSET = 0;
    private final int VERTICES_DATA_UV_OFFSET = VERTICES_DATA_POS_SIZE * BYTES_PER_FLOAT;

    private int mProgram;
    private int mVertexBufferName;

    private final String mVertexShader =
            "attribute vec4 aPosition;\n" +
                    "attribute mediump vec4 aTextureCoord;\n" +
                    "varying mediump vec2 vTextureCoord;\n" +

                    "void main() {\n" +
                    "    gl_Position = aPosition;\n" +
                    "    vTextureCoord = aTextureCoord.xy;\n" +
                    "}\n";

    private final String mFragmentShader2D =
            "precision mediump float;\n" +
                    "varying highp vec2 vTextureCoord;\n" +
                    "uniform lowp sampler2D sTexture;\n" +
                    "void main() {\n" +
                    "vec4 color = texture2D(sTexture, vTextureCoord);\n" +
                    "gl_FragColor = vec4(vTextureCoord.x, vTextureCoord.y, color.b, color.a);\n" +
                    "}\n";

    private int mTarget = GLES20.GL_TEXTURE_2D;

    private int m_textureCoordinateHandle, m_positionHandle;

    IdentityRenderer(int target) {
        mTarget = target;
    }

    void initGL() {

        String fragmentShader = mFragmentShader2D;

        if (mTarget == GLES11Ext.GL_TEXTURE_EXTERNAL_OES) {
            fragmentShader = "#extension GL_OES_EGL_image_external : require\n" + fragmentShader.replace("sampler2D", "samplerExternalOES");
        }

        mProgram = createProgram(mVertexShader, fragmentShader);

        if (mProgram == 0) {
            return;
        }

        m_positionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        checkGlError("glGetAttribLocation aPosition");
        if (m_positionHandle == -1) {
            throw new RuntimeException("Could not get attrib location for aPosition");
        }
        m_textureCoordinateHandle = GLES20.glGetAttribLocation(mProgram, "aTextureCoord");
        checkGlError("glGetAttribLocation aTextureCoord");
        if (m_textureCoordinateHandle == -1) {
            throw new RuntimeException("Could not get attrib location for aTextureCoord");
        }

        mVertexBufferName = GlUtil.createBuffer(VERTICES_DATA);
    }

    void deinitGL() {
        GLES20.glDeleteProgram(mProgram);
        //GLES20.glDeleteShader(mVertexShader);
        //GLES20.glDeleteShader(mFragmentShader);
        GLES20.glDeleteBuffers(1, new int[] {mVertexBufferName}, 0);
    }

    synchronized void draw(int textureID) {

        GLES20.glUseProgram(mProgram);

        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, mVertexBufferName);
        GLES20.glEnableVertexAttribArray(m_positionHandle);
        GLES20.glVertexAttribPointer(m_positionHandle, VERTICES_DATA_POS_SIZE, GLES20.GL_FLOAT, false, VERTICES_DATA_STRIDE_BYTES, VERTICES_DATA_POS_OFFSET);
        GLES20.glEnableVertexAttribArray(m_textureCoordinateHandle);
        GLES20.glVertexAttribPointer(m_textureCoordinateHandle, VERTICES_DATA_UV_SIZE, GLES20.GL_FLOAT, false, VERTICES_DATA_STRIDE_BYTES, VERTICES_DATA_UV_OFFSET);
        checkGlError("glVertexAttribPointer");

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(mTarget, textureID);
        GLES20.glUniform1i(GLES20.glGetAttribLocation(mProgram, "sTexture"), 0);
        checkGlError("glBindTexture");

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        checkGlError("glDrawArrays");

        GLES20.glDisableVertexAttribArray(m_positionHandle);
        GLES20.glDisableVertexAttribArray(m_textureCoordinateHandle);
        GLES20.glBindTexture(mTarget, 0);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);
    }
}
