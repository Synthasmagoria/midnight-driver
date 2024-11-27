#include "raylib.h"
#include "raymath.h"
#include "stdlib.h"
#include "stdint.h"
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

typedef struct {
    Matrix *transforms;
    u32 instanceCount;
    Model _model;
    Mesh mesh;
    Material material;
} InstanceMeshRenderData;
void InstanceMeshRenderDataDestroy(InstanceMeshRenderData imrd);
InstanceMeshRenderData ForestCreate(Image image, float density, v2 worldSize, v2 worldPos, float treeChance, float randTreeOffset);

typedef struct {v2 p1; v2 p2; v2 p3;} QuadraticBezier;
// TODO: SIMD-fy
float QuadraticBezierLerp(QuadraticBezier qb, float val) {
    return Vector2Lerp(Vector2Lerp(qb.p1, qb.p2, val), Vector2Lerp(qb.p2, qb.p3, val), val).y;
}

typedef struct {
    v3 position;
    float life;
} Particle;

#define MAX_PARTICLES 192
typedef struct {
    Texture2D texture;
    float rate;
    v3 speed;
    float life;
    QuadraticBezier alphaCurve;
    Particle particles[MAX_PARTICLES];
    int blendMode;
    float _spawnTimer;
    u32 _number;
    u32 _index;
} ParticleSystem;
void ParticleSystemStep(ParticleSystem *psys, Camera3D camera);
void ParticleSystemDraw(ParticleSystem *psys);

typedef struct {
    void** data;
    u32 size;
    u32 capacity;
} List;
List ListCreate(u32 size);
void ListInit(List *list, u32 size);
void ListDestroy(List *list);
void ListResize(List *list, u32 size);
void ListChangeCapacity(List *list, u32 capacity);
void* ListGet(List *list, u32 ind);
void ListSet(List *list, u32 ind, void* val);
void ListPushBack(List *list, void* val);

typedef struct {
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
} Heightmap;
Heightmap HeightmapCreate(const char* heightmapPath, const char* texturePath, v3 position, v3 size, u32 resdiv);
float HeightmapSampleHeight(Heightmap heightmap, float x, float z);
void HeightmapDestroy(Heightmap heightmap);

typedef struct {
    v3 position;
} Terrainmap;

typedef struct {
    v3 offset;
    Model model;
    float speed;
    float maxSpeed;
    float maxReverseSpeed;
    float acceleration;
    float friction;
    float turn;
    float turnRadius;
    float turnSpeed;
    float breakDeceleration;
    float reverseAcceleration;
    v3 position;
    v3 rotation;
    v3 frontLeftTire;
    v3 frontRightTire;
    v3 backLeftTire;
    v3 backRightTire;
    float tireHorizontalSeparation;
    v3 driversSeat;
    BoundingBox bbox;
} Cab;
void CabUpdate(Cab *cab, float *rotationStepOut, Heightmap *heightmap);

void CameraUpdateDebug(Camera *camera, float speed);
void CameraUpdate(Camera *camera, v3 position, v2 rotationAdd);
v3 GetCameraForwardNorm(Camera *camera);
v3 GetCameraUpNorm(Camera *camera);
v3 GetCameraRightNorm(Camera *camera);

