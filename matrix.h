#ifndef __MATRIX_H__
#define __MATRIX_H__
#ifdef __cplusplus
extern "C"
{
#endif
void setRotateM(float *rm, int rmOffset,
            float a, float x, float y, float z);
void setRotateEulerM(float *rm, int rmOffset,
            float x, float y, float z);
void setLookAtM(float *rm, int rmOffset,
            float eyeX, float eyeY, float eyeZ,
            float centerX, float centerY, float centerZ, float upX, float upY,
            float upZ);
void rotateM(float *m, int mOffset,
            float a, float x, float y, float z);
			
void rotateM2(float *rm, int rmOffset,
            float *m, int mOffset,
            float a, float x, float y, float z);
void translateM(
            float *m, int mOffset,
            float x, float y, float z);
void translateM2(float *tm, int tmOffset,
            float *m, int mOffset,
            float x, float y, float z);
void scaleM(float *m, int mOffset,
            float x, float y, float z);
void scaleM2(float *sm, int smOffset,
            float *m, int mOffset,
            float x, float y, float z);			
void setIdentityM(float *sm, int smOffset);
float length(float x, float y, float z);
void perspectiveM(float *m, int offset,
          float fovy, float aspect, float zNear, float zFar);
int frustumM(float *m, int offset,
            float left, float right, float bottom, float top,
            float near, float far);
int orthoM(float *m, int mOffset,
        float left, float right, float bottom, float top,
        float near, float far);
void transposeM(float *mTrans, int mTransOffset, 
		float *m, int mOffset);
void multiplyMV(float *result, int resultOffset,
            float *lhs, int lhsOffset, float *rhs, int rhsOffset);
void multiplyMM(float *result, int resultOffset,
            float *lhs, int lhsOffset, float *rhs, int rhsOffset);
			
void pushMatrix();
void popMatrix();
void transtate(float x,float y,float z);
void rotate(float angle,float x,float y,float z);
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
);
void setProjectPerspective(float fovy, float aspect, float zNear, float zFar);
void setProjectFrustum(
    	float left,		//near面的left
    	float right,    //near面的right
    	float bottom,   //near面的bottom
    	float top,      //near面的top
    	float near,		//near面距离
    	float far       //far面距离 
);
void setProjectOrtho(
    	float left,		//near面的left
    	float right,    //near面的right
    	float bottom,   //near面的bottom
    	float top,      //near面的top
    	float near,		//near面距离
    	float far       //far面距离
);
float *getFinalMatrix();
float *getMMatrix();
void setLightLocationSun(float x,float y,float z);			

#ifdef __cplusplus
}
#endif
#endif