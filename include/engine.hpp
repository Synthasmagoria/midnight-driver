#ifndef __MD_ENGINE_H
#define __MD_ENGINE_H

#include <cstring>
#ifndef NO_FONT_AWESOME
#define NO_FONT_AWESOME
#endif

#include "stdlib.h"
#include "assert.h"
#include "math.h"
#include "string.h"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"

#include <vector>
#include <unordered_map>
#include <string>

#include "typedefs.hpp"

#define nameof(x) #x
#define FRAME_TIME 1.f / 60.f
#define FRAMERATE 60
#define TAU PI * 2.f
#define GAME_OBJECT_MAX 1000

#define LOAD_MODEL(path)(LoadModel(TextFormat("resources/models/%s", path)))
#define LOAD_SHADER(v,f)(LoadShader(TextFormat("resources/shaders/%s", v), TextFormat("resources/shaders/%s", f)))
#define LOAD_TEXTURE(path)(LoadTexture(TextFormat("resources/textures/%s", path)))
#define LOAD_IMAGE(path)(LoadImage(TextFormat("resources/textures/%s", path)))
#define LOAD_FONT(path)(LoadFont(TextFormat("resources/fonts/%s", path)))
#define MatrixRotateRoll(rad) MatrixRotateX(rad)
#define MatrixRotateYaw(rad) MatrixRotateY(rad)
#define MatrixRotatePitch(rad) MatrixRotateZ(rad)

/*
    Custom math structs
*/
struct QuadraticBezier {v2 p1; v2 p2; v2 p3;};
// https://www.desmos.com/calculator/scz7zhonfw
float QuadraticBezierLerp(QuadraticBezier qb, float val) {
    v2 a = Vector2Lerp(qb.p1, qb.p2, val);
    v2 b = Vector2Lerp(qb.p2, qb.p3, val);
    return Vector2Lerp(a, b, val).y;
}

struct mat2 {
    float x1, x2, y1, y2;
};
inline mat2 Matrix2Rotate(float ang) {
    return mat2{cosf(ang), -sinf(ang), sinf(ang), cosf(ang)};
}
inline mat2 Matrix2Scale(mat2 mat, float scalar) {
    return mat2{mat.x1 * scalar, mat.x2 * scalar, mat.y1 * scalar, mat.y2 * scalar};
}
inline mat2 Matrix2Invert(mat2 mat) {
    float scalar = 1.f / (mat.x1 * mat.y2 - mat.x2 * mat.y1);
    mat2 m = mat2{mat.y2, -mat.x2, -mat.y1, mat.x1};
    return Matrix2Scale(m, scalar);
}

struct ShaderColor {
    float r, g, b, a;
};
inline ShaderColor ColorToShaderColor(Color col) {
    return {(float)col.r / 255.f, (float)col.g / 255.f, (float)col.b / 255.f, (float)col.a / 255.f};
}

/*
    Math util
*/
inline i32 imini(i32 a, i32 b) {return a < b ? a : b;}
inline i64 imini(i64 a, i64 b) {return a < b ? a : b;}
inline u32 uimini(u32 a, u32 b) {return a < b ? a : b;}
inline u64 uimini(u64 a, u64 b) {return a < b ? a : b;}
inline i32 iwrapi(i32 val, i32 min, i32 max) {
    i32 range = max - min;
	return range == 0 ? min : min + ((((val - min) % range) + range) % range);
}
inline float ffractf(float val) {return val - floorf(val);}
inline float fclampf(float val, float min, float max) {return fminf(fmaxf(val, min), max);}
inline float fsignf(float val) {return (float)(!signbit(val)) * 2.f - 1.f;}
inline float ApproachZero(float val, float amount) {return fmaxf(fabsf(val) - amount, 0.f) * fsignf(val);}

inline v2 Vector2Fract(v2 v) {return {ffractf(v.x), ffractf(v.y)};}
inline v2 Vector2FromAngle(float ang) {return {cosf(ang), -sinf(ang)};}
inline v2 Vector2Transform(v2 v, mat2 mat) {
    return {
        v.x * mat.x1 + v.y * mat.y1,
        v.x * mat.x2 + v.y * mat.y2};
}
inline bool PointInRectangle(rect2 rect, v2 p) {
    v2 br = {rect.x + rect.width, rect.y + rect.height};
    return p.x >= rect.x && p.x < br.x && p.y >= rect.y && p.y < br.y;
}
inline bool PointInRectangle(v2 begin, v2 end, v2 pt) {
    return pt.x >= begin.x && pt.x < end.x && pt.y >= begin.y && pt.y < end.y;
}

/*
    Raylib util
*/
#define MAX_MATERIAL_MAPS 12
void DrawMeshInstancedOptimized(Mesh mesh, Material material, const float16 *transforms, int instances) {
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Instancing required variables
    unsigned int instancesVboId = 0;

    // Bind shader program
    rlEnableShader(material.shader.id);

    // Send required data to shader (matrices, values)
    //-----------------------------------------------------
    // Upload to shader material.colDiffuse
    if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
    {
        float values[4] = {
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.r/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.g/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.b/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.colSpecular (if location available)
    if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1)
    {
        float values[4] = {
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.r/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.g/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.b/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Get a copy of current matrices to work with,
    // just in case stereo render is required, and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Enable mesh VAO to attach new buffer
    rlEnableVertexArray(mesh.vaoId);

    // This could alternatively use a static VBO and either glMapBuffer() or glBufferSubData()
    // It isn't clear which would be reliably faster in all cases and on all platforms,
    // anecdotally glMapBuffer() seems very slow (syncs) while glBufferSubData() seems
    // no faster, since we're transferring all the transform matrices anyway
    instancesVboId = rlLoadVertexBuffer(transforms, instances*sizeof(float16), false);

    // Instances transformation matrices are send to shader attribute location: SHADER_LOC_MATRIX_MODEL
    for (unsigned int i = 0; i < 4; i++)
    {
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 4, RL_FLOAT, 0, sizeof(Matrix), i*sizeof(Vector4));
        rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 1);
    }

    rlDisableVertexBuffer();
    rlDisableVertexArray();

    // Accumulate internal matrix transform (push/pop) and view matrix
    // NOTE: In this case, model instance transformation must be computed in the shader
    matModelView = MatrixMultiply(rlGetMatrixTransform(), matView);

    // Upload model normal matrix (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));

#ifdef RL_SUPPORT_MESH_GPU_SKINNING
    // Upload Bone Transforms
    if ((material.shader.locs[SHADER_LOC_BONE_MATRICES] != -1) && mesh.boneMatrices)
    {
        rlSetUniformMatrices(material.shader.locs[SHADER_LOC_BONE_MATRICES], mesh.boneMatrices, mesh.boneCount);
    }
#endif

    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Enable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
            else rlEnableTexture(material.maps[i].texture.id);

            rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
        }
    }

    // Try binding vertex array objects (VAO)
    // or use VBOs if not possible
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
        {
            if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] != 0)
            {
                rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR]);
                rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC4, 4);
                rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

#ifdef RL_SUPPORT_MESH_GPU_SKINNING
        // Bind mesh VBO data: vertex bone ids (shader-location = 6, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_BONEIDS] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEIDS], 4, RL_UNSIGNED_BYTE, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEIDS]);
        }

        // Bind mesh VBO data: vertex bone weights (shader-location = 7, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS]);
        }
#endif

        if (mesh.indices != nullptr) rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh instanced
        if (mesh.indices != nullptr) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount*3, 0, instances);
        else rlDrawVertexArrayInstanced(0, mesh.vertexCount, instances);
    }

    // Unbind all bound texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Disable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
            else rlDisableTexture();
        }
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Remove instance transforms buffer
    rlUnloadVertexBuffer(instancesVboId);
#endif
}

i32 PixelformatGetStride(i32 format) {
    switch (format) {
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        case PIXELFORMAT_UNCOMPRESSED_R32:
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        case PIXELFORMAT_UNCOMPRESSED_R16:
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:
            assert(false);
            return -1;
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            return 1;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            return 2;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            return 3;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            return 4;
    }
    return -1;
}

// NOTE: These functions only support up to 3 decimals
inline float GetRandomValueF(float min, float max) {
    return ((float)GetRandomValue((i32)(min * 1000.f), (i32)(max * 1000.f))) / 1000.f;
}
inline bool GetRandomChanceF(float percentage) {return GetRandomValueF(0.f, 100.f) < percentage;}
float GetClosestWrappedF(float from, float to, float bounds_min, float bounds_max) {
    float dist = bounds_max - bounds_min;
    float a = to - from;
    float b = (to + dist) - from;
    float c = (to - dist) - from;
    float result = fabs(a) < fabs(b) ? a : b;
    return fabs(c) < fabs(result) ? c + from : result + from;
}

inline void DrawTextShadow(const char* text, i32 x, i32 y, i32 fontSize, Color col, Color shadowCol) {
    DrawText(text, x + 1, y + 1, fontSize, shadowCol);
    DrawText(text, x, y, fontSize, col);
}
inline void DrawCrosshair(i32 x, i32 y, Color col) {
    DrawLine(x - 4, y, x + 4, y, col);
    DrawLine(x, y - 4, x, y + 4, col);
}

/*
    Engine
*/
// TODO: Decouple memory allocation calls.
struct MemoryPool {
    void* buffer;
    u64 location;
    u64 size;
};
MemoryPool MemoryPoolCreate(u64 size);
void MemoryPoolDestroy(MemoryPool* mm);
void* MemoryPoolReserve(MemoryPool* mm, u64 size);
void MemoryPoolClear(MemoryPool* mm);
template <typename T>
T* MemoryReserve(MemoryPool* mm);
template <typename T>
T* MemoryReserve(MemoryPool* mm, u64 size);

