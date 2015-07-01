#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"	
	
static float mProjMatrix[16];//4x4矩阵 投影用
static float mVMatrix[16];//摄像机位置朝向9参数矩阵   
static float* currMatrix;//当前变换矩阵
float lightLocationSun[]={0,0,0};//太阳定位光光源位置
//FloatBuffer cameraFB;
//FloatBuffer lightPositionFBSun;
#define STACK_SIZE 128    
typedef struct _stack{
	void *data[128];
	int index;
	int count;
}t_stack;
static t_stack g_stack;
static int g_inited = 0;
static void *MatrixPush(void *data, int size)
{
	void *temp = malloc(size);
	if (temp == NULL){
		return NULL;
	}
	memcpy(temp, data, size);
	g_stack.data[g_stack.index++] = temp;
	g_stack.count++;
	return temp;
}

static void *MatrixPop()
{
	if (g_stack.count == 0)
		return NULL;
	g_stack.index--;
	g_stack.count--;
	return g_stack.data[g_stack.index];
}    
    
static void initStack()//获取不变换初始矩阵
{
	if (g_inited)
		return;
	g_inited = 1;
	currMatrix=malloc(16 * sizeof(float));
	setRotateM(currMatrix, 0, 0, 1, 0, 0);
}
    
void pushMatrix()//保护变换矩阵
{
	initStack();
	MatrixPush(currMatrix, 16 * sizeof(float));
}
    
void popMatrix()//恢复变换矩阵
{
	initStack();
	if (currMatrix)
		free(currMatrix);
	currMatrix=MatrixPop();
}
    
void transtate(float x,float y,float z)//设置沿xyz轴移动
{
	initStack();
    translateM(currMatrix, 0, x, y, z);
}
    
void rotate(float angle,float x,float y,float z)//设置绕xyz轴移动
{
	initStack();
	rotateM(currMatrix,0,angle,x,y,z);
}
    

//设置摄像机
void setCamera
(
    		float cx,	//摄像机位置x
    		float cy,   //摄像机位置y
    		float cz,   //摄像机位置z
    		float tx,   //摄像机目标点x
    		float ty,   //摄像机目标点y
    		float tz,   //摄像机目标点z
    		float upx,  //摄像机UP向量X分量
    		float upy,  //摄像机UP向量Y分量
    		float upz   //摄像机UP向量Z分量		
)
{
	initStack();
	setLookAtM (mVMatrix, 0, cx, cy, cz, tx, ty, tz, upx, upy, upz);
#if 0    	
    	float[] cameraLocation=new float[3];//摄像机位置
    	cameraLocation[0]=cx;
    	cameraLocation[1]=cy;
    	cameraLocation[2]=cz;
    	
    	ByteBuffer llbb = ByteBuffer.allocateDirect(3*4);
        llbb.order(ByteOrder.nativeOrder());//设置字节顺序
        cameraFB=llbb.asFloatBuffer();
        cameraFB.put(cameraLocation);
        cameraFB.position(0);  
#endif		
}
    
    
void setProjectPerspective(float fovy, float aspect, float zNear, float zFar)
{
	initStack();
	perspectiveM(mProjMatrix, 0, fovy, aspect, zNear, zFar);
}
    
//设置透视投影参数
void setProjectFrustum(
    	float left,		//near面的left
    	float right,    //near面的right
    	float bottom,   //near面的bottom
    	float top,      //near面的top
    	float near,		//near面距离
    	float far       //far面距离 
)
{
	initStack();
    frustumM(mProjMatrix, 0, left, right, bottom, top, near, far);    	
}
    
//设置正交投影参数
void setProjectOrtho(
    	float left,		//near面的left
    	float right,    //near面的right
    	float bottom,   //near面的bottom
    	float top,      //near面的top
    	float near,		//near面距离
    	float far       //far面距离
)
{    	
	initStack();
	orthoM(mProjMatrix, 0, left, right, bottom, top, near, far);
}   

//获取具体物体的总变换矩阵
float *getFinalMatrix()
{
	static float mMVPMatrix[16];
	initStack();
	multiplyMM(mMVPMatrix, 0, mVMatrix, 0, currMatrix, 0);
    multiplyMM(mMVPMatrix, 0, mProjMatrix, 0, mMVPMatrix, 0);        
    return mMVPMatrix;
}

//获取具体物体的变换矩阵
float *getMMatrix()
{
	initStack();
    return currMatrix;
}
    
//设置太阳光源位置的方法
void setLightLocationSun(float x,float y,float z)
{
	initStack();
	lightLocationSun[0]=x;
	lightLocationSun[1]=y;
	lightLocationSun[2]=z;
#if 0
	ByteBuffer llbb = ByteBuffer.allocateDirect(3*4);
    llbb.order(ByteOrder.nativeOrder());//设置字节顺序
    lightPositionFBSun=llbb.asFloatBuffer();
    lightPositionFBSun.put(lightLocationSun);
    lightPositionFBSun.position(0);
#endif    
}