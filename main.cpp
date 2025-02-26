
#include <cstring>
#include <string.h>
#define NO_FONT_AWESOME

/*
    TODO: Add Customized Raylib as a submodule to this project.
    Currently LoadGLTF has been changed to fall back on non-indexed geometry for large meshes.
    Additionally, the function DrawMeshInstancedOptimized should be in raylib itself. Not as code in my project.
*/
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
struct Cab;
struct CameraManager;

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
void CabUpdate(Cab* cab);
void CabDraw3d(Cab* cab);
void CabDrawImGui(Cab* cab);
v3 CabGetFrontSeatPosition(Cab* cab);

struct CameraManager {
    Camera playerCamera;
    Camera debugCamera;
    float debugCameraSpeed;
    i32 cameraMode;
    struct_internal constexpr float debugCameraSpeedMin = 0.05f;
    struct_internal constexpr float debugCameraSpeedMax = 20.f;
};
void* CameraManagerCreate(MemoryPool* mp);
void CameraManagerUpdate(CameraManager* camMan);
void CameraManagerDrawUi(CameraManager* camMan);
void CameraManagerFree(CameraManager* camMan);

void CameraUpdateDebug(Camera* camera, float speed);
void CameraUpdateCab(Camera* camera, Cab* cab);
v3 GetCameraForwardNorm(Camera* camera);
v3 GetCameraUpNorm(Camera* camera);
v3 GetCameraRightNorm(Camera* camera);
Camera CameraGetDefault();

/*

*/
struct ForestGenerationInfo {
    v3 position;
    v2 size;
    float density;
    float treeChance;
    float randomPositionOffset;
    float randomYDip;
    float randomTiltDegrees;
    Heightmap* heightmap;
};
void InstanceRendererCreate_InitForest(InstanceRenderer* irOut, Image image, ForestGenerationInfo info, Mesh mesh, Material* material, MemoryPool* sceneMemory, MemoryPool* scratchMemory);

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

namespace resources {
    enum GAME_SHADERS {
        SHADER_UNLIT_INSTANCED,
        SHADER_LIT,
        SHADER_LIT_INSTANCED,
        SHADER_LIT_TERRAIN,
        SHADER_SKYBOX,
        SHADER_PASSTHROUGH,
        SHADER_PRIEST_REACHOUT_00,
        SHADER_PRIEST_REACHOUT_01,
        SHADER_PRIEST_REACHOUT_02,
        SHADER_PRIEST_REACHOUT_03,
        SHADER_PRIEST_REACHOUT_04,
        SHADER_PRIEST_REACHOUT_05,
        SHADER_PRIEST_REACHOUT_06,
        SHADER_PRIEST_REACHOUT_07,
        SHADER_PRIEST_REACHOUT_08,
        SHADER_COUNT
    };
    const char *shaderPaths[SHADER_COUNT * 2] = {
        "unlitInstanced.vs", "unlitInstanced.fs",
        "light.vs", "light.fs",
        "lightInstanced.vs", "lightInstanced.fs",
        "lightTerrain.vs", "lightTerrain.fs",
        "skybox.vs", "skybox.fs",
        "passthrough.vs", "passthrough.fs",
        "priestReachout_00.vs", "priestReachout_00.fs",
        "priestReachout_01.vs", "priestReachout_01.fs",
        "priestReachout_02.vs", "priestReachout_02.fs",
        "priestReachout_03.vs", "priestReachout_03.fs",
        "priestReachout_04.vs", "priestReachout_04.fs",
        "priestReachout_05.vs", "priestReachout_05.fs",
        "priestReachout_06.vs", "priestReachout_06.fs",
        "priestReachout_07.vs", "priestReachout_07.fs",
        "priestReachout_08.vs", "priestReachout_08.fs",
    };
    Shader shaders[SHADER_COUNT];

    enum GAME_MATERIALS {
        MATERIAL_UNLIT_INSTANCED,
        MATERIAL_LIT,
        MATERIAL_LIT_INSTANCED,
        MATERIAL_LIT_INSTANCED_TREE,
        MATERIAL_LIT_TERRAIN,
        MATERIAL_COUNT
    };
    Material materials[MATERIAL_COUNT];

    enum DEFAULT_GAME_MODELS {
        MODEL_DEFAULT_PLANE,
        MODEL_DEFAULT_CUBE,
        MODEL_DEFAULT_SPHERE,
        MODEL_DEFAULT_CYLINDER,
        MODEL_DEFAULT_CONE,
        MODEL_DEFAULT_COUNT
    };
    enum GAME_MODELS {
        MODEL_CAB = MODEL_DEFAULT_COUNT,
        MODEL_TREE,
        MODEL_LEVEL0,
        MODEL_COUNT
    };
    const char *modelPaths[MODEL_COUNT] = {
        "taxi.glb",
        "tree.glb",
        "level0.glb",
    };
    Model models[MODEL_DEFAULT_COUNT + MODEL_COUNT];

    enum GAME_TEXTURES {
        TEXTURE_STAR,
        TEXTURE_GROUND,
        TEXTURE_NPATCH,
        TEXTURE_TREE_MODEL,
        TEXTURE_LEVEL0_HEIGHTMAP,
        TEXTURE_LEVEL0_TERRAINMAP,
        TEXTURE_FBM_VALUE_OCT5_128,
        TEXTURE_PRIEST_REACHOUT_00_MOON,
        TEXTURE_PRIEST_REACHOUT_01_MIDDLE_GROUND,
        TEXTURE_PRIEST_REACHOUT_02_FENCE,
        TEXTURE_PRIEST_REACHOUT_03_BODY_MASK,
        TEXTURE_PRIEST_REACHOUT_04_BODY_DETAIL,
        TEXTURE_PRIEST_REACHOUT_05_ARM_MASK,
        TEXTURE_PRIEST_REACHOUT_06_ARM_DETAIL,
        TEXTURE_PRIEST_REACHOUT_07_CROSS,
        TEXTURE_PRIEST_REACHOUT_08_FOREGROUND,
        TEXTURE_COUNT
    };
    const char *texturePaths[resources::TEXTURE_COUNT] = {
        "star.png",
        "ground.png",
        "npatch.png",
        "tree_model.png",
        "level0_heightmap.png",
        "level0_terrainmap.png",
        "value_fbm_5oct_128.png",
        "priestReachout_00Moon.png",
        "priestReachout_01MiddleGround.png",
        "priestReachout_02Fence.png",
        "priestReachout_03BodyMask.png",
        "priestReachout_04BodyDetail.png",
        "priestReachout_05ArmMask.png",
        "priestReachout_06ArmDetail.png",
        "priestReachout_07Cross.png",
        "priestReachout_08Foreground.png"
    };
    Texture2D textures[resources::TEXTURE_COUNT];