struct TextDrawingStyle {
    Color color;
    Font font;
    float size;
    float charSpacing;
};
inline TextDrawingStyle TextDrawingStyleGetDefault() {
    TextDrawingStyle tds = {};
    tds.charSpacing = 1;
    tds.font = {};
    tds.size = 24;
    tds.color = WHITE;
    return tds;
}

enum INPUT {
    INPUT_ACCELERATE,
    INPUT_BREAK,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,
    INPUT_UP,
    INPUT_DEBUG_TOGGLE,
    INPUT_DEBUG_LEFT,
    INPUT_DEBUG_RIGHT,
    INPUT_DEBUG_FORWARD,
    INPUT_DEBUG_BACKWARD,
    INPUT_DEBUG_UP,
    INPUT_DEBUG_DOWN,
    INPUT_DEBUG_CONTROL,
    INPUT_COUNT
};
#define INPUT_SELECT INPUT_ACCELERATE

// TODO: Keybindings probably shouldn't be part of the engine code
struct Input {
    i32 map[INPUT_COUNT];
    bool pressed[INPUT_COUNT];
    bool down[INPUT_COUNT];
    bool up[INPUT_COUNT];
};
void InputInit(Input* input);
void InputUpdate(Input* input);
bool InputCheckPressedCombination(i32 a, i32 b);
bool InputCheckPressedExclusive(i32 ind, i32 exclude);
bool InputCheckPressedMod(i32 ind, bool shift, bool ctrl, bool alt);

struct StringBuilder {
    char* str;
    char separator = -1;
    i32 capacity;
    i32 position;
};
StringBuilder StringBuilderCreate(i32 capacity, MemoryPool* mp);
i32 StringBuilderAddString(StringBuilder* sb, const char* str);
i32 StringBuilderAddChar(StringBuilder* sb, char chr);
void _StringBuilderAddChar(StringBuilder* sb, char chr);

struct String {
    char* cstr;
    i32 length;
};
String StringCreate(char* text);
String _StringCreate(i32 length);
String StringSet(String str, char* text);
String StringSubstr(String str, i32 start, i32 count);
void StringDestroy(String str);

// TODO: Move this into its own file. algorithmic.hpp or something.
struct MarchingSquaresResult {
    u8* data;
    i32 width;
    i32 height;
};
enum MARCHING_SQUARES_NORMAL {
    MARCHING_SQUARES_NORMAL_RIGHT,
    MARCHING_SQUARES_NORMAL_RIGHT_UP,
    MARCHING_SQUARES_NORMAL_UP,
    MARCHING_SQUARES_NORMAL_LEFT_UP,
    MARCHING_SQUARES_NORMAL_LEFT,
    MARCHING_SQUARES_NORMAL_LEFT_DOWN,
    MARCHING_SQUARES_NORMAL_DOWN,
    MARCHING_SQUARES_NORMAL_RIGHT_DOWN,
    MARCHING_SQUARES_NORMAL_NONE
};
enum MARCHING_SQUARES_DIRECTION {
    MARCHING_SQUARES_DIRECTION_RIGHT,
    MARCHING_SQUARES_DIRECTION_UP,
    MARCHING_SQUARES_DIRECTION_LEFT,
    MARCHING_SQUARES_DIRECTION_DOWN,
    MARCHING_SQUARES_DIRECTION_NONE
};
u8 MarchingSquaresGetData(MarchingSquaresResult* msr, i32 x, i32 y);
u8 MarchingSquaresGetMarchingBit(MarchingSquaresResult* msr, i32 x, i32 y);
u8 MarchingSquaresSetMarchingBit(MarchingSquaresResult* msr, i32 x, i32 y, bool val);
MarchingSquaresResult MarchingSquaresImage(Image* image);
void MarchingSquaresDataDraw(Texture texture, v2 frameSize, MarchingSquaresResult* msr);
void MarchingSquaresDataFree(MarchingSquaresResult* msr);
i32 MarchingSquaresGetDataDirection(u8 data, i32 dirPrev);
i32 MarchingSquaresGetDataNormal(u8 data, i32 dirPrev);
std::vector<v2> MarchingSquaresDataPolygonizeAt(MarchingSquaresResult* msr, i32 x, i32 y);

struct Collider {
    rect2 rect;
    float angle;
    rect2 _bounds;
    mat2 _rotmat;
    mat2 _rotmatInverse;
    float _diagonalLength;
};
void ColliderSetAngle(Collider* collider, float angle);
void ColliderSetRect(Collider* collider, rect2 rect);
Collider ColliderCreate(rect2 rect, float angle);
Collider ColliderCreateFromLine(v2 a, v2 b, float width);
v2 ColliderGetMiddle(Collider* collider);
void ColliderDraw(Collider* collider, Color color);
struct CollisionInformation {
    bool collided;
    v2 pushoutPosition;
};
v2 ColliderGetPushoutDir(Collider* collider);
CollisionInformation ColliderPointCollision(Collider* collider, v2 point);

/*
    Engine (Dependency depth 1)
*/
struct GameObject;
struct StringList;
struct EventArgs_TypewriterLineComplete;
struct EventArgs_DialogueOptionsSelected;
struct TypeList;
struct Typewriter;
struct DialogueOptions;
struct Heightmap;
struct InstanceMeshRenderData;

struct GameObjectDefinition {
    void (*Update) (void*);
    void (*Draw3d) (void*);
    void (*DrawUi) (void*);
    void (*Free) (void*);
    void (*DrawImGui) (void*);
    const char* objectName;
};
GameObjectDefinition GameObjectDefinitionCreate(const char* objectName, MemoryPool* mp);
struct GameObject {
    void (*Update) (void*);
    void (*Draw3d) (void*);
    void (*DrawUi) (void*);
    void (*Free) (void*);
    void (*DrawImGui) (void*);
    void* data;
    const char* objectName;
    const char* instanceName;
    i32 id;
    static i32 idCounter;
    bool visible;
    bool active;
};
i32 GameObject::idCounter = 100000;
GameObject GameObjectCreate(void* data, const char* objectName, const char* instanceName = "<unnamed>");

struct StringList {
    String* data;
    i32 size;
    i32 capacity;
    MemoryPool* _memoryPool;
};
void StringListInit(StringList* list, i32 size, MemoryPool* mp);
void StringListAdd(StringList* list, const char* cstr);
String* StringListGet(StringList* list, i32 ind);

typedef void(*EventCallbackSignature)(void* registrar, void* args);
enum EVENT_HANDLER_EVENTS {
    EVENT_TYPEWRITER_LINE_COMPLETE,
    EVENT_DIALOGUE_OPTIONS_SELECTED,
    EVENT_COUNT
};
struct EventArgs_TypewriterLineComplete {
    i32 lineCurrent;
    i32 lineCount;
};
typedef void(*EventCallbackSignature_TypewriterLineComplete)(void*, EventArgs_TypewriterLineComplete*);
struct EventArgs_DialogueOptionsSelected {
    i32 index;
    i32 count;
};
typedef void(*EventCallbackSignature_DialogueOptionsSelected)(void*, EventArgs_DialogueOptionsSelected*);
struct EventHandler {
    TypeList* callbacks[EVENT_COUNT];
    TypeList* registrars[EVENT_COUNT];
};
EventHandler EventHandlerCreate();
void EventHandlerRegisterEvent(i32 ind, void* registrar, EventCallbackSignature callback);
void EventHandlerUnregisterEvent(i32 ind, void* registrar);
void EventHandlerCallEvent(void* caller, i32 ind, void* args);

struct TypeList {
    void* buffer;
    MemoryPool* memoryProvider;
    i32 bufferStride;
    i32 size;
    i32 capacity;
    i32 typeIndex;
};
enum TYPE_LIST_TYPE {
    TYPE_LIST_I32,
    TYPE_LIST_PTR,
    TYPE_LIST_GAME_OBJECT_DEFINITION,
    __TYPE_LIST_TYPE_COUNT
};
TypeList* TypeListCreate(i32 typeIndex, MemoryPool* memoryPool, i32 capacity = 5);
i32 TypeListTypeGetSize(i32 typeIndex);
void TypeListSetCapacity(TypeList* list, i32 capacity);
void TypeListGrowCapacityIfLimitReached(TypeList* list);
void TypeListPushBackI32(TypeList* list, i32 val);
i32 TypeListGetI32(TypeList* list, i32 ind);
void TypeListSetI32(TypeList* list, i32 ind, i32 val);
i32 TypeListFindI32(TypeList* list, i32 val);
void TypeListPushBackPtr(TypeList* list, void* val);
void* TypeListGetPtr(TypeList* list, i32 ind);
void TypeListSetPtr(TypeList* list, i32 ind, void* val);
i32 TypeListFindPtr(TypeList* list, void* val);
void _TypeListPushBackPtr(TypeList* list, void* val);
void* _TypeListGetPtr(TypeList* list, i32 ind);
void _TypeListSetPtr(TypeList* list, i32 ind, void* val);
i32 _TypeListFindPtr(TypeList* list, void* val);
void TypeListPushBackGameObjectDefinition(TypeList* list, GameObjectDefinition val);
GameObjectDefinition* TypeListGetGameObjectDefinition(TypeList* list, i32 ind);
void TypeListSetGameObjectDefinition(TypeList* list, i32 ind, GameObjectDefinition val);
i32 TypeListFindGameObjectDefinition(TypeList* list, GameObject* val);

struct Typewriter {
    String *text;
    i32 textCount;
    i32 textIndex;
    bool autoAdvance;
    bool autoHide;
    float autoContinueDelay;
    float autoHideOnEndDelay;
    float _autoAdvanceCountdown;
    bool visible;
    i32 x;
    i32 y;
    float speed;
    float progress;
    TextDrawingStyle textDrawingStyle;
};
void TypewriterInit(Typewriter* tw);
void TypewriterUpdate(void* tw);
void TypewriterDraw(void* tw);
void TypewriterEvent_LineComplete(Typewriter* tw);
void TypewriterStart(Typewriter* tw, String* str, i32 strCount);
GameObject TypewriterPack(Typewriter* tw, const char* instanceName = "<unnamed>");
// TODO: Encapsulate this functionality in a signal processing structure
void _TypewriterAdvanceText(Typewriter *tw);
void _TypewriterReset(Typewriter *tw);

