#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "matrix.h"
#include "geometry.h"
#ifdef __cplusplus
extern "C"
{
#endif
extern int sphereSetup(float r, float **ppVertex, float **ppTexture);
extern int cyliderSetup(float r, float lenght, float **ppVertex, float **ppTexture);
#ifdef __cplusplus
}
#endif
//几何体着色器
static const char gVertexShader[] = "uniform mat4 uMVPMatrix;\n" //总变换矩阵
	"attribute vec3 aPosition;\n"  //顶点位置
	"attribute vec2 aTexCoor;\n"    //顶点纹理坐标
	"varying vec2 vTextureCoord;\n"  //用于传递给片元着色器的变量
	"void main()\n"     
	"{\n"                            		
		"gl_Position = uMVPMatrix * vec4(aPosition,1);\n" //根据总变换矩阵计算此次绘制此顶点位置  
		"vTextureCoord=aTexCoor;\n"       //将顶点的纹理坐标传给片元着色器
	"}\n";
	
static const char gFragmentShader[] = "#extension GL_OES_EGL_image_external: require\n"
	"precision mediump float;\n"
	"uniform samplerExternalOES sTexture;\n"
	"varying vec2 vTextureCoord;\n"//接收从顶点着色器过来的参数
	"void main()\n"
	"{\n"  
		"gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
	"}\n";

static void checkGlError(const char* op) {
	GLint error;
    for (error = glGetError(); error; error
            = glGetError()) {
        fprintf(stderr, "after %s() glError (0x%x)\n", op, error);
    }
}

static int mProgram;//自定义渲染管线程序id 
static int muMVPMatrixHandle;//总变换矩阵引用id   
static int maPositionHandle; //顶点位置属性引用id  
static int maTexCoorHandle; //顶点纹理坐标属性引用id
static int maTexSampler;
static float *pVertex; //存放顶点坐标的ArrayList
static float *pTexture;
static int vCount;

static GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    fprintf(stderr, "Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
            } else {
                fprintf(stderr, "Guessing at GL_INFO_LOG_LENGTH size\n");
                char* buf = (char*) malloc(0x1000);
                if (buf) {
                    glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                    fprintf(stderr, "Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

static GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    fprintf(stderr, "Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

static void geometrySetupShader()
{
	fprintf(stderr, "vCount=%d\n", vCount);
	mProgram = createProgram(gVertexShader, gFragmentShader);
	//获取程序中顶点位置属性引用id  
    maPositionHandle = glGetAttribLocation(mProgram, "aPosition");
	checkGlError("glGetAttribLocation aPosition");
    //获取程序中顶点纹理属性引用id   
    maTexCoorHandle=glGetAttribLocation(mProgram, "aTexCoor");
	checkGlError("glGetAttribLocation aTexCoor");	
    //获取程序中总变换矩阵引用id
    muMVPMatrixHandle = glGetUniformLocation(mProgram, "uMVPMatrix"); 
	checkGlError("glGetUniformLocation uMVPMatrix");
	maTexSampler = glGetUniformLocation(mProgram, "sTexture");
	checkGlError("glGetUniformLocation sTexture");
}

int geometrySetup(int mode, float width, float height)
{
	if (mode == SPHERE_MODE){
		vCount = sphereSetup(width, &pVertex, &pTexture);
	}
	if (mode == CYLIDER_MODE){
		vCount = cyliderSetup(width, height, &pVertex, &pTexture);
	}
	if (vCount > 0)
		geometrySetupShader();
	
	return vCount>0?0:-1;
}


void geometryDrawSelf(int texId) {        
    //制定使用某套着色器程序
    glUseProgram(mProgram);
    //将最终变换矩阵传入着色器程序
    glUniformMatrix4fv(muMVPMatrixHandle, 1, GL_FALSE, getFinalMatrix());  
    checkGlError("glUniformMatrix4fv");	      
    glVertexAttribPointer(//为画笔指定顶点位置数据    
         		maPositionHandle,   
         		3, 
         		GL_FLOAT, 
         		GL_FALSE,
                3*4, 
                pVertex   
         );
	checkGlError("glVertexAttribPointer");	
    glVertexAttribPointer(  //为画笔指定顶点纹理数据
        		maTexCoorHandle,  
         		2, 
         		GL_FLOAT, 
         		GL_FALSE,
                2*4,   
                pTexture
         );   
    checkGlError("glVertexAttribPointer");     
    //允许顶点位置数据数组
    glEnableVertexAttribArray(maPositionHandle); 
	checkGlError("glEnableVertexAttribArray");
    glEnableVertexAttribArray(maTexCoorHandle); 
	checkGlError("glEnableVertexAttribArray");
	
	glUniform1i(maTexSampler, 0);
	checkGlError("glUniform1i");
    //绑定纹理   
    glActiveTexture(GL_TEXTURE0);
	checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId); 
	checkGlError("glBindTexture");
    //绘制三角形
    glDrawArrays(GL_TRIANGLES, 0, vCount); 
	checkGlError("glDrawArrays");
}

void geometryEnd()
{
	if (pTexture)
		free(pTexture);
	if (pVertex)
		free(pVertex);
}