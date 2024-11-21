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

#define FRAME_TIME 1.f / 60.f
#define FRAMERATE 60
#define TAU PI * 2.f

typedef struct {
    Vector2 p1;
    Vector2 p2;
    Vector2 p3;
} QuadraticBezier;
float QuadraticBezierLerp(QuadraticBezier qb, float val) {
    return Vector2Lerp(Vector2Lerp(qb.p1, qb.p2, val), Vector2Lerp(qb.p2, qb.p3, val), val).y;
}

typedef struct {
    Vector3 position;
    float life;
} Particle;

#define MAX_PARTICLES 192
typedef struct {
    Texture2D texture;
    float rate;
    Vector3 speed;
    float life;
    QuadraticBezier alphaCurve;
    Particle particles[MAX_PARTICLES];
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
    Vector3 position;
    Vector3 size;
    u32 heightDataResolution;
    u32 heightDataWidth;
    u32 heightDataHeight;
    float *heightData;
    Model debugModel;
    u32 width;
} Heightmap;
Heightmap HeightmapCreate(const char* heightmapPath, const char* texturePath, Vector3 position, Vector3 size, u32 resdiv);
float HeightmapSampleHeight(Heightmap heightmap, float x, float z);
void HeightmapDestroy(Heightmap heightmap);

typedef struct {
    Vector3 position;
} Terrainmap;

typedef struct {
    Vector3 offset;
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
    Vector3 position;
    Vector3 rotation;
    Vector3 frontLeftTire;
    Vector3 frontRightTire;
    Vector3 backLeftTire;
    Vector3 backRightTire;
    float tireHorizontalSeparation;
    Vector3 driversSeat;
    BoundingBox bbox;
} Cab;
void CabUpdate(Cab *cab, float *rotationStepOut, Heightmap *heightmap);

void CameraUpdateDebug(Camera *camera, float speed);
void CameraUpdate(Camera *camera, Vector3 position, Vector2 rotationAdd);
Vector3 GetCameraForwardNorm(Camera *camera);
Vector3 GetCameraUpNorm(Camera *camera);
Vector3 GetCameraRightNorm(Camera *camera);