    enum GAME_IMAGES {
        IMAGE_SKYBOX,
        IMAGE_LEVEL0_HEIGHTMAP,
        IMAGE_LEVEL0_TERRAINMAP,
        IMAGE_COUNT
    };
    const char *imagePaths[IMAGE_COUNT] = {
        "skybox.png",
        "level0_heightmap.png",
        "level0_terrainmap.png",
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
    // TODO: Turn this into an array of sorts
    constexpr i32 GAME_OBJECT_MAX = 1000;
    // TODO: Move this to scene memory for cache efficiency
    GameObject gameObjects[GAME_OBJECT_MAX];
    i32 gameObjectCount;
    struct {
        float falloffDistance = 20.f;
        float luminocity = 1.f;
        v3 ambientColor = {0.08f, 0.08f, 0.12f};
    } lighting;
};

namespace debug {
    bool cameraEnabled = false;
    bool overlayEnabled = false;
    bool cursorEnabled = false;
    bool cursorEnabledPrevious = false;
    ImVec2 inspectorSize = ImVec2(300.f, global::screenHeight);
    ImVec2 inspectorPosition = ImVec2(global::screenWidth - 300.f, 0.f);
    constexpr i32 instanceableObjectInstanceNameMaxLength = 64;
    char instanceableObjectInstanceName[instanceableObjectInstanceNameMaxLength];
    i32 instanceableObjectSelection = -1;
    const char* instanceableObjectNames;
    TypeList* instanceableObjectIndices;
    const char* imguiEditorTexturePaths;
}

// TODO: Make gameObjectCount and gameObjects part of global state
void MdGameObjectAdd(GameObject* gameObjects, i32* gameObjectCount, GameObject obj) {
    gameObjects[*gameObjectCount] = obj;
    (*gameObjectCount)++;
}

void MdDebugInit() {
    debug::instanceableObjectIndices = TypeListCreate(
        MD_TYPE_I32,
        &mdEngine::engineMemory,
        _MD_GAME_ENGINE_OBJECT_COUNT_MAX);
    {
        StringBuilder sb = StringBuilderCreate(2048, &mdEngine::engineMemory);
        sb.separator = '\0';
        for (i32 i = 0; i < _MD_GAME_ENGINE_OBJECT_COUNT_MAX; i++) {
            if (!mdEngine::gameObjectIsDefined[i]) {
                continue;
            }
            GameObjectDefinition def = mdEngine::gameObjectDefinitions[i];
            if (StringBuilderAddString(&sb, def.objectName) != 0) {
                TraceLog(LOG_WARNING, TextFormat("%s: String builder ran out of memory", nameof(DebugInit)));
                break;
            } else {
                TypeListPushBackI32(debug::instanceableObjectIndices, i);
            }
        }
        debug::instanceableObjectNames = sb.str;
    }
    {
        StringBuilder sb = StringBuilderCreate(2048, &mdEngine::engineMemory);
        sb.separator = '\0';
        for (i32 i = 0; i < resources::TEXTURE_COUNT; i++) {
            if (StringBuilderAddString(&sb, resources::texturePaths[i]) != 0) {
                TraceLog(LOG_WARNING, TextFormat("%s: String builder ran out of memory", nameof(DebugInit)));
                break;
            }
        }
        debug::imguiEditorTexturePaths = sb.str;
    }
}

enum MD_GAME_OBJECTS_CUSTOM {
    OBJECT_CAB = _MD_GAME_ENGINE_OBJECTS_COUNT,
    OBJECT_CAMERA_MANAGER
};
void MdGameRegisterObjects() {
    GameObjectDefinition def;
    MemoryPool* mp = &mdEngine::engineMemory;

    def = GameObjectDefinitionCreate("Cab", CabCreate, mp);
    def.Draw3d = (GameInstanceEventFunction)CabDraw3d;
    def.DrawImGui = (GameInstanceEventFunction)CabDrawImGui;
    def.Update = (GameInstanceEventFunction)CabUpdate;
    MdEngineRegisterObject(def, OBJECT_CAB);

    def = GameObjectDefinitionCreate("Camera Manager", CameraManagerCreate, mp);
    def.Update = (GameInstanceEventFunction)CameraManagerUpdate;
    def.DrawUi = (GameInstanceEventFunction)CameraManagerDrawUi;
    def.Free = (GameInstanceEventFunction)CameraManagerFree;
    MdEngineRegisterObject(def, OBJECT_CAMERA_MANAGER);
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
void DebugHandleImGui(GameObject* gameObjects, i32* gameObjectCount);

// TODO: Make game code reloadable
namespace scenes {
namespace priest_reachout {
    void DialogueSequence_InitDemo(DialogueSequence* dseq, MemoryPool* mp) {
        DialogueSequenceSection* dss = nullptr;
        StringList* text = nullptr;
        StringList* opt = nullptr;
        TypeList* link = nullptr;

        dss = DialogueSequenceSectionCreate(3, 3, mp);
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

        dss = DialogueSequenceSectionCreate(1, 0, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "I'm glad we agree, truly!");
        TypeListPushBackI32(link, -1);
        TypeListPushBackPtr(dseq->sections, dss);
    }

    void DialogueSequence_InitPriestHandover(DialogueSequence* dseq, MemoryPool* mp) {
        DialogueSequenceSection* dss = nullptr;
        StringList* text = nullptr;
        StringList* opt = nullptr;
        TypeList* link = nullptr;

        dss = DialogueSequenceSectionCreate(1, 3, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "Thank you; here's your payment.");
        StringListAdd(opt, "Thank you, father");
        StringListAdd(opt, "(Silently take the money)");
        StringListAdd(opt, "I Expected a bit more");
        TypeListPushBackI32(link, 1);
        TypeListPushBackI32(link, 1);
        TypeListPushBackI32(link, 2);
        TypeListPushBackPtr(dseq->sections, dss); // 0

        dss = DialogueSequenceSectionCreate(2, 3, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "I must say, our drive was insightful...");
        StringListAdd(text, "If you ever need to confess, don't hesitate to come and visit me.");
        StringListAdd(opt, "Forget it");
        StringListAdd(opt, "Maybe I will");
        StringListAdd(opt, "I'll think about it");
        TypeListPushBackI32(link, 3);
        TypeListPushBackI32(link, 3);
        TypeListPushBackI32(link, 3);
        TypeListPushBackPtr(dseq->sections, dss); // 1

        // TODO: "The priest stares blankly at you through the window"
        dss = DialogueSequenceSectionCreate(3, 3, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "(The priest gives you blank stare)");
        StringListAdd(text, "I must say, our drive was insightful...");
        StringListAdd(text, "If you ever need to confess, don't hesitate to come and visit me.");
        StringListAdd(opt, "Forget it");
        StringListAdd(opt, "Maybe I will");
        StringListAdd(opt, "I'll think about it");
        TypeListPushBackI32(link, 3);
        TypeListPushBackI32(link, 3);
        TypeListPushBackI32(link, 3);
        TypeListPushBackPtr(dseq->sections, dss); // 2

        dss = DialogueSequenceSectionCreate(1, 3, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "(The priest reaches into his pocket)"); // TODO: Play sound, fade to black
        StringListAdd(text, "..."); // TODO: Change to illustration
        StringListAdd(opt, "(Take the cross)"); // TODO: Cross get notification
        StringListAdd(opt, "...");
        StringListAdd(opt, "A crucifix?");
        TypeListPushBackI32(link, 4);
        TypeListPushBackI32(link, 5);
        TypeListPushBackI32(link, 5);
        TypeListPushBackPtr(dseq->sections, dss); // 3

        dss = DialogueSequenceSectionCreate(1, 0, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "(Without another word the priest scurries off toward the church)"); // TODO: Back to 3d scene
        TypeListPushBackPtr(dseq->sections, dss); // 4

        dss = DialogueSequenceSectionCreate(2, 3, mp);
        text = dss->text;
        opt = dss->options;
        link = dss->link;
        StringListAdd(text, "(The priest drops the cross onto the passenger seat)"); // TODO: Cross get notification
        StringListAdd(text, "(Without another word the priest scurries off toward the church)"); // TODO: Back to 3d scene
        StringListAdd(opt, "Hey!");
        StringListAdd(opt, "What am I supposed to do with this?");
        StringListAdd(opt, "...");
        TypeListPushBackI32(link, -1);
        TypeListPushBackI32(link, -1);
        TypeListPushBackI32(link, -1);
        TypeListPushBackPtr(dseq->sections, dss); // 5
    }

    void TextureInstanceMoon_ScriptInit(GameObject* obj, TextureInstance* ti) {
        ti->tint.a = 0;
        InstanceVariablePush("time", 0);
        InstanceVariablePush("fadeTweenValueStart", (byte)0);
        InstanceVariablePush("fadeTweenValueEnd", (byte)255);

        Tween t = TweenCreate(
            &(ti->tint.a),
            _InstanceVariableGet("fadeTweenValueStart", MD_TYPE_BYTE),
            _InstanceVariableGet("fadeTweenValueEnd", MD_TYPE_BYTE),
            MD_TYPE_BYTE,
            2.f);
        //TweenStart(&t);
        InstanceVariablePush("fadeTween", t);
    }
    void TextureInstanceMoon_ScriptUpdate(GameObject* obj, TextureInstance* ti) {
        i32 time = InstanceVariableGet<i32>("time");
        time++;
        SetShaderValue(
            *ti->shader,
            GetShaderLocation(*ti->shader, "time"),
            &time,
            SHADER_UNIFORM_INT);
        InstanceVariableSet("time", time);
        ShaderColor colorDiffuse = ColorToShaderColor(ti->tint);
        // TODO: Figure out why I have to pass color manually instead of tint just working
        SetShaderValue(
            *ti->shader,
            GetShaderLocation(*ti->shader, "color"),
            &colorDiffuse,
            SHADER_UNIFORM_VEC4);

        Tween t = InstanceVariableGet<Tween>("fadeTween");
        TweenStep(&t);
        InstanceVariableSet("fadeTween", t);
    }
    void Scene(GameObject* go, i32* count) {
        MemoryPool* mp = &mdEngine::sceneMemory;
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_SKYBOX, mp);
            SkyboxInit((Skybox*)obj.data, &resources::shaders[resources::SHADER_SKYBOX], &resources::images[resources::IMAGE_SKYBOX]);
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_MODEL_INSTANCE, mp);
            ModelInstance* mi = (ModelInstance*)obj.data;
            mi->model = resources::models[resources::MODEL_TREE];
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_CAB, mp);
            obj.active = false;
            Cab* data = (Cab*)obj.data;
            MdGameObjectAdd(go, count, obj);
        }
        {
            BoundingBox bb = GetMeshBoundingBox(resources::models[resources::MODEL_LEVEL0].meshes[0]);
            v3 level1_position = bb.min;
            v3 level1_size = bb.max - bb.min;

            Heightmap* hm = MemoryReserve<Heightmap>(mp);
            HeightmapGenerationInfo hgi = {};
            hgi.image = &resources::images[resources::IMAGE_LEVEL0_HEIGHTMAP];
            hgi.resdiv = 0;
            hgi.size = {level1_size.x, level1_size.z};
            hgi.position = level1_position;
            HeightmapInit(hm, hgi);

            ForestGenerationInfo fgi = {};
            fgi.density = 0.25f;
            fgi.heightmap = hm;
            fgi.position = {level1_position.x, 0.f, level1_position.z};
            fgi.size = {level1_size.x, level1_size.z};
            fgi.randomTiltDegrees = 10.f;
            fgi.randomYDip = 0.5f;
            fgi.randomPositionOffset = 2.5f;
            fgi.treeChance = 50.f;

            GameObject obj = MdEngineInstanceGameObject(OBJECT_INSTANCE_RENDERER, mp);
            InstanceRenderer* ir = (InstanceRenderer*)obj.data;
            InstanceRendererCreate_InitForest(
                ir,
                resources::images[resources::IMAGE_LEVEL0_TERRAINMAP],
                fgi,
                resources::models[resources::MODEL_TREE].meshes[0],
                &resources::materials[resources::MATERIAL_LIT_INSTANCED_TREE],
                mp,
                &mdEngine::scratchMemory);
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_MODEL_INSTANCE, mp);
            ModelInstance* mi = (ModelInstance*)obj.data;
            mi->model = resources::models[resources::MODEL_LEVEL0];
            mi->model.materials[0] = resources::materials[resources::MATERIAL_LIT_TERRAIN];
            mi->model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = resources::textures[resources::TEXTURE_LEVEL0_HEIGHTMAP];
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_CAMERA_MANAGER, mp);
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_00];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_00_MOON]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            GameObjectAddScript(
                &obj,
                (GameObjectScriptFunc)TextureInstanceMoon_ScriptInit,
                (GameObjectScriptFunc)TextureInstanceMoon_ScriptUpdate);
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_DIALOGUE_SEQUENCE, mp);
            DialogueSequence* dseq = (DialogueSequence*)obj.data;
            //DialogueSequence_InitPriestHandover(dseq, mp);
            //DialogueSequenceSectionStart(dseq, 0);
            MdGameObjectAdd(go, count, obj);
        }
        return;
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_00];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_00_MOON]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            GameObjectAddScript(
                &obj,
                (GameObjectScriptFunc)TextureInstanceMoon_ScriptInit,
                (GameObjectScriptFunc)TextureInstanceMoon_ScriptUpdate);
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_01];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_01_MIDDLE_GROUND]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_02];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_02_FENCE]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_03];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_03_BODY_MASK]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_04];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_04_BODY_DETAIL]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_05];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_05_ARM_MASK]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_06];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_06_ARM_DETAIL]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_07];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_07_CROSS]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_TEXTURE_INSTANCE, mp);
            TextureInstance* ti = (TextureInstance*)obj.data;
            ti->shader = &resources::shaders[resources::SHADER_PRIEST_REACHOUT_08];
            TextureInstanceSetTexture(ti, &resources::textures[resources::TEXTURE_PRIEST_REACHOUT_08_FOREGROUND]);
            TextureInstanceSetSize(ti, {(float)global::screenWidth, (float)global::screenHeight});
            MdGameObjectAdd(go, count, obj);
        }
    }
}
namespace model_viewer_scene {
    void ModelInstanceObject_Init(GameObject* obj, ModelInstance* mi) {
        i32 shaderType = 0;
        InstanceVariablePush("shaderType", shaderType);
    }
    void ModelInstanceObject_Update(GameObject* obj, ModelInstance* mi) {
        i32 shaderType = InstanceVariableGet<i32>("shaderType");
        if (InputCheckPressedMod(INPUT_DEBUG_ACCEPT, false, true, false)) {
            shaderType = shaderType == 0 ? 1 : 0;
            if (shaderType == 0) {
                mi->model.materials[0].shader = resources::shaders[resources::SHADER_PASSTHROUGH];
            } else {
                mi->model.materials[0].shader = resources::shaders[resources::SHADER_LIT];
            }
            InstanceVariableSet("shaderType", shaderType);
        }
    }

