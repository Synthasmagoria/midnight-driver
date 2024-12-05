#include "raylib.h"
#include "raymath.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "rlgl.h"

typedef uint8_t byte;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef Matrix mat4;
typedef Vector3 v3;
typedef Vector2 v2;

#define FRAME_TIME 1.f / 60.f
#define FRAMERATE 60
#define TAU PI * 2.f

struct Input;
struct ForestGenerationInfo;
struct InstanceMeshRenderData;
struct QuadraticBezier;
struct Particle;
struct ParticleSystem;
struct BillboardParticle;
struct BillboardParticleSystem;
struct List;
struct HeightmapGenerationInfo;
struct Heightmap;
struct Cab;

enum INPUT {
    INPUT_ACCELERATE,
    INPUT_BREAK,
    INPUT_TOGGLE_DEBUG,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DEBUG_LEFT,
    INPUT_DEBUG_RIGHT,
    INPUT_DEBUG_FORWARD,
    INPUT_DEBUG_BACKWARD,
    INPUT_DEBUG_UP,
    INPUT_DEBUG_DOWN,
    INPUT_COUNT
};
struct Input {
    i32 map[INPUT_COUNT];
    i32 pressed[INPUT_COUNT];
    i32 down[INPUT_COUNT];
    i32 up[INPUT_COUNT];
};
void InputInit(Input *input);
void InputUpdate(Input *input);

struct ForestGenerationInfo {
    v2 worldPosition;
    v2 worldSize;
    float density;
    float treeChance;
    float randomPositionOffset;
    float randomYDip;
    float randomTiltDegrees;
    Heightmap* heightmap;
};

struct InstanceMeshRenderData {
    mat4 *transforms;
    u32 instanceCount;
    Model _model;
    Mesh mesh;
    Material material;
};
InstanceMeshRenderData ForestCreate(Image image, ForestGenerationInfo info, Mesh mesh, Material material);

struct QuadraticBezier {v2 p1; v2 p2; v2 p3;};
// https://www.desmos.com/calculator/scz7zhonfw
float QuadraticBezierLerp(QuadraticBezier qb, float val) {
    v2 a = Vector2Lerp(qb.p1, qb.p2, val);
    v2 b = Vector2Lerp(qb.p2, qb.p3, val);
    return Vector2Lerp(a, b, val).y;
}

#define PARTICLE_SYSTEM_MAX_PARTICLES 512
struct ParticleSystem {
    mat4 *_transforms;
    Mesh _quad;
    Material _material;
    v3 velocity;
    i32 count;
};
ParticleSystem ParticleSystemCreate(Texture texture, Material material);
void ParticleSystemDestroy(ParticleSystem *psys);
void ParticleSystemStep(ParticleSystem *psys);
void ParticleSystemDraw(ParticleSystem *psys);

struct BillboardParticle {
    v3 position;
    Color color;
    float life;
};

#define MAX_BILLBOARD_PARTICLES 192
struct BillboardParticleSystem {
    Texture2D texture;
    float rate;
    v3 speed;
    float life;
    QuadraticBezier alphaCurve;
    BillboardParticle particles[MAX_BILLBOARD_PARTICLES];
    int blendMode;
    float _spawnTimer;
    u32 _number;
    u32 _index;
};
void BillboardParticleSystemStep(BillboardParticleSystem *psys, Camera3D camera);

struct List {
    void** data;
    u32 size;
    u32 capacity;
};
List ListCreate(u32 size);
void ListInit(List *list, u32 size);
void ListDestroy(List *list);
void ListResize(List *list, u32 size);
void ListChangeCapacity(List *list, u32 capacity);
void* ListGet(List *list, u32 ind);
void ListSet(List *list, u32 ind, void* val);
void ListPushBack(List *list, void* val);

struct HeightmapGenerationInfo {
    Image *heightmapImage;
    Texture *terrainMapTexture;
    Texture *terrainTexture;
    v3 position;
    v3 size;
    u32 resdiv;
};

struct Heightmap {
    Model model;
    Image heightmap;
    Texture2D texture;
    v3 position;
    v3 size;
    u32 heightDataResolution;
    u32 heightDataWidth;
    u32 heightDataHeight;
    float *heightData;
    Model debugModel;
    u32 width;
};
Heightmap HeightmapCreate(HeightmapGenerationInfo info, Material material);
float HeightmapSampleHeight(Heightmap heightmap, float x, float z);
void HeightmapDestroy(Heightmap heightmap);

struct Cab {
    Model model;
    v3 position;
    v3 frontSeat;
    v3 direction;
    v3 velocity;
    float maxVelocity;
    float acceleration;
    float neutralDeceleration;
    float breakDeceleration;
    float turnAngleMax;
    float turnAngleSpeed;
    float yawRotationStep;
    QuadraticBezier accelerationCurve;
    QuadraticBezier reverseAccelerationCurve;

    float _speed;
    mat4 _transform;
    float _turnAngle;
};
void CabInit(Cab *cab);
void CabUpdate(Cab *cab);
void CabDraw(Cab *cab);
v3 CabGetFrontSeatPosition(Cab *cab);