int PointInRectangle(v2 begin, v2 end, v2 pt) {
    return pt.x >= begin.x && pt.x < end.x && pt.y >= begin.y && pt.y < end.y;
}
const char* TextFormatVector3(v3 v) {
    return TextFormat("[%f], [%f], [%f]", v.x, v.y, v.z);
}
void DrawTextShadow(const char* text, int x, int y, int fontSize, Color col, Color shadowCol) {
    DrawText(text, x + 1, y + 1, fontSize, shadowCol);
    DrawText(text, x, y, fontSize, col);
}
void DrawCrosshair(int x, int y, Color col) {
    DrawLine(x - 4, y, x + 4, y, col);
    DrawLine(x, y - 4, x, y + 4, col);
}
float ffractf(float val) {
    return val - floorf(val);
}
v2 Vector2Fract(v2 v) {
    return {ffractf(v.x), ffractf(v.y)};
}
float fclampf(float val, float min, float max) {
    return fminf(fmaxf(val, min), max);
}
// NOTE: only supports up to 2 decimals
float GetRandomValueF(float min, float max) {
    return ((float)GetRandomValue((i32)(min * 100.f), (i32)(max * 100.f))) / 100.f;
}
bool GetRandomChanceF(float percentage) {
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

Shader passthroughShader = {};
Shader passthroughInstShader = {};

void InitGlobals() {
    passthroughShader = LOAD_SHADER("passthrough.vs", "passthrough.fs");
    passthroughShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(passthroughShader, "mvp");
    passthroughShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(passthroughShader, "viewPos");
}


int main() {
    u32 freecam = 0;

    v2 screenSize = {1440, 800};
    SetTraceLogLevel(4);
    InitWindow(screenSize.x, screenSize.y, "Midnight Driver");
    SetTargetFPS(60);
    DisableCursor();
    InitGlobals();

    Cab cab = {};
    cab.model = LOAD_MODEL("cab.m3d");
    cab.speed = 0.f;
    cab.maxSpeed = 1.f;
    cab.maxReverseSpeed = 0.3f;
    cab.acceleration = 0.007f;
    cab.breakDeceleration = 0.02f;
    cab.reverseAcceleration = 0.0065f;
    cab.friction = 0.005f;
    cab.driversSeat = {-0.1f, 0.7f, -0.25f};
    cab.offset = {0.25f, 0.f, -0.06f};
    cab.frontLeftTire = {0.8f, 0.f, -0.45f};
    cab.frontRightTire = {0.8f, 0.f, 0.45f};
    cab.backLeftTire = {-1.f, 0.f, -0.45f};
    cab.backRightTire = {-1.f, 0.f, 0.45f};
    cab.turnRadius = 30.f;
    cab.turnSpeed = 1.f;
    cab.bbox = GenBoundingBoxMeshes(cab.model.meshes, cab.model.meshCount);

    Camera camera = {};
    camera.fovy = 60.f;
    camera.target = {1.f, 0.f, 0.f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = {0.f, 1.f, 0.f};

    Camera debugCamera = {};
    camera.fovy = 60.f;
    camera.up = {0.f, 1.f, 0.f};
    float cameraSpeed = 0.1f;

    Model skyboxModel = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    Shader skyboxShader = LOAD_SHADER("skybox.vs", "skybox.fs");
    {
        i32 envmapValue = MATERIAL_MAP_CUBEMAP;
        SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "environmentMap"), &envmapValue, SHADER_UNIFORM_INT);
        i32 gammaValue = 0;
        SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "doGamma"), &gammaValue, SHADER_UNIFORM_INT);
        i32 vflippedValue = 0;
        SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "vflipped"), &vflippedValue, SHADER_UNIFORM_INT);
    }
    skyboxModel.materials[0].shader = skyboxShader;
    Image img = LOAD_IMAGE("skybox.png");
    skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(img);

    Image terrainImage = LOAD_IMAGE("terrainmap.png");
    v2 forestSize = {50.f, 50.f};
    InstanceMeshRenderData forestImrd = ForestCreate(terrainImage, 0.6f, forestSize, forestSize / -2.f, 90.f, 0.25f);
    UnloadImage(terrainImage);
    /*
        TODO: Add a random color tint to the trees
        TODO: Add a random texture to the trees
        TODO: Add 3 different tree models
        TODO: Add basic flora
    */

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        v2 mouseScroll = GetMouseWheelMoveV();
        cameraSpeed = fclampf(cameraSpeed + mouseScroll.y * 0.01f, 0.05f, 1.f);

        float rotationStep;
        CabUpdate(&cab, &rotationStep, NULL);
        v2 hDriverPosition = Vector2Rotate({cab.driversSeat.x, cab.driversSeat.z}, (360.f - cab.rotation.x) * DEG2RAD);
        v3 driverPosition = v3{hDriverPosition.x, cab.driversSeat.y, hDriverPosition.y} + cab.position;

        if (IsKeyPressed(KEY_BACKSPACE)) {
            freecam = freecam ? 0 : 1;
            if (freecam) {
                debugCamera.position = camera.position;
                debugCamera.target = camera.target;
                debugCamera.projection = camera.projection;
                debugCamera.fovy = camera.fovy;
                debugCamera.up = {0.f, 1.f, 0.f};
            }
        }

        Camera *usingCamera;
        if (freecam) {
            CameraUpdateDebug(&debugCamera, cameraSpeed);
            usingCamera = &debugCamera;
        } else {
            CameraUpdate(&camera, driverPosition, {-rotationStep * DEG2RAD, 0.f});
            usingCamera = &camera;
        }

        BeginMode3D(*usingCamera);
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
                DrawModel(skyboxModel, {0.f}, 1.0f, WHITE);
            rlEnableBackfaceCulling();
            rlEnableDepthMask();
            DrawMeshInstanced(forestImrd.mesh, forestImrd.material, forestImrd.transforms, forestImrd.instanceCount);
            DrawGrid(20, 1.f);
        EndMode3D();
        DrawFPS(4, 4);
        if (freecam) {
            DrawTextShadow("Debug cam", 4, 20, 16, RED, BLACK);
        }
        DrawCrosshair(screenSize.x / 2, screenSize.y / 2, WHITE);
        EndDrawing();
    }

    InstanceMeshRenderDataDestroy(forestImrd);

    return(0);
}

