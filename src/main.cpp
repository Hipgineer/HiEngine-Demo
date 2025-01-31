#include "context.h"
#include "simbuffer.h"
#include "HiPhysics/hiphysics.h"
#include <vector>
#include <spdlog/spdlog.h>
#include <glad/glad.h> // had to be included before including GLFW
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb/stb_image.h>

// o =========================================================================== o
// |                                                                             |
// |                                                                             |
// |                               GLOBAL VARIABLE                               |
// |                                                                             |
// |                                                                             |
// o =========================================================================== o
GLFWwindow*         g_window = nullptr;
ContextUPtr         g_context = nullptr;
HiPhysicsUPtr       g_hiPhysics = nullptr;
SimBufferPtr        g_buffer = nullptr;

#include "scenes/sceneHelper.h"
#include "scenes/scene.h"
std::vector<Scene*> g_scenes;
uint32_t            g_scene = 0;

bool g_pause = true; // pause update or not
bool g_step  = false; // update only one step when g_pause = true

// common variables


// o =========================================================================== o
// |                                                                             |
// |                                                                             |
// |                           GLFW Callback Functions                           |
// |                                                                             |
// |                                                                             |
// o =========================================================================== o

void OnFramebufferSizeChange(GLFWwindow* window, int width, int height) 
{
    SPDLOG_INFO("framebuffer size changed : ({} x {})", width, height);
    auto context = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
    context->Reshape(width, height);
}

void OnCursorPos(GLFWwindow* window, double x, double y) 
{
    auto context =  reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
    context->MouseMove(x, y);
}

void OnMouseButton(GLFWwindow* window, int button, int action, int modifier) 
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, modifier);
    auto context =  reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    context->MouseButton(button,action, x, y);
}

void OnCharEvent(GLFWwindow* window, unsigned int ch) 
{
    ImGui_ImplGlfw_CharCallback(window, ch);
}

void OnScroll(GLFWwindow* window, double xoffset, double yoffset) 
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    auto context =  reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
    context->MouseWheel(xoffset, yoffset);
}

void OnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    auto context =  reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
    
    SPDLOG_INFO("key: {}, scancode: {}, action: {}, modes: {}{}{}",
        key, scancode,
        action == GLFW_PRESS ? "Pressed" :
        action == GLFW_RELEASE ? "Released" :
        action == GLFW_REPEAT ? "Repeat" : "Unknown",
        mods & GLFW_MOD_CONTROL ? "C" : "-",
        mods & GLFW_MOD_SHIFT ? "S" : "-",
        mods & GLFW_MOD_ALT ? "A" : "-");
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    context->PressKey(key, scancode, action, mods);

    if (key == GLFW_KEY_P && action == GLFW_PRESS) g_pause = !g_pause;
    if (key == GLFW_KEY_O && (action == GLFW_PRESS || action == GLFW_REPEAT)) g_step  = true;
}

// o =========================================================================== o
// |                                                                             |
// |                                                                             |
// |                                                                             |
// o =========================================================================== o

bool InitializeWithScene(int32_t sceneIndex) {
    g_scene = sceneIndex;

    // HiPhysics - initialize solver
    SPDLOG_INFO("Initialize HiPhysics");
    g_hiPhysics = HiPhysics::Create();
    if (!g_hiPhysics)
    {
        SPDLOG_ERROR("failed to create HiPhysics");
        return false;
    }

    // SimBuffer - initialize Buffer
    SPDLOG_INFO("Initialize Simulation Buffer");
    g_buffer = SimBuffer::Create();
    if (!g_buffer)
    {
        SPDLOG_ERROR("failed to create Simulation Buffer");
        return false;
    }

    // Load Current Scene
    g_scenes[g_scene]->Init();

    SPDLOG_INFO("init number of particles : {}", g_buffer->GetNumParticles());

    // Initialize Scene into hiphysics engine

    if (g_scenes[g_scene]->mSceneType == StateOfMatter::FLUID)
    {
        if (!g_hiPhysics->SetMemory(g_buffer))
        {
            SPDLOG_ERROR("CUDA : failed to copy host to device.");
            return false;
        }
    }
    else if (g_scenes[g_scene]->mSceneType == StateOfMatter::CLOTH)
    {
        if (!g_hiPhysics->SetMemoryCloth(g_buffer))
        {
            SPDLOG_ERROR("CUDA : failed to copy host to device.");
            return false;
        }
    }

    // 
    if (!g_context->MapSimBuffer(g_buffer))
    {
        SPDLOG_ERROR("failed to copy simBuffer data to context");
        return false;
    }

    return true;
}

