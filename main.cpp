#include <cstring>
#define NO_FONT_AWESOME

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "assert.h"

#include "imgui/imgui.h"
#include "imgui/rlImGui.h"

#include "typedefs.hpp"
#include "engine.hpp"

/*
    Game objects
*/
enum MD_GAME_OBJECTS_CUSTOM {
    OBJECT_CAB = _MD_GAME_ENGINE_OBJECTS_COUNT,
};

struct Cab;

struct Cab {
    Model model;
    v3 position;
    v3 rotation;
    v3 frontSeatPosition;
    v3 direction;
    v3 velocity;
    float verticalOffset; // TODO: Temporary fix. Fix heightmaps to accomodate this
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

    float _behindCheck;
    float _speed;
    mat4 _transform;
    float _turnAngle;
    float _heightPrev;

    bool meshVisible[10];
};
void* CabCreate(MemoryPool* mp);
void CabUpdate(void* cab);
void CabDraw3d(void* cab);
void CabDrawImGui(void* cab);
v3 CabGetFrontSeatPosition(Cab* cab);
GameObject CabPack(Cab* cab, const char* instanceName = "<unnamed>");

/*

*/
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
InstanceMeshRenderData ForestInstanceMeshRenderDataCreate(Image image, ForestGenerationInfo info, Mesh mesh, Material material);

struct CameraManager {
    Camera playerCamera;
    Camera debugCamera;
    float debugCameraSpeed;
    i32 cameraMode;
};
void CameraManagerInit(CameraManager* camMan, Camera playerCamera);
void CameraManagerUpdate(void* _camMan);
void CameraManagerDrawUi(void* _camMan);
GameObject CameraManagerPack(CameraManager* camMan, const char* instanceName = "<unnamed>");

void CameraUpdateDebug(Camera* camera, float speed);
void CameraUpdateCab(Camera* camera, Cab* cab);
v3 GetCameraForwardNorm(Camera* camera);
v3 GetCameraUpNorm(Camera* camera);
v3 GetCameraRightNorm(Camera* camera);
Camera CameraGetDefault();

// TODO: Make this reusable
struct TextureInstance_PriestReachout {
    TextureInstance* textureInstance;
    Shader* shader;
    Texture* noiseTexture;
    float frequency;
    ShaderColor color_a;
    ShaderColor color_b;
    float time;
};
void TextureInstance_PriestReachoutDrawUi(void* _ti);
GameObject TextureInstance_PriestReachoutPack(TextureInstance_PriestReachout* ti, const char* instanceName = "<unnamed>");

namespace resources {
    enum GAME_SHADERS {
        SHADER_UNLIT_INSTANCED,
        SHADER_LIT,
        SHADER_LIT_INSTANCED,
        SHADER_LIT_TERRAIN,
        SHADER_SKYBOX,
        SHADER_PASSTHROUGH_2D,
        SHADER_PRIEST_REACHOUT_2D,
        SHADER_COUNT
    };
    const char *shaderPaths[SHADER_COUNT * 2] = {
        "unlitInstanced.vs", "passthrough.fs",
        "light.vs", "light.fs",
        "lightInstanced.vs", "light.fs",
        "light.vs", "lightTerrain.fs",
        "skybox.vs", "skybox.fs",
        "passthrough2d.vs", "passthrough2d.fs",
        "priest-reachout.vs", "priest-reachout.fs"
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
        MODEL_LEVEL0,
        MODEL_COUNT
    };
    const char *modelPaths[MODEL_COUNT] = {
        "taxi.glb",
        "tree.glb",
        "level0.glb"
    };
    Model models[MODEL_COUNT];

    enum GAME_TEXTURES {
        TEXTURE_STAR,
        TEXTURE_NPATCH,
        TEXTURE_REACHOUT_BACKGROUND,
        TEXTURE_REACHOUT_PRIEST,
        TEXTURE_FBM_VALUE_OCT5_128,
        TEXTURE_COUNT
    };
    const char *texturePaths[resources::TEXTURE_COUNT] = {
        "star.png",
        "npatch.png",
        "reachout-background.png",
        "reachout-priest.png",
        "value_fbm_5oct_128.png"
    };
    Texture2D textures[resources::TEXTURE_COUNT];