struct DialogueOptions {
    String* options;
    i32 index;
    i32 count;
    i32 x;
    i32 y;
    bool visible;
    float optionSeparationAdd;
    float optionBoxHeightAdd;
    TextDrawingStyle textStyle;
    NPatchInfo npatchInfo;
    Texture2D npatchTexture;
};
void DialogueOptionsInit(DialogueOptions* dopt);
void DialogueOptionsHandleInput(DialogueOptions* dialogueOptions);
void DialogueOptionsUpdate(void* _dopt);
void DialogueOptionsDraw(void* _dopt);
GameObject DialogueOptionsPack(DialogueOptions* dopt, const char* instanceName = "<unnamed>");

struct HeightmapGenerationInfo {
    Image *heightmapImage;
    v3 position;
    v3 size;
    i32 resdiv;
};
struct Heightmap {
    v3 position;
    v3 size;
    i32 heightDataWidth;
    i32 heightDataHeight;
    float *heightData;
    i32 width;
};
void HeightmapInit(Heightmap* hm, HeightmapGenerationInfo info);
float HeightmapSampleHeight(Heightmap* heightmap, float x, float z);
void HeightmapFree(Heightmap* hm);

struct InstanceMeshRenderData {
    float16 *transforms;
    i32 instanceCount;
    Model _model;
    Mesh mesh;
    Material material;
};
void InstanceMeshRenderDataDraw3d(void* _imrd);
GameObject InstanceMeshRenderDataPack(InstanceMeshRenderData* forest, const char* instanceName);



/*
    Game Objects
*/
struct DialogueSequence;
struct DialogueSequenceSection;
struct ModelInstance;
struct ParticleSystem;
struct TextureInstance;
struct Skybox;

struct DialogueSequence {
    Typewriter typewriter;
    DialogueOptions options;
    TypeList* sections;
    i32 sectionIndex;
};
void DialogueSequenceInit(DialogueSequence* dseq, i32 ind);
void DialogueSequenceSectionStart(DialogueSequence* dseq, i32 ind);
void DialogueSequenceUpdate(void* _dseq);
void DialogueSequenceDrawUi(void* _dseq);
GameObject DialogueSequencePack(DialogueSequence* dseq, const char* instanceName = "<unnamed>");
void DialogueSequenceStartSection(i32 ind);
void DialogueSequenceHandleTypewriter_TextAdvance(void* _dseq, EventArgs_TypewriterLineComplete* _args);
void DialogueSequenceHandleOptions_Selected(void* _dseq, EventArgs_DialogueOptionsSelected* _args);
struct DialogueSequenceSection {
    StringList* text;
    StringList* options;
    TypeList* link;
};
DialogueSequenceSection* DialogueSequenceSectionGet(DialogueSequence* dseq, i32 ind);
DialogueSequenceSection* DialogueSequenceSectionCreate(i32 textCount, i32 optionCount);

struct ModelInstance {
    Model model;
    v3 position;
    float scale;
    Color tint;
};
ModelInstance* ModelInstanceCreate(MemoryPool* mp);
void ModelInstanceDraw3d(void* _mi);
void ModelInstanceDrawImGui(void* _mi);
GameObject ModelInstancePack(ModelInstance* mi, const char* instanceName = "<unnamed>");

#define PARTICLE_SYSTEM_MAX_PARTICLES 512
struct ParticleSystem {
    mat4 *_transforms;
    Mesh _quad;
    Material _material;
    v3 velocity;
    i32 count;
};
void ParticleSystemInit(ParticleSystem* psys, Texture texture, Material material);
void ParticleSystemFree(void* psys);
void ParticleSystemUpdate(void* psys);
void ParticleSystemDraw3d(void* psys);
GameObject ParticleSystemPack(ParticleSystem *psys, const char* instanceName = "<unnamed>");

struct TextureInstance {
    Texture* texture;
    float rotation;
    v2 position;
    v2 origin;
    Color tint;
    v2 _scale;
    rect2 _drawSource;
    rect2 _drawDestination;
};
void TextureInstanceSetSize(TextureInstance* ti, v2 size);
void TextureInstanceInit(TextureInstance* ti, Texture* tex);
void TextureInstanceDrawUi(void* _ti);
GameObject TextureInstancePack(TextureInstance* ti, const char* instanceName = "<unnamed>");

struct Skybox {
    Model model;
    Shader shader;
};
void SkyboxDraw3d(void* _skybox);
GameObject SkyboxPack(Skybox* skybox);
/*
    Engine dependent utility definitions
*/
const char* CstringDuplicate(const char* cstr, MemoryPool* mm);

/*
    Init
*/
#define _MD_GAME_OBJECT_COUNT_MAX 500

namespace mdEngine {
    bool initialized = false;
    MemoryPool scratchMemory;
    MemoryPool sceneMemory;
    MemoryPool persistentMemory;
    MemoryPool engineMemory;
    EventHandler eventHandler;
    Input input;
    std::unordered_map<std::string, void*> groups = std::unordered_map<std::string, void*>();
    GameObjectDefinition gameObjectDefinitions[_MD_GAME_OBJECT_COUNT_MAX];
    bool gameObjectIsDefined[_MD_GAME_OBJECT_COUNT_MAX];
};

enum MD_GAME_ENGINE_OBJECTS {
    OBJECT_DIALOGUE_SEQUENCE,
    OBJECT_MODEL_INSTANCE,
    OBJECT_PARTICLE_SYSTEM,
    OBJECT_TEXTURE_INSTANCE,
    OBJECT_SKYBOX,
    _MD_GAME_ENGINE_OBJECTS_COUNT
};

void MdEngineRegisterObject(GameObjectDefinition def, i32 ind) {
    assert(ind < _MD_GAME_OBJECT_COUNT_MAX);
    assert(!mdEngine::gameObjectIsDefined[ind]);
    mdEngine::gameObjectDefinitions[ind] = def;
    mdEngine::gameObjectIsDefined[ind] = true;
}

// TODO: Move definition creation to where the object is so that it becomes more easily editable
void MdEngineRegisterObjects() {
    MemoryPool* mp = &mdEngine::engineMemory;
    GameObjectDefinition def = {};
    def = GameObjectDefinitionCreate("Dialogue Sequence", mp);
    def.DrawUi = DialogueSequenceDrawUi;
    def.Update = DialogueSequenceUpdate;
    MdEngineRegisterObject(def, OBJECT_DIALOGUE_SEQUENCE);

    def = GameObjectDefinitionCreate("Model Instance", mp);
    def.Draw3d = ModelInstanceDraw3d;
    def.DrawImGui = ModelInstanceDrawImGui;
    MdEngineRegisterObject(def, OBJECT_MODEL_INSTANCE);

    def = GameObjectDefinitionCreate("Particle System", mp);
    def.Draw3d = ParticleSystemDraw3d;
    def.Update = ParticleSystemUpdate;
    def.Free = ParticleSystemFree;
    MdEngineRegisterObject(def, OBJECT_PARTICLE_SYSTEM);

    def = GameObjectDefinitionCreate("Texture Instance", mp);
    def.DrawUi = TextureInstanceDrawUi;
    MdEngineRegisterObject(def, OBJECT_TEXTURE_INSTANCE);

    def = GameObjectDefinitionCreate("Skybox", mp);
    def.Draw3d = SkyboxDraw3d;
    MdEngineRegisterObject(def, OBJECT_SKYBOX);
}

void MdEngineInit(i32 memoryPoolSize) {
    if (mdEngine::initialized) {
        return;
    }
    mdEngine::initialized = true;
    mdEngine::scratchMemory = MemoryPoolCreate(memoryPoolSize);
    mdEngine::sceneMemory = MemoryPoolCreate(memoryPoolSize);
    mdEngine::persistentMemory = MemoryPoolCreate(memoryPoolSize);
    mdEngine::engineMemory = MemoryPoolCreate(memoryPoolSize);
    mdEngine::eventHandler = EventHandlerCreate();
    MdEngineRegisterObjects();
    InputInit(&mdEngine::input);
}

/*
    Engine dependent utility implementations
*/
const char* CstringDuplicate(const char* cstr, MemoryPool* mm) {
    i32 len = (i32)strlen(cstr);
    char* cstrNew = MemoryReserve<char>(mm, len + 1);
    strcpy_s(cstrNew, len + 1, cstr);
    cstrNew[len] = '\0';
    return cstrNew;
}

/*
    Engine function implementations
*/
MemoryPool MemoryPoolCreate(u64 size) {
    MemoryPool mm = {};
    mm.buffer = calloc(size, sizeof(byte));
    mm.location = 0;
    mm.size = size;
    return mm;
}
void MemoryPoolDestroy(MemoryPool* mm) {
    if (mm->buffer != nullptr) {
        free(mm->buffer);
    }
}
void* MemoryPoolReserve(MemoryPool* mm, u64 size) {
    assert(mm->location + size < mm->size); // Out of memory check
    assert(size > 0);
    void* reserve = (byte*)mm->buffer + mm->location;
    mm->location += size;
    if (mm->location % sizeof(void*) != 0) {
        u64 alignedLocation = mm->location + (sizeof(void*) - (mm->location % sizeof(void*)));
        mm->location = uimini(alignedLocation, mm->size);
    }
    return reserve;
}
void MemoryPoolClear(MemoryPool* mm) {
    assert(mm->size > 0);
    memset(mm->buffer, 0, mm->location);
    mm->location = 0;
}
template <typename T>
T* MemoryReserve(MemoryPool* mm) {
    return (T*)MemoryPoolReserve(mm, sizeof(T));
}
template <typename T>
T* MemoryReserve(MemoryPool* mm, u64 size) {
    return (T*)MemoryPoolReserve(mm, sizeof(T) * size);
}