int PointInRectangle(Vector2 begin, Vector2 end, Vector2 pt) {
    return pt.x >= begin.x && pt.x < end.x && pt.y >= begin.y && pt.y < end.y;
}
const char* TextFormatVector3(Vector3 v) {
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
Vector2 Vector2Fract(Vector2 v) {
    return (Vector2){ffractf(v.x), ffractf(v.y)};
}
float fclampf(float val, float min, float max) {
    return fminf(fmaxf(val, min), max);
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
    Vector2 xbounds = {0.f}, ybounds = {0.f}, zbounds = {0.f};
    for (int i = 0; i < mesh.vertexCount; i+=3) {
        float* cv = mesh.vertices + i;
        xbounds.x = fminf(*cv, xbounds.x);
        xbounds.y = fmaxf(*cv, xbounds.y);
        ybounds.x = fminf(*(cv+1), ybounds.x);
        ybounds.y = fmaxf(*(cv+1), ybounds.y);
        zbounds.x = fminf(*(cv+2), zbounds.x);
        zbounds.y = fmaxf(*(cv+2), zbounds.y);
    }
    return (BoundingBox){
        (Vector3){xbounds.x, ybounds.x, zbounds.x},
        (Vector3){xbounds.y, ybounds.y, zbounds.y}};
}
BoundingBox GenBoundingBoxMeshes(Mesh* meshes, int meshCount) {
    Vector2 xbounds = {0.f}, ybounds = {0.f}, zbounds = {0.f};
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
    return (BoundingBox){
        (Vector3){xbounds.x, ybounds.x, zbounds.x},
        (Vector3){xbounds.y, ybounds.y, zbounds.y}};
}

Shader passthroughShader = {0};
Shader passthroughInstShader = {0};

void InitGlobals() {
    passthroughShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/passthrough.fs");
    passthroughShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(passthroughShader, "mvp");
    passthroughShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(passthroughShader, "viewPos");

    passthroughInstShader = LoadShader("resources/shaders/passthroughInstancing.vs", "resources/shaders/passthrough.fs");
    passthroughInstShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(passthroughInstShader, "mvp");
    passthroughInstShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(passthroughInstShader, "instanceTransform");
}

int main() {
    u32 freecam = 0;

    Vector2 screenSize = {1440, 800};
    SetTraceLogLevel(4);
    InitWindow(screenSize.x, screenSize.y, "Midnight Driver");
    SetTargetFPS(60);
    DisableCursor();
    InitGlobals();

    Cab cab = {0};
    cab.model = LoadModel("resources/models/cab.m3d");
    cab.speed = 0.f;
    cab.maxSpeed = 1.f;
    cab.maxReverseSpeed = 0.3f;
    cab.acceleration = 0.007f;
    cab.breakDeceleration = 0.02f;
    cab.reverseAcceleration = 0.0065f;
    cab.friction = 0.005f;
    cab.driversSeat = (Vector3){-0.1f, 0.7f, -0.25f};
    cab.offset = (Vector3){0.25f, 0.f, -0.06f};
    cab.frontLeftTire = (Vector3){0.8f, 0.f, -0.45f};
    cab.frontRightTire = (Vector3){0.8f, 0.f, 0.45f};
    cab.backLeftTire = (Vector3){-1.f, 0.f, -0.45f};
    cab.backRightTire = (Vector3){-1.f, 0.f, 0.45f};
    cab.turnRadius = 30.f;
    cab.turnSpeed = 1.f;
    cab.bbox = GenBoundingBoxMeshes(cab.model.meshes, cab.model.meshCount);

    Model coneModel = LoadModelFromMesh(GenMeshCube(2.f, 2.f, 2.f));
    coneModel.materials[0].shader = LoadShader("resources/shaders/light.vs", "resources/shaders/light.fs");
    coneModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("resources/textures/box.png");
    coneModel.transform = MatrixTranslate(0.f, 3.f, 0.f);

    Camera camera = {0};
    camera.fovy = 60.f;
    camera.target = (Vector3){1.f, 0.f, 0.f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = (Vector3){0.f, 1.f, 0.f};

    Camera debugCamera = {0};
    camera.fovy = 60.f;
    camera.up = (Vector3){0.f, 1.f, 0.f};

    Vector3 lightOrigin = (Vector3){2.f, 3.f, -1.f};
    Vector3 lightPosition = (Vector3){2.f, 3.f, -1.f};
    float lightFalloffDistance = 5.f;
    float lightMovementTimer = 0.f;

    Heightmap heightmap = HeightmapCreate(
        "resources/textures/heightmap.png",
        "resources/textures/heightmap_texture.png",
        (Vector3){4.f, 0.f, 0.f},
        (Vector3){16.f, 4.f, 16.f},
        3);

    Model skyboxModel = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    Shader skyboxShader = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "environmentMap"), (i32[1]){MATERIAL_MAP_CUBEMAP}, SHADER_UNIFORM_INT);
    SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "doGamma"), (int[1]){0}, SHADER_UNIFORM_INT);
    SetShaderValue(skyboxShader, GetShaderLocation(skyboxShader, "vflipped"), (int[1]){0}, SHADER_UNIFORM_INT);
    skyboxModel.materials[0].shader = skyboxShader;
    Image img = LoadImage("resources/textures/skybox.png");
    skyboxModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(img);

    Texture2D fogTexture = LoadTexture("resources/textures/fog128.png");
    ParticleSystem fogSystem = {0};
    fogSystem.rate = 20.f;
    fogSystem.texture = fogTexture;
    fogSystem.life = 2.f;
    fogSystem.alphaCurve = (QuadraticBezier){
        (Vector2){0.f, 0.f}, 
        (Vector2){0.5f, 2.f},
        (Vector2){1.f, 0.f}};
    fogSystem.speed = (Vector3){0.f, 1.f, 0.f};
    for (u32 i = 0; i < MAX_PARTICLES; i++) {
        fogSystem.particles[i].position = (Vector3) {
            (float)GetRandomValue(-400, 400) / 100.f,
            0.f,
            (float)GetRandomValue(-400, 400) / 100.f};
    }

    Vector3 fogPositions[10] = {0};
    for (u32 i = 0; i < 10; i++) {
        fogPositions[i] = Vector3Add((Vector3){4.f, 0.f, 4.f}, Vector3Scale(Vector3One(), 0.1f * i));
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        float rotationStep;
        CabUpdate(&cab, &rotationStep, &heightmap);
        Vector2 hDriverPosition = Vector2Rotate((Vector2){cab.driversSeat.x, cab.driversSeat.z}, (360.f - cab.rotation.x) * DEG2RAD);
        Vector3 driverPosition = Vector3Add(cab.position, (Vector3){hDriverPosition.x, cab.driversSeat.y, hDriverPosition.y});

        if (IsKeyPressed(KEY_BACKSPACE)) {
            freecam = freecam ? 0 : 1;
            if (freecam) {
                debugCamera.position = camera.position;
                debugCamera.target = camera.target;
                debugCamera.projection = camera.projection;
                debugCamera.fovy = camera.fovy;
                debugCamera.up = (Vector3){0.f, 1.f, 0.f};
            }
        }

        Camera *usingCamera;
        if (freecam) {
            CameraUpdateDebug(&debugCamera, 0.1f);
            usingCamera = &debugCamera;
        } else {
            CameraUpdate(&camera, driverPosition, (Vector2){-rotationStep * DEG2RAD, 0.f});
            usingCamera = &camera;
        }

        lightMovementTimer += 0.016f * 2.f;
        lightPosition = (Vector3){
            lightOrigin.x + sinf(lightMovementTimer) * 2.f,
            lightOrigin.y,
            lightOrigin.z + cosf(lightMovementTimer) * 2.f};

        BeginMode3D(*usingCamera);

            rlDisableBackfaceCulling();
            rlDisableDepthMask();
                DrawModel(skyboxModel, (Vector3){0.f}, 1.0f, WHITE);
            rlEnableBackfaceCulling();
            rlEnableDepthMask();

            DrawGrid(20, 1.f);
            DrawModel(heightmap.model, heightmap.position, 1.f, WHITE);
            SetShaderValueMatrix(
                coneModel.materials[0].shader,
                GetShaderLocation(coneModel.materials[0].shader, "model_mat"),
                coneModel.transform);
            SetShaderValue(
                coneModel.materials[0].shader,
                GetShaderLocation(coneModel.materials[0].shader, "lightPosition"),
                (float[3]){lightPosition.x, lightPosition.y, lightPosition.z},
                SHADER_UNIFORM_VEC3);
            SetShaderValue(
                coneModel.materials[0].shader,
                GetShaderLocation(coneModel.materials[0].shader, "lightFalloff"),
                &lightFalloffDistance,
                SHADER_UNIFORM_FLOAT);
            DrawModel(coneModel, (Vector3){0.f}, 1.f, WHITE);
            ParticleSystemStep(&fogSystem, *usingCamera);

            //for (u32 i = 0; i < 10; i++) {
            //    DrawBillboard(*usingCamera, fogTexture, fogPositions[i], 1.f, RED);
            //}
            //Quaternion cabQuat = QuaternionFromEuler(0.f, cab.rotation.x * DEG2RAD, cab.rotation.y * DEG2RAD);
            //Matrix cabMatrix = QuaternionToMatrix(cabQuat);
            //cabMatrix = MatrixMultiply(cabMatrix, MatrixTranslate(cab.position.x, cab.position.y, cab.position.z));
            //
            //for (int i = 0; i < cab.model.meshCount; i++) {
            //    Color prevColor = cab.model.materials[cab.model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;
            //    cab.model.materials[cab.model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
            //    DrawMesh(cab.model.meshes[i], cab.model.materials[cab.model.meshMaterial[i]], cabMatrix);
            //    cab.model.materials[cab.model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = prevColor;
            //}
            //DrawBoundingBox(cab.bbox, RED);
            //DrawSphere(lightPosition, 0.5f, RED);
        EndMode3D();
        DrawFPS(4, 4);
        if (freecam) {
            DrawTextShadow("Debug cam", 4, 20, 16, RED, BLACK);
        }
        DrawCrosshair(screenSize.x / 2, screenSize.y / 2, WHITE);
        EndDrawing();
    }
    return(0);
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
    Vector2 cameraPosition = (Vector2){camera.position.x, camera.position.z};
    Vector2 cameraTarget = (Vector2){camera.target.x, camera.target.z};
    for (u32 i = 0; i < psys->_number; i++) {
        Particle *p = &psys->particles[(psys->_index + i) % MAX_PARTICLES];

        p->position = Vector3Add(p->position, Vector3Scale(psys->speed, FRAME_TIME));
        p->life -= FRAME_TIME;

        Vector2 ppos = (Vector2){p->position.x, p->position.z};
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

    for (u32 i = 0; i < psys->_number; i++) {
        Particle *p = sortedParts[i];
        float blendProgress = fclampf(1.f - p->life / psys->life, 0.f, 1.f);
        u8 alpha = (u8)(QuadraticBezierLerp(psys->alphaCurve, blendProgress) * 255.f);
        Color blend = (Color){255, 255, 255, alpha};
        DrawBillboard(camera, psys->texture, p->position, 1.f, blend);
    }

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
    Matrix cabRotationMatrix = {
        cabRotCos, 0.f, -cabRotSin, 0.f,
        0.f, 1.f,  0.f, 0.f,
        cabRotSin, 0.f, cabRotCos, 0.f,
        0.f, 0.f, 0.f, 1.f};
    Vector3 cabDirection = Vector3Transform((Vector3){1.f, 0.f, 0.f}, cabRotationMatrix);
    Vector3 cabMoveStep = Vector3Scale(cabDirection, cab->speed);
    cab->position = Vector3Add(cab->position, cabMoveStep);

    Vector3 flTire = Vector3Add(cab->position, Vector3Transform(cab->frontLeftTire, cabRotationMatrix));
    Vector3 frTire = Vector3Add(cab->position, Vector3Transform(cab->frontRightTire, cabRotationMatrix));
    Vector3 frontPoint = Vector3Lerp(flTire, frTire, 0.5f);
    frontPoint.y = HeightmapSampleHeight(*heightmap, frontPoint.x, frontPoint.z);

    Vector3 blTire = Vector3Add(cab->position, Vector3Transform(cab->backLeftTire, cabRotationMatrix));
    Vector3 brTire = Vector3Add(cab->position, Vector3Transform(cab->backRightTire, cabRotationMatrix));
    Vector3 backPoint = Vector3Lerp(blTire, brTire, 0.5f);
    backPoint.y = HeightmapSampleHeight(*heightmap, backPoint.x, backPoint.z);

    cab->position.y = Lerp(frontPoint.y, backPoint.y, 0.5f);
    if (backPoint.y != 0.f || frontPoint.y != 0.f) {
        float tireHorSep = cab->frontLeftTire.x - cab->backLeftTire.x;
        // Can probably be changed to just using a value
        Vector2 pitchNormal = (Vector2Normalize(Vector2Subtract(
                (Vector2){cab->frontLeftTire.x, frontPoint.y},
                (Vector2){cab->backLeftTire.x, backPoint.y})));
        cab->rotation.y = atan2(pitchNormal.y, pitchNormal.x) * RAD2DEG;
    } else {
        cab->rotation.y = 0.f;
    }
    *rotationStepOut = rotationStep;
}