    enum GAME_IMAGES {
        IMAGE_SKYBOX,
        IMAGE_HEIGHTMAP_LEVEL0,
        IMAGE_HEIGHTMAP_LEVEL1,
        IMAGE_TERRAINMAP_LEVEL1,
        IMAGE_COUNT
    };
    const char *imagePaths[IMAGE_COUNT] = {
        "skybox.png",
        "heightmap_level0.png",
        "heightmap_level1.png",
        "terrainmap_level1.png"
    };
    Image images[IMAGE_COUNT];

    enum GAME_FONT_TYPES {
        FONT_TYPE_PNG,
        FONT_TYPE_TTF
    };
    enum GAME_FONTS {
        FONT_GAME,
        FONT_DEBUG,
        FONT_COUNT
    };
    const char* fontPaths[FONT_COUNT] = {
        "silver.ttf",
        "mecha.png"
    };
    const i32 fontSizes[FONT_COUNT] = {
        32,
        28
    };
    Font fonts[FONT_COUNT];
}

namespace global {
    Camera* currentCamera = nullptr;
    Camera2D currentCameraUi = {};
    const i32 screenWidth = 1440;
    const i32 screenHeight = 800;
};

namespace debug {
    bool cameraEnabled = false;
    bool overlayEnabled = false;
    bool cursorEnabled = false;
    bool cursorEnabledPrevious = false;
    ImVec2 inspectorSize = ImVec2(300.f, global::screenHeight);
    ImVec2 inspectorPosition = ImVec2(global::screenWidth - 300.f, 0.f);

    i32 currentObjectIndexSelection = -1;
    const char* instanceableObjectNames;
}

void MdDebugInit() {
    StringBuilder sb = StringBuilderCreate(2048, &mdEngine::engineMemory);
    sb.separator = '\0';
    for (i32 i = 0; i < _MD_GAME_OBJECT_COUNT_MAX; i++) {
        if (!mdEngine::gameObjectIsDefined[i]) {
            continue;
        }
        GameObjectDefinition def = mdEngine::gameObjectDefinitions[i];
        if (StringBuilderAddString(&sb, def.objectName) != 0) {
            TraceLog(LOG_WARNING, TextFormat("%s: String builder ran out of memory", nameof(DebugInit)));
            break;
        }
    }
    debug::instanceableObjectNames = sb.str;
}

void MdGameRegisterObjects() {
    GameObjectDefinition def;
    MemoryPool* mp = &mdEngine::engineMemory;

    def = GameObjectDefinitionCreate("Cab", CabCreate, mp);
    def.Draw3d = CabDraw3d;
    def.DrawImGui = CabDrawImGui;
    def.Update = CabUpdate;
    MdEngineRegisterObject(def, OBJECT_CAB);
}

void MdGameInit() {
    MdGameRegisterObjects();

    global::currentCameraUi.offset = {0.f, 0.f};
    global::currentCameraUi.rotation = 0.f;
    global::currentCameraUi.target = {0.f, 0.f};
    global::currentCameraUi.zoom = 1.f;
}

void LoadGameShaders();
void UnloadGameShaders();
void LoadGameMaterials();
void UpdateGameMaterials(v3 lightPosition);
void UnloadGameMaterials();
void LoadGameModels();
void UnloadGameModels();
void LoadGameTextures();
void UnloadGameTextures();
void LoadGameImages();
void UnloadGameImages();
void LoadGameFonts();
void UnloadGameFonts();
void LoadGameResources();
void UnloadGameResources();

void DrawDebug3d();
void DrawDebugUi();
void DrawDebugImGui();