    void Scene(GameObject* go, i32* count) {
        debug::cameraEnabled = true;
        MemoryPool* mp = &mdEngine::sceneMemory;

        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_MODEL_INSTANCE, mp, "Model");
            ModelInstance *mi = (ModelInstance*)obj.data;
            mi->model = resources::models[resources::MODEL_TREE];
            GameObjectAddScript(&obj, (GameObjectScriptFunc)ModelInstanceObject_Init, (GameObjectScriptFunc)ModelInstanceObject_Update);
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_CAMERA_MANAGER, mp);
            MdGameObjectAdd(go, count, obj);
        }
    }
}
// TODO: Remove these once custom raylib has been set up and added as a submodule
Mesh DuplicateMesh(Mesh src) {
    Mesh m = {};
    m.vertexCount = src.vertexCount;
    m.triangleCount = src.triangleCount;
    if (src.vertices != NULL) {
        i32 size = sizeof(*m.vertices) * m.vertexCount * 3;
        m.vertices = (float*)malloc(size);
        memcpy(m.vertices, src.vertices, size);
    }
    if (src.texcoords != NULL) {
        i32 size = sizeof(*m.texcoords) * m.vertexCount * 2;
        m.texcoords = (float*)malloc(size);
        memcpy(m.texcoords, src.texcoords, size);
    }
    if (src.texcoords2 != NULL) {
        i32 size = sizeof(*m.texcoords2) * m.vertexCount * 2;
        m.texcoords2 = (float*)malloc(size);
        memcpy(m.texcoords2, src.texcoords2, size);
    }
    if (src.normals != NULL) {
        i32 size = sizeof(*m.normals) * m.vertexCount * 3;
        m.normals = (float*)malloc(size);
        memcpy(m.normals, src.normals, size);
    }
    if (src.tangents != NULL) {
        i32 size = sizeof(*m.tangents) * m.vertexCount * 4;
        m.tangents = (float*)malloc(size);
        memcpy(m.tangents, src.tangents, size);
    }
    if (src.colors != NULL) {
        i32 size = sizeof(*m.colors) * m.vertexCount * 4;
        m.colors = (unsigned char*)malloc(size);
        memcpy(m.colors, src.colors, size);
    }
    if (src.indices != NULL) {
        i32 size = sizeof(*m.indices) * m.triangleCount * 3;
        m.indices = (unsigned short*)malloc(size);
        memcpy(m.indices, src.indices, size);
    }
    UploadMesh(&m, false);
    return m;
}