void InstanceMeshRenderDataDestroy(InstanceMeshRenderData imrd) {
    UnloadModel(imrd._model);
    UnloadMaterial(imrd.material);
    // TODO: Double check if shader is unloaded as part of material
    RL_FREE(imrd.transforms);
}

InstanceMeshRenderData ForestCreate(Image image, float density, v2 worldSize, v2 worldPos, float treeChance, float randTreeOffset) {
    const i32 stride = PixelformatGetStride(image.format);
    if (stride < 3) {
        TraceLog(LOG_WARNING, "ForestCreate: Passed Image isn't valid for creating a forst");
        return {}; // TODO: Implement renderable default data for when InstanceMeshRenderData creation fails
    }
    const v2 imageSize = {(float)image.width, (float)image.height};
    const u32 treesMax = (u32)ceilf((worldSize.x * density) * (worldSize.y * density));
    u32 treeCount = 0;
    Matrix *transforms = (Matrix*)RL_CALLOC(treesMax, sizeof(Matrix));
    v2 pixelIncrF = imageSize / worldSize / density;
    const u32 incrx = pixelIncrF.x < 1.f ? 1 : (u32)floorf(pixelIncrF.x);
    const u32 incry = pixelIncrF.y < 1.f ? 1 : (u32)floorf(pixelIncrF.y);
    const byte* imageData = (byte*)image.data;
    for (u32 x = 0; x < image.width; x += incrx) {
        for (u32 y = 0; y < image.height; y += incry) {
            u32 i = (x + y * image.width) * stride;
            Color color = {imageData[i], imageData[i+1], imageData[i+2], 0.f};
            if (color.g == 255 && GetRandomChanceF(treeChance)) {
                v2 treePos = v2{(float)x, (float)y} / imageSize * worldSize + worldPos;
                transforms[treeCount] = MatrixTranslate(
                    treePos.x + GetRandomValueF(-randTreeOffset, randTreeOffset),
                    0.f, // TODO: Sample a heightmap to get a proper vertical tree position
                    treePos.y + GetRandomValueF(-randTreeOffset, randTreeOffset));
                treeCount++;
            }
        }
    }

    // TODO: Put into another function or something
    Model model = LOAD_MODEL("tree.glb");
    Mesh mesh = model.meshes[0];
    Shader shader = LOAD_SHADER("instance.vs", "instance.fs");
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instanceTransform");
    Material material = LoadMaterialDefault();
    material.shader = shader;

    InstanceMeshRenderData imrd = {};
    imrd.instanceCount = treeCount;
    imrd.transforms = transforms;
    imrd._model = model;
    imrd.mesh = mesh;
    imrd.material = material;
    return imrd;
}