void InputInit(Input* input) {
    input->map[INPUT_ACCELERATE] = KEY_SPACE;
    input->map[INPUT_BREAK] = KEY_LEFT_SHIFT;
    input->map[INPUT_LEFT] = KEY_LEFT;
    input->map[INPUT_RIGHT] = KEY_RIGHT;
    input->map[INPUT_DOWN] = KEY_DOWN;
    input->map[INPUT_UP] = KEY_UP;
    input->map[INPUT_DEBUG_TOGGLE] = KEY_BACKSPACE;
    input->map[INPUT_DEBUG_LEFT] = KEY_A;
    input->map[INPUT_DEBUG_RIGHT] = KEY_D;
    input->map[INPUT_DEBUG_FORWARD] = KEY_W;
    input->map[INPUT_DEBUG_BACKWARD] = KEY_S;
    input->map[INPUT_DEBUG_UP] = KEY_Q;
    input->map[INPUT_DEBUG_DOWN] = KEY_E;
    input->map[INPUT_DEBUG_CONTROL] = KEY_LEFT_CONTROL;
}
void InputUpdate(Input* input) {
    for (i32 i = 0; i < INPUT_COUNT; i++) {
        input->pressed[i] = IsKeyPressed(input->map[i]);
        input->down[i] = IsKeyDown(input->map[i]);
        input->up[i] = IsKeyUp(input->map[i]);
    }
}
bool InputCheckPressedCombination(i32 a, i32 b) {
    return (mdEngine::input.down[a] && mdEngine::input.pressed[b]) || (mdEngine::input.down[b] && mdEngine::input.pressed[a]);
}
bool InputCheckPressedExclusive(i32 ind, i32 exclude) {
    return mdEngine::input.pressed[ind] && !mdEngine::input.down[exclude];
}
bool InputCheckPressedMod(i32 ind, bool shift, bool ctrl, bool alt) {
    bool pressed = mdEngine::input.pressed[ind];
    bool modifiersCheck = true;
    if (shift) {
        if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
            modifiersCheck = false;
        }
    } else {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            modifiersCheck = false;
        }
    }
    if (ctrl) {
        if (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) {
            modifiersCheck = false;
        }
    } else {
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
            modifiersCheck = false;
        }
    }
    if (alt) {
        if (!IsKeyDown(KEY_LEFT_ALT) && !IsKeyDown(KEY_RIGHT_ALT)) {
            modifiersCheck = false;
        }
    } else {
        if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {
            modifiersCheck = false;
        }
    }
    return pressed && modifiersCheck;
}

StringBuilder StringBuilderCreate(i32 capacity, MemoryPool* mp) {
    StringBuilder sb = {};
    sb.str = (char*)MemoryReserve<char>(mp, capacity);
    sb.separator = 0;
    sb.capacity = capacity;
    sb.position = 0;
    return sb;
}
i32 StringBuilderAddString(StringBuilder* sb, const char* str) {
    i32 separatorLength = (i32)(sb->separator != -1);
    i32 length = (i32)strlen(str);
    if (sb->capacity <= sb->position + length + separatorLength) {
        return 1;
    }
    memcpy(sb->str + sb->position, str, length);
    sb->position += length;
    _StringBuilderAddChar(sb, sb->separator);
    return 0;
}
i32 StringBuilderAddChar(StringBuilder* sb, char chr) {
    if (sb->capacity <= sb->position) {
        return 1;
    }
    _StringBuilderAddChar(sb, chr);
    return 0;
}
void _StringBuilderAddChar(StringBuilder* sb, char chr) {
    sb->str[sb->position] = chr;
    sb->position++;
}

// TODO: Stop calling malloc
String StringCreate(char* text) {
    String str = {};
    str.cstr = nullptr;
    str.length = 0;
    return StringSet(str, text);
}
String _StringCreate(i32 length) {
    String str = {};
    str.cstr = (char*)malloc(length + 1);
    memset(str.cstr, 0, length);
    str.cstr[length] = '\0';
    str.length = length;
    return str;
}
String StringSet(String str, char* text) {
    i32 len = (i32)strlen(text) + 1;
    if (str.cstr != nullptr) {
        free(str.cstr);
    }
    str.cstr = (char*)malloc(len);
    _memccpy(str.cstr, text, '\0', len);
    str.length = len;
    return str;
}
String StringSubstr(String str, i32 start, i32 count) {
    if (count >= (str.length + 1) - start) {
        count = str.length - start;
    }
    String substr = _StringCreate(count);
    _memccpy(substr.cstr, &str.cstr[start], '\0', count);
    return substr;
}
void StringDestroy(String str) {
    if (str.cstr != nullptr) {
        free(str.cstr);
    }
}

u8 MarchingSquaresGetData(MarchingSquaresResult* msr, i32 x, i32 y) {
    u8 data = msr->data[x + y * msr->width];
    return data & 15;
}
u8 MarchingSquaresGetMarchingBit(MarchingSquaresResult* msr, i32 x, i32 y) {
    return (msr->data[x + y * msr->width]) & 0b00010000;
}
u8 MarchingSquaresSetMarchingBit(MarchingSquaresResult* msr, i32 x, i32 y, bool val) {
    return ((msr->data[x + y * msr->width]) & 0b00010000) | ((u8)val << 4);
}
MarchingSquaresResult MarchingSquaresImage(Image* image) {
    MarchingSquaresResult msr = {};
    msr.width = image->width - 1;
    msr.height = image->height - 1;
    msr.data = (u8*)calloc(msr.width * msr.height, sizeof(u8));
    u8* imageData = (u8*)image->data;
    i32 stride = PixelformatGetStride(image->format);
    for (i32 x = 0; x < image->width - 1; x++) {
        for (i32 y = 0; y < image->height - 1; y++) {
            i32 itl = x + y * image->width;
            i32 itr = (x + 1) + y * image->width;
            i32 ibl = x + (y + 1) * image->height;
            i32 ibr = (x + 1) + (y + 1) * image->width;
            u8 tl = imageData[itl * stride + stride - 1] == 0;
            u8 tr = imageData[itr * stride + stride - 1] == 0;
            u8 bl = imageData[ibl * stride + stride - 1] == 0;
            u8 br = imageData[ibr * stride + stride - 1] == 0;
            msr.data[x + y * msr.width] = bl | (br << 1) | (tr << 2) | (tl << 3);
        }
    }
    return msr;
}
void MarchingSquaresDataDraw(Texture texture, v2 frameSize, MarchingSquaresResult* msr) {
    for (i32 x = 0; x < msr->width; x++) {
        for (i32 y = 0; y < msr->height; y++) {
            u8 index = msr->data[x + y * msr->width];
            rect2 srcRect = {frameSize.x * (i32)index, 0, frameSize.x, frameSize.y};
            v2 position = {x * frameSize.x, y * frameSize.y};
            Color transparentWhite = {255, 255, 255, 128};
            DrawTextureRec(texture, srcRect, position, WHITE);
            DrawLineV(position, {position.x + frameSize.x, position.y}, transparentWhite);
            DrawLineV(position, {position.x, position.y + frameSize.y}, transparentWhite);
            DrawText(TextFormat("%u", index), (i32)position.x, (i32)position.y, 12, transparentWhite);
        }
    }
}
void MarchingSquaresDataFree(MarchingSquaresResult* msr) {
    free(msr->data);
    msr->data = nullptr;
    msr->width = 0;
    msr->height = 0;
}
i32 MarchingSquaresGetDataDirection(u8 data, i32 dirPrev) {
    switch (data) {
        case 2: // >
        case 3:
        case 11:
            return MARCHING_SQUARES_DIRECTION_RIGHT;
        case 4: // ^
        case 6:
        case 7:
            return MARCHING_SQUARES_DIRECTION_UP;
        case 8: // <
        case 12:
        case 14:
            return MARCHING_SQUARES_DIRECTION_LEFT;
        case 1: // v
        case 9:
        case 13:
            return MARCHING_SQUARES_DIRECTION_DOWN;
        case 5:
            if (dirPrev == MARCHING_SQUARES_DIRECTION_RIGHT) {
                return MARCHING_SQUARES_DIRECTION_UP;
            } else if (dirPrev == MARCHING_SQUARES_DIRECTION_LEFT) {
                return MARCHING_SQUARES_DIRECTION_DOWN;
            } else {
                const char* msg = TextFormat("MarchingSquaresDataPolygonizeAt: Invalid previous move");
                TraceLog(LOG_ERROR, msg);
                return MARCHING_SQUARES_DIRECTION_NONE;
            }
            break;
        case 10:
            if (dirPrev == MARCHING_SQUARES_DIRECTION_UP) {
                return MARCHING_SQUARES_DIRECTION_RIGHT;
            } else if (dirPrev == MARCHING_SQUARES_DIRECTION_DOWN) {
                return MARCHING_SQUARES_DIRECTION_LEFT;
            } else {
                const char* msg = TextFormat("MarchingSquaresDataPolygonizeAt: Invalid previous move");
                TraceLog(LOG_ERROR, msg);
                return MARCHING_SQUARES_DIRECTION_NONE;
            }
            break;
        default:
            TraceLog(LOG_ERROR, "MarchingSquaresDataPolygonizeAt: Invalid marching square data");
            return MARCHING_SQUARES_DIRECTION_NONE;
            break;
    }
}
i32 MarchingSquaresGetDataNormal(u8 data, i32 dirPrev) {
    switch (data) {
        case 6:
            return MARCHING_SQUARES_NORMAL_RIGHT;
        case 4:
        case 14:
            return MARCHING_SQUARES_NORMAL_RIGHT_UP;
        case 12:
            return MARCHING_SQUARES_NORMAL_UP;
        case 8:
        case 13:
            return MARCHING_SQUARES_NORMAL_LEFT_UP;
        case 9:
            return MARCHING_SQUARES_NORMAL_LEFT;
        case 1:
        case 11:
            return MARCHING_SQUARES_NORMAL_LEFT_DOWN;
        case 3:
            return MARCHING_SQUARES_NORMAL_DOWN;
        case 2:
        case 7:
            return MARCHING_SQUARES_NORMAL_RIGHT_DOWN;
        case 5:
            if (dirPrev == MARCHING_SQUARES_DIRECTION_RIGHT) {
                return MARCHING_SQUARES_NORMAL_RIGHT_DOWN;
            } else if (dirPrev == MARCHING_SQUARES_DIRECTION_LEFT) {
                return MARCHING_SQUARES_NORMAL_LEFT_UP;
            }
            return MARCHING_SQUARES_NORMAL_NONE;
        case 10:
            if (dirPrev == MARCHING_SQUARES_DIRECTION_DOWN) {
                return MARCHING_SQUARES_NORMAL_LEFT_DOWN;
            } else if (dirPrev == MARCHING_SQUARES_DIRECTION_UP) {
                return MARCHING_SQUARES_NORMAL_RIGHT_UP;
            }
            return MARCHING_SQUARES_NORMAL_NONE;
    }
    return MARCHING_SQUARES_NORMAL_NONE;
}
std::vector<v2> MarchingSquaresDataPolygonizeAt(MarchingSquaresResult* msr, i32 x, i32 y) {
    std::vector<v2> polygon;
    MarchingSquaresSetMarchingBit(msr, x, y, true);
    u8 firstData = MarchingSquaresGetData(msr, x, y);
    if (firstData == 0 || firstData == 15) {
        TraceLog(LOG_ERROR, "MarchingSquaresDataPolygonizeAt: Invalid starting position");
        return polygon;
    } else if (firstData == 5 || firstData == 10) {
        TraceLog(LOG_ERROR, "MarchingSquaresDataPolygonizeAt: Ambiguous starting position");
        return polygon;
    }
    i32 startX = x;
    i32 startY = y;
    i32 currentX = x;
    i32 currentY = y;
    i32 movePrevX = 0;
    i32 movePrevY = 0;
    i32 moveDirPrev = 0;
    i32 normDirPrev = 0;
#define MARCHING_SQUARES_POLYGONIZE_AT_MAX_ITERATION_COUNT 512
    i32 iterations = 0;
    bool error = false;
    do {
        u8 data = MarchingSquaresGetData(msr, currentX, currentY);
        i32 moveDir = MarchingSquaresGetDataDirection(data, moveDirPrev);
        error |= moveDir == MARCHING_SQUARES_DIRECTION_NONE;
        i32 normDir = MarchingSquaresGetDataNormal(data, moveDirPrev);
        error |= normDir == MARCHING_SQUARES_NORMAL_NONE;

        if (normDir != normDirPrev) {
            polygon.push_back(v2{(float)currentX - (float)movePrevX / 2.f, (float)currentY - (float)movePrevY / 2.f});
        }
        i32 moveX = 0;
        i32 moveY = 0;
        switch (moveDir) {
            case MARCHING_SQUARES_DIRECTION_RIGHT: moveX = 1; break;
            case MARCHING_SQUARES_DIRECTION_UP: moveY = -1; break;
            case MARCHING_SQUARES_DIRECTION_LEFT: moveX = -1; break;
            case MARCHING_SQUARES_DIRECTION_DOWN: moveY = 1; break;
        }
        currentX += moveX;
        currentY += moveY;
        movePrevX = moveX;
        movePrevY = moveY;
        moveDirPrev = moveDir;
        normDirPrev = normDir;
        iterations++;
        MarchingSquaresSetMarchingBit(msr, currentX, currentY, true);

        if (currentX < 0 || currentX >= msr->width || currentY < 0 || currentY >= msr->height) {
            break;
        }
        if (iterations >= MARCHING_SQUARES_POLYGONIZE_AT_MAX_ITERATION_COUNT) {
            TraceLog(LOG_WARNING, "MarchingSquaresDataPolygonizeAt: Stopped because max iterations has been reached");
            break;
        }
    } while ((currentX != startX || currentY != startY) && !error);
    return polygon;
}