Mesh DuplicateMeshNonIndexed(Mesh src) {
    if (src.indices == NULL) {
        return {};
    }
    Mesh m = {};
    m.vertexCount = src.triangleCount * 3;
    m.triangleCount = src.triangleCount;
    if (src.vertices != NULL) {
        i32 size = sizeof(*m.vertices) * m.vertexCount * 3;
        m.vertices = (float*)malloc(size);
    }
    if (src.texcoords != NULL) {
        i32 size = sizeof(*m.texcoords) * m.vertexCount * 2;
        m.texcoords = (float*)malloc(size);
    }
    if (src.texcoords2 != NULL) {
        i32 size = sizeof(*m.texcoords2) * m.vertexCount * 2;
        m.texcoords2 = (float*)malloc(size);
    }
    if (src.normals != NULL) {
        i32 size = sizeof(*m.normals) * m.vertexCount * 3;
        m.normals = (float*)malloc(size);
    }
    if (src.tangents != NULL) {
        i32 size = sizeof(*m.tangents) * m.vertexCount * 4;
        m.tangents = (float*)malloc(size);
    }
    if (src.colors != NULL) {
        i32 size = sizeof(*m.colors) * m.vertexCount * 4;
        m.colors = (unsigned char*)malloc(size);
    }
    for (i32 i = 0; i < m.vertexCount; i++) {
        if (src.vertices != NULL) {
            m.vertices[i * 3 + 0] = src.vertices[src.indices[i] * 3 + 0];
            m.vertices[i * 3 + 1] = src.vertices[src.indices[i] * 3 + 1];
            m.vertices[i * 3 + 2] = src.vertices[src.indices[i] * 3 + 2];
        }
    }
    for (i32 i = 0; i < m.vertexCount; i++) {
        if (src.texcoords != NULL) {
            m.texcoords[i * 2 + 0] = src.texcoords[src.indices[i] * 2 + 0];
            m.texcoords[i * 2 + 1] = src.texcoords[src.indices[i] * 2 + 1];
        }
    }
    for (i32 i = 0; i < m.vertexCount; i++) {
        if (src.texcoords2 != NULL) {
            m.texcoords2[i * 2 + 0] = src.texcoords2[src.indices[i] * 2 + 0];
            m.texcoords2[i * 2 + 1] = src.texcoords2[src.indices[i] * 2 + 1];
        }
    }
    for (i32 i = 0; i < m.vertexCount; i++) {
        if (src.normals != NULL) {
            m.normals[i * 3 + 0] = src.normals[src.indices[i] * 3 + 0];
            m.normals[i * 3 + 1] = src.normals[src.indices[i] * 3 + 1];
            m.normals[i * 3 + 2] = src.normals[src.indices[i] * 3 + 2];
        }
    }
    for (i32 i = 0; i < m.vertexCount; i++) {
        if (src.tangents != NULL) {
            m.tangents[i * 3 + 0] = src.tangents[src.indices[i] * 3 + 0];
            m.tangents[i * 3 + 1] = src.tangents[src.indices[i] * 3 + 1];
            m.tangents[i * 3 + 2] = src.tangents[src.indices[i] * 3 + 2];
            m.tangents[i * 3 + 3] = src.tangents[src.indices[i] * 3 + 3];
        }
    }
    for (i32 i = 0; i < m.vertexCount; i++) {
        if (src.colors != NULL) {
            m.colors[i * 3 + 0] = src.colors[src.indices[i] * 3 + 0];
            m.colors[i * 3 + 1] = src.colors[src.indices[i] * 3 + 1];
            m.colors[i * 3 + 2] = src.colors[src.indices[i] * 3 + 2];
            m.colors[i * 3 + 3] = src.colors[src.indices[i] * 3 + 3];
        }
    }
    UploadMesh(&m, false);
    return m;
}
Mesh DuplicateMeshNonIndexedReconstruct(Mesh src, unsigned int* indices)
{
    Mesh m = Mesh{0};
    m.vertexCount = src.triangleCount * 3;
    m.triangleCount = src.triangleCount;
    if (src.vertices != NULL) {
        m.vertices = (float*)RL_MALLOC(sizeof(*m.vertices) * m.vertexCount * 3);
    }
    if (src.texcoords != NULL) {
        m.texcoords = (float*)RL_MALLOC(sizeof(*m.texcoords) * m.vertexCount * 2);
    }
    if (src.texcoords2 != NULL) {
        m.texcoords2 = (float*)RL_MALLOC(sizeof(*m.texcoords2) * m.vertexCount * 2);
    }
    if (src.normals != NULL) {
        m.normals = (float*)RL_MALLOC(sizeof(*m.normals) * m.vertexCount * 3);
    }
    if (src.tangents != NULL) {
        m.tangents = (float*)RL_MALLOC(sizeof(*m.tangents) * m.vertexCount * 4);
    }
    if (src.colors != NULL) {
        m.colors = (unsigned char*)RL_MALLOC(sizeof(*m.colors) * m.vertexCount * 4);
    }
    for (int i = 0; i < m.vertexCount; i++) {
        if (src.vertices != NULL) {
            m.vertices[i * 3 + 0] = src.vertices[indices[i] * 3 + 0];
            m.vertices[i * 3 + 1] = src.vertices[indices[i] * 3 + 1];
            m.vertices[i * 3 + 2] = src.vertices[indices[i] * 3 + 2];
        }
    }
    for (int i = 0; i < m.vertexCount; i++) {
        if (src.texcoords != NULL) {
            m.texcoords[i * 2 + 0] = src.texcoords[indices[i] * 2 + 0];
            m.texcoords[i * 2 + 1] = src.texcoords[indices[i] * 2 + 1];
        }
    }
    for (int i = 0; i < m.vertexCount; i++) {
        if (src.texcoords2 != NULL) {
            m.texcoords2[i * 2 + 0] = src.texcoords2[indices[i] * 2 + 0];
            m.texcoords2[i * 2 + 1] = src.texcoords2[indices[i] * 2 + 1];
        }
    }
    for (int i = 0; i < m.vertexCount; i++) {
        if (src.normals != NULL) {
            m.normals[i * 3 + 0] = src.normals[indices[i] * 3 + 0];
            m.normals[i * 3 + 1] = src.normals[indices[i] * 3 + 1];
            m.normals[i * 3 + 2] = src.normals[indices[i] * 3 + 2];
        }
    }
    for (int i = 0; i < m.vertexCount; i++) {
        if (src.tangents != NULL) {
            m.tangents[i * 4 + 0] = src.tangents[indices[i] * 4 + 0];
            m.tangents[i * 4 + 1] = src.tangents[indices[i] * 4 + 1];
            m.tangents[i * 4 + 2] = src.tangents[indices[i] * 4 + 2];
            m.tangents[i * 4 + 3] = src.tangents[indices[i] * 4 + 3];
        }
    }
    for (int i = 0; i < m.vertexCount; i++) {
        if (src.colors != NULL) {
            m.colors[i * 4 + 0] = src.colors[indices[i] * 4 + 0];
            m.colors[i * 4 + 1] = src.colors[indices[i] * 4 + 1];
            m.colors[i * 4 + 2] = src.colors[indices[i] * 4 + 2];
            m.colors[i * 4 + 3] = src.colors[indices[i] * 4 + 3];
        }
    }
    return m;
}
namespace mesh_index_removal {
    void Scene(GameObject* go, i32* count) {
        debug::cameraEnabled = true;
        MemoryPool* mp = &mdEngine::sceneMemory;
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_MODEL_INSTANCE, mp, "Model");
            ModelInstance *mi = (ModelInstance*)obj.data;
            // Model treeModel = resources::models[resources::MODEL_TREE];
            // mi->mesh = DuplicateMeshNonIndexed(treeModel.meshes[0]);
            // mi->material = treeModel.materials[treeModel.meshMaterial[0]];
            mi->model = resources::models[resources::MODEL_LEVEL0];
            MdGameObjectAdd(go, count, obj);
        }
        {
            GameObject obj = MdEngineInstanceGameObject(OBJECT_CAMERA_MANAGER, mp);
            MdGameObjectAdd(go, count, obj);
        }
    }
}
}

