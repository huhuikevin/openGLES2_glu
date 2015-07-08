#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//圆柱着色器


static float *pVertex; //存放顶点坐标的ArrayList
static float *pTexture;
static int VertexPointCnt;
static int TexturePointCnt;
static int vCount;
static const float angleSpan=2.0f;
static const float cyBlocks = 5.0f;
static double PI = 3.14159265358979323846;
#define toRadians(x) (((x)/180) * PI)
#define addVertext(x) do {pVertex[VertexPointCnt++] = x; }while(0);
static void generateVertexData(float length, float radius){
    //顶点坐标数据的初始化================begin============================    	
	const float UNIT_SIZE=0.5f;
	float blocks;
	float hAngle;
	float len = length/cyBlocks;
	//将球进行单位切分的角度
	for(blocks=0;blocks<cyBlocks;blocks++){//垂直方向angleSpan度一份
    	for(hAngle=360;hAngle>0;hAngle=hAngle-angleSpan){//水平方向angleSpan度一份
			//(right)左上角顶点坐标
			float z1=(float) (radius*cosf(toRadians(hAngle)));
			float y1=(length/2-blocks*len);
			float x1=(float) (radius*sinf(toRadians(hAngle)));			

			//(left)右上角顶点坐标
			float z2=(float) (radius*cosf(toRadians(hAngle-angleSpan)));
			float y2=y1;
			float x2=(float) (radius*sinf(toRadians(hAngle-angleSpan)));

			//(left)右下角顶点坐标
			float x3=x2;
			float y3= y1-len;
			float z3=z2;

			//(right)左下角顶点坐标
			float x4=x1;
			float y4=y3;
			float z4=z1;
			//fprintf(stderr, "x1=%f, y1=%f, z1=%f, x2=%f, y2=%f, z2=%f, x3=%f, y3=%f, z3=%f, x4=%f, y4=%f, z4=%f\n", x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
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
	int bh = (int)(cyBlocks);
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

		#define loads(x) (result[c++]= (x))
		#define loadt(x) (result[c++]= (x))
		
			loads(s);
			loadt(t);	
			loads(s+sizew);			
			loadt(t);				
			loads(s);
			loadt(t+sizeh);	
			
			loads(s);
			loadt(t+sizeh);		
			loads(s+sizew);			
			loadt(t);
			loads(s+sizew);			
			loadt(t+sizeh);			
	 			
		}
	}    	
}


//r --> 半径
int cyliderSetup(float r, float lenght, float **ppVertex, float **ppTexture)
{
	int count = (cyBlocks * ((int)(360/angleSpan)));
	
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
	
	generateVertexData(lenght, r);
	generateTexCoor();
	
	vCount = VertexPointCnt/3;
	
	*ppVertex = pVertex;
	*ppTexture = pTexture;
	
	return vCount;	
}
