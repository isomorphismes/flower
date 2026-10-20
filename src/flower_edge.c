#include "flower_edge.h"
#include <stddef.h>

/* At full scene resolution this is a SCREEN-SPACE silhouette coverage
 * comparison, not super-sampled 3D geometry. Background is the actual
 * Flower scene clear color. Only pixels next to the mesh/background
 * transition are changed. A1/C67 frame cost is NOT accepted until measured.
 * The GLES2 loops have constant bounds; branch selects the 3x3/5x5 window. */
static const char edge_fragment[] =
"#version 100\n"
"precision mediump float;\n"
"varying vec2 fragTexCoord;\n"
"varying vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform vec2 pixelStep;\n"
"uniform int edgeRadius;\n"
"void main(void) {\n"
"  vec3 emptyColor=vec3(14.0/255.0,16.0/255.0,22.0/255.0);\n"
"  vec3 center=texture2D(texture0,fragTexCoord).rgb;\n"
"  float foregroundCount=0.0;\n"
"  float sampleCount=0.0;\n"
"  vec3 foregroundSum=vec3(0.0);\n"
"  for(int row=-2;row<=2;row++) {\n"
"    for(int column=-2;column<=2;column++) {\n"
"      if(row>edgeRadius||row< -edgeRadius||column>edgeRadius||column< -edgeRadius) continue;\n"
"      vec2 q=clamp(fragTexCoord+vec2(float(column),float(row))*pixelStep,vec2(0.0),vec2(1.0));\n"
"      vec3 rgb=texture2D(texture0,q).rgb;\n"
"      float inside=step(0.13,distance(rgb,emptyColor));\n"
"      foregroundCount+=inside;\n"
"      foregroundSum+=rgb*inside;\n"
"      sampleCount+=1.0;\n"
"    }\n"
"  }\n"
"  vec3 outputColor=center;\n"
"  if(foregroundCount>0.0 && foregroundCount<sampleCount)\n"
"    outputColor=mix(emptyColor,foregroundSum/foregroundCount,foregroundCount/sampleCount);\n"
"  gl_FragColor=vec4(outputColor,1.0)*fragColor;\n"
"}\n";

bool flower_edge_prepare(FlowerEdge *edge,int width,int height) {
    if (!edge || width<1 || height<1) return false;
    if (!edge->attempted) {
        edge->attempted=true;
        edge->shader=LoadShaderFromMemory(NULL,edge_fragment);
        if (edge->shader.id) {
            edge->texel_location=GetShaderLocation(edge->shader,"pixelStep");
            edge->radius_location=GetShaderLocation(edge->shader,"edgeRadius");
        }
        if (!edge->shader.id || edge->texel_location<0 || edge->radius_location<0) {
            edge->failed=true;
            TraceLog(LOG_ERROR,"Flower optional image-space edge shader unavailable");
            return false;
        }
    }
    if (edge->failed) return false;
    if (edge->target.id &&
        (edge->target.texture.width!=width || edge->target.texture.height!=height)) {
        UnloadRenderTexture(edge->target);
        edge->target=(RenderTexture2D){0};
    }
    if (!edge->target.id) {
        edge->target=LoadRenderTexture(width,height);
        if (!edge->target.id) {
            edge->failed=true;
            TraceLog(LOG_ERROR,"Flower optional edge render target unavailable");
            return false;
        }
        SetTextureFilter(edge->target.texture,TEXTURE_FILTER_POINT);
    }
    return true;
}
void flower_edge_draw(FlowerEdge *edge,int model,int width,int height) {
    if (!edge || !edge->shader.id || !edge->target.id ||
        (model!=3 && model!=5) || width<1 || height<1) return;
    float texel[2]={1.0f/width,1.0f/height};
    int radius=model==3?1:2;
    SetShaderValue(edge->shader,edge->texel_location,texel,SHADER_UNIFORM_VEC2);
    SetShaderValue(edge->shader,edge->radius_location,&radius,SHADER_UNIFORM_INT);
    BeginShaderMode(edge->shader);
    DrawTextureRec(edge->target.texture,
                   (Rectangle){0,0,(float)width,-(float)height},
                   (Vector2){0,0},WHITE);
    /* Pixel magnifier makes the difference inspectable rather than a
     * label change. The crop is from the real 3D render, not the Net pane. */
    float sx=width*0.5f-20.0f, sy=height*0.27f-20.0f;
    if (sx<0) sx=0;
    if (sy<0) sy=0;
    DrawTexturePro(edge->target.texture,
                   (Rectangle){sx,sy,40,-40},
                   (Rectangle){12,142,160,160},(Vector2){0,0},0,WHITE);
    EndShaderMode();
    DrawRectangleLines(11,141,162,162,LIGHTGRAY);
    DrawRectangle(12,303,160,22,(Color){24,27,35,245});
    DrawText(model==3?"EDGE 3x3 - pixel x4":"EDGE 5x5 - pixel x4",
             15,307,13,LIGHTGRAY);
}
void flower_edge_free(FlowerEdge *edge) {
    if (!edge) return;
    if (edge->target.id) UnloadRenderTexture(edge->target);
    if (edge->shader.id) UnloadShader(edge->shader);
    *edge=(FlowerEdge){0};
}