i32 main() {
    SetTraceLogLevel(4);
    InitWindow(global::screenWidth, global::screenHeight, "Midnight Driver");
    SetTargetFPS(FRAMERATE);
    rlImGuiSetup(true);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    DisableCursor();

    LoadGameResources();
    MdEngineInit(
        MEGABYTES(128),
        MEGABYTES(128),
        MEGABYTES(4),
        MEGABYTES(4));
    mdEngine::textDrawingStyleDefault.font = resources::fonts[resources::FONT_GAME];
    mdEngine::textDrawingStyleDefault.size = 48;
    MdGameInit();
    MdDebugInit();

    scenes::priest_reachout::Scene(global::gameObjects, &global::gameObjectCount);
    //scenes::mesh_index_removal::Scene(global::gameObjects, &global::gameObjectCount);

    while (!WindowShouldClose()) {
        InputUpdate(&mdEngine::input);

        if (InputCheckPressedMod(INPUT_DEBUG_TOGGLE, false, false, false)) {
            debug::cameraEnabled = !debug::cameraEnabled;
        } else if (InputCheckPressedMod(INPUT_DEBUG_TOGGLE, false, true, false)) {
            debug::overlayEnabled = !debug::overlayEnabled;
        } else if (InputCheckPressedMod(INPUT_DEBUG_TOGGLE, true, true, false)) {
            debug::cursorEnabled = !debug::cursorEnabled;
        }

        GameObjectsUpdate(global::gameObjects, global::gameObjectCount);

        if (global::currentCamera != nullptr) {
            UpdateGameMaterials(global::currentCamera->position);
        } else {
            UpdateGameMaterials(v3{0.f, 0.f, 0.f});
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if (global::currentCamera != nullptr) {
            BeginMode3D(*global::currentCamera);
                GameObjectsDraw3d(global::gameObjects, global::gameObjectCount);
                if (debug::overlayEnabled) {
                    DrawDebug3d();
                }
            EndMode3D();
        }
        BeginMode2D(global::currentCameraUi);
        GameObjectsDrawUi(global::gameObjects, global::gameObjectCount);
        EndMode2D();
        if (debug::overlayEnabled) {
            DrawDebugUi();
            rlImGuiBegin();
            ImGui::SetWindowPos(debug::inspectorPosition);
            ImGui::SetWindowSize(debug::inspectorSize);
            DebugHandleImGui(global::gameObjects, &global::gameObjectCount);
            GameObjectsDrawImGui(global::gameObjects, global::gameObjectCount);
            rlImGuiEnd();
        }
        if (!debug::cursorEnabled) {
            DisableCursor();
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }
        debug::cursorEnabledPrevious = debug::cursorEnabled;
        EndDrawing();
    }

    GameObjectsFree(global::gameObjects, global::gameObjectCount);
    MemoryPoolDestroy(&mdEngine::sceneMemory);
    MemoryPoolDestroy(&mdEngine::persistentMemory);
    UnloadGameResources();

    return 0;
}

// TODO: This implementation is only here temporarily because resources don't exist in engine.hpp
// Separate texture selection combo from the instance to fix this
void TextureInstanceDrawImGui(TextureInstance* ti) {
    v2 position = ti->_position;
    if (ImGui::DragFloat2("Position", (float*)&position)) {
        TextureInstanceSetPosition(ti, position);
    }
    ImGui::DragFloat("Rotation", &ti->_rotation);
    v2 size = v2{ti->_drawDestination.width, ti->_drawDestination.height};
    if (ImGui::DragFloat2("Size", (float*)&size)) {
        TextureInstanceSetSize(ti, size);
    }
    // TODO: Move texture selection combo out of instance
    if (ImGui::Combo("Texture", &ti->_imguiTextureIndex, debug::imguiEditorTexturePaths)) {
        TextureInstanceSetTexture(ti, &resources::textures[ti->_imguiTextureIndex]);
    }
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

    sh = resources::shaders[resources::SHADER_LIT_INSTANCED];
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_LIT_INSTANCED] = mat;

    sh = resources::shaders[resources::SHADER_LIT_INSTANCED];
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    mat.maps[MATERIAL_MAP_ALBEDO].texture = resources::textures[resources::TEXTURE_TREE_MODEL];
    resources::materials[resources::MATERIAL_LIT_INSTANCED_TREE] = mat;

    sh = resources::shaders[resources::SHADER_LIT];
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_LIT] = mat;

    sh = resources::shaders[resources::SHADER_UNLIT_INSTANCED];
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(sh, "instanceTransform");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    resources::materials[resources::MATERIAL_UNLIT_INSTANCED] = mat;

    sh = resources::shaders[resources::SHADER_LIT_TERRAIN];
    sh.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(sh, "modelMat");
    sh.locs[SHADER_LOC_COLOR_SPECULAR] = GetShaderLocation(sh, "texture1");
    mat = LoadMaterialDefault();
    mat.shader = sh;
    mat.maps[SHADER_LOC_MAP_ALBEDO].texture = resources::textures[resources::TEXTURE_LEVEL0_TERRAINMAP];
    mat.maps[SHADER_LOC_MAP_SPECULAR].texture = resources::textures[resources::TEXTURE_GROUND];
    resources::materials[resources::MATERIAL_LIT_TERRAIN] = mat;
}
void UpdateGameMaterials(v3 lightPosition) {
    float pos[3] = {lightPosition.x, lightPosition.y, lightPosition.z};
    i32 lightUpdateMaterials[] = {
        resources::MATERIAL_LIT,
        resources::MATERIAL_LIT_INSTANCED,
        resources::MATERIAL_LIT_INSTANCED_TREE,
        resources::MATERIAL_LIT_TERRAIN
    };
    for (i32 i = 0; i < sizeof(lightUpdateMaterials) / sizeof(i32); i++) {
        Shader shader = resources::materials[lightUpdateMaterials[i]].shader;
        SetShaderValue(
            shader,
            GetShaderLocation(shader, "lightPosition"),
            (void*)pos,
            SHADER_UNIFORM_VEC3);
        SetShaderValue(
            shader,
            GetShaderLocation(shader, "lightFalloffDistance"),
            &global::lighting.falloffDistance,
            SHADER_UNIFORM_FLOAT);
        SetShaderValue(
            shader,
            GetShaderLocation(shader, "lightAmbientColor"),
            &global::lighting.ambientColor.x,
            SHADER_UNIFORM_VEC3);
        SetShaderValue(
            shader,
            GetShaderLocation(shader, "lightLuminocity"),
            &global::lighting.luminocity,
            SHADER_UNIFORM_FLOAT);
    }
}
void UnloadGameMaterials() {
    for (i32 i = 0; i < resources::MATERIAL_COUNT; i++) {
        UnloadMaterial(resources::materials[i]);
    }
}