// o =========================================================================== o
// |                                                                             |
// |                                                                             |
// |                                 MAIN                                        |
// |                                                                             |
// |                                                                             |
// o =========================================================================== o
int main(int argc, const char** argv)
{
    SPDLOG_INFO("START PROGRAM.");

    // o ---------------------------------------------------------------------- o
    // |                      LOAD & INITIALIZE LIBRARIES                       |
    // o ---------------------------------------------------------------------- o

    // GLFW
    // - initialize the library
    SPDLOG_INFO("Initialize glfw");
    if (!glfwInit()) {
        const char* description = nullptr;
        glfwGetError(&description);
        SPDLOG_ERROR("failed to intialize glfw : {}", description);
        return -1;
    }

    // GLFW
    // - set openGL information
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // GLFW
    // - create window
    SPDLOG_INFO("Create glfw window");
    g_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    if(!g_window){
        SPDLOG_ERROR("faied to create glfw window");
        glfwTerminate();
        return -1;
    }
    
    // GLFW
    // - Set Window Icon
    GLFWimage images[1]; 
    images[0].pixels = stbi_load("./image/HiIcon.jpg", &images[0].width, &images[0].height, 0, 4); //rgba channels 
    glfwSetWindowIcon(g_window, 1, images); 
    stbi_image_free(images[0].pixels);

    // GLFW
    // - make context on created windnow
    glfwMakeContextCurrent(g_window);

    // GLAD
    //  - load opengl for glfw (had to be load after creating OpenGL context)
    //  - - glfwGetProcAddress: getting process of glfw, and load opengl on it
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        SPDLOG_ERROR("failed to initialze glad");
        glfwTerminate();
        return -1;
    }

    // openGL
    //  - Now can use opengl fucntions in glfw window
    auto glVersion = glGetString(GL_VERSION); 
    SPDLOG_INFO("OpenGL context version: {}", glVersion);
    
    // imgui 
    // - init imgui context
    auto imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    ImGui_ImplGlfw_InitForOpenGL(g_window, false);
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    ImGui_ImplOpenGL3_CreateDeviceObjects();

    // o ---------------------------------------------------------------------- o
    // |                           MAIN PROGRAM                                 |
    // o ---------------------------------------------------------------------- o

    // class Context - Initialize
    SPDLOG_INFO("Initialize Context");
    g_context = Context::Create();
    if(!g_context) {
        SPDLOG_ERROR("failed to create context");
        glfwTerminate();
        return -1;
    }

    // Extract c_Context Pointer to use glfw callback outside of c_Context
    // -- after here, use like "auto pointer = (Context*)glfwGetWindowUserPointer(window);"
    glfwSetWindowUserPointer(g_window, g_context.get());

    // Callback functions
    OnFramebufferSizeChange(g_window, WINDOW_WIDTH, WINDOW_HEIGHT); // manually set first frame buffer size
    glfwSetFramebufferSizeCallback(g_window, OnFramebufferSizeChange);
    glfwSetKeyCallback(g_window, OnKeyEvent);
    glfwSetCharCallback(g_window, OnCharEvent);
    glfwSetCursorPosCallback(g_window, OnCursorPos);
    glfwSetMouseButtonCallback(g_window, OnMouseButton);
    glfwSetScrollCallback(g_window, OnScroll);

    // Load All Scenes
    g_scenes.push_back(new SphereDrop("Sphere Drop"));
    g_scenes.push_back(new SphereCollision("Sphere Collision"));
    g_scenes.push_back(new DamBreak("Dam Break")); 
    g_scenes.push_back(new Cloth("Cloth"));
    g_scenes.push_back(new MultiCloth("Multi Cloth"));
    std::vector<Scene*>::iterator scenePtr;
    for (scenePtr = g_scenes.begin(); scenePtr != g_scenes.end(); ++scenePtr)
    {
        g_context->m_sceneList.push_back((**scenePtr).mName);
    }

    // Load Current Scene
    g_scene = 0;
    g_context->m_reloadScene = false;
    InitializeWithScene(g_scene);

    // Main Loop
    SPDLOG_INFO("Start main loop");
    while (!glfwWindowShouldClose(g_window)) {

        // Change Scene
        if (g_context->m_reloadScene)
        {
            g_hiPhysics->ClearMemory();
            g_hiPhysics.reset();
            g_buffer.reset();
            g_scene = g_context->m_selectedScene;
            InitializeWithScene(g_scene);
            g_pause = true;
            g_context->m_reloadScene = false;
            SPDLOG_INFO("Scene Reload");
        }

        glfwPollEvents(); 
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (g_scenes[g_scene]->mSceneType == StateOfMatter::FLUID)
        {
            if (!g_pause || g_step)
            {
                g_hiPhysics->UpdateSolver(g_buffer);
                g_step = false;
            }

            if (!g_hiPhysics->GetMemory(g_buffer)){
                SPDLOG_ERROR("CUDA : failed to copy device to host.");
                return -1;
            }
        }
        else if (g_scenes[g_scene]->mSceneType == StateOfMatter::CLOTH)
        {
            if (!g_pause || g_step)
            {
                g_hiPhysics->UpdateSolverCloth(g_buffer);
                g_step = false;
            }

            if (!g_hiPhysics->GetMemoryCloth(g_buffer)){
                SPDLOG_ERROR("CUDA : failed to copy device to host.");
                return -1;
            }
        }

        g_context->ProcessInput(g_window);
        g_context->Render();
        
        ImGui::Render(); // Prepare the data for rendering so you can call GetDrawData()
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // render GUI collected after "ImGui::NewFrame();"
        glfwSwapBuffers(g_window);
    }   

    // o ---------------------------------------------------------------------- o
    // |                           QUIT PROGRAM                                 |
    // o ---------------------------------------------------------------------- o

    g_hiPhysics.reset(); //or g_solver = nullptr;
    g_context.reset(); // or g_context = nullptr;
    
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imguiContext);

    glfwTerminate();
    
    g_window = nullptr;

    return 0;
}