void ColliderSetAngle(Collider* collider, float angle) {
    collider->angle = angle;
    collider->_rotmat = Matrix2Rotate(angle);
    collider->_rotmatInverse = Matrix2Invert(collider->_rotmat);
}
void ColliderSetRect(Collider* collider, rect2 rect) {
    collider->rect = rect;
    float diagonalLength = sqrtf(rect.width * rect.width + rect.height * rect.height);
    v2 middle = {collider->rect.x + collider->rect.width / 2.f, collider->rect.y + collider->rect.height / 2.f};
    collider->_bounds = rect2{middle.x - diagonalLength / 2.f, middle.y - diagonalLength / 2.f, diagonalLength, diagonalLength};
}
Collider ColliderCreate(rect2 rect, float angle) {
    Collider collider;
    ColliderSetRect(&collider, rect);
    ColliderSetAngle(&collider, angle);
    return collider;
}
Collider ColliderCreateFromLine(v2 a, v2 b, float width) {
    float angle = atan2(b.y - a.y, b.x - a.x);
    float length = Vector2Distance(a, b);
    v2 colliderSize = {length, width};
    v2 colliderPosition = Vector2Lerp(a, b, 0.5) - colliderSize / 2.f;
    rect2 colliderRect = {colliderPosition.x, colliderPosition.y, colliderSize.x, colliderSize.y};
    return ColliderCreate(colliderRect, -angle);
}
v2 ColliderGetMiddle(Collider* collider) {
    return v2{collider->rect.x, collider->rect.y} + v2{collider->rect.width, collider->rect.height} / 2.f;
}
void ColliderDraw(Collider* collider, Color color) {
    v2 middle = ColliderGetMiddle(collider);
    v2 topLeft = Vector2Transform(
        v2{collider->rect.x, collider->rect.y} - middle,
        collider->_rotmat);
    v2 topRight = Vector2Transform(
        v2{collider->rect.x + collider->rect.width, collider->rect.y} - middle,
        collider->_rotmat);
    v2 bottomRight = Vector2Transform(
        v2{collider->rect.x, collider->rect.y} + v2{collider->rect.width, collider->rect.height} - middle,
        collider->_rotmat);
    v2 bottomLeft = Vector2Transform(
        v2{collider->rect.x, collider->rect.y + collider->rect.height} - middle,
        collider->_rotmat);
    v2 pts[4] = {topLeft, topRight, bottomRight, bottomLeft};
    for (i32 i = 0; i < 4; i++) {
        DrawLineV(pts[i] + middle, pts[(i+1)%4] + middle, color);
    }
    v2 arrowTail = {0.f, -8.f};
    v2 arrowTip = {0.f, 8.f};
    v2 arrowEndTop = arrowTip + v2{-4.f, -4.f};
    v2 arrowEndBottom = arrowTip + v2{4.f, -4.f};
    arrowTail = Vector2Transform(arrowTail, collider->_rotmat) + middle;
    arrowTip = Vector2Transform(arrowTip, collider->_rotmat) + middle;
    arrowEndTop = Vector2Transform(arrowEndTop, collider->_rotmat) + middle;
    arrowEndBottom = Vector2Transform(arrowEndBottom, collider->_rotmat) + middle;
    DrawLineV(arrowTail, arrowTip, color);
    DrawLineV(arrowTip, arrowEndTop, color);
    DrawLineV(arrowTip, arrowEndBottom, color);
}
v2 ColliderGetPushoutDir(Collider* collider) {
    return Vector2Transform(v2{0.f, 1.f}, collider->_rotmat);
}
CollisionInformation ColliderPointCollision(Collider* collider, v2 point) {
    CollisionInformation ci;
    if (PointInRectangle(collider->_bounds, point)) {
        v2 middle = ColliderGetMiddle(collider);
        v2 pointRotated = Vector2Transform(point - middle, collider->_rotmatInverse);
        ci.collided = PointInRectangle(collider->rect, pointRotated + middle);
        ci.pushoutPosition = Vector2Transform(v2{pointRotated.x, collider->rect.y - middle.y}, collider->_rotmat) + middle;
        return ci;
    }
    ci.collided = false;
    ci.pushoutPosition = point;
    return ci;
}

// TODO: Rename because the memory pool parameter causes confusion
GameObjectDefinition GameObjectDefinitionCreate(const char* objectName, MemoryPool* mp) {
    GameObjectDefinition def = {};
    def.objectName = CstringDuplicate(objectName, mp);
    return def;
}
GameObject GameObjectCreate(void* data, const char* objectName, const char* instanceName) {
    GameObject go = {};
    go.data = data;
    go.objectName = CstringDuplicate(objectName, &mdEngine::sceneMemory);
    go.instanceName = CstringDuplicate(instanceName, &mdEngine::sceneMemory);
    go.id = GameObject::idCounter;
    GameObject::idCounter++;
    return go;
}

void StringListInit(StringList* list, i32 size, MemoryPool* mp) {
    list->data = MemoryReserve<String>(mp, size);
    list->capacity = size;
    list->size = 0;
}
void StringListAdd(StringList* list, const char* cstr) {
    if (list->size >= list->capacity) {
        TraceLog(LOG_ERROR, "StringList: Out of strings");
        return;
    }
    String* data = (String*)list->data;
    i32 cstrLen = (i32)strlen(cstr);
    String* str = data + list->size;
    str->cstr = MemoryReserve<char>(list->_memoryPool, cstrLen + 1);
    str->length = cstrLen;
    strcpy_s(str->cstr, cstrLen + 1, cstr);
    str->cstr[cstrLen] = '\0';
    list->size++;
}
String* StringListGet(StringList* list, i32 ind) {
    if (ind >= list->size) {
        TraceLog(LOG_ERROR, "StringList: index '%i' is out of range");
        return nullptr;
    }
    return list->data + ind;
}