void LoadGameModels() {
    resources::models[resources::MODEL_DEFAULT_CONE] = LoadModelFromMesh(GenMeshCone(1.f, 1.f, 16));
    resources::models[resources::MODEL_DEFAULT_CUBE] = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    resources::models[resources::MODEL_DEFAULT_CYLINDER] = LoadModelFromMesh(GenMeshCylinder(1.f, 1.f, 16));
    resources::models[resources::MODEL_DEFAULT_PLANE] = LoadModelFromMesh(GenMeshPlane(1.f, 1.f, 1, 1));
    resources::models[resources::MODEL_DEFAULT_SPHERE] = LoadModelFromMesh(GenMeshSphere(1.f, 16, 16));
    // resources::models[resources::MODEL_CAB] = LOAD_MODEL(resources::modelPaths[0]);
    // resources::models[resources::MODEL_TREE] = LOAD_MODEL(resources::modelPaths[1]);
    // resources::models[resources::MODEL_LEVEL0] = LOAD_MODEL(resources::modelPaths[2]);
    for (i32 i = 0; i < resources::MODEL_COUNT - resources::MODEL_DEFAULT_COUNT; i++) {
       resources::models[i + resources::MODEL_DEFAULT_COUNT] = LOAD_MODEL(resources::modelPaths[i]);
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
    LoadGameImages();
    LoadGameModels();
    LoadGameTextures();
    LoadGameMaterials();
    LoadGameFonts();
}
void UnloadGameResources() {
    UnloadGameShaders();
    UnloadGameImages();
    UnloadGameModels();
    UnloadGameTextures();
    UnloadGameMaterials();
    UnloadGameFonts();
}

void DrawDebug3d() {
    DrawGrid(20, 1.f);
}
void DrawDebugUi() {
    DrawCrosshair(global::screenWidth / 2, global::screenHeight / 2, WHITE);
}
void DebugHandleImGui(GameObject* gameObjects, i32* gameObjectCount) {
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
        // TODO: Make sure this works
        if (ImGui::Button("Reload shaders")) {
            UnloadGameShaders();
            LoadGameShaders();
        }
        if (global::currentCamera != nullptr) {
            ImGui::InputFloat3("Current camera position: ", (float*)&global::currentCamera->position);
        }
    }
    if (ImGui::CollapsingHeader("Instancing", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Combo("Objects", &debug::instanceableObjectSelection, debug::instanceableObjectNames);
        ImGui::InputText("Instance name", debug::instanceableObjectInstanceName, debug::instanceableObjectInstanceNameMaxLength);
        if (ImGui::Button("Instance")) {
            i32 objectIndex = TypeListGetI32(debug::instanceableObjectIndices, debug::instanceableObjectSelection);
            GameObject obj = MdEngineInstanceGameObject(objectIndex, &mdEngine::sceneMemory);
            MdGameObjectAdd(gameObjects, gameObjectCount, obj);
        }
    }
    if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Falloff distance", &global::lighting.falloffDistance, 0.1f);
        ImGui::DragFloat3("Ambient Color", &global::lighting.ambientColor.x, 0.05f, 0.f, 1.f);
        ImGui::DragFloat("Luminocity", &global::lighting.luminocity, 0.05f);
    }
}