void CameraUpdateDebug(Camera *camera, float speed);
void CameraUpdate(Camera *camera, v3 position, v2 rotationAdd, v3 viewMiddle);
v3 GetCameraForwardNorm(Camera *camera);
v3 GetCameraUpNorm(Camera *camera);
v3 GetCameraRightNorm(Camera *camera);

inline int PointInRectangle(v2 begin, v2 end, v2 pt) {
    return pt.x >= begin.x && pt.x < end.x && pt.y >= begin.y && pt.y < end.y;
}
inline const char* TextFormatVector3(v3 v) {
    return TextFormat("[%f], [%f], [%f]", v.x, v.y, v.z);
}
inline void DrawTextShadow(const char* text, int x, int y, int fontSize, Color col, Color shadowCol) {
    DrawText(text, x + 1, y + 1, fontSize, shadowCol);
    DrawText(text, x, y, fontSize, col);
}
inline void DrawCrosshair(int x, int y, Color col) {
    DrawLine(x - 4, y, x + 4, y, col);
    DrawLine(x, y - 4, x, y + 4, col);
}
inline float ffractf(float val) {
    return val - floorf(val);
}
inline v2 Vector2Fract(v2 v) {
    return {ffractf(v.x), ffractf(v.y)};
}
inline float fclampf(float val, float min, float max) {
    return fminf(fmaxf(val, min), max);
}
inline float fsignf(float val) {
    return (float)(!signbit(val)) * 2.f - 1.f;
}
inline v2 v2clampv2(v2 val, v2 min, v2 max) {
    return {fclampf(val.x, min.x, max.x), fclampf(val.y, min.y, max.y)};
}
// NOTE: only supports up to 3 decimals
inline float GetRandomValueF(float min, float max) {
    return ((float)GetRandomValue((i32)(min * 1000.f), (i32)(max * 1000.f))) / 1000.f;
}
inline bool GetRandomChanceF(float percentage) {
    return GetRandomValueF(0.f, 100.f) < percentage;
}

