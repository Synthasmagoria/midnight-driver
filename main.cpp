#include "raylib.h"
#include "raymath.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "rlgl.h"

#include <unordered_map>
#include <string>

using std::string;
using std::unordered_map;

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
typedef Rectangle rect2;
typedef Vector4 v4;
typedef Vector3 v3;
typedef Vector2 v2;

#define FRAME_TIME 1.f / 60.f
#define FRAMERATE 60
#define TAU PI * 2.f
#define GAME_OBJECT_MAX 1000

struct MemoryManager;
struct String;
struct Typewriter;
struct DialogueOptions;
struct TextDrawingStyle;
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
struct GameObject;
struct Skybox;

void DrawMeshInstancedOptimized(Mesh mesh, Material material, const float16 *transforms, int instances);

struct MemoryManager {
    void* buffer;
    i32 location;
    i32 size;
};
MemoryManager MemoryManagerCreate(i32 size);
void MemoryManagerDestroy(MemoryManager* mm);
void* MemoryManagerReserve(MemoryManager* mm, i32 size);

struct String {
    char* cstr;
    i32 length;
};
String StringCreate(char* text);
String _StringCreate(i32 length);
String StringSet(String str, char* text);
String StringSubstr(String str, i32 start, i32 count);
void StringDestroy(String str);

struct Typewriter {
    String *text;
    i32 textCount;
    i32 textIndex;
    float autoContinueDelay;
    float _autoContinueTime;
    float autoHideOnEndDelay;
    float _autoHideOnEndTime;
    i32 visible;
    i32 x;
    i32 y;
    float speed;
    float progress;
    TextDrawingStyle* textStyle;
};
void TypewriterUpdate(void* tw);
void TypewriterDraw(void* tw);
GameObject TypewriterPack(Typewriter* tw);
void _TypewriterAdvanceText(Typewriter *tw);
void _TypewriterReset(Typewriter *tw);

struct DialogueOptions {
    String* options;
    i32 index;
    i32 number;
    i32 x;
    i32 y;
    float optionSeparationAdd;
    float optionBoxHeightAdd;
    TextDrawingStyle *textStyle;
    NPatchInfo npatchInfo;
    Texture2D npatchTexture;
};
void DialogueOptionsHandleInput(DialogueOptions* dialogueOptions);
void DialogueOptionsUpdate(void* _dopt);
void DialogueOptionsDraw(void* _dopt);
GameObject DialogueOptionsPack(DialogueOptions* dopt);

struct TextDrawingStyle {
    Color color;
    Font font;
    float size;
    i32 charSpacing;
};