void InstanceRendererCreate_InitForest(InstanceRenderer* irOut, Image image, ForestGenerationInfo info, Mesh mesh, Material* material, MemoryPool* sceneMemory, MemoryPool* scratchMemory) {
    const i32 stride = PixelformatGetStride(image.format);
    if (stride < 3) {
        TraceLog(LOG_WARNING, TextFormat("%s: Passed Image isn't valid for creating a forest", nameof(InstanceRendererCreate_InitForest)));
        return; // TODO: Implement renderable default data for when InstanceMeshRenderData creation fails
    }
    const v2 imageSize = {(float)image.width, (float)image.height};
    const i32 treesMax = (i32)ceilf((info.size.x * info.density) * (info.size.y * info.density));
    i32 treeCount = 0;
    mat4 *transforms = (mat4*)MemoryReserve<mat4>(scratchMemory, treesMax);
    v2 pixelIncrF = imageSize / info.size / info.density;
    const byte* imageData = (byte*)image.data;
    for (float x = 0; x < info.size.x; x += 1.f / info.density) {
        for (float y = 0; y < info.size.y; y += 1.f / info.density) {
            v2 imagePosition = v2{x, y} / info.size * imageSize;
            i32 imagePositionX = (i32)imagePosition.x;
            i32 imagePositionY = (i32)imagePosition.y;
            if (imagePositionX + 1 >= (i32)imageSize.x &&
                imagePositionY + 1 >= (i32)imageSize.y) {
                continue;
            }

            i32 itl = (imagePositionX + imagePositionY * image.width) * stride;
            i32 itr = ((imagePositionX + 1) + imagePositionY * image.width) * stride;
            i32 ibl = (imagePositionX + imagePositionY * image.width) * stride;
            i32 ibr = ((imagePositionX + 1) + (imagePositionY + 1) * image.width) * stride;
            Color colorTl = {imageData[itl], imageData[itl+1], imageData[itl+2], 255};
            Color colorTr = {imageData[itr], imageData[itr+1], imageData[itr+2], 255};
            Color colorBl = {imageData[ibl], imageData[ibl+1], imageData[ibl+2], 255};
            Color colorBr = {imageData[ibr], imageData[ibr+1], imageData[ibr+2], 255};

            v2 imagePositionFactor = Vector2Fract(imagePosition);
            Color colorHorizontalTop = ColorLerp(colorTl, colorTr, imagePositionFactor.x);
            Color colorHorizontalBottom = ColorLerp(colorBl, colorBr, imagePositionFactor.x);
            Color colorFinal = ColorLerp(colorHorizontalTop, colorHorizontalBottom, imagePositionFactor.y);

            float localTreeChance = (float)colorFinal.g / 255.f * 100.f;
            if (GetRandomChanceF(localTreeChance * info.treeChance)) {
                v3 treePos = v3{x, 0.f, y} + info.position;
                transforms[treeCount] = MatrixRotateYaw(GetRandomValueF(0.f, PI));
                transforms[treeCount] *= MatrixRotatePitch(GetRandomValueF(0.f, info.randomTiltDegrees) * DEG2RAD);
                v3 translate = {
                    treePos.x + GetRandomValueF(-info.randomPositionOffset, info.randomPositionOffset),
                    treePos.y + GetRandomValueF(-info.randomYDip, 0.f) + HeightmapSampleHeight(info.heightmap, treePos.x, treePos.y), // TODO: Sample a heightmap to get a proper vertical tree position
                    treePos.z + GetRandomValueF(-info.randomPositionOffset, info.randomPositionOffset)
                };
                transforms[treeCount] *= MatrixTranslate(translate.x, translate.y, translate.z);
                treeCount++;
            }
        }
    }

    if (treeCount == 0) {
        TraceLog(LOG_WARNING, "%s: Didn't generate any trees", nameof(InstanceRendererCreate_InitForest));
        irOut->instanceCount = 0;
        irOut->transforms = nullptr;
        irOut->mesh = mesh;
        irOut->material = material;
        return;
    }

    float16 *transforms16 = MemoryReserve<float16>(sceneMemory, treeCount);
    for (i32 i = 0; i < treeCount; i++) {
        transforms16[i] = MatrixToFloatV(transforms[i]);
    }

    irOut->instanceCount = treeCount;
    irOut->transforms = transforms16;
    irOut->mesh = mesh;
    irOut->material = material;

    MemoryPoolClear(scratchMemory);
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
    memset(cab->meshVisible, 1, sizeof(cab->meshVisible));
    mdEngine::groups["cab"] = (void*)cab;
    return cab;
}
void CabUpdate(Cab* cab) {
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
void CabDraw3d(Cab* cab) {
    for (i32 i = 0; i < cab->model.meshCount; i++) {
        if (cab->meshVisible[i]) {
            DrawMesh(cab->model.meshes[i], cab->model.materials[cab->model.meshMaterial[i]], cab->_transform);
        }
    }
}
void CabDrawImGui(Cab* cab) {
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

void* CameraManagerCreate(MemoryPool* mp) {
    CameraManager* camMan = MemoryReserve<CameraManager>(mp);
    camMan->playerCamera = CameraGetDefault();
    camMan->debugCamera = {};
    camMan->debugCamera.fovy = 60.f;
    camMan->debugCamera.up = {0.f, 1.f, 0.f};
    camMan->debugCameraSpeed = 0.1f;
    global::currentCamera = &camMan->playerCamera;
    return camMan;
}
void CameraManagerUpdate(CameraManager* camMan) {
    camMan->cameraMode = debug::cameraEnabled;

    if (debug::cameraEnabled && global::currentCamera != &camMan->debugCamera) {
        camMan->debugCamera.position = camMan->playerCamera.position;
        camMan->debugCamera.target = camMan->playerCamera.target;
        camMan->debugCamera.projection = camMan->playerCamera.projection;
        camMan->debugCamera.fovy = camMan->playerCamera.fovy;
        camMan->debugCamera.up = {0.f, 1.f, 0.f};
    }

    v2 mouseScroll = GetMouseWheelMoveV();
    camMan->debugCameraSpeed = fclampf(
        camMan->debugCameraSpeed + mouseScroll.y * 0.01f,
        camMan->debugCameraSpeedMin,
        camMan->debugCameraSpeedMax);
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
void CameraManagerDrawUi(CameraManager* camMan) {
    if (camMan->cameraMode == 1) {
        DrawTextShadow("Debug cam", 4, global::screenHeight - 20, 16, RED, BLACK);
    }
    DrawCrosshair(global::screenWidth / 2, global::screenHeight / 2, WHITE);
}
void CameraManagerFree(CameraManager* camMan) {
    if (global::currentCamera == &camMan->debugCamera || global::currentCamera == &camMan->playerCamera) {
        global::currentCamera = nullptr;
    }
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