void ParticleSystemStep(ParticleSystem *psys, Camera3D camera) {
    psys->_spawnTimer += psys->rate * FRAME_TIME;
    while (psys->_spawnTimer >= 1.f) {
        psys->_spawnTimer -= 1.f;
        if (psys->_number + 1 < MAX_PARTICLES) {
            psys->_number++;
            Particle *p = &psys->particles[(psys->_index + psys->_number) % MAX_PARTICLES];
            p->position.y = 0.f;
            p->life = psys->life;
        }
    }

    Particle *parts[MAX_PARTICLES] = {NULL};
    float dists[MAX_PARTICLES] = {-1.f};
    v2 cameraPosition = {camera.position.x, camera.position.z};
    v2 cameraTarget = {camera.target.x, camera.target.z};
    for (u32 i = 0; i < psys->_number; i++) {
        Particle *p = &psys->particles[(psys->_index + i) % MAX_PARTICLES];

        p->position = p->position + psys->speed * FRAME_TIME;
        p->life -= FRAME_TIME;

        v2 ppos = {p->position.x, p->position.z};
        dists[i] = Vector2Distance(cameraPosition, ppos);
        parts[i] = p;
    }

    // TODO: Sort from camera tangent line instead of position to fix clipping
    // TODO: Make max particles settable
    Particle *sortedParts[MAX_PARTICLES] = {NULL};
    float sortedDists[MAX_PARTICLES] = {1.f};
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
        Particle *p = sortedParts[i];
        float blendProgress = fclampf(1.f - p->life / psys->life, 0.f, 1.f);
        u8 alpha = (u8)(QuadraticBezierLerp(psys->alphaCurve, blendProgress) * 255.f);
        Color blend = {255, 255, 255, alpha};
        DrawBillboard(camera, psys->texture, p->position, 1.f, blend);
    }
    rlSetBlendMode(BLEND_ALPHA);

    while (psys->_number > 0 && psys->particles[psys->_index].life <= 0.f) {
        psys->_index = (psys->_index + 1) % MAX_PARTICLES;
        psys->_number--;
    }
}

void CabUpdate(Cab *cab, float *rotationStepOut, Heightmap* heightmap) {
    cab->speed = fmaxf(cab->speed - cab->friction, 0.f);
    if (IsKeyDown(KEY_SPACE)) {
        cab->speed = fminf(cab->speed + cab->acceleration, cab->maxSpeed);
    } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
        if (cab->speed >= 0.f) {
            cab->speed -= cab->breakDeceleration;
        } else {
            cab->speed = fmaxf(cab->speed - cab->reverseAcceleration, cab->maxReverseSpeed);
        }
    }

    float turnStep = ((float)IsKeyDown(KEY_LEFT) - (float)IsKeyDown(KEY_RIGHT)) * cab->turnSpeed;
    cab->turn = fclampf(cab->turn + turnStep, -cab->turnRadius, cab->turnRadius);
    float rotationStep = cab->turn * cab->speed;
    cab->rotation.x += rotationStep;
    if (cab->rotation.x < 0.f) {
        cab->rotation.x += 360.f;
    } else if (cab->rotation.x >= 360.f) {
        cab->rotation.x -= 360.f;
    }

    float cabRotCos = cosf((360.f - cab->rotation.x) * DEG2RAD);
    float cabRotSin = sinf((360.f - cab->rotation.x) * DEG2RAD);
    mat4 cabRotationMatrix = {
        cabRotCos, 0.f, -cabRotSin, 0.f,
        0.f, 1.f,  0.f, 0.f,
        cabRotSin, 0.f, cabRotCos, 0.f,
        0.f, 0.f, 0.f, 1.f};
    v3 cabDirection = Vector3Transform({1.f, 0.f, 0.f}, cabRotationMatrix);
    v3 cabMoveStep = Vector3Scale(cabDirection, cab->speed);
    cab->position = cab->position + cabMoveStep;

    v3 flTire = cab->position + cab->frontLeftTire * cabRotationMatrix;
    v3 frTire = cab->position + cab->frontRightTire * cabRotationMatrix;
    v3 frontPoint = Vector3Lerp(flTire, frTire, 0.5f);
    frontPoint.y = heightmap ? HeightmapSampleHeight(*heightmap, frontPoint.x, frontPoint.z) : 0.f;

    v3 blTire = cab->position + cab->backLeftTire * cabRotationMatrix;
    v3 brTire = cab->position + cab->backRightTire * cabRotationMatrix;
    v3 backPoint = Vector3Lerp(blTire, brTire, 0.5f);
    backPoint.y = heightmap ? HeightmapSampleHeight(*heightmap, backPoint.x, backPoint.z) : 0.f;

    cab->position.y = Lerp(frontPoint.y, backPoint.y, 0.5f);
    if (backPoint.y != 0.f || frontPoint.y != 0.f) {
        float tireHorSep = cab->frontLeftTire.x - cab->backLeftTire.x;
        v2 pitchNormal = (Vector2Normalize(v2{cab->frontLeftTire.x, frontPoint.y} - v2{cab->backLeftTire.x, backPoint.y}));
        cab->rotation.y = atan2(pitchNormal.y, pitchNormal.x) * RAD2DEG;
    } else {
        cab->rotation.y = 0.f;
    }
    *rotationStepOut = rotationStep;
}