void GameObjectsUpdate(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsDraw3d(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsDrawUi(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsFree(GameObject* gameObjects, i32 gameObjectCount);
void GameObjectsDrawImGui(GameObject* gameObjects, i32 gameObjectCount);

i32 SceneSetup01(GameObject* goOut);
i32 SceneSetup_PriestReachout(GameObject* go);
i32 SceneSetup_DialogueTest(GameObject* go);
i32 SceneSetup_ModelTest(GameObject* go);

i32 main() {
    SetTraceLogLevel(4);
    InitWindow(global::screenWidth, global::screenHeight, "Midnight Driver");
    SetTargetFPS(FRAMERATE);
    rlImGuiSetup(true);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    DisableCursor();

    LoadGameResources();
    MdEngineInit(1 << 16);
    MdGameInit();
    MdDebugInit();

    GameObject gameObject[GAME_OBJECT_MAX];
    i32 gameObjectCount = SceneSetup_PriestReachout(gameObject);

    while (!WindowShouldClose()) {
        InputUpdate(&mdEngine::input);

        if (InputCheckPressedMod(INPUT_DEBUG_TOGGLE, false, false, false)) {
            debug::cameraEnabled = !debug::cameraEnabled;
        } else if (InputCheckPressedMod(INPUT_DEBUG_TOGGLE, false, true, false)) {
            debug::overlayEnabled = !debug::overlayEnabled;
        } else if (InputCheckPressedMod(INPUT_DEBUG_TOGGLE, true, true, false)) {
            debug::cursorEnabled = !debug::cursorEnabled;
        }

        GameObjectsUpdate(gameObject, gameObjectCount);

        if (global::currentCamera != nullptr) {
            UpdateGameMaterials(global::currentCamera->position);
        } else {
            UpdateGameMaterials(v3{0.f, 0.f, 0.f});
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if (global::currentCamera != nullptr) {
            BeginMode3D(*global::currentCamera);
                GameObjectsDraw3d(gameObject, gameObjectCount);
                if (debug::overlayEnabled) {
                    DrawDebug3d();
                }
            EndMode3D();
        }
        BeginMode2D(global::currentCameraUi);
        GameObjectsDrawUi(gameObject, gameObjectCount);
        EndMode2D();
        if (debug::overlayEnabled) {
            DrawDebugUi();
            rlImGuiBegin();
            ImGui::SetWindowPos(debug::inspectorPosition);
            ImGui::SetWindowSize(debug::inspectorSize);
            DrawDebugImGui();
            GameObjectsDrawImGui(gameObject, gameObjectCount);
            rlImGuiEnd();
        }
        if (!debug::cursorEnabled) {
            DisableCursor();
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }
        debug::cursorEnabledPrevious = debug::cursorEnabled;
        EndDrawing();
    }

    GameObjectsFree(gameObject, gameObjectCount);
    MemoryPoolDestroy(&mdEngine::sceneMemory);
    MemoryPoolDestroy(&mdEngine::persistentMemory);
    UnloadGameResources();

    return 0;
}

i32 SceneSetup01(GameObject* goOut) {
    i32 goCount = 0;

    Skybox* skybox = MemoryReserve<Skybox>(&mdEngine::sceneMemory);
    skybox->model = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    skybox->shader = resources::shaders[resources::SHADER_SKYBOX];
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
        resources::images[resources::IMAGE_SKYBOX],
        CUBEMAP_LAYOUT_AUTO_DETECT);
    goOut[goCount] = SkyboxPack(skybox); goCount++;

    ModelInstance* mi = MemoryReserve<ModelInstance>(&mdEngine::sceneMemory);
    mi->model = resources::models[resources::MODEL_LEVEL0];
    mi->position = {0.f, 0.f, 0.f};
    mi->scale = 1.f;
    mi->tint = WHITE;
    goOut[goCount] = ModelInstancePack(mi, "Terrain"); goCount++;
    
    BoundingBox bb = GetModelBoundingBox(mi->model);
    HeightmapGenerationInfo hgi = {};
    hgi.heightmapImage = &resources::images[resources::IMAGE_HEIGHTMAP_LEVEL1];
    hgi.position = bb.min;
    hgi.size = bb.max - bb.min;
    hgi.resdiv = 2;
    Heightmap* hm = MemoryReserve<Heightmap>(&mdEngine::sceneMemory);
    HeightmapInit(hm, hgi);
    mdEngine::groups["terrainHeightmap"] = hm;

    goOut[goCount] = CabPack((Cab*)CabCreate(&mdEngine::sceneMemory)); goCount++;

    CameraManager* camMan = MemoryReserve<CameraManager>(&mdEngine::sceneMemory);
    CameraManagerInit(camMan, CameraGetDefault());
    goOut[goCount] = CameraManagerPack(camMan); goCount++;

    return goCount;
};
i32 SceneSetup_PriestReachout(GameObject* go) {
    i32 goCount = 0;

    {
        ModelInstance* mi = MemoryReserve<ModelInstance>(&mdEngine::sceneMemory);
        mi->model = resources::models[resources::MODEL_LEVEL0];
        mi->position = {0.f, 0.f, 0.f};
        mi->scale = 1.f;
        mi->tint = WHITE;
        GameObject obj = ModelInstancePack(mi, "Terrain");
        go[goCount] = obj;
        go++;
    }

    return goCount;
}
i32 SceneSetup_DialogueTest(GameObject* go) {
    i32 goCount = 0;

    //DialogueSequence* dseq = MemoryReserve<DialogueSequence>(&mdEngine::sceneMemory);
    //DialogueSequenceCreate(dseq, 0);
    //go[goCount] = DialogueSequencePack(dseq);
    //goCount++;

    return goCount;
}
i32 SceneSetup_ModelTest(GameObject* go) {
    i32 goCount = 0;

    CameraManager* cm = MemoryReserve<CameraManager>(&mdEngine::sceneMemory);
    CameraManagerInit(cm, CameraGetDefault());
    go[goCount] = CameraManagerPack(cm);
    goCount++;

    ModelInstance* mi = MemoryReserve<ModelInstance>(&mdEngine::sceneMemory);
    mi->model = LOAD_MODEL("level_prototypeB.glb");
    //mi->model.materials[0] = global::materials[global::MATERIAL_LIT];
    mi->tint = WHITE;
    mi->position = {0.f};
    mi->scale = 1.f;
    go[goCount] = ModelInstancePack(mi);
    goCount++;

    return goCount;
}

void LoadGameShaders() {
    for (i32 i = 0; i < resources::SHADER_COUNT; i++) {
        resources::shaders[i] = LOAD_SHADER(resources::shaderPaths[i*2], resources::shaderPaths[i*2+1]);
    }
}
void UnloadGameShaders() {
    for (i32 i = 0; i < resources::SHADER_COUNT; i++) {
        UnloadShader(resources::shaders[i]);
    }
}

void LoadGameMaterials() {
    Shader sh;
    Material mat;

    sh = LOAD_SHADER("lightInstanced.vs", "light.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_LIT_INSTANCED] = mat;

    sh = LOAD_SHADER("light.vs", "light.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_LIT] = mat;

    sh = LOAD_SHADER("unlitInstanced.vs", "passthrough.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_UNLIT] = mat;

    sh = LOAD_SHADER("light.vs", "lightTerrain.fs");
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    sh.locs[SHADER_LOC_COLOR_SPECULAR] = GetShaderLocation(sh, "texture1");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_LIT_TERRAIN] = mat;
}
void UpdateGameMaterials(v3 lightPosition) {
    float pos[3] = {lightPosition.x, lightPosition.y, lightPosition.z};
    SetShaderValue(
        resources::materials[resources::MATERIAL_LIT_INSTANCED].shader,
        GetShaderLocation(resources::materials[resources::MATERIAL_LIT_INSTANCED].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
    SetShaderValue(
        resources::materials[resources::MATERIAL_LIT].shader,
        GetShaderLocation(resources::materials[resources::MATERIAL_LIT].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
    SetShaderValue(
        resources::materials[resources::MATERIAL_LIT_TERRAIN].shader,
        GetShaderLocation(resources::materials[resources::MATERIAL_LIT_TERRAIN].shader, "lightPosition"),
        (void*)pos,
        SHADER_UNIFORM_VEC3);
}
void UnloadGameMaterials() {
    for (i32 i = 0; i < resources::MATERIAL_COUNT; i++) {
        UnloadMaterial(resources::materials[i]);
    }
}

void LoadGameModels() {
    for (i32 i = 0; i < resources::MODEL_COUNT; i++) {
        resources::models[i] = LOAD_MODEL(resources::modelPaths[i]);
    }
}
void UnloadGameModels() {
    for (i32 i = 0; i < resources::MODEL_COUNT; i++) {
        UnloadModel(resources::models[i]);
    }
}

void LoadGameTextures() {
    for (i32 i = 0; i < resources::TEXTURE_COUNT; i++) {
        resources::textures[i] = LOAD_TEXTURE(resources::texturePaths[i]);
    }
}
void UnloadGameTextures() {
    for (i32 i = 0; i < resources::TEXTURE_COUNT; i++) {
        UnloadTexture(resources::textures[i]);
    }
}

void LoadGameImages() {
    for (i32 i = 0; i < resources::IMAGE_COUNT; i++) {
        resources::images[i] = LOAD_IMAGE(resources::imagePaths[i]);
    }
}
void UnloadGameImages() {
    for (i32 i = 0; i < resources::IMAGE_COUNT; i++) {
        UnloadImage(resources::images[i]);
    }
}

void LoadGameFonts() {
    for (i32 i = 0; i < resources::FONT_COUNT; i++) {
        if (strstr(resources::fontPaths[i], ".ttf") != nullptr) {
            resources::fonts[i] = LoadFontEx(TextFormat("resources/fonts/%s",resources::fontPaths[i]), resources::fontSizes[i], NULL, 0);
        } else {
            resources::fonts[i] = LOAD_FONT(resources::fontPaths[i]);
        }
    }
}
void UnloadGameFonts() {
    for (i32 i = 0; i < resources::FONT_COUNT; i++) {
        UnloadFont(resources::fonts[i]);
    }
}

void LoadGameResources() {
    LoadGameShaders();
    LoadGameModels();
    LoadGameTextures();
    LoadGameImages();
    LoadGameMaterials();
    LoadGameFonts();
}
void UnloadGameResources() {
    UnloadGameShaders();
    UnloadGameModels();
    UnloadGameTextures();
    UnloadGameImages();
    UnloadGameMaterials();
    UnloadGameFonts();
}

void DrawDebug3d() {
    DrawGrid(20, 1.f);
}
void DrawDebugUi() {
    DrawCrosshair(global::screenWidth / 2, global::screenHeight / 2, WHITE);
}
void DrawDebugImGui() {
    if (ImGui::CollapsingHeader("General info", ImGuiTreeNodeFlags_DefaultOpen)) {
        // TODO: TextFormat Text expired
        // TODO: Add remaining memory pools
        // TODO: Add bar instead of text (or both)
        ImGui::Text(TextFormat(
            "Local memory: %ub / %ub",
            &mdEngine::sceneMemory.location,
            &mdEngine::sceneMemory.size));
        ImGui::Text(
            TextFormat("Global memory: %ub / %ub",
            &mdEngine::persistentMemory.location,
            &mdEngine::persistentMemory.size));
    }
    if (ImGui::CollapsingHeader("Instancing", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Combo("Objects", &debug::currentObjectIndexSelection, debug::instanceableObjectNames);
    }
    ImGui::ShowDemoWindow();
}

void GameObjectsUpdate(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].Update != nullptr) {
            gameObjects[i].Update(gameObjects[i].data);
        }
    }
}
void GameObjectsDraw3d(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].Draw3d != nullptr) {
            gameObjects[i].Draw3d(gameObjects[i].data);
        }
    }
}
void GameObjectsDrawUi(GameObject* gameObjects, i32 gameObjectCount) {
    for (i32 i = 0; i < gameObjectCount; i++) {
        if (gameObjects[i].DrawUi != nullptr) {
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

void InstanceMeshRenderDataDestroy(InstanceMeshRenderData imrd) {
    UnloadModel(imrd._model);
    UnloadMaterial(imrd.material);
    // TODO: Double check if shader is unloaded as part of material
    RL_FREE(imrd.transforms);
}

InstanceMeshRenderData ForestInstanceMeshRenderDataCreate(Image image, ForestGenerationInfo info, Mesh mesh, Material material) {
    const i32 stride = PixelformatGetStride(image.format);
    if (stride < 3) {
        TraceLog(LOG_WARNING, "ForestCreate: Passed Image isn't valid for creating a forst");
        return {}; // TODO: Implement renderable default data for when InstanceMeshRenderData creation fails
    }
    const v2 imageSize = {(float)image.width, (float)image.height};
    const i32 treesMax = (i32)ceilf((info.worldSize.x * info.density) * (info.worldSize.y * info.density));
    i32 treeCount = 0;
    mat4 *transforms = (mat4*)RL_CALLOC(treesMax, sizeof(mat4));
    v2 pixelIncrF = imageSize / info.worldSize / info.density;
    const i32 incrx = pixelIncrF.x < 1.f ? 1 : (i32)floorf(pixelIncrF.x);
    const i32 incry = pixelIncrF.y < 1.f ? 1 : (i32)floorf(pixelIncrF.y);
    const byte* imageData = (byte*)image.data;
    for (i32 x = 0; x < image.width; x += incrx) {
        for (i32 y = 0; y < image.height; y += incry) {
            i32 i = (x + y * image.width) * stride;
            Color color = {imageData[i], imageData[i+1], imageData[i+2], 0};
            if (color.g == 255 && GetRandomChanceF(info.treeChance)) {
                v2 treePos = v2{(float)x, (float)y} / imageSize * info.worldSize + info.worldPosition;
                transforms[treeCount] = MatrixRotateYaw(GetRandomValueF(0.f, PI));
                transforms[treeCount] *= MatrixRotatePitch(GetRandomValueF(0.f, info.randomTiltDegrees) * DEG2RAD);
                transforms[treeCount] *= MatrixTranslate(
                    treePos.x + GetRandomValueF(-info.randomPositionOffset, info.randomPositionOffset),
                    GetRandomValueF(-info.randomYDip, 0.f) + HeightmapSampleHeight(info.heightmap, treePos.x, treePos.y), // TODO: Sample a heightmap to get a proper vertical tree position
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

void* CabCreate(MemoryPool* mp) {
    Cab* cab = MemoryReserve<Cab>(mp);
    cab->model = resources::models[resources::MODEL_CAB];
    cab->frontSeatPosition = {-0.16f, 1.85f, -0.44f};
    cab->verticalOffset = -0.f;
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
    cab->_heightPrev = 0.f;
    cab->_speed = 0.f;
    cab->_turnAngle = 0.f;
    mdEngine::groups["cab"] = (void*)cab;
    return cab;
}
void CabUpdate(void* _cab) {
    Cab* cab = (Cab*)_cab;
    bool inputAccelerate = mdEngine::input.down[INPUT_ACCELERATE];
    bool inputBreak = mdEngine::input.down[INPUT_BREAK];
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

    float inputTurn = ((float)mdEngine::input.down[INPUT_RIGHT] - (float)mdEngine::input.down[INPUT_LEFT]) * -1.f;
    if (inputTurn != 0.f) {
        float turnAmountStep = inputTurn * FRAME_TIME * cab->turnAngleSpeed;
        cab->_turnAngle = fclampf(cab->_turnAngle + turnAmountStep, -cab->turnAngleMax, cab->turnAngleMax);
    } else {
        cab->_turnAngle = ApproachZero(cab->_turnAngle, cab->turnNeutralReturn * FRAME_TIME);
    }
    cab->rotation.x += cab->_turnAngle * DEG2RAD * stepSpeed;
    mat4 yawRotationMatrix = MatrixRotateYaw(cab->rotation.x);

    // TODO: Inefficient matrix multiplication
    v3 horizontalDirection = v3{1.f, 0.f, 0.f} * yawRotationMatrix;
    v2 horizontalVelocity = Vector2Normalize({horizontalDirection.x, horizontalDirection.z}) * stepSpeed;
    cab->position.x += horizontalVelocity.x;
    cab->position.z += horizontalVelocity.y;
    
    Heightmap* hm = (Heightmap*)mdEngine::groups["terrainHeightmap"];
    v3 positionBehind = v3{
            cab->position.x - horizontalDirection.x,
            0.f,
            cab->position.z - horizontalDirection.z};
    if (hm != nullptr) {
        positionBehind.y = HeightmapSampleHeight(hm, positionBehind.x, positionBehind.z) + cab->verticalOffset;
        cab->position.y = HeightmapSampleHeight(hm, cab->position.x, cab->position.z) + cab->verticalOffset;
    }
    float incline = cab->position.y - positionBehind.y;
    float inclineAngle = atan2f(incline, 1.f);
    cab->rotation.y = inclineAngle;
    mat4 pitchRotationMatrix = MatrixRotatePitch(cab->rotation.y);

    cab->direction = {1.f, 0.f, 0.f};
    cab->direction = cab->direction * pitchRotationMatrix;
    cab->direction = cab->direction * yawRotationMatrix;

    cab->_transform = MatrixIdentity();
    cab->_transform *= pitchRotationMatrix;
    cab->_transform *= yawRotationMatrix;
    cab->_transform *= MatrixTranslate(cab->position.x, cab->position.y, cab->position.z);
}
void CabDraw3d(void* _cab) {
    Cab* cab = (Cab*)_cab;
    for (i32 i = 0; i < cab->model.meshCount; i++) {
        if (cab->meshVisible[i]) {
            DrawMesh(cab->model.meshes[i], cab->model.materials[cab->model.meshMaterial[i]], cab->_transform);
        }
    }
}
void CabDrawImGui(void* _cab) {
    Cab* cab = (Cab*)_cab;
    ImGui::DragFloat3("Position", (float*)&cab->position, 0.05f);
    ImGui::DragFloat3("Rotation", (float*)&cab->rotation, 0.05f);
    ImGui::DragFloat3("Direction", (float*)&cab->direction, 0.05f);
    ImGui::DragFloat3("Front seat", (float*)&cab->frontSeatPosition, 0.01f);
    ImGui::DragFloat("Vertical offset", (float*)&cab->verticalOffset, 0.01f);
    for (i32 i = 0; i < cab->model.meshCount; i++) {
        ImGui::Checkbox(TextFormat("%i", i), (bool*)&cab->meshVisible[i]);
    }
}
v3 CabGetFrontSeatPosition(Cab *cab) {
    return cab->frontSeatPosition * cab->_transform;
}
GameObject CabPack(Cab* cab, const char* instanceName) {
    GameObject go = GameObjectCreate(cab, "Cab", instanceName);
    return go;
}

void CameraManagerInit(CameraManager* camMan, Camera playerCamera) {
    camMan->playerCamera = playerCamera;
    camMan->debugCamera = {};
    camMan->debugCamera.fovy = 60.f;
    camMan->debugCamera.up = {0.f, 1.f, 0.f};
    camMan->debugCameraSpeed = 0.1f;
    global::currentCamera = &camMan->playerCamera;
}
void CameraManagerUpdate(void* _camMan) {
    CameraManager* camMan = (CameraManager*)_camMan;
    camMan->cameraMode = debug::cameraEnabled;

    if (debug::cameraEnabled && global::currentCamera != &camMan->debugCamera) {
        camMan->debugCamera.position = camMan->playerCamera.position;
        camMan->debugCamera.target = camMan->playerCamera.target;
        camMan->debugCamera.projection = camMan->playerCamera.projection;
        camMan->debugCamera.fovy = camMan->playerCamera.fovy;
        camMan->debugCamera.up = {0.f, 1.f, 0.f};
    }

    v2 mouseScroll = GetMouseWheelMoveV();
    camMan->debugCameraSpeed = fclampf(camMan->debugCameraSpeed + mouseScroll.y * 0.01f, 0.05f, 1.f);
    if (camMan->cameraMode) {
        CameraUpdateDebug(&camMan->debugCamera, camMan->debugCameraSpeed);
        global::currentCamera = &camMan->debugCamera;
    } else {
        Cab* cab = (Cab*)mdEngine::groups["cab"];
        if (cab != nullptr) {
            CameraUpdateCab(&camMan->playerCamera, cab);
            global::currentCamera = &camMan->playerCamera;
        }
    }
}
void CameraManagerDrawUi(void* _camMan) {
    CameraManager* camMan = (CameraManager*)_camMan;
    if (camMan->cameraMode == 1) {
        DrawTextShadow("Debug cam", 4, global::screenHeight - 20, 16, RED, BLACK);
    }
    DrawCrosshair(global::screenWidth / 2, global::screenHeight / 2, WHITE);
}
void CameraManagerFree(void* _camMan) {
    CameraManager* camMan = (CameraManager*)_camMan;
    if (global::currentCamera == &camMan->debugCamera || global::currentCamera == &camMan->playerCamera) {
        global::currentCamera = nullptr;
    }
}
GameObject CameraManagerPack(CameraManager* cm, const char* instanceName) {
    GameObject go = GameObjectCreate(cm, "Camera Manager", instanceName);
    go.Update = CameraManagerUpdate;
    go.DrawUi = CameraManagerDrawUi;
    go.Free = CameraManagerFree;
    return go;
}
void CameraUpdateCab(Camera* camera, Cab* cab) {
    v3 frontSeatPosition = CabGetFrontSeatPosition(cab);
    camera->position = frontSeatPosition;
    camera->target = camera->position + cab->direction;
}
void CameraUpdateDebug(Camera* camera, float speed) {
    if (debug::cursorEnabled) {
        return;
    }
    v3 targetDirection = camera->target - camera->position;
    float sensitivity = 0.002f;
    v2 mouseDelta = GetMouseDelta();
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraUpNorm(camera), -mouseDelta.x * sensitivity);
    targetDirection = Vector3RotateByAxisAngle(targetDirection, GetCameraRightNorm(camera), -mouseDelta.y * sensitivity);
    v3 input = {
        (float)mdEngine::input.down[INPUT_DEBUG_FORWARD] - (float)mdEngine::input.down[INPUT_DEBUG_BACKWARD],
        (float)mdEngine::input.down[INPUT_DEBUG_UP] - (float)mdEngine::input.down[INPUT_DEBUG_DOWN],
        (float)mdEngine::input.down[INPUT_DEBUG_RIGHT] - (float)mdEngine::input.down[INPUT_DEBUG_LEFT]};
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

v3 GetCameraForwardNorm(Camera* camera) {
    return Vector3Normalize(camera->target - camera->position);
}
v3 GetCameraUpNorm(Camera* camera) {
    return Vector3Normalize(camera->up);
}
v3 GetCameraRightNorm(Camera* camera) {
    v3 forward = GetCameraForwardNorm(camera);
    v3 up = GetCameraUpNorm(camera);
    return Vector3Normalize(Vector3CrossProduct(forward, up));
}
Camera CameraGetDefault() {
    Camera camera = {};
    camera.fovy = 60.f;
    camera.position = {0.f, 0.f, 0.f};
    camera.target = {1.f, 0.f, 0.f};
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = {0.f, 1.f, 0.f};
    return camera;
}

void TextureInstance_PriestReachoutDrawUi(void* _ti) {
    TextureInstance_PriestReachout* ti = (TextureInstance_PriestReachout*)_ti;
    ti->time += FRAME_TIME;
    Shader shd = *ti->shader;
    SetShaderValue(
        shd,
        GetShaderLocation(shd, "time"), 
        &ti->time,
        SHADER_UNIFORM_FLOAT);
    SetShaderValueTexture(
        shd,
        GetShaderLocation(shd, "noiseTexture"),
        *ti->noiseTexture);
    SetShaderValue(
        shd,
        GetShaderLocation(shd, "frequency"),
        &ti->frequency,
        SHADER_UNIFORM_FLOAT);
    SetShaderValue(
        shd,
        GetShaderLocation(shd, "color_a"),
        &ti->color_a,
        SHADER_UNIFORM_VEC4);
    SetShaderValue(
        shd,
        GetShaderLocation(shd, "color_b"),
        &ti->color_a,
        SHADER_UNIFORM_VEC4);
    BeginShaderMode(shd);
    TextureInstanceDrawUi(ti->textureInstance);
    EndShaderMode();
}
GameObject TextureInstance_PriestReachoutPack(TextureInstance_PriestReachout* ti, const char* instanceName) {
    GameObject go = GameObjectCreate(ti, "Texture Instance (Priest Reachout)", instanceName);
    go.DrawUi = TextureInstance_PriestReachoutDrawUi;
    return go;
}