void CameraUpdateDebug(Camera *camera, float speed) {
    Vector3 targetDirection = Vector3Subtract(camera->target, camera->position);
    float sensitivity = 0.002f;
    Vector2 mouseDelta = GetMouseDelta();
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraUpNorm(camera), -mouseDelta.x * sensitivity);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraRightNorm(camera), -mouseDelta.y * sensitivity);
    Vector3 input = {
        (float)IsKeyDown(KEY_W) - (float)IsKeyDown(KEY_S),
        (float)IsKeyDown(KEY_Q) - (float)IsKeyDown(KEY_E),
        (float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A)};
    Vector2 hInput = (Vector2){input.x, input.z};
    if (Vector2LengthSqr(hInput) > 0.f) {
        Vector2 hInputNorm = Vector2Normalize(hInput);
        Vector2 hDirection = Vector2Rotate((Vector2){targetDirection.x, targetDirection.z}, atan2f(hInputNorm.y, hInputNorm.x));
        camera->position.x += hDirection.x * speed;
        camera->position.z += hDirection.y * speed;
    }
    camera->position.y += input.y * speed;
    camera->target = Vector3Add(camera->position, targetDirection);
}
void CameraUpdate(Camera *camera, Vector3 position, Vector2 rotationAdd) {
    Vector3 targetDirection = Vector3Subtract(camera->target, camera->position);
    float sensitivity = 0.002f;
    Vector2 rotationDelta = Vector2Add(Vector2Scale(GetMouseDelta(), sensitivity), rotationAdd);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraUpNorm(camera), -rotationDelta.x);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraRightNorm(camera), -rotationDelta.y);
    camera->position = position;
    camera->target = Vector3Add(camera->position, targetDirection);
}
Vector3 GetCameraForwardNorm(Camera *camera) {
    return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
}
Vector3 GetCameraUpNorm(Camera *camera) {
    return Vector3Normalize(camera->up);
}
Vector3 GetCameraRightNorm(Camera *camera) {
    Vector3 forward = GetCameraForwardNorm(camera);
    Vector3 up = GetCameraUpNorm(camera);
    return Vector3Normalize(Vector3CrossProduct(forward, up));
}

