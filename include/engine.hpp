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

#include <vector>
#include <unordered_map>
#include <string>

#include "typedefs.hpp"
#include "shadinclude.hpp"

#define nameof(x) #x
#define FRAME_TIME 1.f / 60.f
#define FRAMERATE 60
#define TAU PI * 2.f
#define KILOBYTES(bytes)(bytes * 1000)
#define MEGABYTES(bytes)(bytes * 1000000)
#define GIGABYTES(bytes)(bytes * 1000000000)
#define global_variable static // when used in global scope
#define local_persist static // when used inside a function
#define struct_internal static // when used in a struct definition

#define LOAD_MODEL(path)(LoadModel(TextFormat("resources/models/%s", path)))
inline Shader LOAD_SHADER(const char* vsPath, const char* fsPath) {
    std::string vsCodeStr = Shadinclude::load(std::string(TextFormat("resources/shaders/%s", vsPath)));
    std::string fsCodeStr = Shadinclude::load(std::string(TextFormat("resources/shaders/%s", fsPath)));
    if (vsCodeStr.empty() || fsCodeStr.empty()) {
        if (vsCodeStr.empty()) {
            TraceLog(LOG_ERROR, TextFormat("%s: Couldn't load vertex shader '%s'", nameof(LOAD_SHADER), vsPath));
        }
        if (fsCodeStr.empty()) {
            TraceLog(LOG_ERROR, TextFormat("%s: Couldn't load fragment shader '%s'", nameof(LOAD_SHADER), fsPath));
        }
        return {};
    }
    const char* vsCode = vsCodeStr.c_str();
    const char* fsCode = fsCodeStr.c_str();
    return LoadShaderFromMemory(vsCode, fsCode);
}
#define LOAD_TEXTURE(path)(LoadTexture(TextFormat("resources/textures/%s", path)))
#define LOAD_IMAGE(path)(LoadImage(TextFormat("resources/textures/%s", path)))
#define LOAD_FONT(path)(LoadFont(TextFormat("resources/fonts/%s", path)))
#define MatrixRotateRoll(rad) MatrixRotateX(rad)
#define MatrixRotateYaw(rad) MatrixRotateY(rad)
#define MatrixRotatePitch(rad) MatrixRotateZ(rad)

/*
    Custom math structs
*/
enum MD_TRANSFORM_ROTATION_ORDER {
    TRANSFORM_ROTATION_ORDER_XYZ,
    TRANSFORM_ROTATION_ORDER_XZY,
    TRANSFORM_ROTATION_ORDER_YXZ,
    TRANSFORM_ROTATION_ORDER_YZX,
    TRANSFORM_ROTATION_ORDER_ZXY,
    TRANSFORM_ROTATION_ORDER_ZYX
};
struct MdTransform {
    v3 _translation;
    v3 _scale;
    v3 _rotation;
    mat4 matrix;
    i32 rotationOrder;
    struct_internal const char* mdTransformRotationOrderNames[];
};
const char* MdTransform::mdTransformRotationOrderNames[] = {
    "xyz",
    "xzy",
    "yxz",
    "yzx",
    "zxy",
    "zyx\0"
};
MdTransform TransformCreate() {
    MdTransform transform = {};
    transform._scale = {1.f, 1.f, 1.f};
    transform.matrix = MatrixIdentity();
    return transform;
}
void TransformUpdate(MdTransform* transform) {
    mat4 matrix;
    v3 rotation = transform->_rotation;
    switch (transform->rotationOrder) {
        case TRANSFORM_ROTATION_ORDER_XYZ:
            matrix = MatrixRotateX(rotation.x);
            matrix *= MatrixRotateY(rotation.y);
            matrix *= MatrixRotateZ(rotation.z);
            break;
        case TRANSFORM_ROTATION_ORDER_XZY:
            matrix = MatrixRotateX(rotation.x);
            matrix *= MatrixRotateZ(rotation.z);
            matrix *= MatrixRotateY(rotation.y);
            break;
        case TRANSFORM_ROTATION_ORDER_YXZ:
            matrix *= MatrixRotateY(rotation.y);
            matrix = MatrixRotateX(rotation.x);
            matrix *= MatrixRotateZ(rotation.z);
            break;
        case TRANSFORM_ROTATION_ORDER_YZX:
            matrix *= MatrixRotateY(rotation.y);
            matrix *= MatrixRotateZ(rotation.z);
            matrix = MatrixRotateX(rotation.x);
            break;
        case TRANSFORM_ROTATION_ORDER_ZXY:
            matrix *= MatrixRotateZ(rotation.z);
            matrix = MatrixRotateX(rotation.x);
            matrix *= MatrixRotateY(rotation.y);
            break;
        case TRANSFORM_ROTATION_ORDER_ZYX:
            matrix *= MatrixRotateZ(rotation.z);
            matrix *= MatrixRotateY(rotation.y);
            matrix = MatrixRotateX(rotation.x);
            break;
    }
    matrix *= MatrixScale(transform->_scale.x, transform->_scale.y, transform->_scale.z);
    matrix *= MatrixTranslate(transform->_translation.x, transform->_translation.y, transform->_translation.z);
    transform->matrix = matrix;
}
void TransformSetTranslation(MdTransform* transform, v3 translation) {
    transform->_translation = translation;
    TransformUpdate(transform);
}
void TransformSetScale(MdTransform* transform, v3 scale) {
    transform->_scale = scale;
    TransformUpdate(transform);
}
void TransformSetRotation(MdTransform* transform, v3 rotation) {
    transform->_rotation = rotation;
    TransformUpdate(transform);
}
void TransformDrawImGui(MdTransform* transform) {
    if (ImGui::DragFloat3("Position", (float*)&transform->_translation, 0.05f)) {
        TransformUpdate(transform);
    }
    if (ImGui::DragFloat3("Scale", (float*)&transform->_scale, 0.05f)) {
        TransformUpdate(transform);
    }
    if (ImGui::DragFloat3("Rotation", (float*)&transform->_rotation, 0.05f)) {
        TransformUpdate(transform);
    }
    if (ImGui::Combo("Rotation Order", &transform->rotationOrder, MdTransform::mdTransformRotationOrderNames[0])) {
        TransformUpdate(transform);
    }
}

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
void DrawModelTransform(Model model, mat4 transform, Color tint) {
    for (i32 i = 0; i < model.meshCount; i++) {
        DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], transform);
    }
}

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
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        case PIXELFORMAT_UNCOMPRESSED_R16:
        case PIXELFORMAT_UNCOMPRESSED_R32:
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
    Engine (raylib dependency only)