void CameraUpdateDebug(Camera *camera, float speed) {
    v3 targetDirection = camera->target - camera->position;
    float sensitivity = 0.002f;
    v2 mouseDelta = GetMouseDelta();
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraUpNorm(camera), -mouseDelta.x * sensitivity);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraRightNorm(camera), -mouseDelta.y * sensitivity);
    v3 input = {
        (float)IsKeyDown(KEY_W) - (float)IsKeyDown(KEY_S),
        (float)IsKeyDown(KEY_Q) - (float)IsKeyDown(KEY_E),
        (float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A)};
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
void CameraUpdate(Camera *camera, v3 position, v2 rotationAdd) {
    v3 targetDirection = camera->target - camera->position;
    float sensitivity = 0.002f;
    v2 rotationDelta = GetMouseDelta() * sensitivity - rotationAdd;
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraUpNorm(camera), -rotationDelta.x);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraRightNorm(camera), -rotationDelta.y);
    camera->position = position;
    camera->target = camera->position * targetDirection;
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

Heightmap HeightmapCreate(const char* heightmapPath, const char* texturePath, v3 position, v3 size, u32 resdiv) {
    Heightmap heightmap = {};
    heightmap.heightmap = LoadImage(heightmapPath);
    u32 stride = 0;
    u32 len = heightmap.heightmap.width * heightmap.heightmap.height;
    switch (heightmap.heightmap.format) {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            stride = 1;
            break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            stride = 2;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            stride = 3;
            break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            stride = 4;
            break;
        default:
            TraceLog(4, "Heightmap: Heightmap was not of a compatible format");
            UnloadImage(heightmap.heightmap);
            return {};
    }

    heightmap.heightDataWidth = heightmap.heightmap.width >> resdiv;
    heightmap.heightDataHeight = heightmap.heightmap.height >> resdiv;

    float *heightData = (float*)malloc(heightmap.heightDataWidth * heightmap.heightDataHeight * sizeof(float));
    heightmap.heightData = heightData;
    byte* data = (byte*)heightmap.heightmap.data;
    u32 dataW = heightmap.heightDataWidth;
    u32 imgw = heightmap.heightmap.width;

    for (u32 x = 0; x < heightmap.heightDataWidth; x++) {
        for (u32 y = 0; y < heightmap.heightDataHeight; y++) {
            byte value = data[((x << resdiv) + ((y << resdiv) * imgw)) * stride];
            heightData[x + y * dataW] = ((float)value / 255.f) * size.y;
        }
    }

    Texture2D hmTexture = LoadTextureFromImage(heightmap.heightmap);
    heightmap.size = size;
    Mesh hmMesh = GenMeshHeightmap(heightmap.heightmap, size);
    heightmap.texture = LoadTexture(texturePath);
    heightmap.model = LoadModelFromMesh(hmMesh);
    heightmap.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightmap.texture;
    heightmap.model.materials[0].shader = LoadShader("resources/shaders/heightmap.vs", "resources/shaders/heightmap.fs");
    heightmap.position = position;
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