Heightmap HeightmapCreate(const char* heightmapPath, const char* texturePath, Vector3 position, Vector3 size, u32 resdiv) {
    Heightmap heightmap = {0};
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
            return (Heightmap){0};
    }

    heightmap.heightDataWidth = heightmap.heightmap.width >> resdiv;
    heightmap.heightDataHeight = heightmap.heightmap.height >> resdiv;

    float *heightData = malloc(heightmap.heightDataWidth * heightmap.heightDataHeight * sizeof(float));
    heightmap.heightData = heightData;
    byte* data = heightmap.heightmap.data;
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
    heightmap.debugModel = (Model){0};
    return heightmap;
}
float HeightmapSampleHeight(Heightmap heightmap, float x, float z) {
    Vector2 rectBegin = (Vector2){heightmap.position.x, heightmap.position.z};
    Vector2 rectEnd = Vector2Add(rectBegin, (Vector2){heightmap.size.x, heightmap.size.z});
    if (PointInRectangle(rectBegin, rectEnd, (Vector2){x, z})) {
        Vector2 abspos = Vector2Subtract((Vector2){x, z}, rectBegin);
        Vector2 normpos = Vector2Divide(abspos, (Vector2){heightmap.size.x, heightmap.size.z});
        Vector2 datapos = Vector2Multiply(normpos, (Vector2){heightmap.heightDataWidth, heightmap.heightDataHeight});
        Vector2 dataposFract = Vector2Fract(datapos);
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
    List list = {0};
    ListInit(&list, size);
    return list;
}
void ListInit(List* list, u32 size) {
    if (size == 0) {
        list->data = NULL;
    } else {
        list->data = malloc(size * sizeof(void*));
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
        list->data = realloc(list->data, sizeof(void*) * size);
    } else {
        list->data = malloc(sizeof(void*) * size);
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
        list->data = realloc(list->data, sizeof(void*) * capacity);
    } else {
        list->data = malloc(sizeof(void*) * capacity);
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