enum INPUT {
    INPUT_ACCELERATE,
    INPUT_BREAK,
    INPUT_TOGGLE_DEBUG,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,
    INPUT_UP,
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

// TODO: Removed deprecated
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
struct ForestGenerationInfoBlocks {
    v2 worldPosition;
    v2 worldSize;
    i32 blockDivide;
    float density;
    float treeChance;
    float randomPositionOffset;
    float randomYDip;
    float randomTiltDegrees;
    Heightmap* heightmap;
};
struct InstanceMeshRenderData {
    float16 *transforms;
    u32 instanceCount;
    Model _model;
    Mesh mesh;
    Material material;
};
struct InstanceMeshRenderDataBlocks {
    float16 **transformBlocks;
    u32 instanceCount;
    i32 blockCount;
    v3 position;
    Model _model;
    Mesh mesh;
    Material material;
};
InstanceMeshRenderData ForestCreate(Image image, ForestGenerationInfo info, Mesh mesh, Material material);
InstanceMeshRenderData ForestCreateBlocks(Image image, ForestGenerationInfo info, Mesh mesh, Material material);

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
    i32 blendMode;
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
void HeightmapInit(Heightmap* hm, HeightmapGenerationInfo info, Material material);
float HeightmapSampleHeight(Heightmap heightmap, float x, float z);
void HeightmapDraw(void* hm);
void HeightmapFree(void* hm);
GameObject HeightmapPack(Heightmap* hm);

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
    float turnNeutralReturn;
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

struct GameObject {
    void (*Update) (void*);
    void (*Draw3d) (void*);
    void (*DrawUi) (void*);
    void (*Free) (void*);
    void *data;
};
GameObject GameObjectCreate(void* data, void(*Update)(void*), void(*Draw3d)(void*), void(*DrawUi)(void*), void(*Free)(void*));

struct Skybox {
    Model model;
    Shader shader;
};
void SkyboxDraw(void* _skybox);
GameObject SkyboxPack(Skybox* skybox);

inline i32 PointInRectangle(v2 begin, v2 end, v2 pt) {return pt.x >= begin.x && pt.x < end.x && pt.y >= begin.y && pt.y < end.y;}
inline const char* TextFormatVector3(v3 v) {return TextFormat("[%f], [%f], [%f]", v.x, v.y, v.z);}
inline void DrawTextShadow(const char* text, i32 x, i32 y, i32 fontSize, Color col, Color shadowCol) {
    DrawText(text, x + 1, y + 1, fontSize, shadowCol);
    DrawText(text, x, y, fontSize, col);
}
inline void DrawCrosshair(i32 x, i32 y, Color col) {
    DrawLine(x - 4, y, x + 4, y, col);
    DrawLine(x, y - 4, x, y + 4, col);
}
inline i32 imini(i32 a, i32 b) {return a < b ? a : b;}
inline i32 iwrapi(i32 val, i32 min, i32 max) {
    i32 range = max - min;
	return range == 0 ? min : min + ((((val - min) % range) + range) % range);
}
inline float ffractf(float val) {return val - floorf(val);}
inline float fclampf(float val, float min, float max) {return fminf(fmaxf(val, min), max);}
inline float fsignf(float val) {return (float)(!signbit(val)) * 2.f - 1.f;}
inline v2 v2fractv2(v2 v) {return {ffractf(v.x), ffractf(v.y)};}
inline v2 v2clampv2(v2 val, v2 min, v2 max) {return {fclampf(val.x, min.x, max.x), fclampf(val.y, min.y, max.y)};}
inline v2 v2maxv2(v2 a, v2 b) {return {fmaxf(a.x, b.x), fmaxf(a.y, b.y)};}
inline v2 v2minv2(v2 a, v2 b) {return {fmaxf(a.x, b.x), fmaxf(a.y, b.y)};}
inline float ApproachZero(float val, float amount) {return fmaxf(fabsf(val) - amount, 0.f) * fsignf(val);}
// NOTE: only supports up to 3 decimals
inline float GetRandomValueF(float min, float max) {
    return ((float)GetRandomValue((i32)(min * 1000.f), (i32)(max * 1000.f))) / 1000.f;
}
inline bool GetRandomChanceF(float percentage) {return GetRandomValueF(0.f, 100.f) < percentage;}

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
    for (i32 i = 0; i < mesh.vertexCount; i+=3) {
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
BoundingBox GenBoundingBoxMeshes(Mesh* meshes, i32 meshCount) {
    v2 xbounds = {0.f}, ybounds = {0.f}, zbounds = {0.f};
    for (i32 i = 0; i < meshCount; i++) {
        Mesh *mesh = meshes + i;
        for (i32 j = 0; j < mesh->vertexCount; j+=3) {
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
    Material materials[MATERIAL_COUNT];

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
        TEXTURE_NPATCH,
        TEXTURE_COUNT
    };
    const char *texturePaths[global::TEXTURE_COUNT] = {
        "star.png",
        "terrain.png",
        "terrainmap2.png",
        "npatch.png"
    };
    Texture2D textures[global::TEXTURE_COUNT];

    enum GAME_IMAGES {
        IMAGE_SKYBOX,
        IMAGE_HEIGHTMAP,
        IMAGE_TERRAINMAP,
        IMAGE_TERRAINMAP_TEST,
        IMAGE_COUNT
    };
    const char *imagePaths[IMAGE_COUNT] = {
        "skybox.png",
        "heightmap2.png",
        "terrainmap2.png",
        "terrainmapTest.png"
    };
    Image images[IMAGE_COUNT];

    Input input;

    unordered_map<string, void*> groups = unordered_map<string, void*>();

    const i32 dialogueCount = 3;
    String dialogue[dialogueCount] = {
       StringCreate("This is a string instance"),
       StringCreate("Second sentence hits harder than the first for some reason"),
       StringCreate("Holy fuck there's a third")
    };

    const i32 dialogueOptionsCount = 3;
    String dialogueOptions[dialogueOptionsCount] = {
        StringCreate("Hello!"),
        StringCreate("What can I do for you?"),
        StringCreate("Want a cigarette?")
    };
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

void LoadGameMaterials() {
    Shader sh;
    Material mat;
    
    sh = LOAD_SHADER("lightInstanced.vs", "light.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    global::materials[global::MATERIAL_LIT_INSTANCED] = mat;

    sh = LOAD_SHADER("light.vs", "light.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    global::materials[global::MATERIAL_LIT] = mat;

    sh = LOAD_SHADER("unlitInstanced.vs", "passthrough.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    global::materials[global::MATERIAL_UNLIT] = mat;

    sh = LOAD_SHADER("light.vs", "lightTerrain.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    sh.locs[SHADER_LOC_COLOR_SPECULAR] = GetShaderLocation(sh, "texture1");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    global::materials[global::MATERIAL_LIT_TERRAIN] = mat;
}
void UpdateGameMaterials(v3 lightPosition) {
    float pos[3] = {lightPosition.x, lightPosition.y, lightPosition.z};
    SetShaderValue(
        global::materials[global::MATERIAL_LIT_INSTANCED].shader,
        GetShaderLocation(global::materials[global::MATERIAL_LIT_INSTANCED].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
    SetShaderValue(
        global::materials[global::MATERIAL_LIT].shader,
        GetShaderLocation(global::materials[global::MATERIAL_LIT].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
    SetShaderValue(
        global::materials[global::MATERIAL_LIT_TERRAIN].shader,
        GetShaderLocation(global::materials[global::MATERIAL_LIT_TERRAIN].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
}
void UnloadGameMaterials() {
    for (i32 i = 0; i < global::MATERIAL_COUNT; i++) {
        UnloadMaterial(global::materials[i]);
    }
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
    LoadGameMaterials();
}
void UnloadGameResources() {
    UnloadGameShaders();
    UnloadGameModels();
    UnloadGameTextures();
    UnloadGameImages();
    UnloadGameMaterials();
}

const i32 screenWidth = 1440;
const i32 screenHeight = 800;

i32 SceneSetup01(GameObject* goOut, MemoryManager* mm) {
    i32 goCount = 0;

    v3 terrainSize = {1000.f, 100.f, 1000.f};
    v3 terrainPosition = terrainSize / -2.f;
    terrainPosition.y = 0.f;

    HeightmapGenerationInfo hgi = {};
    hgi.heightmapImage = &global::images[global::IMAGE_HEIGHTMAP];
    hgi.terrainMapTexture = &global::textures[global::TEXTURE_TERRAINMAP_BLURRED];
    hgi.terrainTexture = &global::textures[global::TEXTURE_TERRAIN];
    hgi.position = terrainPosition;
    hgi.size = terrainSize;
    hgi.resdiv = 2;
    Heightmap* hm = (Heightmap*)MemoryManagerReserve(mm, sizeof(Heightmap));
    HeightmapInit(hm, hgi, global::materials[global::MATERIAL_LIT_TERRAIN]);
    global::groups["terrainHeightmap"] = (void*)&hm;
    goOut[goCount] = HeightmapPack(hm); goCount++;

    Typewriter* tw = (Typewriter*)MemoryManagerReserve(mm, sizeof(Typewriter));
    tw->text = &global::dialogue[1];
    tw->visible = false;
    tw->textCount = 2;
    tw->speed = 16.f;
    tw->autoContinueDelay = 1.f;
    tw->autoHideOnEndDelay = 3.f;
    tw->x = screenWidth / 2;
    tw->y = screenHeight / 2 + screenHeight / 4;
    tw->textStyle = (TextDrawingStyle*)global::groups["mainTextDrawingStyle"];
    goOut[goCount] = TypewriterPack(tw); goCount++;

    DialogueOptions* dialogueOptions = (DialogueOptions*)MemoryManagerReserve(mm, sizeof(DialogueOptions));
    dialogueOptions->index = 0;
    dialogueOptions->options = &global::dialogueOptions[0];
    dialogueOptions->number = 3;
    dialogueOptions->x = screenWidth / 2;
    dialogueOptions->y = screenHeight / 2 + screenHeight / 4;
    dialogueOptions->textStyle = (TextDrawingStyle*)global::groups["mainTextDrawingStyle"];
    dialogueOptions->npatchInfo = {{0.f, 0.f, 96.f, 96.f}, 32, 32, 32, 32, NPATCH_NINE_PATCH};
    dialogueOptions->npatchTexture = global::textures[global::TEXTURE_NPATCH];
    dialogueOptions->optionBoxHeightAdd = 32.f;
    dialogueOptions->optionSeparationAdd = 48.f;
    goOut[goCount] = DialogueOptionsPack(dialogueOptions); goCount++;

    Skybox* skybox = (Skybox*)MemoryManagerReserve(mm, sizeof(Skybox));
    skybox->model = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    skybox->shader = global::shaders[global::SHADER_SKYBOX];
    {
        i32 envmapValue = MATERIAL_MAP_CUBEMAP;
        SetShaderValue(skybox->shader, GetShaderLocation(skybox->shader, "environmentMap"), &envmapValue, SHADER_UNIFORM_INT);
        i32 gammaValue = 0;
        SetShaderValue(skybox->shader, GetShaderLocation(skybox->shader, "doGamma"), &gammaValue, SHADER_UNIFORM_INT);
        i32 vflippedValue = 0;
        SetShaderValue(skybox->shader, GetShaderLocation(skybox->shader, "vflipped"), &vflippedValue, SHADER_UNIFORM_INT);
    }
    skybox->model.materials[0].shader = skybox->shader;
    skybox->model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(
        global::images[global::IMAGE_SKYBOX],
        CUBEMAP_LAYOUT_AUTO_DETECT);
    goOut[goCount] = SkyboxPack(skybox); goCount++;

    return goCount;
};


i32 main() {
    u32 freecam = 0; 
    
    SetTraceLogLevel(4);
    InitWindow(screenWidth, screenHeight, "Midnight Driver");
    SetTargetFPS(60);
    DisableCursor();

    MemoryManager memoryManager = MemoryManagerCreate(1 << 16);

    LoadGameResources();
    InputInit(&global::input);

    TextDrawingStyle textDrawingStyle = {};
    textDrawingStyle.charSpacing = 1;
    textDrawingStyle.font = GetFontDefault();
    textDrawingStyle.size = 24.f;
    textDrawingStyle.color = WHITE;
    global::groups["mainTextDrawingStyle"] = (void*)&textDrawingStyle;

    GameObject gameObject[GAME_OBJECT_MAX];
    i32 gameObjectCount = SceneSetup01(gameObject, &memoryManager);

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

    v3 terrainSize = {1000.f, 100.f, 1000.f};
    v3 terrainPosition = terrainSize / -2.f;
    terrainPosition.y = 0.f;

    ParticleSystem psys = ParticleSystemCreate(global::textures[global::TEXTURE_STAR], global::materials[global::MATERIAL_UNLIT]);
    psys.velocity = {1.f, 0.f, 0.f};

    ForestGenerationInfo forestInfo = {};
    forestInfo.worldPosition = {0.f, 0.f};
    forestInfo.worldSize = {16.f, 16.f};
    InstanceMeshRenderData imrd = ForestCreateBlocks(
        global::images[global::IMAGE_TERRAINMAP_TEST],
        forestInfo,
        global::models[global::MODEL_TREE].meshes[0],
        global::materials[global::MATERIAL_UNLIT]);

    while (!WindowShouldClose()) {
        InputUpdate(&global::input);

        for (i32 i = 0; i < gameObjectCount; i++) {
            if (gameObject[i].Update != nullptr) {
                gameObject[i].Update(gameObject[i].data);
            }
        }

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

        UpdateGameMaterials(usingCamera->position);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(*usingCamera);
            for (i32 i = 0; i < gameObjectCount; i++) {
                if (gameObject[i].Draw3d != nullptr) {
                    gameObject[i].Draw3d(gameObject[i].data);
                }
            }
            DrawMeshInstancedOptimized(imrd.mesh, imrd.material, imrd.transforms, imrd.instanceCount);
            CabDraw(&cab);
            ParticleSystemStep(&psys);
            ParticleSystemDraw(&psys);
            DrawGrid(20, 1.f);
        EndMode3D();
        for (i32 i = 0; i < gameObjectCount; i++) {
            if (gameObject[i].DrawUi != nullptr) {
                gameObject[i].DrawUi(gameObject[i].data);
            }
        }
        DrawFPS(4, 4);
        if (freecam) {
            DrawTextShadow("Debug cam", 4, 20, 16, RED, BLACK);
        }
        DrawCrosshair(screenWidth / 2, screenHeight / 2, WHITE);
        EndDrawing();
    }

    

    MemoryManagerDestroy(&memoryManager);
    UnloadGameResources();
    ParticleSystemDestroy(&psys);

    return(0);
}

MemoryManager MemoryManagerCreate(i32 size) {
    MemoryManager mm;
    mm.buffer = calloc(size, sizeof(byte));
    mm.location = 0;
    mm.size = size;
    return mm;
}
void MemoryManagerDestroy(MemoryManager* mm) {
    if (mm->buffer != nullptr) {
        free(mm->buffer);
    }
}
void* MemoryManagerReserve(MemoryManager* mm, i32 size) {
    if (mm->location + size >= mm->size) {
        return nullptr;
    }
    void* reserve = (byte*)mm->buffer + mm->location;
    mm->location += size;
    if (mm->location % sizeof(void*) != 0) {
        i32 alignedLocation = mm->location + (sizeof(void*) - (mm->location % sizeof(void*)));
        mm->location = imini(alignedLocation, mm->size);
    }
    return reserve;
}

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
    i32 len = strlen(text) + 1;
    if (str.cstr != nullptr) {
        free(str.cstr);
    }
    str.cstr = (char*)malloc(len);
    memccpy(str.cstr, text, '\0', len);
    str.length = len;
    return str;
}
String StringSubstr(String str, i32 start, i32 count) {
    if (count >= (str.length + 1) - start) {
        count = str.length - start;
    }
    String substr = _StringCreate(count);
    memccpy(substr.cstr, &str.cstr[start], '\0', count);
    return substr;
}
void StringDestroy(String str) {
    if (str.cstr != nullptr) {
        free(str.cstr);
    }
}

void TypewriterUpdate(void *_tw) {
    Typewriter* tw = (Typewriter*)_tw;
    if (tw->text == nullptr) {
        return;
    }

    float length = (float)tw->text[tw->textIndex].length;
    float textProgressPrevious = tw->progress;
    tw->progress = fminf(tw->progress + tw->speed * FRAME_TIME, length);
    if (tw->progress == length) {
        if (tw->textIndex + 1 < tw->textCount) {
            if (textProgressPrevious < tw->progress) {
                tw->_autoContinueTime = tw->autoContinueDelay;
            } else {
                tw->_autoContinueTime = fmaxf(tw->_autoContinueTime - FRAME_TIME, 0.f);
                if (tw->_autoContinueTime == 0.f) {
                    _TypewriterAdvanceText(tw);
                }
            }
        } else if (tw->textIndex + 1 == tw->textCount) {
            if (textProgressPrevious < tw->progress) {
                tw->_autoHideOnEndTime = tw->autoHideOnEndDelay;
            } else {
                tw->_autoHideOnEndTime = fmaxf(tw->_autoHideOnEndTime - FRAME_TIME, 0.f);
                if (tw->_autoHideOnEndTime == 0.f) {
                    _TypewriterReset(tw);
                    tw->visible = false;
                }
            }
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
    TextDrawingStyle* textStyle = tw->textStyle;
    String substr = StringSubstr(tw->text[tw->textIndex], 0, (i32)tw->progress);
    v2 textAlign = MeasureTextEx(textStyle->font, substr.cstr, textStyle->size, textStyle->charSpacing) / 2.f;
    DrawTextPro(textStyle->font, substr.cstr, {(float)tw->x, (float)tw->y}, textAlign, 0.f, 18.f, 1.f, WHITE);
    StringDestroy(substr);
}

GameObject TypewriterPack(Typewriter* tw) {
    GameObject go;
    go.data = (void*)tw;
    go.Update = TypewriterUpdate;
    go.Draw3d = nullptr;
    go.DrawUi = TypewriterDraw;
    return go;
}

void DialogueOptionsHandleInput(DialogueOptions* dialogueOptions) {
    i32 input = global::input.pressed[INPUT_DOWN] - global::input.pressed[INPUT_UP];
    dialogueOptions->index = iwrapi(dialogueOptions->index + input, 0, dialogueOptions->number);
}
void DialogueOptionsUpdate(void* _dopt) {
    DialogueOptions* dopt = (DialogueOptions*)_dopt;
    DialogueOptionsHandleInput(dopt);
}
void DialogueOptionsDraw(void *_dopt) {
    DialogueOptions* dialogueOptions = (DialogueOptions*)_dopt;
    String* options = dialogueOptions->options;
    TextDrawingStyle* textStyle = dialogueOptions->textStyle;
    v2* textSize = (v2*)malloc(dialogueOptions->number * sizeof(v2));
    v2 boxSize = {0.f, 0.f};
    // TODO: These measurements can be done as preprocessing. No need for the memory allocation every fucking frame
    for (i32 i = 0; i < dialogueOptions->number; i++) {
        String option = dialogueOptions->options[i];
        textSize[i] = MeasureTextEx(textStyle->font, option.cstr, textStyle->size, textStyle->charSpacing);
        boxSize = v2maxv2(boxSize, textSize[i]);
    }
    boxSize.x += 64.f;
    for (i32 i = 0; i < dialogueOptions->number; i++) {
        Color textTint = GRAY;
        Color boxTint = GRAY;
        if (i == dialogueOptions->index) {
            textTint = WHITE;
            boxTint = WHITE;
        }

        String option = dialogueOptions->options[i];
        i32 x = dialogueOptions->x;
        i32 y = dialogueOptions->y + i * ((i32)textStyle->size + dialogueOptions->optionSeparationAdd);
        float xf = (float)x;
        float yf = (float)y;
        float boxHeightAdd = 24.f;
        rect2 dialogueBoxRect = {
            xf - boxSize.x / 2.f,
            yf - boxSize.y / 2.f - dialogueOptions->optionBoxHeightAdd / 2.f,
            boxSize.x,
            boxSize.y + dialogueOptions->optionBoxHeightAdd};
        DrawTextureNPatch(
            dialogueOptions->npatchTexture,
            dialogueOptions->npatchInfo,
            dialogueBoxRect,
            {0.f, 0.f},
            0.f,
            boxTint);
        DrawTextPro(
            textStyle->font,
            option.cstr,
            {xf, yf},
            textSize[i] / 2.0,
            0.f,
            textStyle->size,
            textStyle->charSpacing,
            textTint);
    }
    free(textSize);
}
GameObject DialogueOptionsPack(DialogueOptions* dopt) {
    GameObject go = {0};
    go.data = (void*)dopt;
    go.Draw3d = nullptr;
    go.DrawUi = DialogueOptionsDraw;
    go.Update = DialogueOptionsUpdate;
    return go;
}

void InputInit(Input *input) {
    input->map[INPUT_ACCELERATE] = KEY_SPACE;
    input->map[INPUT_BREAK] = KEY_LEFT_SHIFT;
    input->map[INPUT_LEFT] = KEY_LEFT;
    input->map[INPUT_RIGHT] = KEY_RIGHT;
    input->map[INPUT_DOWN] = KEY_DOWN;
    input->map[INPUT_UP] = KEY_UP;
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

    float16 *transforms16 = (float16*)RL_CALLOC(treeCount, sizeof(float16));
    for (i32 i = 0; i < treeCount; i++) {
        transforms16[i] = MatrixToFloatV(transforms[i]);
    }
    RL_FREE(transforms);

    InstanceMeshRenderData imrd = {};
    imrd.instanceCount = treeCount;
    imrd.transforms = transforms16;
    imrd.mesh = mesh;
    imrd.material = material;
    return imrd;
}
InstanceMeshRenderData ForestCreateBlocks(Image image, ForestGenerationInfo info, Mesh mesh, Material material) {
    const i32 pixelStride = PixelformatGetStride(image.format);
    const i32 pixelCount = image.width * image.height;
    byte* imageData = (byte*)image.data;
    mat4 *transforms = (mat4*)RL_CALLOC(pixelCount, sizeof(mat4));
    i32 transformCount = 0;
    v2 imageSize = {(float)image.width, (float)image.height};
    for (i32 i = 0; i < pixelCount; i++) {
        float x = (float)i;
        x = fmodf(x, imageSize.x);
        float y = ((float)i / imageSize.x);
        if (x + 1.f == imageSize.x || y + 1.f == imageSize.y) {
            continue;
        }
        
        // TODO: SIMD
        i32 dataIndex = i * pixelStride;
        byte* pixelData = &imageData[dataIndex];
        byte* pixelDataRight = &imageData[dataIndex + pixelStride];
        byte* pixelDataDown = &imageData[dataIndex + pixelStride * image.width];
        byte* pixelDataRightDown = &imageData[dataIndex + pixelStride + pixelStride * image.width];
        v4 value = {(float)pixelData[0], (float)pixelData[1], (float)pixelData[2], 1.f};
        v4 valueRight = {(float)pixelDataRight[0], (float)pixelDataRight[1], (float)pixelDataRight[2], 1.f};
        v4 valueDown = {(float)pixelDataDown[0], (float)pixelDataDown[1], (float)pixelDataDown[2], 1.f};
        v4 valueRightDown = {(float)pixelDataRightDown[0], (float)pixelDataRightDown[1], (float)pixelDataRightDown[2], 1.f};
        float lerpRight = ffractf(x);
        float lerpDown = ffractf(y);
        v4 bilinearValue = Vector4Lerp(Vector4Lerp(value, valueRight, lerpRight), Vector4Lerp(valueDown, valueRightDown, lerpRight), lerpDown);

        if (bilinearValue.x > 128.f) {
            v2 worldPosition = v2{x / imageSize.x, y / imageSize.y} * info.worldSize + info.worldPosition;
            transforms[transformCount] = MatrixTranslate(worldPosition.x, 0.f, worldPosition.y);
            transformCount++;
        }
    }

    float16 *transformsV = (float16*)RL_CALLOC(transformCount, sizeof(float16));
    for (i32 i = 0; i < transformCount; i++) {
        transformsV[i] = MatrixToFloatV(transforms[i]);
    }
    RL_FREE(transforms);

    InstanceMeshRenderData imrd = {};
    imrd.instanceCount = transformCount;
    imrd.material = material;
    imrd.mesh = mesh;
    imrd.transforms = transformsV;
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
    cab->turnAngleSpeed = 20.f;
    cab->turnNeutralReturn = 30.f;
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
        cab->_speed = ApproachZero(cab->_speed, cab->neutralDeceleration);
    }
    cab->_speed = fclampf(cab->_speed, -1.f, 1.f);

    float stepSpeed = 0.f;
    if (cab->_speed > 0.f) {
        stepSpeed = QuadraticBezierLerp(cab->accelerationCurve, cab->_speed) * cab->maxVelocity;
    } else if (cab->_speed < 0.f) {
        stepSpeed = QuadraticBezierLerp(cab->reverseAccelerationCurve, fabsf(cab->_speed)) * cab->maxVelocity;
    }

    float inputTurn = ((float)global::input.down[INPUT_RIGHT] - (float)global::input.down[INPUT_LEFT]) * -1.f;
    if (inputTurn != 0.f) {
        float turnAmountStep = inputTurn * FRAME_TIME * cab->turnAngleSpeed;
        cab->_turnAngle = fclampf(cab->_turnAngle + turnAmountStep, -cab->turnAngleMax, cab->turnAngleMax);
    } else {
        cab->_turnAngle = ApproachZero(cab->_turnAngle, cab->turnNeutralReturn * FRAME_TIME);
    }
    cab->yawRotationStep = cab->_turnAngle * DEG2RAD * stepSpeed;
    mat4 yawRotationMatrix = MatrixRotateYaw(cab->yawRotationStep);
    cab->_transform *= yawRotationMatrix;

    // TODO: Inefficient matrix multiplication
    cab->direction = cab->direction * yawRotationMatrix;
    cab->velocity = cab->direction * stepSpeed;
    cab->position += cab->velocity;
    cab->position.y = HeightmapSampleHeight(
        *(Heightmap*)global::groups["terrainHeightmap"],
        cab->position.x,
        cab->position.z);
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
    v2 eulerNext = euler + mouseRotationStep + rotationAdd;
    //v2 eulerNext = v2clampv2(euler + mouseRotationStep + rotationAdd, v2{0.1f, PI / -2.f + 0.3f}, v2{PI - 0.1f, PI / 2.f - 0.3f});
    v2 eulerDiff = eulerNext - euler;
    mat4 rot = MatrixRotateYaw(eulerDiff.x) * MatrixRotatePitch(eulerDiff.y);
    v3 rotatedTarget = target * rot;
    
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

void SkyboxDraw(void* _skybox) {
    Skybox* skybox = (Skybox*)_skybox;
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
        DrawModel(skybox->model, {0.f}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
GameObject SkyboxPack(Skybox* skybox) {
    GameObject go = {};
    go.data = (void*)skybox;
    go.Draw3d = SkyboxDraw;
    go.DrawUi = nullptr;
    go.Update = nullptr;
    return go;
}

GameObject GameObjectCreate(void* data, void(*update)(void*), void(*draw3d)(void*), void(*drawUi)(void*), void(*free)(void*)) {
    GameObject go = {};
    go.data = data;
    go.Update = update;
    go.Draw3d = draw3d;
    go.DrawUi = drawUi;
    go.Free = free;
    return go;
}

void HeightmapInit(Heightmap* hm, HeightmapGenerationInfo info, Material material) {
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
    hm->heightData = heightData;
    hm->heightmap = *image;
    hm->heightDataWidth = heightDataWidth;
    hm->heightDataHeight = heightDataHeight;
    hm->size = info.size;
    hm->texture = *info.terrainMapTexture;
    hm->model = LoadModelFromMesh(hmMesh);
    hm->model.materials[0] = material;
    hm->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *info.terrainMapTexture;
    hm->model.materials[0].maps[MATERIAL_MAP_SPECULAR].texture = *info.terrainTexture;
    hm->position = info.position;
    hm->debugModel = {};
}
float HeightmapSampleHeight(Heightmap heightmap, float x, float z) {
    v2 rectBegin = {heightmap.position.x, heightmap.position.z};
    v2 rectEnd = rectBegin + v2{heightmap.size.x, heightmap.size.z};
    if (PointInRectangle(rectBegin, rectEnd, {x, z})) {
        v2 abspos = v2{x, z} - rectBegin;
        v2 normpos = abspos / v2{heightmap.size.x, heightmap.size.z};
        v2 datapos = normpos * v2{(float)heightmap.heightDataWidth, (float)heightmap.heightDataHeight};
        v2 dataposFract = v2fractv2(datapos);
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
void HeightmapFree(void* heightmap) {
    free(((Heightmap*)heightmap)->heightData);
}
void HeightmapDraw(void* _hm) {
    Heightmap* hm = (Heightmap*)_hm;
    DrawModel(hm->model, hm->position, 1.f, WHITE);
}
GameObject HeightmapPack(Heightmap* hm) {
    return GameObjectCreate(hm, nullptr, HeightmapDraw, nullptr, HeightmapFree);
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

        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
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
        if (mesh.indices != NULL) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount*3, 0, instances);
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