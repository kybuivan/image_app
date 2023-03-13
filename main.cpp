#include <stdio.h>
#include <fstream>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#include <glad/gl.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <rttr/type>
#include <nlohmann/json.hpp>
#include <entt/entity/registry.hpp>
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

using json = nlohmann::json;

// Forward declarations
class Component;
class Entity;
class System;


class Component {
    RTTR_ENABLE();

  public:
    virtual ~Component() = default;
    [[nodiscard]] rttr::type type() const;

    virtual void serialize(json& j);
    virtual void deserialize(const json& j);
    virtual void renderEditor() {}
    virtual void on_inspector();

    virtual void init(Entity *entity) {}
    virtual void destroy() {}
};

// Entity class
class Entity {
public:
    Entity() {}
    ~Entity() {}

    void addComponent(Component* component) {
        m_components.push_back(component);
    }

    void removeComponent(Component* component) {
        auto it = std::find(m_components.begin(), m_components.end(), component);
        if (it != m_components.end()) {
            m_components.erase(it);
        }
    }

    void serialize(json& j) {
        for (auto component : m_components) {
            json componentJson;
            component->serialize(componentJson);
            j["components"].push_back(componentJson);
        }
    }

    void deserialize(const json& j) {
        for (auto& componentJson : j["components"]) {
            const std::string typeName = componentJson["type"];
            auto type = rttr::type::get_by_name(typeName);
            if (type.is_valid()) {
                Component* component = type.create().get_value<Component*>();
                component->deserialize(componentJson);
                addComponent(component);
            }
        }
    }

    void renderEditor() {
        ImGui::Text("Entity %d", m_id);
        ImGui::Separator();
        for (auto component : m_components) {
            component->renderEditor();
        }
    }

    int m_id;
    std::vector<Component*> m_components;
};

// System class
class System {
public:
    virtual ~System() {}
    virtual void update() {}
};

// Position component
class Position : public Component {
public:
    RTTR_ENABLE(Component)
    Position() : x(0.0f), y(0.0f) {}
    Position(float x, float y) : x(x), y(y) {}

    void serialize(json& j) override {
        j["type"] = "Position";
        j["x"] = x;
        j["y"] = y;
    }

    void deserialize(const json& j) override {
        x = j["x"];
        y = j["y"];
    }

    void renderEditor() override {
        ImGui::Text("Position");
        ImGui::InputFloat("x", &x);
        ImGui::InputFloat("y", &y);
    }

    float x;
    float y;
};

int main(int, char**)
{
    // read a JSON file
    std::ifstream i("file.json");
    nlohmann::json j;
    i >> j;

    // Setup window
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}