EventHandler EventHandlerCreate() {
    EventHandler eh = {};
    for (i32 i = 0; i < EVENT_COUNT; i++) {
        eh.callbacks[i] = TypeListCreate(TYPE_LIST_PTR, &mdEngine::persistentMemory);
        eh.registrars[i] = TypeListCreate(TYPE_LIST_PTR, &mdEngine::persistentMemory);
    }
    return eh;
}
void EventHandlerRegisterEvent(i32 ind, void* registrar, EventCallbackSignature callback) {
    assert(ind < EVENT_COUNT); // event doesn't exist
    assert(TypeListFindPtr(mdEngine::eventHandler.registrars[ind], registrar) == -1); // event already registered
    TypeListPushBackPtr(mdEngine::eventHandler.callbacks[ind], callback);
    TypeListPushBackPtr(mdEngine::eventHandler.registrars[ind], registrar);
}
void EventHandlerUnregisterEvent(i32 ind, void* registrar) {
    assert(ind < EVENT_COUNT);
    assert(TypeListFindPtr(mdEngine::eventHandler.registrars[ind], registrar) != -1);
    // TODO: remove event
}
void EventHandlerCallEvent(void* caller, i32 ind, void* _args) {
    assert(ind < EVENT_COUNT);
    TypeList* registrars = mdEngine::eventHandler.registrars[ind];
    TypeList* callbacks = mdEngine::eventHandler.callbacks[ind];
    for (i32 i = 0; i < registrars->size; i++) {
        switch(ind) {
            case EVENT_TYPEWRITER_LINE_COMPLETE: {
                EventArgs_TypewriterLineComplete* args = (EventArgs_TypewriterLineComplete*)_args;
                EventCallbackSignature_TypewriterLineComplete callback = (EventCallbackSignature_TypewriterLineComplete)TypeListGetPtr(callbacks, i);
                callback(TypeListGetPtr(registrars, i), args);
            }
            case EVENT_DIALOGUE_OPTIONS_SELECTED: {
                EventArgs_DialogueOptionsSelected* args = (EventArgs_DialogueOptionsSelected*)_args;
                EventCallbackSignature_DialogueOptionsSelected callback = (EventCallbackSignature_DialogueOptionsSelected)TypeListGetPtr(callbacks, i);
                callback(TypeListGetPtr(registrars, i), args);
            }
        }
    }
}

TypeList* TypeListCreate(i32 typeIndex, MemoryPool* memoryPool, i32 capacity) {
    TypeList* list = (TypeList*)MemoryPoolReserve(memoryPool, sizeof(TypeList));
    list->memoryProvider = memoryPool;
    list->typeIndex = typeIndex;
    list->bufferStride = TypeListTypeGetSize(typeIndex);
    list->size = 0;
    list->capacity = 0;
    TypeListSetCapacity(list, capacity);
    return list;
}
void TypeListSetCapacity(TypeList* list, i32 capacity) {
    if (capacity < 0) {
        TraceLog(LOG_WARNING, "%s: Tried changing capacity to below 0", nameof(TypeList));
        assert(false);
        return;
    }
    if (capacity < list->size) {
        TraceLog(LOG_WARNING, "%s: Tried changing capacity to less than list size", nameof(TypeList));
        assert(false);
        return;
    }
    if (capacity == 0) {
        list->buffer = nullptr;
        list->capacity = capacity;
        return;
    }
    if (capacity > 0 && capacity != list->capacity) {
        if (capacity < list->capacity) {
            list->capacity = capacity;
        } else {
            void* newBuffer = MemoryPoolReserve(list->memoryProvider, capacity * list->bufferStride);
            memcpy(newBuffer, list->buffer, list->bufferStride * list->size);
            list->buffer = newBuffer;
            list->capacity = capacity;
        }
    }
}
i32 TypeListTypeGetSize(i32 typeIndex) {
    switch (typeIndex) {
        case TYPE_LIST_I32:
            return sizeof(i32);
        case TYPE_LIST_PTR:
            return sizeof(char*);
        case TYPE_LIST_GAME_OBJECT_DEFINITION:
            return sizeof(GameObjectDefinition);
        default:
            assert(false);
            return -1;
    }
}
void TypeListGrowCapacityIfLimitReached(TypeList* list) {
    if (list->capacity == list->size) {
        if (list->capacity == 0) {
            TypeListSetCapacity(list, 5);
        } else {
            TypeListSetCapacity(list, list->capacity * 5);
        }
    }
}
void TypeListPushBackI32(TypeList* list, i32 val) {
    assert(list->typeIndex == TYPE_LIST_I32);
    TypeListGrowCapacityIfLimitReached(list);
    ((i32*)list->buffer)[list->size] = val;
    list->size++;
}
i32 TypeListGetI32(TypeList* list, i32 ind) {
    assert(list->typeIndex == TYPE_LIST_I32);
    assert(ind < list->size);
    return ((i32*)list->buffer)[ind];
}
void TypeListSetI32(TypeList* list, i32 ind, i32 val) {
    assert(list->typeIndex == TYPE_LIST_I32);
    assert(ind < list->size);
    ((i32*)list->buffer)[ind] = val;
}
i32 TypeListFindI32(TypeList* list, i32 val) {
    assert(list->typeIndex == TYPE_LIST_I32);
    i32* buffer = (i32*)list->buffer;
    for (i32 i = 0; i < list->size; i++) {
        if (buffer[i] == val) {
            return i;
        }
    }
    return -1;
}
void TypeListPushBackPtr(TypeList* list, void* val) {
    assert(list->typeIndex == TYPE_LIST_PTR);
    _TypeListPushBackPtr(list, val);
}
void* TypeListGetPtr(TypeList* list, i32 ind) {
    assert(list->typeIndex == TYPE_LIST_PTR);
    assert(ind < list->size);
    return _TypeListGetPtr(list, ind);
}
void TypeListSetPtr(TypeList* list, i32 ind, void* val) {
    assert(list->typeIndex == TYPE_LIST_PTR);
    assert(ind < list->size);
    _TypeListSetPtr(list, ind, val);
}
i32 TypeListFindPtr(TypeList* list, void* val) {
    assert(list->typeIndex == TYPE_LIST_PTR);
    return _TypeListFindPtr(list, val);
}
void _TypeListPushBackPtr(TypeList* list, void* val) {
    TypeListGrowCapacityIfLimitReached(list);
    ((void**)list->buffer)[list->size] = val;
    list->size++;
}
void* _TypeListGetPtr(TypeList* list, i32 ind) {
    return ((void**)list->buffer)[ind];
}
void _TypeListSetPtr(TypeList* list, i32 ind, void* val) {
    ((void**)list->buffer)[ind] = val;
}
i32 _TypeListFindPtr(TypeList* list, void* val) {
    void** buffer = (void**)list->buffer;
    for (i32 i = 0; i < list->size; i++) {
        if (buffer[i] == val) {
            return i;
        }
    }
    return -1;
}
void TypeListPushBackGameObjectDefinition(TypeList* list, GameObjectDefinition val) {
    assert(list->typeIndex == TYPE_LIST_GAME_OBJECT_DEFINITION);
    TypeListGrowCapacityIfLimitReached(list);
    ((GameObjectDefinition*)list->buffer)[list->size] = val;
    list->size++;
}
GameObjectDefinition* TypeListGetGameObjectDefinition(TypeList* list, i32 ind) {
    assert(list->typeIndex == TYPE_LIST_GAME_OBJECT_DEFINITION);
    assert(ind < list->size);
    return &((GameObjectDefinition*)list->buffer)[ind];
}
void TypeListSetGameObjectDefinition(TypeList* list, i32 ind, GameObjectDefinition val) {
    assert(list->typeIndex == TYPE_LIST_GAME_OBJECT_DEFINITION);
    assert(ind < list->size);
    ((GameObjectDefinition*)list->buffer)[ind] = val;
}
i32 TypeListFindGameObjectDefinitionShallow(TypeList* list, GameObjectDefinition* val) {
    assert(list->typeIndex == TYPE_LIST_GAME_OBJECT_DEFINITION);
    GameObjectDefinition* buffer = (GameObjectDefinition*)list->buffer;
    for (i32 i = 0; i < list->size; i++) {
        if (buffer + i == val) {
            return i;
        }
    }
    return -1;
}