*/
// TODO: Decouple memory allocation calls.
// TODO: Add functionality to extend the top memory location of the buffer when the same thing is requesting more memory
struct MemoryPool {
    void* buffer;
    u64 location;
    u64 size;
    u64 alignment;
};
MemoryPool MemoryPoolCreate(u64 size);
MemoryPool MemoryPoolCreateInsideMemoryPool(MemoryPool* srcMp, u64 size);
void MemoryPoolExpand(MemoryPool* mp);
void MemoryPoolDestroy(MemoryPool* mp);
void* MemoryPoolReserve(MemoryPool* mp, u64 size);
void MemoryPoolClear(MemoryPool* mp);
template <typename T>
T* MemoryReserve(MemoryPool* mp);
template <typename T>
T* MemoryReserve(MemoryPool* mp, u64 size);

struct TextDrawingStyle {
    Color color;
    Font font;
    float size;
    float charSpacing;
};
enum INPUT {
    INPUT_ACCELERATE,
    INPUT_BREAK,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,
    INPUT_UP,
    INPUT_DEBUG_TOGGLE,
    INPUT_DEBUG_ACCEPT,
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
// TODO: A system for consuming input so that it doesn't propagate to things that are not supposed to have them
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
struct Tween;
struct VariableDynamicBuffer;
struct GameObjectDefinition;
struct GameObject;
struct StringList;
struct EventArgs_TypewriterLineComplete;
struct EventArgs_DialogueOptionsSelected;
struct TypeList;
struct Typewriter;
struct DialogueOptions;
struct Heightmap;
struct InstanceRenderer;

// TODO: Consider using generics for this
struct Tween {
    void* data;
    void* start;
    void* end;
    i32 dataType;
    float duration;
    float _time;
    bool _started;
};
Tween TweenCreate(void* variable, void* start, void* end, i32 dataType, float duration);
void TweenStart(Tween* t);
void TweenStep(Tween* t);
void _TweenEnd(Tween* t);

// TODO: Consider creating a list that returns variable dynamic buffer entries rather than having 3 separate lists
struct VariableDynamicBuffer {
    i32 variableCount;
    TypeList* types;
    TypeList* locations;
    MemoryPool memoryPool;
    static constexpr i32 BUFFER_SIZE = 64; // TODO: Make buffer size not hardcoded
};
VariableDynamicBuffer VariableDynamicBufferCreate(MemoryPool* mp);
// TODO: Unify this with the TypeList types
i32 VariableDynamicBufferPush(VariableDynamicBuffer* vdb, void* value, i32 type);
void* VariableDynamicBufferGet(VariableDynamicBuffer* vdb, i32 index, i32 type);
void VariableDynamicBufferSet(VariableDynamicBuffer* vdb, i32 index, i32 type, void* value);

typedef void*(*GameInstanceCreateFunction)(MemoryPool*);
typedef void(*GameInstanceEventFunction)(void*);
struct GameObjectDefinition {
    GameInstanceCreateFunction Create;
    GameInstanceEventFunction Update;
    GameInstanceEventFunction Draw3d;
    GameInstanceEventFunction DrawUi;
    GameInstanceEventFunction Free;
    GameInstanceEventFunction DrawImGui;
    const char* objectName;
};
GameObjectDefinition GameObjectDefinitionCreate(const char* objectName, GameInstanceCreateFunction createFunc, MemoryPool* mp);
typedef void (*GameObjectScriptFunc)(GameObject* obj, void* inst);
struct GameObject {
    GameInstanceEventFunction Update;
    GameInstanceEventFunction Draw3d;
    GameInstanceEventFunction DrawUi;
    GameInstanceEventFunction Free;
    GameInstanceEventFunction DrawImGui;
    GameObjectScriptFunc UpdateScript;
    std::unordered_map<std::string, i32> variableIndices;
    VariableDynamicBuffer variableBuffer;
    void* data;
    const char* objectName;
    const char* instanceName;
    i32 id;
    struct_internal i32 idCounter;
    bool visible;
    bool active;
};
i32 GameObject::idCounter = 100000;
GameObject GameObjectCreate(void* data, MemoryPool* mp, const char* objectName, const char* instanceName = "<unnamed>");
void GameObjectAddScript(GameObject* obj, GameObjectScriptFunc initScript, GameObjectScriptFunc updateScript);
void GameObjectsUpdate(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsDraw3d(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsDrawUi(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsFree(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsDrawImGui(GameObject* gameObjects, i32 gameObjectCount);
void InstanceVariablePush(std::string name, i32 value);
void InstanceVariablePush(std::string name, byte value);
void InstanceVariablePush(std::string name, void* value);
void InstanceVariablePush(std::string name, float value);
void InstanceVariablePush(std::string name, Tween value);
template <typename T> T InstanceVariableGet(std::string name);
void InstanceVariableSet(std::string name, i32 value);
void InstanceVariableSet(std::string name, byte value);
void InstanceVariableSet(std::string name, void* value);
void InstanceVariableSet(std::string name, float value);
void InstanceVariableSet(std::string name, Tween value);
void _InstanceVariablePush(std::string name, i32 type, void* value);
void* _InstanceVariableGet(std::string name, i32 type);
void _InstanceVariableSet(std::string name, i32 type, void* value);

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

// TODO: Support U64
struct TypeList {
    void* buffer;
    MemoryPool* memoryProvider;
    i32 bufferStride;
    i32 size;
    i32 capacity;
    i32 typeIndex;
};
TypeList* TypeListCreate(i32 typeIndex, MemoryPool* memoryPool, i32 capacity = 5);
void TypeListSetCapacity(TypeList* list, i32 capacity);
void TypeListGrowCapacityIfLimitReached(TypeList* list);
void TypeListResize(TypeList* list, i32 size);
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

struct HeightmapGenerationInfo {
    Image *image;
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

struct InstanceRenderer {
    float16 *transforms;
    i32 instanceCount;
    Mesh mesh;
    Material* material;
};
void* InstanceRendererCreate(MemoryPool* mp);
void InstanceRendererDraw3d(InstanceRenderer* is);

/*
    Game Objects
*/
struct DialogueSequence;
struct DialogueSequenceSection;
struct ModelInstance;
struct ModelInstanceInstanced;
struct MeshInstance;
struct ParticleSystem;
struct TextureInstance;
struct Skybox;

struct DialogueSequence {
    Typewriter typewriter;
    DialogueOptions options;
    TypeList* sections;
    i32 sectionIndex;
    bool active;
};
void* DialogueSequenceCreate(MemoryPool* mp);
void DialogueSequenceUpdate(DialogueSequence* dseq);
void DialogueSequenceDrawUi(DialogueSequence* dseq);
enum DIALOGUE_SEQUENCE_LAYOUT {
    DIALOGUE_SEQUENCE_LAYOUT_PLACEHOLDER,
    DIALOGUE_SEQUENCE_LAYOUT_PLACEHOLDER_BOXLESS,
    DIALOGUE_SEQUENCE_LAYOUT_COUNT
};
void DialogueSequenceSetLayout(DialogueSequence* dseq, i32 layout);
void DialogueSequenceHandleTypewriter_TextAdvance(DialogueSequence* _dseq, EventArgs_TypewriterLineComplete* _args);
void DialogueSequenceHandleOptions_Selected(DialogueSequence* dseq, EventArgs_DialogueOptionsSelected* _args);
struct DialogueSequenceSection {
    StringList* text;
    StringList* options;
    TypeList* link;
};
void DialogueSequenceSectionStart(DialogueSequence* dseq, i32 ind);
DialogueSequenceSection* DialogueSequenceSectionGet(DialogueSequence* dseq, i32 ind);
DialogueSequenceSection* DialogueSequenceSectionCreate(i32 textCount, i32 optionCount, MemoryPool* mp);

struct ModelInstance {
    Model model;
    Color tint;
    MdTransform transform;
};
void* ModelInstanceCreate(MemoryPool* mp);
void ModelInstanceDraw3d(ModelInstance* mi);
void ModelInstanceDrawImGui(ModelInstance* mi);

struct MeshInstance {
    Mesh mesh;
    Material material;
    Color tint;
    MdTransform transform;
};
void* MeshInstanceCreate(MemoryPool* mp);
void MeshInstanceDraw3d(MeshInstance* mi);
void MeshInstanceDrawImGui(MeshInstance* mi);

#define PARTICLE_SYSTEM_MAX_PARTICLES 512
struct ParticleSystem {
    mat4* _transforms;
    Mesh _quad;
    Material* _material;
    v3 velocity;
    i32 count;
};
void* ParticleSystemCreate(MemoryPool* mp);
void ParticleSystemFree(ParticleSystem* psys);
void ParticleSystemUpdate(ParticleSystem* psys);
void ParticleSystemDraw3d(ParticleSystem* psys);

struct TextureInstance {
    Color tint;
    v2 origin;
    Shader* shader;
    Texture* _texture;
    float _rotation;
    v2 _position;
    v2 _scale;
    rect2 _drawSource;
    rect2 _drawDestination;
    // TODO: Temporary remove once texture selection combo has been moved out of ImGui function!!!!!!
    i32 _imguiTextureIndex = -1;
};
// TODO: Implement texture instance set texture
void* TextureInstanceCreate(MemoryPool* mp);
void TextureInstanceDrawUi(TextureInstance* ti);
void TextureInstanceDrawImGui(TextureInstance* ti);
void TextureInstanceSetTexture(TextureInstance* ti, Texture* texture);
void TextureInstanceSetSize(TextureInstance* ti, v2 size);
void TextureInstanceSetPosition(TextureInstance* ti, v2 position);

struct Skybox {
    Model model;
    Shader* shader;
    Texture _texture;
};
void* SkyboxCreate(MemoryPool* mp);
void SkyboxInit(Skybox* sb, Shader* shader, Image* image);
void SkyboxDraw3d(Skybox* sb);
void SkyboxFree(Skybox* sb);

/*
    Engine dependent utility definitions
*/
const char* CstringDuplicate(const char* cstr, MemoryPool* mm);

/*
    Init
*/
#define _MD_GAME_ENGINE_OBJECT_COUNT_MAX 500

enum MD_TYPES {
    MD_TYPE_I32,
    MD_TYPE_BYTE,
    MD_TYPE_PTR,
    MD_TYPE_FLOAT,
    MD_TYPE_TWEEN,
    MD_TYPE_GAME_OBJECT_DEFINITION,
    MD_TYPE_COUNT
};
i32 MdTypeGetSize(i32 type) {
    switch (type) {
        case MD_TYPE_I32:
            return sizeof(i32);
        case MD_TYPE_BYTE:
            return sizeof(byte);
        case MD_TYPE_PTR:
            return sizeof(void*);
        case MD_TYPE_FLOAT:
            return sizeof(float);
        case MD_TYPE_TWEEN:
            return sizeof(Tween);
        case MD_TYPE_GAME_OBJECT_DEFINITION:
            return sizeof(GameObjectDefinition);
        default:
            return -1;
            break;
    }
}

namespace mdEngine {
    bool initialized = false;
    MemoryPool scratchMemory;
    MemoryPool sceneMemory;
    MemoryPool persistentMemory;
    MemoryPool engineMemory;
    EventHandler eventHandler;
    GameObject* currentGameObjectInstance;
    Input input;
    Texture missingTexture;
    TextDrawingStyle textDrawingStyleDefault;
    std::unordered_map<std::string, void*> groups = std::unordered_map<std::string, void*>();
    GameObjectDefinition gameObjectDefinitions[_MD_GAME_ENGINE_OBJECT_COUNT_MAX];
    bool gameObjectIsDefined[_MD_GAME_ENGINE_OBJECT_COUNT_MAX];
    Shader passthroughShader;
};

enum MD_GAME_ENGINE_OBJECTS {
    OBJECT_DIALOGUE_SEQUENCE,
    OBJECT_MODEL_INSTANCE,
    OBJECT_MESH_INSTANCE,
    OBJECT_INSTANCE_RENDERER,
    OBJECT_PARTICLE_SYSTEM,
    OBJECT_TEXTURE_INSTANCE,
    OBJECT_SKYBOX,
    _MD_GAME_ENGINE_OBJECTS_COUNT
};

void MdEngineRegisterObject(GameObjectDefinition def, i32 ind) {
    assert(ind < _MD_GAME_ENGINE_OBJECT_COUNT_MAX);
    assert(!mdEngine::gameObjectIsDefined[ind]);
    mdEngine::gameObjectDefinitions[ind] = def;
    mdEngine::gameObjectIsDefined[ind] = true;
}

// TODO: Move definition creation to where the object is so that it becomes more easily editable
void MdEngineRegisterObjects() {
    MemoryPool* mp = &mdEngine::engineMemory;
    GameObjectDefinition def = {};
    def = GameObjectDefinitionCreate("Dialogue Sequence", DialogueSequenceCreate, mp);
    def.DrawUi = (GameInstanceEventFunction)DialogueSequenceDrawUi;
    def.Update = (GameInstanceEventFunction)DialogueSequenceUpdate;
    MdEngineRegisterObject(def, OBJECT_DIALOGUE_SEQUENCE);

    def = GameObjectDefinitionCreate("Model Instance", ModelInstanceCreate, mp);
    def.Draw3d = (GameInstanceEventFunction)ModelInstanceDraw3d;
    def.DrawImGui = (GameInstanceEventFunction)ModelInstanceDrawImGui;
    MdEngineRegisterObject(def, OBJECT_MODEL_INSTANCE);

    def = GameObjectDefinitionCreate("Mesh Instance", MeshInstanceCreate, mp);
    def.Draw3d = (GameInstanceEventFunction)MeshInstanceDraw3d;
    def.DrawImGui = (GameInstanceEventFunction)MeshInstanceDrawImGui;
    MdEngineRegisterObject(def, OBJECT_MESH_INSTANCE);

    def = GameObjectDefinitionCreate("Instance Renderer", InstanceRendererCreate, mp);
    def.Draw3d = (GameInstanceEventFunction)InstanceRendererDraw3d;
    MdEngineRegisterObject(def, OBJECT_INSTANCE_RENDERER);

    def = GameObjectDefinitionCreate("Particle System", ParticleSystemCreate, mp);
    def.Draw3d = (GameInstanceEventFunction)ParticleSystemDraw3d;
    def.Update = (GameInstanceEventFunction)ParticleSystemUpdate;
    def.Free = (GameInstanceEventFunction)ParticleSystemFree;
    MdEngineRegisterObject(def, OBJECT_PARTICLE_SYSTEM);

    def = GameObjectDefinitionCreate("Texture Instance", TextureInstanceCreate, mp);
    def.DrawUi = (GameInstanceEventFunction)TextureInstanceDrawUi;
    def.DrawImGui = (GameInstanceEventFunction)TextureInstanceDrawImGui;
    MdEngineRegisterObject(def, OBJECT_TEXTURE_INSTANCE);

    def = GameObjectDefinitionCreate("Skybox", SkyboxCreate, mp);
    def.Draw3d = (GameInstanceEventFunction)SkyboxDraw3d;
    MdEngineRegisterObject(def, OBJECT_SKYBOX);
}

Shader MdEngineLoadPassthroughShader() {
    // Vertex shader directly defined, no external file required
    const char *defaultVShaderCode =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330                       \n"
    "in vec3 vertexPosition;            \n"
    "in vec2 vertexTexCoord;            \n"
    "in vec4 vertexColor;               \n"
    "out vec2 fragTexCoord;             \n"
    "out vec4 fragColor;                \n"
#endif

#if defined(GRAPHICS_API_OPENGL_ES3)
    "#version 300 es                    \n"
    "precision mediump float;           \n"     // Precision required for OpenGL ES3 (WebGL 2) (on some browsers)
    "in vec3 vertexPosition;            \n"
    "in vec2 vertexTexCoord;            \n"
    "in vec4 vertexColor;               \n"
    "out vec2 fragTexCoord;             \n"
    "out vec4 fragColor;                \n"
#elif defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "precision mediump float;           \n"     // Precision required for OpenGL ES2 (WebGL) (on some browsers)
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#endif

    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vertexTexCoord; \n"
    "    fragColor = vertexColor;       \n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
    "}                                  \n";

    // Fragment shader directly defined, no external file required
    const char *defaultFShaderCode =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330       \n"
    "in vec2 fragTexCoord;              \n"
    "in vec4 fragColor;                 \n"
    "out vec4 finalColor;               \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
    "    finalColor = texelColor*colDiffuse*fragColor;        \n"
    "}                                  \n";
#endif

#if defined(GRAPHICS_API_OPENGL_ES3)
    "#version 300 es                    \n"
    "precision mediump float;           \n"     // Precision required for OpenGL ES3 (WebGL 2)
    "in vec2 fragTexCoord;              \n"
    "in vec4 fragColor;                 \n"
    "out vec4 finalColor;               \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
    "    finalColor = texelColor*colDiffuse*fragColor;        \n"
    "}                                  \n";
#elif defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "precision mediump float;           \n"     // Precision required for OpenGL ES2 (WebGL)
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#endif
    return LoadShaderFromMemory(defaultVShaderCode, defaultFShaderCode);
}

void MdEngineInit(u64 scratchMemorySize, u64 sceneMemorySize, u64 persistentMemorySize, u64 engineMemorySize) {
    if (mdEngine::initialized) {
        return;
    }
    mdEngine::initialized = true;
    mdEngine::scratchMemory = MemoryPoolCreate(scratchMemorySize);
    mdEngine::sceneMemory = MemoryPoolCreate(sceneMemorySize);
    mdEngine::persistentMemory = MemoryPoolCreate(persistentMemorySize);
    mdEngine::engineMemory = MemoryPoolCreate(engineMemorySize);
    mdEngine::eventHandler = EventHandlerCreate();
    mdEngine::passthroughShader = MdEngineLoadPassthroughShader();
    {
        TextDrawingStyle tds;
        tds.color = WHITE;
        tds.font = {};
        tds.size = 24;
        tds.charSpacing = 1;
        mdEngine::textDrawingStyleDefault = tds;
    }
    // TODO: Generate this instead of loading it from disk
    mdEngine::missingTexture = LOAD_TEXTURE("missing_texture.png");
    MdEngineRegisterObjects();
    InputInit(&mdEngine::input);
}

// TODO: Consider removing this or GameObjectCreate and just have one function for this
GameObject MdEngineInstanceGameObject(i32 ind, MemoryPool* mp, const char* instanceName = "") {
    assert(mdEngine::gameObjectIsDefined[ind]);
    GameObjectDefinition def = mdEngine::gameObjectDefinitions[ind];
    GameObject go = GameObjectCreate(def.Create(mp), mp, def.objectName, instanceName);
    go.Draw3d = def.Draw3d;
    go.DrawUi = def.DrawUi;
    go.DrawImGui = def.DrawImGui;
    go.Free = def.Free;
    go.Update = def.Update;
    return go;
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
    MemoryPool mp = {};
    mp.buffer = calloc(size, sizeof(byte));
    mp.location = 0;
    mp.size = size;
    mp.alignment = sizeof(void*);
    return mp;
}
MemoryPool MemoryPoolCreateInsideMemoryPool(MemoryPool* srcMp, u64 size) {
    MemoryPool mp = {};
    mp.buffer = MemoryPoolReserve(srcMp, size);
    mp.location = 0;
    mp.size = size;
    return mp;
}
void MemoryPoolExpand(MemoryPool* mp) {
    // TODO: Implement
    /*
        Cases in which its allowed to expand memory
        1. request to expand comes from front of memory pool
        2. request to expand comes from foremost nested memory pool
        This would help all the memory usages to be checked at the beginning of the game
    */
    assert(false);
}
void MemoryPoolDestroy(MemoryPool* mp) {
    if (mp->buffer != nullptr) {
        free(mp->buffer);
    }
}
void* MemoryPoolReserve(MemoryPool* mp, u64 size) {
    assert(mp->location + size < mp->size); // Out of memory check
    assert(size > 0);
    void* reserve = (byte*)mp->buffer + mp->location;
    mp->location += size;
    if (mp->alignment != 0 && mp->location % mp->alignment != 0) {
        u64 alignedLocation = mp->location + (sizeof(void*) - (mp->location % sizeof(void*)));
        mp->location = uimini(alignedLocation, mp->size);
    }
    return reserve;
}
void MemoryPoolClear(MemoryPool* mp) {
    assert(mp->size > 0);
    memset(mp->buffer, 0, mp->location);
    mp->location = 0;
}
template <typename T>
T* MemoryReserve(MemoryPool* mp) {
    return (T*)MemoryPoolReserve(mp, sizeof(T));
}
template <typename T>
T* MemoryReserve(MemoryPool* mp, u64 size) {
    return (T*)MemoryPoolReserve(mp, sizeof(T) * size);
}

void InputInit(Input* input) {
    input->map[INPUT_ACCELERATE] = KEY_SPACE;
    input->map[INPUT_BREAK] = KEY_LEFT_SHIFT;
    input->map[INPUT_LEFT] = KEY_LEFT;
    input->map[INPUT_RIGHT] = KEY_RIGHT;
    input->map[INPUT_DOWN] = KEY_DOWN;
    input->map[INPUT_UP] = KEY_UP;
    input->map[INPUT_DEBUG_TOGGLE] = KEY_BACKSPACE;
    input->map[INPUT_DEBUG_ACCEPT] = KEY_ENTER;
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

Tween TweenCreate(void* variable, void* start, void* end, i32 dataType, float duration) {
    Tween t = {};
    switch (dataType) {
        case MD_TYPE_FLOAT:
        case MD_TYPE_BYTE:
            break;
        default:
            assert(false);
            TraceLog(LOG_WARNING, TextFormat("%s: Data type '%i' not supported", nameof(TweenCreate), dataType));
            t.data = nullptr;
            t.start = nullptr;
            t.end = nullptr;
            t.dataType = -1;
            t.duration = -1;
            t._time = 0;
            return t;
    }
    t.data = variable;
    t.start = start;
    t.end = end;
    t.dataType = dataType;
    t.duration = duration;
    t._time = 0;
    return t;
}
void TweenStart(Tween* t) {
    if (t->duration <= 0.f) {
        _TweenEnd(t);
        return;
    }
    t->_time = 0;
    t->_started = true;
}
void TweenStep(Tween* t) {
    if (!t->_started) {
        return;
    }
    t->_time = fminf(t->_time + FRAME_TIME, t->duration);
    float progress = t->_time / t->duration;
    if (progress == 1.f) {
        _TweenEnd(t);
        return;
    }
    switch (t->dataType) {
        case MD_TYPE_FLOAT:
        {
            float start = *(float*)t->start;
            float end = *(float*)t->end;
            *(float*)t->data = (end - start) * progress + start;
            break;
        }
        case MD_TYPE_BYTE:
        {
            byte start = *(byte*)t->start;
            byte end = *(byte*)t->end;
            byte value = (byte)((float)(end - start) * progress) + start;
            memcpy(t->data, &value, MdTypeGetSize(MD_TYPE_BYTE));
            break;
        }
    }
}
void _TweenEnd(Tween* t) {
    t->_time = 1.f;
    t->_started = false;
    memcpy(t->data, t->end, MdTypeGetSize(t->dataType));
}

VariableDynamicBuffer VariableDynamicBufferCreate(MemoryPool* mp) {
    VariableDynamicBuffer vdb = {};
    vdb.memoryPool = MemoryPoolCreateInsideMemoryPool(mp, VariableDynamicBuffer::BUFFER_SIZE);
    vdb.memoryPool.alignment = 0;
    vdb.locations = TypeListCreate(MD_TYPE_I32, mp, 5);
    vdb.types = TypeListCreate(MD_TYPE_I32, mp, 5);
    vdb.variableCount = 0;
    return vdb;
}
i32 VariableDynamicBufferPush(VariableDynamicBuffer* vdb, void* value, i32 type) {
    i32 typeSize = MdTypeGetSize(type);
    u64 location = vdb->memoryPool.location;
    void* valueDest = MemoryPoolReserve(&vdb->memoryPool, typeSize);
    TypeListPushBackI32(vdb->types, type);
    TypeListPushBackI32(vdb->locations, location);
    memcpy(valueDest, value, typeSize);
    i32 index = vdb->variableCount;
    vdb->variableCount++;
    return index;
}
void* VariableDynamicBufferGet(VariableDynamicBuffer* vdb, i32 index, i32 type) {
    i32 variableType = TypeListGetI32(vdb->types, index);
    if (variableType != type) {
        assert(false);
        TraceLog(LOG_ERROR, TextFormat("%s: Variable types of index didn't match", nameof(VariableDynamicBufferGet)));
        return nullptr;
    }
    i32 variableLocation = TypeListGetI32(vdb->locations, index);
    return (byte*)vdb->memoryPool.buffer + variableLocation;
}
void VariableDynamicBufferSet(VariableDynamicBuffer* vdb, i32 index, i32 type, void* value) {
    i32 variableType = TypeListGetI32(vdb->types, index);
    if (variableType != type) {
        assert(false);
        TraceLog(LOG_ERROR, TextFormat("%s: Variable types of index didn't match", nameof(VariableDynamicBufferGet)));
        return;
    }
    i32 variableLocation = TypeListGetI32(vdb->locations, index);
    void* variablePtr = (byte*)vdb->memoryPool.buffer + variableLocation;
    memcpy(variablePtr, value, MdTypeGetSize(type));
}

// TODO: Rename because the memory pool parameter causes confusion
GameObjectDefinition GameObjectDefinitionCreate(const char* objectName, GameInstanceCreateFunction createFunc, MemoryPool* mp) {
    GameObjectDefinition def = {};
    def.Create = createFunc;
    def.objectName = CstringDuplicate(objectName, mp);
    return def;
}
GameObject GameObjectCreate(void* data, MemoryPool* mp, const char* objectName, const char* instanceName) {
    GameObject go = {};
    go.variableBuffer = VariableDynamicBufferCreate(mp);
    go.variableIndices = std::unordered_map<std::string, i32>();
    go.data = data;
    go.active = true;
    go.visible = true;
    go.objectName = CstringDuplicate(objectName, mp);
    go.instanceName = CstringDuplicate(instanceName, mp);
    go.id = GameObject::idCounter;
    GameObject::idCounter++;
    return go;
}
// TODO: Change variable dynamic buffer initialization to happen here
void GameObjectAddScript(GameObject* obj, GameObjectScriptFunc initScript, GameObjectScriptFunc updateScript) {
    mdEngine::currentGameObjectInstance = obj;
    initScript(obj, obj->data);
    mdEngine::currentGameObjectInstance = NULL;
    // TODO: Freeze memory pool. We don't want to be able to create more variables during the scene
    // TODO: Consider adding a create function that runs once all game objects have been added
    obj->UpdateScript = updateScript;
}
void GameObjectsUpdate(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].active && gameObjects[i].Update != nullptr) {
            gameObjects[i].Update(gameObjects[i].data);
        }
        if (gameObjects[i].active && gameObjects[i].UpdateScript != nullptr) {
            mdEngine::currentGameObjectInstance = &gameObjects[i];
            gameObjects[i].UpdateScript(&gameObjects[i], gameObjects[i].data);
        }
    }
    mdEngine::currentGameObjectInstance = NULL;
}
void GameObjectsDraw3d(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].visible && gameObjects[i].Draw3d != nullptr) {
            gameObjects[i].Draw3d(gameObjects[i].data);
        }
    }
}
void GameObjectsDrawUi(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].visible && gameObjects[i].DrawUi != nullptr) {
            gameObjects[i].DrawUi(gameObjects[i].data);
        }
    }
}
void GameObjectsFree(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].Free != nullptr) {
            gameObjects[i].Free(gameObjects[i].data);
        }
    }
}
void GameObjectsDrawImGui(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].DrawImGui != nullptr) {
            if (ImGui::CollapsingHeader(TextFormat("%s - %s (%i)", gameObjects[i].objectName, gameObjects[i].instanceName, gameObjects[i].id))) {
                gameObjects[i].DrawImGui(gameObjects[i].data);
            }
        }
    }
}
void InstanceVariablePush(std::string name, i32 value) {
    _InstanceVariablePush(name, MD_TYPE_I32, &value);
}
void InstanceVariablePush(std::string name, byte value) {
    _InstanceVariablePush(name, MD_TYPE_BYTE, &value);
}
void InstanceVariablePush(std::string name, void* value) {
    _InstanceVariablePush(name, MD_TYPE_PTR, &value);
}
void InstanceVariablePush(std::string name, float value) {
    _InstanceVariablePush(name, MD_TYPE_FLOAT, &value);
}
void InstanceVariablePush(std::string name, Tween value) {
    _InstanceVariablePush(name, MD_TYPE_TWEEN, &value);
}
template <> i32 InstanceVariableGet(std::string name) {
    return *(i32*)_InstanceVariableGet(name, MD_TYPE_I32);
}
template <> byte InstanceVariableGet(std::string name) {
    return *(byte*)_InstanceVariableGet(name, MD_TYPE_BYTE);
}
template <> void* InstanceVariableGet(std::string name) {
    return _InstanceVariableGet(name, MD_TYPE_PTR);
}
template <> float InstanceVariableGet(std::string name) {
    return *(float*)_InstanceVariableGet(name, MD_TYPE_FLOAT);
}
template <> Tween InstanceVariableGet(std::string name) {
    return *(Tween*)_InstanceVariableGet(name, MD_TYPE_TWEEN);
}
// TODO: Create some sort of global state where calling this function sets the currently scoped game instance.
// This is to get rid of a lot of the vars passed to this function. 'obj' and 'type' can be removed enirely.
// 'value' can be replaced with an actual typed variable. Same thing should go for GameObjectVariableGet
void InstanceVariableSet(std::string name, i32 value) {
    _InstanceVariableSet(name, MD_TYPE_I32, &value);
}
void InstanceVariableSet(std::string name, byte value) {
    _InstanceVariableSet(name, MD_TYPE_BYTE, &value);
}
void InstanceVariableSet(std::string name, void* value) {
    _InstanceVariableSet(name, MD_TYPE_PTR, &value);
}
void InstanceVariableSet(std::string name, float value) {
    _InstanceVariableSet(name, MD_TYPE_FLOAT, &value);
}
void InstanceVariableSet(std::string name, Tween value) {
    _InstanceVariableSet(name, MD_TYPE_TWEEN, &value);
}
void _InstanceVariablePush(std::string name, i32 type, void* value) {
    GameObject* obj = mdEngine::currentGameObjectInstance;
    if (obj->variableIndices.find(name) != obj->variableIndices.end()) {
        const char* log = TextFormat(
            "%s: Variable with name '%s' already exists in object '%s'",
            nameof(GameObjectVariablePush),
            name.c_str(),
            obj->objectName);
        TraceLog(LOG_WARNING, log);
        return;
    }
    obj->variableIndices[name] = VariableDynamicBufferPush(&obj->variableBuffer, value, type);
}
void* _InstanceVariableGet(std::string name, i32 type) {
    GameObject* obj = mdEngine::currentGameObjectInstance;
    if (obj->variableIndices.find(name) == obj->variableIndices.end()) {
        const char* log = TextFormat(
            "%s: Variable with name '%s' doesn't exists in object '%s'",
            nameof(GameObjectVariableGet),
            name.c_str(),
            obj->objectName);
        TraceLog(LOG_WARNING, log);
        return nullptr;
    }
    return VariableDynamicBufferGet(
        &obj->variableBuffer,
        obj->variableIndices[name],
        type);
}
void _InstanceVariableSet(std::string name, i32 type, void* value) {
    GameObject* obj = mdEngine::currentGameObjectInstance;
    if (obj->variableIndices.find(name) == obj->variableIndices.end()) {
        const char* log = TextFormat(
            "%s: Variable with name '%s' doesn't exists in object '%s'",
            nameof(GameObjectVariableGet),
            name.c_str(),
            obj->objectName);
        TraceLog(LOG_WARNING, log);
        return;
    }
    VariableDynamicBufferSet(&obj->variableBuffer, obj->variableIndices[name], type, value);
}

