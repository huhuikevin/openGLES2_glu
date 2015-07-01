#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "matrix.h"

//地球着色器
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
static int VertexPointCnt;
static int TexturePointCnt;
static int vCount;
static const float angleSpan=2.0f;

static double PI = 3.14159265358979323846;
#define toRadians(x) (((x)/180) * PI)
#define addVertext(x) do {pVertex[VertexPointCnt++] = x; }while(0);
static void generateVertexData(float r){
    //顶点坐标数据的初始化================begin============================    	
	const float UNIT_SIZE=0.5f;
	float vAngle;
	float hAngle;
	//将球进行单位切分的角度
	for(vAngle=90;vAngle>-90;vAngle=vAngle-angleSpan){//垂直方向angleSpan度一份
    	for(hAngle=360;hAngle>0;hAngle=hAngle-angleSpan){//水平方向angleSpan度一份
    		//纵向横向各到一个角度后计算对应的此点在球面上的坐标
    		double xozLength=r*UNIT_SIZE*cos(toRadians(vAngle));
    		float x1=(float)(xozLength*cos(toRadians(hAngle)));
    		float z1=(float)(xozLength*sin(toRadians(hAngle)));
    		float y1=(float)(r*UNIT_SIZE*sin(toRadians(vAngle)));
    		xozLength=r*UNIT_SIZE*cos(toRadians(vAngle-angleSpan));
    		float x2=(float)(xozLength*cos(toRadians(hAngle)));
    		float z2=(float)(xozLength*sin(toRadians(hAngle)));
    		float y2=(float)(r*UNIT_SIZE*sin(toRadians(vAngle-angleSpan)));
    		xozLength=r*UNIT_SIZE*cos(toRadians(vAngle-angleSpan));
    		float x3=(float)(xozLength*cos(toRadians(hAngle-angleSpan)));
    		float z3=(float)(xozLength*sin(toRadians(hAngle-angleSpan)));
    		float y3=(float)(r*UNIT_SIZE*sin(toRadians(vAngle-angleSpan)));
    		xozLength=r*UNIT_SIZE*cos(toRadians(vAngle));
    		float x4=(float)(xozLength*cos(toRadians(hAngle-angleSpan)));
    		float z4=(float)(xozLength*sin(toRadians(hAngle-angleSpan)));
    		float y4=(float)(r*UNIT_SIZE*sin(toRadians(vAngle)));   
    		//构建第一三角形
    		addVertext(x1);addVertext(y1);addVertext(z1);
    		addVertext(x2);addVertext(y2);addVertext(z2);
    		addVertext(x4);addVertext(y4);addVertext(z4);        		
    		//构建第二三角形
    		addVertext(x4);addVertext(y4);addVertext(z4);
    		addVertext(x2);addVertext(y2);addVertext(z2);
    		addVertext(x3);addVertext(y3);addVertext(z3); 
        }
	} 	
}		
//自动切分纹理产生纹理数组的方法
static void generateTexCoor(){
	int bw = (int)(360/angleSpan);
	int bh = (int)(180/angleSpan);
	float *result= pTexture;
	float sizew=1.0f/bw;//列数
	float sizeh=1.0f/bh;//行数
	int c=0;
	int i,j;
	for(i=0;i<bh;i++){
		for(j=0;j<bw;j++){
			//每行列一个矩形，由两个三角形构成，共六个点，12个纹理坐标
			float s=j*sizew;
			float t=i*sizeh;

		#define loads(x) (result[c++]= 1.0 - (x))
		#define loadt(x) (result[c++]= (x))
			loads(s);
			loadt(t);
			loads(s);
			loadt(t+sizeh);			
			loads(s+sizew);			
			loadt(t);
			
			loads(s+sizew);
			loadt(t);
			loads(s);
			loadt(t+sizeh);
			loads(s+sizew);			
			loadt(t+sizeh);			 			
		}
	}    	
}

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



//r --> 半径
int earthSetup(float r)
{
	int count = 0;
	float vAngle;
	float hAngle;
    for(vAngle=90;vAngle>-90;vAngle=vAngle-angleSpan){//垂直方向angleSpan度一份
		for(hAngle=360;hAngle>0;hAngle=hAngle-angleSpan){//水平方向angleSpan度一份
			count++;
		}
	}
	VertexPointCnt = count * 18;
	TexturePointCnt = count * 12;
	pVertex = malloc(VertexPointCnt * sizeof(float));
	pTexture = malloc(TexturePointCnt * sizeof(float));
	VertexPointCnt = 0;
	if (!pTexture || !pVertex){
		if (pTexture)
			free(pTexture);
		if (pVertex)
			free(pVertex);
		
		return -1;
	}
	
	generateVertexData(r);
	generateTexCoor();
	
	vCount = VertexPointCnt/3;
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
	return 0;
}

void earthDrawSelf(int texId) {        
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