#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

int sphereSetup(float r, float **ppVertex, float **ppTexture)
{
	int count = 0;
	int bw = (int)(360/angleSpan);
	int bh = (int)(180/angleSpan);
	
	count = bw*bh;
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
	
	*ppVertex = pVertex;
	*ppTexture = pTexture;
	return vCount;
}