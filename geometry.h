#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#ifdef __cplusplus
extern "C"
{
#endif
int geometrySetup(int mode, float width, float height);
//int geometrySetupShader();
void geometryDrawSelf(int texId);
void geometryEnd();
#ifdef __cplusplus
}
#endif

#define SPHERE_MODE 1
#define CYLIDER_MODE 2

#endif /* __GEOMETRY_H__*/