void StringListInit(StringList* list, i32 size, MemoryPool* mp) {
    if (size == 0) {
        list->data = nullptr;
    } else {
        list->data = MemoryReserve<String>(mp, size);
    }
    list->capacity = size;
    list->size = 0;
    list->_memoryPool = mp;
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
        eh.callbacks[i] = TypeListCreate(MD_TYPE_PTR, &mdEngine::persistentMemory);
        eh.registrars[i] = TypeListCreate(MD_TYPE_PTR, &mdEngine::persistentMemory);
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
                break;
            }
            case EVENT_DIALOGUE_OPTIONS_SELECTED: {
                EventArgs_DialogueOptionsSelected* args = (EventArgs_DialogueOptionsSelected*)_args;
                EventCallbackSignature_DialogueOptionsSelected callback = (EventCallbackSignature_DialogueOptionsSelected)TypeListGetPtr(callbacks, i);
                callback(TypeListGetPtr(registrars, i), args);
                break;
            }
        }
    }
}

TypeList* TypeListCreate(i32 typeIndex, MemoryPool* mp, i32 capacity) {
    TypeList* list = MemoryReserve<TypeList>(mp);
    list->memoryProvider = mp;
    list->typeIndex = typeIndex;
    list->bufferStride = MdTypeGetSize(typeIndex);
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
void TypeListGrowCapacityIfLimitReached(TypeList* list) {
    if (list->capacity == list->size) {
        if (list->capacity == 0) {
            TypeListSetCapacity(list, 5);
        } else {
            TypeListSetCapacity(list, list->capacity * 5);
        }
    }
}
void TypeListResize(TypeList* list, i32 size) {
    if (size > list->capacity) {
        TypeListSetCapacity(list, size);
    } else if (size < list->size) {
        memset((void*)((byte*)list->buffer + size), 0, list->size - size);
    }
    list->size = size;
}
void TypeListPushBackI32(TypeList* list, i32 val) {
    assert(list->typeIndex == MD_TYPE_I32);
    TypeListGrowCapacityIfLimitReached(list);
    ((i32*)list->buffer)[list->size] = val;
    list->size++;
}
i32 TypeListGetI32(TypeList* list, i32 ind) {
    assert(list->typeIndex == MD_TYPE_I32);
    assert(ind < list->size);
    return ((i32*)list->buffer)[ind];
}
void TypeListSetI32(TypeList* list, i32 ind, i32 val) {
    assert(list->typeIndex == MD_TYPE_I32);
    assert(ind < list->size);
    ((i32*)list->buffer)[ind] = val;
}
i32 TypeListFindI32(TypeList* list, i32 val) {
    assert(list->typeIndex == MD_TYPE_I32);
    i32* buffer = (i32*)list->buffer;
    for (i32 i = 0; i < list->size; i++) {
        if (buffer[i] == val) {
            return i;
        }
    }
    return -1;
}
void TypeListPushBackPtr(TypeList* list, void* val) {
    assert(list->typeIndex == MD_TYPE_PTR);
    _TypeListPushBackPtr(list, val);
}
void* TypeListGetPtr(TypeList* list, i32 ind) {
    assert(list->typeIndex == MD_TYPE_PTR);
    assert(ind < list->size);
    return _TypeListGetPtr(list, ind);
}
void TypeListSetPtr(TypeList* list, i32 ind, void* val) {
    assert(list->typeIndex == MD_TYPE_PTR);
    assert(ind < list->size);
    _TypeListSetPtr(list, ind, val);
}
i32 TypeListFindPtr(TypeList* list, void* val) {
    assert(list->typeIndex == MD_TYPE_PTR);
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
    assert(list->typeIndex == MD_TYPE_GAME_OBJECT_DEFINITION);
    TypeListGrowCapacityIfLimitReached(list);
    ((GameObjectDefinition*)list->buffer)[list->size] = val;
    list->size++;
}
GameObjectDefinition* TypeListGetGameObjectDefinition(TypeList* list, i32 ind) {
    assert(list->typeIndex == MD_TYPE_GAME_OBJECT_DEFINITION);
    assert(ind < list->size);
    return &((GameObjectDefinition*)list->buffer)[ind];
}
void TypeListSetGameObjectDefinition(TypeList* list, i32 ind, GameObjectDefinition val) {
    assert(list->typeIndex == MD_TYPE_GAME_OBJECT_DEFINITION);
    assert(ind < list->size);
    ((GameObjectDefinition*)list->buffer)[ind] = val;
}
i32 TypeListFindGameObjectDefinitionShallow(TypeList* list, GameObjectDefinition* val) {
    assert(list->typeIndex == MD_TYPE_GAME_OBJECT_DEFINITION);
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
    tw->textDrawingStyle = mdEngine::textDrawingStyleDefault;
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
    String substr = StringSubstr(tw->text[tw->textIndex], 0, (i32)tw->progress);
    v2 textAlign = MeasureTextEx(
        tw->textDrawingStyle.font,
        substr.cstr,
        tw->textDrawingStyle.size,
        tw->textDrawingStyle.charSpacing) / 2.f;
    DrawTextPro(tw->textDrawingStyle.font, substr.cstr, {truncf((float)tw->x), truncf((float)tw->y)}, textAlign, 0.f, tw->textDrawingStyle.size, 1.f, WHITE);
    StringDestroy(substr);
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
    dopt->textStyle = mdEngine::textDrawingStyleDefault;
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

void* DialogueSequenceCreate(MemoryPool* mp) {
    DialogueSequence* dseq = MemoryReserve<DialogueSequence>(mp);
    dseq->sectionIndex = 0;
    dseq->sections = TypeListCreate(MD_TYPE_PTR, mp, 50);
    TypewriterInit(&dseq->typewriter);
    dseq->typewriter.autoAdvance = true;
    dseq->typewriter.autoHide = false;
    DialogueOptionsInit(&dseq->options);
    DialogueSequenceSetLayout(dseq, DIALOGUE_SEQUENCE_LAYOUT_PLACEHOLDER_BOXLESS);
    EventHandlerRegisterEvent(EVENT_TYPEWRITER_LINE_COMPLETE, dseq, (EventCallbackSignature)DialogueSequenceHandleTypewriter_TextAdvance);
    EventHandlerRegisterEvent(EVENT_DIALOGUE_OPTIONS_SELECTED, dseq, (EventCallbackSignature)DialogueSequenceHandleOptions_Selected);
    return dseq;
}
void DialogueSequenceSectionStart(DialogueSequence* dseq, i32 ind) {
    DialogueSequenceSection* dss = DialogueSequenceSectionGet(dseq, ind);
    TypewriterStart(&dseq->typewriter, dss->text->data, dss->text->size);
    DialogueOptionsSet(&dseq->options, dss->options->data, dss->options->size);
    if (dss->options->size == 0) {
        dseq->typewriter.autoHide = true;
    }
}
void DialogueSequenceUpdate(DialogueSequence* dseq) {
    if (dseq->active) {
        DialogueOptionsUpdate(&dseq->options);
        TypewriterUpdate(&dseq->typewriter);
    }
}
void DialogueSequenceDrawUi(DialogueSequence* dseq) {
    if (dseq->active) {
        DialogueOptionsDraw(&dseq->options);
        TypewriterDraw(&dseq->typewriter);
    }
}
// TODO: Move this out of the engine
void DialogueSequenceSetLayout(DialogueSequence* dseq, i32 layout) {
    switch (layout) {
        case DIALOGUE_SEQUENCE_LAYOUT_PLACEHOLDER_BOXLESS:
        dseq->typewriter.x = GetScreenWidth() / 2;
        dseq->typewriter.y = GetScreenHeight() / 2 - GetScreenHeight() / 3;
        dseq->options.optionSeparationAdd = 0.f;
        dseq->options.optionBoxHeightAdd = 0.f;
        dseq->options.x = GetScreenWidth() / 2;
        dseq->options.y = GetScreenHeight() / 2 + GetScreenHeight() / 4;
        break;

        case DIALOGUE_SEQUENCE_LAYOUT_PLACEHOLDER:
        dseq->typewriter.x = GetScreenWidth() / 2;
        dseq->typewriter.y = GetScreenHeight() / 2 - GetScreenHeight() / 3;
        dseq->options.optionBoxHeightAdd = 32.f;
        dseq->options.optionSeparationAdd = 48.f;
        dseq->options.x = GetScreenWidth() / 2;
        dseq->options.y = GetScreenHeight() / 2 + GetScreenHeight() / 4;
        break;

        default:
        TraceLog(LOG_WARNING, TextFormat("%s, No layout with index '%i", nameof(DialogueSequenceSetLayout), layout));
        break;
    }
}
DialogueSequenceSection* DialogueSequenceSectionGet(DialogueSequence* dseq, i32 ind) {
    return (DialogueSequenceSection*)TypeListGetPtr(dseq->sections, ind);
}
DialogueSequenceSection* DialogueSequenceSectionCreate(i32 textCount, i32 optionCount, MemoryPool* mp) {
    DialogueSequenceSection* dss = MemoryReserve<DialogueSequenceSection>(mp);
    dss->text = MemoryReserve<StringList>(mp);
    StringListInit(dss->text, textCount, mp);
    dss->options = MemoryReserve<StringList>(mp);
    StringListInit(dss->options, optionCount, mp);
    dss->link = TypeListCreate(MD_TYPE_I32, mp);
    return dss;
}

void DialogueSequenceHandleTypewriter_TextAdvance(DialogueSequence* dseq, EventArgs_TypewriterLineComplete* _args) {
    EventArgs_TypewriterLineComplete* args = (EventArgs_TypewriterLineComplete*)_args;
    DialogueSequenceSection* dss = DialogueSequenceSectionGet(dseq, dseq->sectionIndex);
    if (args->lineCurrent == args->lineCount - 1 && dss->options->size > 0) {
        dseq->options.visible = true;
    }
}
void DialogueSequenceHandleOptions_Selected(DialogueSequence* dseq, EventArgs_DialogueOptionsSelected* args) {
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
    const Image *image = info.image;
    const i32 stride = PixelformatGetStride(image->format);
    const i32 heightDataWidth = image->width >> resdiv;
    const i32 heightDataHeight = image->height >> resdiv;

    float *heightData = (float*)malloc(heightDataWidth * heightDataHeight * sizeof(float));
    byte* data = (byte*)info.image->data;
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

void* InstanceRendererCreate(MemoryPool* mp) {
    InstanceRenderer* ir = MemoryReserve<InstanceRenderer>(mp);
    ir->transforms = nullptr;
    return ir;
}
void InstanceRendererDraw3d(InstanceRenderer* is) {
    DrawMeshInstancedOptimized(is->mesh, *is->material, is->transforms, is->instanceCount);
}

void* ModelInstanceCreate(MemoryPool* mp) {
    ModelInstance* mi = MemoryReserve<ModelInstance>(mp);
    mi->tint = WHITE;
    mi->transform = TransformCreate();
    return mi;
}
void ModelInstanceDraw3d(ModelInstance* mi) {
    DrawModelTransform(mi->model, mi->transform.matrix, mi->tint);
}
void ModelInstanceDrawImGui(ModelInstance* mi) {
    TransformDrawImGui(&mi->transform);
}

void* MeshInstanceCreate(MemoryPool* mp) {
    MeshInstance* mi = MemoryReserve<MeshInstance>(mp);
    mi->mesh = {};
    mi->material = LoadMaterialDefault();
    mi->tint = WHITE;
    mi->transform = TransformCreate();
    return mi;
}
void MeshInstanceDraw3d(MeshInstance* mi) {
    DrawMesh(mi->mesh, mi->material, mi->transform.matrix);
}
void MeshInstanceDrawImGui(MeshInstance* mi) {
    TransformDrawImGui(&mi->transform);
}

void* ParticleSystemCreate(MemoryPool* mp) {
    ParticleSystem* psys = MemoryReserve<ParticleSystem>(mp);
    psys->_quad = GenMeshPlane(1.f, 1.f, 1, 1);
    psys->_transforms = (mat4*)RL_CALLOC(PARTICLE_SYSTEM_MAX_PARTICLES, sizeof(mat4));
    psys->count = 64;
    for (i32 i = 0; i < psys->count; i++) {
        psys->_transforms[i] = MatrixTranslate(
            GetRandomValueF(-10.f, 10.f),
            GetRandomValueF(-10.f, 10.f),
            GetRandomValueF(-10.f, 10.f));
    }
    return psys;
}
void ParticleSystemFree(ParticleSystem* psys) {
    UnloadMesh(psys->_quad);
    RL_FREE(psys->_transforms);
}
void ParticleSystemUpdate(ParticleSystem* psys) {
    v3 stepVelocity = psys->velocity * FRAME_TIME;
    mat4 transpose = MatrixTranslate(stepVelocity.x, stepVelocity.y, stepVelocity.z);
    for (i32 i = 0; i < psys->count; i++) {
        psys->_transforms[i] *= transpose;
    }
}
void ParticleSystemDraw3d(ParticleSystem* psys) {
    DrawMeshInstanced(psys->_quad, *psys->_material, psys->_transforms, psys->count);
}

void* TextureInstanceCreate(MemoryPool* mp) {
    TextureInstance* ti = MemoryReserve<TextureInstance>(mp);
    Texture* tex = &mdEngine::missingTexture;
    ti->shader = &mdEngine::passthroughShader;
    ti->_texture = tex;
    ti->_position = {0.f, 0.f};
    ti->origin = {0.f, 0.f};
    ti->_rotation = 0.f;
    ti->tint = WHITE;
    ti->_drawSource = {0.f, 0.f, (float)tex->width, (float)tex->height};
    ti->_drawDestination = {0.f, 0.f, (float)tex->width, (float)tex->height};
    ti->_scale = {1.f, 1.f};
    return ti;
}
void TextureInstanceSetTexture(TextureInstance* ti, Texture* texture) {
    ti->_texture = texture;
    ti->_drawSource = {0.f, 0.f, (float)texture->width, (float)texture->height};
}
void TextureInstanceSetSize(TextureInstance* ti, v2 size) {
    ti->_drawDestination.width = size.x;
    ti->_drawDestination.height = size.y;
}
void TextureInstanceSetPosition(TextureInstance* ti, v2 position) {
    ti->_drawDestination.x = position.x;
    ti->_drawDestination.y = position.y;
    ti->_position = position;
}
void TextureInstanceDrawUi(TextureInstance* ti) {
    BeginShaderMode(*ti->shader);
    DrawTexturePro(*ti->_texture, ti->_drawSource, ti->_drawDestination, ti->origin, ti->_rotation, {255, 255, 255, 128});//ti->tint);
    EndShaderMode();
}

void* SkyboxCreate(MemoryPool* mp) {
    Skybox* sb = MemoryReserve<Skybox>(mp);
    return sb;
}
void SkyboxInit(Skybox* sb, Shader* shader, Image* image) {
    sb->model = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    sb->shader = shader;
    {
        i32 envmapValue = MATERIAL_MAP_CUBEMAP;
        SetShaderValue(*sb->shader, GetShaderLocation(*sb->shader, "environmentMap"), &envmapValue, SHADER_UNIFORM_INT);
        i32 gammaValue = 0;
        SetShaderValue(*sb->shader, GetShaderLocation(*sb->shader, "doGamma"), &gammaValue, SHADER_UNIFORM_INT);
        i32 vflippedValue = 0;
        SetShaderValue(*sb->shader, GetShaderLocation(*sb->shader, "vflipped"), &vflippedValue, SHADER_UNIFORM_INT);
    }
    sb->model.materials[0].shader = *sb->shader;
    sb->model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(
        *image,
        CUBEMAP_LAYOUT_AUTO_DETECT);
}
void SkyboxDraw3d(Skybox* sb) {
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
        DrawModel(sb->model, {0.f}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
void SkyboxFree(Skybox* sb) {
    UnloadTexture(sb->_texture);
}
#endif // __MD_ENGINE_H