void TypewriterInit(Typewriter* tw) {
    tw->text = nullptr;
    tw->visible = true;
    tw->textCount = 0;
    tw->speed = 16.f;
    tw->autoAdvance = true;
    tw->autoHide = true;
    tw->autoContinueDelay = 1.f;
    tw->autoHideOnEndDelay = 3.f;
    tw->_autoAdvanceCountdown = -1.f;
    tw->x = 0;
    tw->y = 0;
    tw->textDrawingStyle = TextDrawingStyleGetDefault();
}
void TypewriterUpdate(void* _tw) {
    Typewriter* tw = (Typewriter*)_tw;
    if (tw->text == nullptr) {
        return;
    }
    float length = (float)tw->text[tw->textIndex].length;
    float textProgressPrevious = tw->progress;
    tw->progress = fminf(tw->progress + tw->speed * FRAME_TIME, length);
    if (tw->progress == length) {
        i32 endJustReached = textProgressPrevious < tw->progress;
        i32 lastLine = tw->textIndex + 1 == tw->textCount;
        if (endJustReached) {
            TypewriterEvent_LineComplete(tw);
            if (tw->autoAdvance && !lastLine) {
                tw->_autoAdvanceCountdown = tw->autoContinueDelay;
            } else if (tw->autoHide && lastLine) {
                tw->_autoAdvanceCountdown = tw->autoHideOnEndDelay;
            }
        } else {
            tw->_autoAdvanceCountdown = fmaxf(0.f, tw->_autoAdvanceCountdown - FRAME_TIME);
        }

        i32 autoCountdownFinished = tw->_autoAdvanceCountdown == 0.f;
        if (tw->autoAdvance && !lastLine && autoCountdownFinished) {
            _TypewriterAdvanceText(tw);
        } else if (tw->autoHide && lastLine && autoCountdownFinished) {
            _TypewriterReset(tw);
            tw->visible = false;
        }
    }
}
void _TypewriterAdvanceText(Typewriter *tw) {
    if (tw->textIndex + 1 < tw->textCount) {
        tw->textIndex++;
        tw->progress = 0.f;
    }
}
void _TypewriterReset(Typewriter *tw) {
    tw->text = nullptr;
    tw->progress = 0.f;
    tw->textCount = 0;
    tw->textIndex = 0;
}
// TODO: optimize by only measuring text when the rendered string changes
void TypewriterDraw(void* _tw) {
    Typewriter* tw = (Typewriter*)_tw;
    if (!tw->visible || tw->text == nullptr) {
        return;
    }

    String substr = StringSubstr(tw->text[tw->textIndex], 0, (i32)tw->progress);
    v2 textAlign = MeasureTextEx(
        tw->textDrawingStyle.font,
        substr.cstr,
        tw->textDrawingStyle.size,
        tw->textDrawingStyle.charSpacing) / 2.f;
    DrawTextPro(tw->textDrawingStyle.font, substr.cstr, {truncf((float)tw->x), truncf((float)tw->y)}, textAlign, 0.f, tw->textDrawingStyle.size, 1.f, WHITE);
    StringDestroy(substr);
}
GameObject TypewriterPack(Typewriter* tw, const char* instanceName) {
    GameObject go = GameObjectCreate(tw, "Typewriter", instanceName);
    go.Update = TypewriterUpdate;
    go.DrawUi = TypewriterDraw;
    return go;
}
void TypewriterEvent_LineComplete(Typewriter* tw) {
    EventArgs_TypewriterLineComplete args;
    args.lineCurrent = tw->textIndex;
    args.lineCount = tw->textCount;
    EventHandlerCallEvent(tw, EVENT_TYPEWRITER_LINE_COMPLETE, &args);
}
void TypewriterStart(Typewriter* tw, String* str, i32 strCount) {
    tw->visible = true;
    tw->text = str;
    tw->textCount = strCount;
    tw->textIndex = 0;
    tw->progress = 0.f;
}

void DialogueOptionsInit(DialogueOptions* dopt) {
    dopt->index = 0;
    dopt->options = nullptr;
    dopt->count = 0;
    dopt->visible = false;
    dopt->x = 0;
    dopt->y = 0;
    dopt->textStyle = TextDrawingStyleGetDefault();
    dopt->npatchInfo = {{0.f, 0.f, 96.f, 96.f}, 32, 32, 32, 32, NPATCH_NINE_PATCH};
    dopt->npatchTexture = {};
    dopt->optionBoxHeightAdd = 32.f;
    dopt->optionSeparationAdd = 48.f;
}
void DialogueOptionsSet(DialogueOptions* dopt, String* str, i32 strCount) {
    dopt->index = 0;
    dopt->options = str;
    dopt->count = strCount;
}
void DialogueOptionsUpdate(void* _dopt) {
    DialogueOptions* dopt = (DialogueOptions*)_dopt;
    if (!dopt->visible) {
        return;
    }
    i32 move = mdEngine::input.pressed[INPUT_DOWN] - mdEngine::input.pressed[INPUT_UP];
    dopt->index = iwrapi(dopt->index + move, 0, dopt->count);
    if (mdEngine::input.pressed[INPUT_SELECT]) {
        EventArgs_DialogueOptionsSelected args;
        args.count = dopt->count;
        args.index = dopt->index;
        EventHandlerCallEvent(dopt, EVENT_DIALOGUE_OPTIONS_SELECTED, &args);
    }
}
void DialogueOptionsDraw(void* _dopt) {
    DialogueOptions* dopt = (DialogueOptions*)_dopt;
    if (!dopt->visible) {
        return;
    }

    v2* textSize = (v2*)malloc(dopt->count * sizeof(v2));
    v2 boxSize = {0.f, 0.f};
    // TODO: These measurements can be done as preprocessing. No need for the memory allocation every fucking frame
    for (i32 i = 0; i < dopt->count; i++) {
        String option = dopt->options[i];
        textSize[i] = MeasureTextEx(dopt->textStyle.font, option.cstr, dopt->textStyle.size, dopt->textStyle.charSpacing);
        boxSize = Vector2Max(boxSize, textSize[i]);
    }
    boxSize.x += 64.f;
    for (i32 i = 0; i < dopt->count; i++) {
        Color textTint = GRAY;
        Color boxTint = GRAY;
        if (i == dopt->index) {
            textTint = WHITE;
            boxTint = WHITE;
        }

        String option = dopt->options[i];
        i32 x = dopt->x;
        i32 y = dopt->y + i * (i32)(dopt->textStyle.size + dopt->optionSeparationAdd);
        float xf = (float)x;
        float yf = (float)y;
        rect2 dialogueBoxRect = {
            xf - boxSize.x / 2.f,
            yf - boxSize.y / 2.f - dopt->optionBoxHeightAdd / 2.f,
            boxSize.x,
            boxSize.y + dopt->optionBoxHeightAdd};
        DrawTextureNPatch(
            dopt->npatchTexture,
            dopt->npatchInfo,
            dialogueBoxRect,
            {0.f, 0.f},
            0.f,
            boxTint);
        DrawTextPro(
            dopt->textStyle.font,
            option.cstr,
            {xf, yf},
            textSize[i] / 2.0,
            0.f,
            dopt->textStyle.size,
            dopt->textStyle.charSpacing,
            textTint);
    }
    free(textSize);
}
GameObject DialogueOptionsPack(DialogueOptions* dopt, const char* instanceName) {
    GameObject go = GameObjectCreate(dopt, "Dialogue Options", instanceName);
    go.data = dopt;
    go.Update = DialogueOptionsUpdate;
    go.DrawUi = DialogueOptionsDraw;
    return go;
}

void DialogueSequenceInit(DialogueSequence* dseq, i32 ind) {
    dseq->sectionIndex = 0;
    dseq->sections = TypeListCreate(TYPE_LIST_PTR, &mdEngine::sceneMemory, 50);
    TypewriterInit(&dseq->typewriter);
    dseq->typewriter.autoAdvance = true;
    dseq->typewriter.autoHide = false;
    DialogueOptionsInit(&dseq->options);

    switch (ind) {
        case 0:
        {
            DialogueSequenceSection* dss = nullptr;
            StringList* text = nullptr;
            StringList* opt = nullptr;
            TypeList* link = nullptr;

            dss = DialogueSequenceSectionCreate(3, 3);
            text = dss->text;
            opt = dss->options;
            link = dss->link;
            StringListAdd(text, "This is definitely text");
            StringListAdd(text, "Surely this is pretty close in memory");
            StringListAdd(text, "Hopefully I won't run out lol");
            StringListAdd(opt, "You will");
            StringListAdd(opt, "No");
            StringListAdd(opt, "Repeat that please");
            TypeListPushBackI32(link, 1);
            TypeListPushBackI32(link, -1);
            TypeListPushBackI32(link, 0);
            TypeListPushBackPtr(dseq->sections, dss);

            dss = DialogueSequenceSectionCreate(1, 0);
            text = dss->text;
            opt = dss->options;
            link = dss->link;
            StringListAdd(text, "I'm glad we agree, truly!");
            TypeListPushBackI32(link, -1);
            TypeListPushBackPtr(dseq->sections, dss);
            break;
        }
    }

    DialogueSequenceSectionStart(dseq, 0);
    EventHandlerRegisterEvent(EVENT_TYPEWRITER_LINE_COMPLETE, dseq, (EventCallbackSignature)DialogueSequenceHandleTypewriter_TextAdvance);
    EventHandlerRegisterEvent(EVENT_DIALOGUE_OPTIONS_SELECTED, dseq, (EventCallbackSignature)DialogueSequenceHandleOptions_Selected);
}
void DialogueSequenceSectionStart(DialogueSequence* dseq, i32 ind) {
    DialogueSequenceSection* dss = DialogueSequenceSectionGet(dseq, ind);
    TypewriterStart(&dseq->typewriter, dss->text->data, dss->text->size);
    DialogueOptionsSet(&dseq->options, dss->options->data, dss->options->size);
    if (dss->options->size == 0) {
        dseq->typewriter.autoHide = true;
    }
}
void DialogueSequenceUpdate(void* _dseq) {
    DialogueSequence* dseq = (DialogueSequence*)_dseq;
    DialogueOptionsUpdate(&dseq->options);
    TypewriterUpdate(&dseq->typewriter);
}
void DialogueSequenceDrawUi(void* _dseq) {
    DialogueSequence* dseq = (DialogueSequence*)_dseq;
    DialogueOptionsDraw(&dseq->options);
    TypewriterDraw(&dseq->typewriter);
}