u32 Fnv32Buf(void *buf, u64 len, u32 hval) {
    byte *bp = (byte*)buf;
    byte *be = bp + len;
    while (bp < be) {
        hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
        hval ^= (u32)*bp;
        bp++;
    }
    return hval;
}
BoundingBox GenBoundingBoxMesh(Mesh mesh) {
    v2 xbounds = {0.f}, ybounds = {0.f}, zbounds = {0.f};
    for (int i = 0; i < mesh.vertexCount; i+=3) {
        float* cv = mesh.vertices + i;
        xbounds.x = fminf(*cv, xbounds.x);
        xbounds.y = fmaxf(*cv, xbounds.y);
        ybounds.x = fminf(*(cv+1), ybounds.x);
        ybounds.y = fmaxf(*(cv+1), ybounds.y);
        zbounds.x = fminf(*(cv+2), zbounds.x);
        zbounds.y = fmaxf(*(cv+2), zbounds.y);
    }
    return {{xbounds.x, ybounds.x, zbounds.x}, {xbounds.y, ybounds.y, zbounds.y}};
}
BoundingBox GenBoundingBoxMeshes(Mesh* meshes, int meshCount) {
    v2 xbounds = {0.f}, ybounds = {0.f}, zbounds = {0.f};
    for (int i = 0; i < meshCount; i++) {
        Mesh *mesh = meshes + i;
        for (int j = 0; j < mesh->vertexCount; j+=3) {
            float* cv = mesh->vertices + j;
            xbounds.x = fminf(*cv, xbounds.x);
            xbounds.y = fmaxf(*cv, xbounds.y);
            ybounds.x = fminf(*(cv+1), ybounds.x);
            ybounds.y = fmaxf(*(cv+1), ybounds.y);
            zbounds.x = fminf(*(cv+2), zbounds.x);
            zbounds.y = fmaxf(*(cv+2), zbounds.y);
        }
    }
    return {{xbounds.x, ybounds.x, zbounds.x}, {xbounds.y, ybounds.y, zbounds.y}};
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

#define LOAD_MODEL(path)(LoadModel(TextFormat("resources/models/%s", path)))
#define LOAD_SHADER(v,f)(LoadShader(TextFormat("resources/shaders/%s", v), TextFormat("resources/shaders/%s", f)))
#define LOAD_TEXTURE(path)(LoadTexture(TextFormat("resources/textures/%s", path)))
#define LOAD_IMAGE(path)(LoadImage(TextFormat("resources/textures/%s", path)))
#define MatrixRotateRoll(rad) MatrixRotateX(rad)
#define MatrixRotateYaw(rad) MatrixRotateY(rad)
#define MatrixRotatePitch(rad) MatrixRotateZ(rad)

namespace global {
    enum GAME_SHADERS {
        SHADER_UNLIT_INSTANCED,
        SHADER_LIT,
        SHADER_LIT_INSTANCED,
        SHADER_LIT_TERRAIN,
        SHADER_SKYBOX,
        SHADER_COUNT
    };
    const char *shaderPaths[SHADER_COUNT * 2] = {
        "unlitInstanced.vs", "passthrough.fs",
        "light.vs", "light.fs",
        "lightInstanced.vs", "light.fs",
        "light.vs", "lightTerrain.fs",
        "skybox.vs", "skybox.fs"
    };
    Shader shaders[SHADER_COUNT];

    enum GAME_MATERIALS {
        MATERIAL_UNLIT,
        MATERIAL_LIT,
        MATERIAL_LIT_INSTANCED,
        MATERIAL_LIT_TERRAIN,
        MATERIAL_COUNT
    };

    enum GAME_MODELS {
        MODEL_CAB,
        MODEL_TREE,
        MODEL_COUNT
    };
    const char *modelPaths[MODEL_COUNT] = {
        "cab.glb",
        "tree.glb"
    };
    Model models[MODEL_COUNT];

    enum GAME_TEXTURES {
        TEXTURE_STAR,
        TEXTURE_TERRAIN,
        TEXTURE_TERRAINMAP_BLURRED,
        TEXTURE_COUNT
    };
    const char *texturePaths[global::TEXTURE_COUNT] = {
        "star.png",
        "terrain.png",
        "terrainmap_blurred.png"
    };
    Texture2D textures[global::TEXTURE_COUNT];

    enum GAME_IMAGES {
        IMAGE_SKYBOX,
        IMAGE_HEIGHTMAP,
        IMAGE_TERRAINMAP,
        IMAGE_COUNT
    };
    const char *imagePaths[IMAGE_COUNT] = {
        "skybox.png",
        "heightmap.png",
        "terrainmap.png"
    };
    Image images[IMAGE_COUNT];

    Input input;
}

void LoadGameShaders() {
    for (i32 i = 0; i < global::SHADER_COUNT; i++) {
        global::shaders[i] = LOAD_SHADER(global::shaderPaths[i*2], global::shaderPaths[i*2+1]);
    }
}
void UnloadGameShaders() {
    for (i32 i = 0; i < global::SHADER_COUNT; i++) {
        UnloadShader(global::shaders[i]);
    }
}

void InitSharedMaterials(Material* materials) {
    Shader sh;
    Material mat;
    
    sh = LOAD_SHADER("lightInstanced.vs", "light.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    materials[global::MATERIAL_LIT_INSTANCED] = mat;

    sh = LOAD_SHADER("light.vs", "light.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    materials[global::MATERIAL_LIT] = mat;

    sh = LOAD_SHADER("unlitInstanced.vs", "passthrough.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    materials[global::MATERIAL_UNLIT] = mat;

    sh = LOAD_SHADER("light.vs", "lightTerrain.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    sh.locs[SHADER_LOC_COLOR_SPECULAR] = GetShaderLocation(sh, "texture1");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    materials[global::MATERIAL_LIT_TERRAIN] = mat;
}
void UpdateGlobalMaterials(Material *materials, v3 lightPosition) {
    float pos[3] = {lightPosition.x, lightPosition.y, lightPosition.z};
    SetShaderValue(
        materials[global::MATERIAL_LIT_INSTANCED].shader,
        GetShaderLocation(materials[global::MATERIAL_LIT_INSTANCED].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
    SetShaderValue(
        materials[global::MATERIAL_LIT].shader,
        GetShaderLocation(materials[global::MATERIAL_LIT].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
    SetShaderValue(
        materials[global::MATERIAL_LIT_TERRAIN].shader,
        GetShaderLocation(materials[global::MATERIAL_LIT_TERRAIN].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
}

void LoadGameModels() {
    for (i32 i = 0; i < global::MODEL_COUNT; i++) {
        global::models[i] = LOAD_MODEL(global::modelPaths[i]);
    }
}
void UnloadGameModels() {
    for (i32 i = 0; i < global::MODEL_COUNT; i++) {
        UnloadModel(global::models[i]);
    }
}

void LoadGameTextures() {
    for (i32 i = 0; i < global::TEXTURE_COUNT; i++) {
        global::textures[i] = LOAD_TEXTURE(global::texturePaths[i]);
    }
}
void UnloadGameTextures() {
    for (i32 i = 0; i < global::TEXTURE_COUNT; i++) {
        UnloadTexture(global::textures[i]);
    }
}

void LoadGameImages() {
    for (i32 i = 0; i < global::IMAGE_COUNT; i++) {
        global::images[i] = LOAD_IMAGE(global::imagePaths[i]);
    }
}
void UnloadGameImages() {
    for (i32 i = 0; i < global::IMAGE_COUNT; i++) {
        UnloadImage(global::images[i]);
    }
}

void LoadGameResources() {
    LoadGameShaders();
    LoadGameModels();
    LoadGameTextures();
    LoadGameImages();
}
void UnloadGameResources() {
    UnloadGameShaders();
    UnloadGameModels();
    UnloadGameTextures();
    UnloadGameImages();
}

int main() {
    u32 freecam = 0; 

    v2 screenSize = {1440, 800};
    SetTraceLogLevel(4);
    InitWindow(screenSize.x, screenSize.y, "Midnight Driver");
    SetTargetFPS(60);
    DisableCursor();

    Material sharedMaterials[global::MATERIAL_COUNT];
    LoadGameResources();
    InputInit(&global::input);
    InitSharedMaterials(sharedMaterials);

    Camera camera = {};
    camera.fovy = 60.f;
    camera.position = {0.f, 0.f, 0.f};
    camera.target = {1.f, 0.f, 0.f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = {0.f, 1.f, 0.f};

    Cab cab = {};
    CabInit(&cab);
    
    Camera debugCamera = {};
    camera.fovy = 60.f;
    camera.up = {0.f, 1.f, 0.f};
    float debugCameraSpeed = 0.1f;

    Model skyboxModel = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    Shader skyboxShader = global::shaders[global::SHADER_SKYBOX];
    {
        i32 envmapValue = MATERIAL_MAP_CUBEMAP;
        SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "environmentMap"), &envmapValue, SHADER_UNIFORM_INT);
        i32 gammaValue = 0;
        SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "doGamma"), &gammaValue, SHADER_UNIFORM_INT);
        i32 vflippedValue = 0;
        SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "vflipped"), &vflippedValue, SHADER_UNIFORM_INT);
    }
    skyboxModel.materials[0].shader = skyboxShader;
    skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(
        global::images[global::IMAGE_SKYBOX],
        CUBEMAP_LAYOUT_AUTO_DETECT);

    v3 terrainSize = {50.f, 8.f, 50.f};
    v3 terrainPosition = terrainSize / -2.f;
    terrainPosition.y = 0.f;

    HeightmapGenerationInfo hgi = {};
    hgi.heightmapImage = &global::images[global::IMAGE_HEIGHTMAP];
    hgi.terrainMapTexture = &global::textures[global::TEXTURE_TERRAINMAP_BLURRED];
    hgi.terrainTexture = &global::textures[global::TEXTURE_TERRAIN];
    hgi.position = terrainPosition;
    hgi.size = terrainSize;
    hgi.resdiv = 2;
    Heightmap terrainHeightmap = HeightmapCreate(hgi, sharedMaterials[global::MATERIAL_LIT_TERRAIN]);

    ParticleSystem psys = ParticleSystemCreate(global::textures[global::TEXTURE_STAR], sharedMaterials[global::MATERIAL_UNLIT]);
    psys.velocity = {1.f, 0.f, 0.f};

    Model treeModel = global::models[global::MODEL_TREE];
    ForestGenerationInfo forestGenInfo = {};
    forestGenInfo.density = 0.6f;
    forestGenInfo.worldSize = {terrainSize.x, terrainSize.z};
    forestGenInfo.worldPosition = v2{terrainPosition.x, terrainPosition.z};
    forestGenInfo.treeChance = 90.f;
    forestGenInfo.randomPositionOffset = 0.25f;
    forestGenInfo.randomYDip = 0.f;
    forestGenInfo.randomTiltDegrees = 10.f;
    forestGenInfo.heightmap = &terrainHeightmap;
    /*
        TODO: Add a random color tint to the trees
        TODO: Add a random texture to the trees
        TODO: Add 3 different tree models
        TODO: Add basic flora
    */
    Material forestMaterial = sharedMaterials[global::MATERIAL_LIT_INSTANCED];
    forestMaterial.maps[MATERIAL_MAP_ALBEDO].texture = treeModel.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
    InstanceMeshRenderData forestImrd = ForestCreate(
        global::images[global::IMAGE_TERRAINMAP],
        forestGenInfo,
        treeModel.meshes[0],
        sharedMaterials[global::MATERIAL_LIT_INSTANCED]);

    while (!WindowShouldClose()) {
        InputUpdate(&global::input);

        if (global::input.pressed[INPUT_TOGGLE_DEBUG]) {
            freecam = freecam ? 0 : 1;
            if (freecam) {
                debugCamera.position = camera.position;
                debugCamera.target = camera.target;
                debugCamera.projection = camera.projection;
                debugCamera.fovy = camera.fovy;
                debugCamera.up = {0.f, 1.f, 0.f};
            }
        }

        v2 mouseScroll = GetMouseWheelMoveV();
        debugCameraSpeed = fclampf(debugCameraSpeed + mouseScroll.y * 0.01f, 0.05f, 1.f);
        Camera *usingCamera;
        if (freecam) {
            CameraUpdateDebug(&debugCamera, debugCameraSpeed);
            usingCamera = &debugCamera;
        } else {
            CabUpdate(&cab);
            CameraUpdate(&camera, CabGetFrontSeatPosition(&cab), {cab.yawRotationStep, 0.f}, cab.direction);
            usingCamera = &camera;
        }

        UpdateGlobalMaterials(sharedMaterials, usingCamera->position);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(*usingCamera);
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
                DrawModel(skyboxModel, {0.f}, 1.0f, WHITE);
            rlEnableBackfaceCulling();
            rlEnableDepthMask();
            DrawMeshInstanced(forestImrd.mesh, forestImrd.material, forestImrd.transforms, forestImrd.instanceCount);
            DrawModel(terrainHeightmap.model, terrainHeightmap.position, 1.f, WHITE);
            CabDraw(&cab);
            ParticleSystemStep(&psys);
            ParticleSystemDraw(&psys);
            DrawGrid(20, 1.f);
        EndMode3D();
        DrawFPS(4, 4);
        if (freecam) {
            DrawTextShadow("Debug cam", 4, 20, 16, RED, BLACK);
        }
        DrawCrosshair(screenSize.x / 2, screenSize.y / 2, WHITE);
        EndDrawing();
    }

    UnloadGameResources();
    ParticleSystemDestroy(&psys);

    return(0);
}

void InputInit(Input *input) {
    input->map[INPUT_ACCELERATE] = KEY_SPACE;
    input->map[INPUT_BREAK] = KEY_LEFT_SHIFT;
    input->map[INPUT_LEFT] = KEY_LEFT;
    input->map[INPUT_RIGHT] = KEY_RIGHT;
    input->map[INPUT_TOGGLE_DEBUG] = KEY_BACKSPACE;
    input->map[INPUT_DEBUG_LEFT] = KEY_A;
    input->map[INPUT_DEBUG_RIGHT] = KEY_D;
    input->map[INPUT_DEBUG_FORWARD] = KEY_W;
    input->map[INPUT_DEBUG_BACKWARD] = KEY_S;
    input->map[INPUT_DEBUG_UP] = KEY_Q;
    input->map[INPUT_DEBUG_DOWN] = KEY_E;
}
void InputUpdate(Input *input) {
    for (i32 i = 0; i < INPUT_COUNT; i++) {
        input->pressed[i] = IsKeyPressed(input->map[i]);
        input->down[i] = IsKeyDown(input->map[i]);
        input->up[i] = IsKeyUp(input->map[i]);
    }
}

void InstanceMeshRenderDataDestroy(InstanceMeshRenderData imrd) {
    UnloadModel(imrd._model);
    UnloadMaterial(imrd.material);
    // TODO: Double check if shader is unloaded as part of material
    RL_FREE(imrd.transforms);
}

InstanceMeshRenderData ForestCreate(Image image, ForestGenerationInfo info, Mesh mesh, Material material) {
    const i32 stride = PixelformatGetStride(image.format);
    if (stride < 3) {
        TraceLog(LOG_WARNING, "ForestCreate: Passed Image isn't valid for creating a forst");
        return {}; // TODO: Implement renderable default data for when InstanceMeshRenderData creation fails
    }
    const v2 imageSize = {(float)image.width, (float)image.height};
    const u32 treesMax = (u32)ceilf((info.worldSize.x * info.density) * (info.worldSize.y * info.density));
    u32 treeCount = 0;
    mat4 *transforms = (mat4*)RL_CALLOC(treesMax, sizeof(mat4));
    v2 pixelIncrF = imageSize / info.worldSize / info.density;
    const u32 incrx = pixelIncrF.x < 1.f ? 1 : (u32)floorf(pixelIncrF.x);
    const u32 incry = pixelIncrF.y < 1.f ? 1 : (u32)floorf(pixelIncrF.y);
    const byte* imageData = (byte*)image.data;
    for (u32 x = 0; x < image.width; x += incrx) {
        for (u32 y = 0; y < image.height; y += incry) {
            u32 i = (x + y * image.width) * stride;
            Color color = {imageData[i], imageData[i+1], imageData[i+2], 0};
            if (color.g == 255 && GetRandomChanceF(info.treeChance)) {
                v2 treePos = v2{(float)x, (float)y} / imageSize * info.worldSize + info.worldPosition;
                transforms[treeCount] = MatrixRotateYaw(GetRandomValueF(0.f, PI));
                transforms[treeCount] *= MatrixRotatePitch(GetRandomValueF(0.f, info.randomTiltDegrees) * DEG2RAD);
                transforms[treeCount] *= MatrixTranslate(
                    treePos.x + GetRandomValueF(-info.randomPositionOffset, info.randomPositionOffset),
                    GetRandomValueF(-info.randomYDip, 0.f) + HeightmapSampleHeight(*info.heightmap, treePos.x, treePos.y), // TODO: Sample a heightmap to get a proper vertical tree position
                    treePos.y + GetRandomValueF(-info.randomPositionOffset, info.randomPositionOffset));
                treeCount++;
            }
        }
    }

    InstanceMeshRenderData imrd = {};
    imrd.instanceCount = treeCount;
    imrd.transforms = transforms;
    imrd.mesh = mesh;
    imrd.material = material;
    return imrd;
}

ParticleSystem ParticleSystemCreate(Texture texture, Material material) {
    ParticleSystem psys = {};
    psys._material = material;
    psys._material.maps[MATERIAL_MAP_ALBEDO].texture = texture;
    psys._quad = GenMeshPlane(1.f, 1.f, 1.f, 1);
    psys._transforms = (mat4*)RL_CALLOC(PARTICLE_SYSTEM_MAX_PARTICLES, sizeof(mat4));
    psys.count = 64;
    for (i32 i = 0; i < psys.count; i++) {
        psys._transforms[i] = MatrixTranslate(
            GetRandomValueF(-10.f, 10.f),
            GetRandomValueF(-10.f, 10.f),
            GetRandomValueF(-10.f, 10.f));
    }
    return psys;
}

void ParticleSystemDestroy(ParticleSystem *psys) {
    UnloadMesh(psys->_quad);
    RL_FREE(psys->_transforms);
}

void ParticleSystemStep(ParticleSystem *psys) {
    v3 stepVelocity = psys->velocity * FRAME_TIME;
    mat4 transpose = MatrixTranslate(stepVelocity.x, stepVelocity.y, stepVelocity.z);
    for (i32 i = 0; i < psys->count; i++) {
        psys->_transforms[i] *= transpose;
    }
}

void ParticleSystemDraw(ParticleSystem *psys) {
    DrawMeshInstanced(psys->_quad, psys->_material, psys->_transforms, psys->count);
}

void BillboardParticleSystemStep(BillboardParticleSystem *psys, Camera3D camera) {
    psys->_spawnTimer += psys->rate * FRAME_TIME;
    while (psys->_spawnTimer >= 1.f) {
        psys->_spawnTimer -= 1.f;
        if (psys->_number + 1 < MAX_BILLBOARD_PARTICLES) {
            psys->_number++;
            BillboardParticle *p = &psys->particles[(psys->_index + psys->_number) % MAX_BILLBOARD_PARTICLES];
            p->position.y = 0.f;
            p->life = psys->life;
        }
    }

    BillboardParticle *parts[MAX_BILLBOARD_PARTICLES] = {NULL};
    float dists[MAX_BILLBOARD_PARTICLES] = {-1.f};
    v2 cameraPosition = {camera.position.x, camera.position.z};
    v2 cameraTarget = {camera.target.x, camera.target.z};
    for (u32 i = 0; i < psys->_number; i++) {
        BillboardParticle *p = &psys->particles[(psys->_index + i) % MAX_BILLBOARD_PARTICLES];

        p->position = p->position + psys->speed * FRAME_TIME;
        p->life -= FRAME_TIME;

        v2 ppos = {p->position.x, p->position.z};
        dists[i] = Vector2Distance(cameraPosition, ppos);
        parts[i] = p;
    }

    // TODO: Sort from camera tangent line instead of position to fix clipping
    // TODO: Make max particles settable
    BillboardParticle *sortedParts[MAX_BILLBOARD_PARTICLES] = {NULL};
    float sortedDists[MAX_BILLBOARD_PARTICLES] = {1.f};
    sortedParts[0] = parts[0];
    sortedDists[0] = dists[0];
    for (i32 i = 1; i < psys->_number; i++) {
        bool biggest = true;
        for (i32 j = 0; j < i; j++) {
            if (dists[i] > sortedDists[j]) {
                for (i32 k = i; k >= j; k--) {
                    sortedDists[k+1] = sortedDists[k];
                    sortedParts[k+1] = sortedParts[k];
                }
                sortedDists[j] = dists[i];
                sortedParts[j] = parts[i];
                biggest = false;
                break;
            }
        }
        if (biggest) {
            sortedDists[i] = dists[i];
            sortedParts[i] = parts[i];
        }
    }

    rlSetBlendMode(psys->blendMode);
    for (u32 i = 0; i < psys->_number; i++) {
        BillboardParticle *p = sortedParts[i];
        float blendProgress = fclampf(1.f - p->life / psys->life, 0.f, 1.f);
        u8 alpha = (u8)(QuadraticBezierLerp(psys->alphaCurve, blendProgress) * 255.f);
        Color blend = {255, 255, 255, alpha};
        DrawBillboard(camera, psys->texture, p->position, 1.f, blend);
    }
    rlSetBlendMode(BLEND_ALPHA);

    while (psys->_number > 0 && psys->particles[psys->_index].life <= 0.f) {
        psys->_index = (psys->_index + 1) % MAX_BILLBOARD_PARTICLES;
        psys->_number--;
    }
}

void CabInit(Cab *cab) {
    cab->model = global::models[global::MODEL_CAB];
    cab->frontSeat = {-0.13f, 1.65f, -0.44f};
    cab->direction = {1.f, 0.f, 0.f};
    cab->maxVelocity = 0.4f;
    cab->acceleration = 0.01f;
    cab->neutralDeceleration = 0.004f;
    cab->breakDeceleration = 0.015f;
    cab->turnAngleMax = 24.f;
    cab->turnAngleSpeed = 64.f;
    cab->accelerationCurve = {{0.f, 0.f}, {0.1f, 0.55f}, {1.f, 1.f}};
    cab->reverseAccelerationCurve = {{0.f, 0.f}, {-0.236f, -0.252f}, {-1.f, -0.4f}};
    cab->_transform = MatrixIdentity();
}
void CabUpdate(Cab *cab) {
    bool inputAccelerate = global::input.down[INPUT_ACCELERATE];
    bool inputBreak = global::input.down[INPUT_BREAK];
    if (inputAccelerate && inputBreak) {
        // TODO: drift
    } else if (inputAccelerate) {
        cab->_speed += cab->acceleration;
    } else if (inputBreak) {
        cab->_speed -= cab->breakDeceleration;
    } else {
        float absSpeed = fabsf(cab->_speed);
        float speedSign = fsignf(cab->_speed);
        cab->_speed = fmaxf(absSpeed - cab->neutralDeceleration, 0.f) * speedSign;
    }
    cab->_speed = fclampf(cab->_speed, -1.f, 1.f);

    float stepSpeed = 0.f;
    if (cab->_speed > 0.f) {
        stepSpeed = QuadraticBezierLerp(cab->accelerationCurve, cab->_speed) * cab->maxVelocity;
    } else if (cab->_speed < 0.f) {
        stepSpeed = QuadraticBezierLerp(cab->reverseAccelerationCurve, fabsf(cab->_speed)) * cab->maxVelocity;
    }

    float inputTurn = ((float)global::input.down[INPUT_RIGHT] - (float)global::input.down[INPUT_LEFT]) * -1.f;
    float turnAmountStep = inputTurn * FRAME_TIME * cab->turnAngleSpeed;
    cab->_turnAngle = fclampf(cab->_turnAngle + turnAmountStep, -cab->turnAngleMax, cab->turnAngleMax);
    cab->yawRotationStep = cab->_turnAngle * DEG2RAD * stepSpeed;
    mat4 yawRotationMatrix = MatrixRotateYaw(cab->yawRotationStep);
    cab->_transform *= yawRotationMatrix;

    // TODO: Inefficient matrix multiplication
    cab->direction = cab->direction * yawRotationMatrix;
    cab->velocity = cab->direction * stepSpeed;
    cab->position += cab->velocity;
}
void CabDraw(Cab *cab) {
    mat4 transform = cab->_transform * MatrixTranslate(cab->position.x, cab->position.y, cab->position.z);
    for (i32 i = 0; i < cab->model.meshCount; i++) {
        DrawMesh(cab->model.meshes[i], cab->model.materials[cab->model.meshMaterial[i]], transform);
    }
}
v3 CabGetFrontSeatPosition(Cab *cab) {
    return cab->position + cab->frontSeat * cab->_transform;
}

void CameraUpdateDebug(Camera *camera, float speed) {
    v3 targetDirection = camera->target - camera->position;
    float sensitivity = 0.002f;
    v2 mouseDelta = GetMouseDelta();
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraUpNorm(camera), -mouseDelta.x * sensitivity);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraRightNorm(camera), -mouseDelta.y * sensitivity);
    v3 input = {
        (float)global::input.down[INPUT_DEBUG_FORWARD] - (float)global::input.down[INPUT_DEBUG_BACKWARD],
        (float)global::input.down[INPUT_DEBUG_UP] - (float)global::input.down[INPUT_DEBUG_DOWN],
        (float)global::input.down[INPUT_DEBUG_RIGHT] - (float)global::input.down[INPUT_DEBUG_LEFT]};
    v2 hInput = {input.x, input.z};
    if (Vector2LengthSqr(hInput) > 0.f) {
        v2 hInputNorm = Vector2Normalize(hInput);
        v2 hDirection = Vector2Rotate({targetDirection.x, targetDirection.z}, atan2f(hInputNorm.y, hInputNorm.x));
        camera->position.x += hDirection.x * speed;
        camera->position.z += hDirection.y * speed;
    }
    camera->position.y += input.y * speed;
    camera->target = camera->position + targetDirection;
}

void CameraUpdate(Camera *camera, v3 position, v2 rotationAdd, v3 viewMiddle) {
    v3 target = Vector3Normalize(camera->target - camera->position);
    v2 euler = {atan2f(target.x, target.z), atan2f(target.y, target.x)};
    v2 mouseRotationStep = GetMouseDelta() * FRAME_TIME * -0.05f;
    v2 eulerNext = v2clampv2(euler + mouseRotationStep + rotationAdd, v2{0.1f, PI / -2.f + 0.3f}, v2{PI - 0.1f, PI / 2.f - 0.3f});
    v2 eulerDiff = eulerNext - euler;
    mat4 rot = MatrixRotateYaw(eulerDiff.x) * MatrixRotatePitch(eulerDiff.y);
    v3 rotatedTarget = target * rot;
    float yawMiddleDifference = Vector2DotProduct({viewMiddle.x, viewMiddle.z}, {rotatedTarget.x, rotatedTarget.z});
    float pitchMiddleDifference = Vector2DotProduct({viewMiddle.x, viewMiddle.y}, {rotatedTarget.x, rotatedTarget.y});
    
    camera->position = position;
    camera->target = camera->position + target * rot;
}
v3 GetCameraForwardNorm(Camera *camera) {
    return Vector3Normalize(camera->target - camera->position);
}
v3 GetCameraUpNorm(Camera *camera) {
    return Vector3Normalize(camera->up);
}
v3 GetCameraRightNorm(Camera *camera) {
    v3 forward = GetCameraForwardNorm(camera);
    v3 up = GetCameraUpNorm(camera);
    return Vector3Normalize(Vector3CrossProduct(forward, up));
}

Heightmap HeightmapCreate(HeightmapGenerationInfo info, Material material) {
    const u32 resdiv = info.resdiv;
    const Image *image = info.heightmapImage;
    const u32 stride = PixelformatGetStride(image->format);
    const u32 len = info.heightmapImage->width * info.heightmapImage->height;
    const u32 heightDataWidth = image->width >> resdiv;
    const u32 heightDataHeight = image->height >> resdiv;

    float *heightData = (float*)malloc(heightDataWidth * heightDataHeight * sizeof(float));
    byte* data = (byte*)info.heightmapImage->data;
    u32 dataW = heightDataWidth;
    u32 imgw = image->width;

    for (u32 x = 0; x < heightDataWidth; x++) {
        for (u32 y = 0; y < heightDataHeight; y++) {
            byte value = data[((x << resdiv) + ((y << resdiv) * imgw)) * stride];
            heightData[x + y * dataW] = ((float)value / 255.f) * info.size.y;
        }
    }

    Mesh hmMesh = GenMeshHeightmap(*image, info.size);
    Heightmap heightmap = {};
    heightmap.heightData = heightData;
    heightmap.heightmap = *image;
    heightmap.heightDataWidth = heightDataWidth;
    heightmap.heightDataHeight = heightDataHeight;
    heightmap.size = info.size;
    heightmap.texture = *info.terrainMapTexture;
    heightmap.model = LoadModelFromMesh(hmMesh);
    heightmap.model.materials[0] = material;
    heightmap.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *info.terrainMapTexture;
    heightmap.model.materials[0].maps[MATERIAL_MAP_SPECULAR].texture = *info.terrainTexture;
    heightmap.position = info.position;
    heightmap.debugModel = {};
    return heightmap;
}
float HeightmapSampleHeight(Heightmap heightmap, float x, float z) {
    v2 rectBegin = {heightmap.position.x, heightmap.position.z};
    v2 rectEnd = rectBegin + v2{heightmap.size.x, heightmap.size.z};
    if (PointInRectangle(rectBegin, rectEnd, {x, z})) {
        v2 abspos = v2{x, z} - rectBegin;
        v2 normpos = abspos / v2{heightmap.size.x, heightmap.size.z};
        v2 datapos = normpos * v2{(float)heightmap.heightDataWidth, (float)heightmap.heightDataHeight};
        v2 dataposFract = Vector2Fract(datapos);
        u32 dataX = datapos.x;
        u32 dataY = datapos.y;
        bool nextX = dataX + 1 < heightmap.heightDataWidth;
        bool nextY = dataY + 1 < heightmap.heightDataHeight;
        // TODO: Could probably just pad the data by 1 on the right and bottom
        float a = heightmap.heightData[dataX + dataY * heightmap.heightDataWidth];
        float b = nextX ? heightmap.heightData[(dataX+1) + dataY * heightmap.heightDataWidth] : 0.f;
        float c = nextY ? heightmap.heightData[dataX + (dataY + 1) * heightmap.heightDataWidth] : 0.f;
        float d = nextX && nextY ? heightmap.heightData[(dataX + 1) + (dataY + 1) * heightmap.heightDataWidth] : 0.f;
        return Lerp(Lerp(a, b, dataposFract.x), Lerp(c, d, dataposFract.x), dataposFract.y);
    }
    return 0.f;
}
void HeightmapDestroy(Heightmap heightmap) {
    free(heightmap.heightData);
}

List ListCreate(u32 size) {
    List list = {};
    ListInit(&list, size);
    return list;
}
void ListInit(List* list, u32 size) {
    if (size == 0) {
        list->data = NULL;
    } else {
        list->data = (void**)malloc(size * sizeof(void*));
    }
    list->size = 0;
    while (list->size < size) {
        list->data[list->size] = 0;
        list->size++;
    }
    list->capacity = list->size;
}
void ListDestroy(List *list) {
    if (list->data != NULL) {
        free(list->data);
        list->data = NULL;
        list->size = 0;
    }
}
void ListResize(List *list, u32 size) {
    if (list->data != NULL) {
        list->data = (void**)realloc(list->data, sizeof(void*) * size);
    } else {
        list->data = (void**)malloc(sizeof(void*) * size);
    }
    while (list->size < size) {
        list->data[list->size] = 0;
        list->size++;
    }
    list->capacity = size;
    list->size = size;
}
void ListChangeCapacity(List *list, u32 capacity) {
    if (list->data != NULL) {
        list->data = (void**)realloc(list->data, sizeof(void*) * capacity);
    } else {
        list->data = (void**)malloc(sizeof(void*) * capacity);
    }
    list->capacity = capacity;
}
void* ListGet(List *list, u32 ind) {
    if (ind >= list->size && ind < list->capacity) {
        TraceLog(LOG_ERROR, "List: Tried getting value outside of size in list, returned '-1'");
        return NULL;
    }
    return list->data[ind];
}
void ListSet(List *list, u32 ind, void* val) {
    list->data[ind] = val;
}
void ListPushBack(List *list, void* val) {
    if (list->capacity == list->size) {
        if (list->capacity == 0) {
            ListChangeCapacity(list, 8 * sizeof(void*));
        } else {
            ListChangeCapacity(list, list->capacity * 2 * sizeof(void*));
        }
    }
    list->data[list->size] = val;
    list->size++;
}