DialogueSequenceSection* DialogueSequenceSectionGet(DialogueSequence* dseq, i32 ind) {
    return (DialogueSequenceSection*)TypeListGetPtr(dseq->sections, ind);
}
DialogueSequenceSection* DialogueSequenceSectionCreate(i32 textCount, i32 optionCount) {
    DialogueSequenceSection* dss = MemoryReserve<DialogueSequenceSection>(&mdEngine::sceneMemory);
    dss->text = MemoryReserve<StringList>(&mdEngine::sceneMemory);
    StringListInit(dss->text, textCount, &mdEngine::sceneMemory);
    dss->options = MemoryReserve<StringList>(&mdEngine::sceneMemory);
    StringListInit(dss->options, optionCount, &mdEngine::sceneMemory);
    dss->link = TypeListCreate(TYPE_LIST_PTR, &mdEngine::sceneMemory);
    return dss;
}
GameObject DialogueSequencePack(DialogueSequence* dseq, const char* instanceName) {
    GameObject go = GameObjectCreate(dseq, "Dialogue Sequence", instanceName);
    go.data = dseq;
    go.Update = DialogueSequenceUpdate;
    go.DrawUi = DialogueSequenceDrawUi;
    return go;
}
void DialogueSequenceHandleTypewriter_TextAdvance(void* _dseq, EventArgs_TypewriterLineComplete* _args) {
    DialogueSequence* dseq = (DialogueSequence*)_dseq;
    EventArgs_TypewriterLineComplete* args = (EventArgs_TypewriterLineComplete*)_args;
    DialogueSequenceSection* dss = DialogueSequenceSectionGet(dseq, dseq->sectionIndex);
    if (args->lineCurrent == args->lineCount - 1 && dss->options->size > 0) {
        dseq->options.visible = true;
    }
}
void DialogueSequenceHandleOptions_Selected(void* _dseq, EventArgs_DialogueOptionsSelected* args) {
    DialogueSequence* dseq = (DialogueSequence*)_dseq;
    DialogueSequenceSection* dss = DialogueSequenceSectionGet(dseq, dseq->sectionIndex);
    i32 nextSectionIndex = TypeListGetI32(dss->link, args->index);
    if (nextSectionIndex == -1) {
        dseq->options.visible = false;
        dseq->typewriter.visible = false;
    } else {
        dseq->options.visible = false;
        dseq->sectionIndex = nextSectionIndex;
        DialogueSequenceSectionStart(dseq, dseq->sectionIndex);
    }
}

void HeightmapInit(Heightmap* hm, HeightmapGenerationInfo info) {
    const i32 resdiv = info.resdiv;
    const Image *image = info.heightmapImage;
    const i32 stride = PixelformatGetStride(image->format);
    const i32 heightDataWidth = image->width >> resdiv;
    const i32 heightDataHeight = image->height >> resdiv;

    float *heightData = (float*)malloc(heightDataWidth * heightDataHeight * sizeof(float));
    byte* data = (byte*)info.heightmapImage->data;
    i32 dataW = heightDataWidth;
    i32 imgw = image->width;

    for (i32 x = 0; x < heightDataWidth; x++) {
        for (i32 y = 0; y < heightDataHeight; y++) {
            byte value = data[((x << resdiv) + ((y << resdiv) * imgw)) * stride];
            heightData[x + y * dataW] = ((float)value / 255.f) * info.size.y;
        }
    }

    hm->heightData = heightData;
    hm->heightDataWidth = heightDataWidth;
    hm->heightDataHeight = heightDataHeight;
    hm->size = info.size;
    hm->position = info.position;
}
float HeightmapSampleHeight(Heightmap* hm, float x, float z) {
    v2 rectBegin = {hm->position.x, hm->position.z};
    v2 rectEnd = rectBegin + v2{hm->size.x, hm->size.z};
    if (PointInRectangle(rectBegin, rectEnd, {x, z})) {
        v2 abspos = v2{x, z} - rectBegin;
        v2 normpos = abspos / v2{hm->size.x, hm->size.z};
        v2 datapos = normpos * v2{(float)hm->heightDataWidth, (float)hm->heightDataHeight};
        v2 dataposFract = Vector2Fract(datapos);
        i32 dataX = (i32)datapos.x;
        i32 dataY = (i32)datapos.y;
        bool nextX = dataX + 1 < hm->heightDataWidth;
        bool nextY = dataY + 1 < hm->heightDataHeight;
        // TODO: Could probably just pad the data by 1 on the right and bottom
        float a = hm->heightData[dataX + dataY * hm->heightDataWidth];
        float b = nextX ? hm->heightData[(dataX+1) + dataY * hm->heightDataWidth] : 0.f;
        float c = nextY ? hm->heightData[dataX + (dataY + 1) * hm->heightDataWidth] : 0.f;
        float d = nextX && nextY ? hm->heightData[(dataX + 1) + (dataY + 1) * hm->heightDataWidth] : 0.f;
        return Lerp(Lerp(a, b, dataposFract.x), Lerp(c, d, dataposFract.x), dataposFract.y);
    }
    return 0.f;
}
void HeightmapFree(Heightmap* hm) {
    free(hm->heightData);
    memset(hm, NULL, sizeof(Heightmap));
}

void InstanceMeshRenderDataDraw3d(void* _imrd) {
    InstanceMeshRenderData* imrd = (InstanceMeshRenderData*)_imrd;
    DrawMeshInstancedOptimized(imrd->mesh, imrd->material, imrd->transforms, imrd->instanceCount);
}
GameObject InstanceMeshRenderDataPack(InstanceMeshRenderData* forest, const char* instanceName = "<unnamed>") {
    GameObject go = GameObjectCreate(forest, "Forest Manager", instanceName);
    go.Draw3d = InstanceMeshRenderDataDraw3d;
    return go;
}

ModelInstance* ModelInstanceCreate(MemoryPool* mp) {
    ModelInstance* mi = MemoryReserve<ModelInstance>(mp);
    mi->scale = 1.f;
    mi->tint = WHITE;
    return mi;
}
void ModelInstanceDraw3d(void* _mi) {
    ModelInstance *mi = (ModelInstance*)_mi;
    DrawModel(mi->model, mi->position, mi->scale, mi->tint);
}
void ModelInstanceDrawImGui(void* _mi) {
    ModelInstance* mi = (ModelInstance*)_mi;
    ImGui::DragFloat3("Position", (float*)&mi->position, 0.05f);
    ImGui::DragFloat("Scale", (float*)&mi->scale, 0.05f);
}
GameObject ModelInstancePack(ModelInstance* mi, const char* instanceName) {
    GameObject go = GameObjectCreate(mi, "Model Instance", instanceName);
    go.Draw3d = ModelInstanceDraw3d;
    go.DrawImGui = ModelInstanceDrawImGui;
    return go;
}

void ParticleSystemInit(ParticleSystem* psys, Texture texture, Material material) {
    psys->_material = material;
    psys->_material.maps[MATERIAL_MAP_ALBEDO].texture = texture;
    psys->_quad = GenMeshPlane(1.f, 1.f, 1, 1);
    psys->_transforms = (mat4*)RL_CALLOC(PARTICLE_SYSTEM_MAX_PARTICLES, sizeof(mat4));
    psys->count = 64;
    for (i32 i = 0; i < psys->count; i++) {
        psys->_transforms[i] = MatrixTranslate(
            GetRandomValueF(-10.f, 10.f),
            GetRandomValueF(-10.f, 10.f),
            GetRandomValueF(-10.f, 10.f));
    }
}
void ParticleSystemFree(void* _psys) {
    ParticleSystem* psys = (ParticleSystem*)_psys;
    UnloadMesh(psys->_quad);
    RL_FREE(psys->_transforms);
}
void ParticleSystemUpdate(void* _psys) {
    ParticleSystem* psys = (ParticleSystem*)_psys;
    v3 stepVelocity = psys->velocity * FRAME_TIME;
    mat4 transpose = MatrixTranslate(stepVelocity.x, stepVelocity.y, stepVelocity.z);
    for (i32 i = 0; i < psys->count; i++) {
        psys->_transforms[i] *= transpose;
    }
}
void ParticleSystemDraw3d(void* _psys) {
    ParticleSystem* psys = (ParticleSystem*)_psys;
    DrawMeshInstanced(psys->_quad, psys->_material, psys->_transforms, psys->count);
}
GameObject ParticleSystemPack(ParticleSystem *psys, const char* instanceName) {
    GameObject go = GameObjectCreate(psys, "Particle System", instanceName);
    go.Update = ParticleSystemUpdate;
    go.Draw3d = ParticleSystemDraw3d;
    go.Free = ParticleSystemFree;
    return go;
}

void TextureInstanceInit(TextureInstance* ti, Texture* tex) {
    ti->texture = tex;
    if (ti->texture == nullptr) {
        return;
    }
    ti->position = {0.f, 0.f};
    ti->origin = {0.f, 0.f};
    ti->rotation = 0.f;
    ti->tint = WHITE;

    ti->_drawSource = {0.f, 0.f, (float)tex->width, (float)tex->height};
    ti->_drawDestination = {0.f, 0.f, (float)tex->width, (float)tex->height};
    ti->_scale = {1.f, 1.f};
}
void TextureInstanceSetSize(TextureInstance* ti, v2 size) {
    ti->_drawDestination.width = size.x;
    ti->_drawDestination.height = size.y;
}
void TextureInstanceDrawUi(void* _ti) {
    TextureInstance* ti = (TextureInstance*)_ti;
    DrawTexturePro(*ti->texture, ti->_drawSource, ti->_drawDestination, ti->origin, ti->rotation, ti->tint);
}
GameObject TextureInstancePack(TextureInstance* ti, const char* instanceName) {
    GameObject go = GameObjectCreate(ti, "Texture Instance", instanceName);
    go.DrawUi = TextureInstanceDrawUi;
    return go;
}

void SkyboxDraw3d(void* _skybox) {
    Skybox* skybox = (Skybox*)_skybox;
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
        DrawModel(skybox->model, {0.f}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
GameObject SkyboxPack(Skybox* sb) {
    GameObject go = GameObjectCreate(sb, "Skybox");
    go.Draw3d = SkyboxDraw3d;
    return go;
}
#endif // __MD_ENGINE_H