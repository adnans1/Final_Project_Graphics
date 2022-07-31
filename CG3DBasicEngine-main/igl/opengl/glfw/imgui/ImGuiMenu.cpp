// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2018 Jérémie Dumas <jeremie.dumas@ens-lyon.org>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
////////////////////////////////////////////////////////////////////////////////
//#include "ImGuiMenu.h"
//#include "ImGuiHelpers.h"
#include <igl/project.h>
#include "ImGuiHelpers.h"

#include "ImGuiMenu.h"
#include "../imgui.h"
#include "igl/opengl/glfw/imgui/imgui_impl_glfw.h"
#include "igl/opengl/glfw/imgui/imgui_impl_opengl3.h"

//#include <imgui_fonts_droid_sans.h>
//#include <GLFW/glfw3.h>
#include <iostream>
////////////////////////////////////////////////////////////////////////////////

namespace igl
{
    namespace opengl
    {
        namespace glfw
        {
            namespace imgui
            {

                IGL_INLINE void ImGuiMenu::init(Display* disp)
                {
                    // Setup ImGui binding
                    if (disp->window)
                    {
                        IMGUI_CHECKVERSION();
                        if (!context_)
                        {
                            // Single global context by default, but can be overridden by the user
                            static ImGuiContext* __global_context = ImGui::CreateContext();
                            context_ = __global_context;
                        }
                        const char* glsl_version = "#version 150";

                        ImGui_ImplGlfw_InitForOpenGL(disp->window, true);
                        ImGui_ImplOpenGL3_Init(glsl_version);
                        ImGui::GetIO().IniFilename = nullptr;
                        ImGui::StyleColorsDark();
                        ImGuiStyle& style = ImGui::GetStyle();
                        style.FrameRounding = 5.0f;
                        reload_font();
                    }

                    imgui_l.push_back("Layer 1");
                }

                IGL_INLINE void ImGuiMenu::reload_font(int font_size)
                {
                    hidpi_scaling_ = hidpi_scaling();
                    pixel_ratio_ = pixel_ratio();
                    ImGuiIO& io = ImGui::GetIO();
                    io.Fonts->Clear();
                    // io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data,
                    //   droid_sans_compressed_size, font_size * hidpi_scaling_);
                    io.FontGlobalScale = 1.0 / pixel_ratio_;
                }

                IGL_INLINE void ImGuiMenu::shutdown()
                {
                    // Cleanup
                    ImGui_ImplOpenGL3_Shutdown();
                    ImGui_ImplGlfw_Shutdown();
                    // User is responsible for destroying context if a custom context is given
                    // ImGui::DestroyContext(*context_);
                }

                IGL_INLINE bool ImGuiMenu::pre_draw()
                {
                    glfwPollEvents();

                    // Check whether window dpi has changed
                    float scaling = hidpi_scaling();
                    if (std::abs(scaling - hidpi_scaling_) > 1e-5)
                    {
                        reload_font();
                        ImGui_ImplOpenGL3_DestroyDeviceObjects();
                    }

                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();
                    return false;
                }

                IGL_INLINE bool ImGuiMenu::post_draw() {
                    //draw_menu(viewer,core);
                    ImGui::Render();
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                    return false;
                }

                IGL_INLINE void ImGuiMenu::post_resize(int width, int height)
                {
                    if (context_)
                    {
                        ImGui::GetIO().DisplaySize.x = float(width);
                        ImGui::GetIO().DisplaySize.y = float(height);
                    }
                }

                // Mouse IO
                IGL_INLINE bool ImGuiMenu::mouse_down(GLFWwindow* window, int button, int modifier)
                {
                    ImGui_ImplGlfw_MouseButtonCallback(window, button, GLFW_PRESS, modifier);
                    return ImGui::GetIO().WantCaptureMouse;
                }

                IGL_INLINE bool ImGuiMenu::mouse_up(GLFWwindow* window, int button, int modifier)
                {
                    //return ImGui::GetIO().WantCaptureMouse;
                    // !! Should not steal mouse up
                    return false;
                }

                IGL_INLINE bool ImGuiMenu::mouse_move(GLFWwindow* window, int mouse_x, int mouse_y)
                {
                    return ImGui::GetIO().WantCaptureMouse;
                }

                IGL_INLINE bool ImGuiMenu::mouse_scroll(GLFWwindow* window, float delta_y)
                {
                    ImGui_ImplGlfw_ScrollCallback(window, 0.f, delta_y);
                    return ImGui::GetIO().WantCaptureMouse;
                }

                // Keyboard IO
                IGL_INLINE bool ImGuiMenu::key_pressed(GLFWwindow* window, unsigned int key, int modifiers)
                {
                    ImGui_ImplGlfw_CharCallback(nullptr, key);
                    return ImGui::GetIO().WantCaptureKeyboard;
                }

                IGL_INLINE bool ImGuiMenu::key_down(GLFWwindow* window, int key, int modifiers)
                {
                    ImGui_ImplGlfw_KeyCallback(window, key, 0, GLFW_PRESS, modifiers);
                    return ImGui::GetIO().WantCaptureKeyboard;
                }

                IGL_INLINE bool ImGuiMenu::key_up(GLFWwindow* window, int key, int modifiers)
                {
                    ImGui_ImplGlfw_KeyCallback(window, key, 0, GLFW_RELEASE, modifiers);
                    return ImGui::GetIO().WantCaptureKeyboard;
                }



                IGL_INLINE void ImGuiMenu::draw_viewer_menu(igl::opengl::glfw::Viewer* viewer, std::vector<igl::opengl::Camera*>& camera, Eigen::Vector4i& viewWindow, std::vector<DrawInfo*> drawInfos)
                {
                    bool* p_open = NULL;
                    static bool no_titlebar = false;
                    static bool no_scrollbar = false;
                    static bool no_menu = true;
                    static bool no_move = false;
                    static bool no_resize = false;
                    static bool no_collapse = false;
                    static bool no_close = false;
                    static bool no_nav = false;
                    static bool no_background = false;
                    static bool no_bring_to_front = false;

                    ImGuiWindowFlags window_flags = 0;
                    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
                    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
                    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
                    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
                    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
                    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
                    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
                    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
                    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

                    ImGui::Begin(
                        "Viewer", p_open,
                        window_flags
                    );

                    ImGui::SetWindowPos(ImVec2((float)0, (float)0), ImGuiCond_Always);
                    ImGui::SetWindowSize(ImVec2((float)0, (float)0), ImGuiCond_Always);
                    ImGui::End();
                    no_move = true;
                    no_resize = true;

                    ImGui::Begin(
                        "Viewer", p_open,
                        window_flags
                    );


                    if (ImGui::CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        float w = ImGui::GetContentRegionAvailWidth();
                        float p = ImGui::GetStyle().FramePadding.x;


                        if (ImGui::CollapsingHeader("Name Shape : ", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::InputText("##Nameobj", viewer->name_obj, 30);
                        }

                        if (ImGui::CollapsingHeader("Layer Shape : ", ImGuiTreeNodeFlags_DefaultOpen))
                        {

                            if (ImGui::BeginCombo("##shape layer combo", layer_now.c_str())) {
                                for (int i = 0; i < imgui_l.size(); i++) {
                                    bool isSelected = layer_now == imgui_l[i];

                                    if (ImGui::Selectable(imgui_l[i].c_str(), isSelected)) {
                                        viewer->idLay = i;
                                        layer_now = imgui_l[i];
                                    }
                                    if (isSelected) {
                                        ImGui::SetItemDefaultFocus();
                                    }
                                }

                                ImGui::EndCombo();

                            }
                        }


                        if (ImGui::Button("Add Shape##Mesh", ImVec2((w - p) / 2.f, 0)))
                        {
                            auto stringname_obj = std::string(viewer->name_obj);
                            viewer->add_shape_file(stringname_obj);
                            viewer->name_obj = strdup("");

                        }


                        if (ImGui::CollapsingHeader("All Shapes##Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
                            float w_i = ImGui::GetContentRegionAvailWidth();
                            float p_i = ImGui::GetStyle().FramePadding.x;

                            for (int i = 0; i < viewer->Myshapes_checkbox.size(); i++) {
                                std::string shape_checkbox = "";
                                shape_checkbox = viewer->allnames_myshape[i];

                                if (ImGui::Checkbox(shape_checkbox.c_str(), viewer->Myshapes_checkbox[i])) {
                                    viewer->OptionChange(3);
                                }
                            }
                        }
                    }

                    if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen)) {
                        float w = ImGui::GetContentRegionAvailWidth();
                        float p = ImGui::GetStyle().FramePadding.x;

                        if (ImGui::CollapsingHeader("Name layer : ", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::InputText("##Namelayer", viewer->name_layer, 30);
                        }


                        if (ImGui::Button("Add##Layers", ImVec2((w - p), 0))) {
                            viewer->Layers_checkbox.push_back(new bool(true));
                            auto stringname_layer = std::string(viewer->name_layer);
                            imgui_l.push_back(stringname_layer);
                        }

                        for (int i = 0; i < viewer->Layers_checkbox.size(); i++) {

                            if (ImGui::Checkbox(imgui_l[i].c_str(), viewer->Layers_checkbox[i])) {
                                for (int j = 0; j < viewer->MyShapes_map.size(); j++) {
                                    MyShape& s = viewer->MyShapes_map[j];
                                    if (s.shape_layer == i) {
                                        viewer->data_list[s.index_shape]->hide = !(*(viewer->Layers_checkbox[i]));
                                    }
                                }
                            }
                        }
                    }


                    if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        float w = ImGui::GetContentRegionAvailWidth();
                        float p = ImGui::GetStyle().FramePadding.x;


                        if (ImGui::BeginCombo("##mat combo", viewer->all_material[viewer->idMat].c_str())) {


                            for (int i = 0; i < viewer->all_material.size(); i++) {

                                bool isSelected = viewer->all_material[viewer->idMat] == viewer->all_material[i];

                                if (ImGui::Selectable(viewer->all_material[i].c_str(), isSelected)) {
                                    viewer->idMat = i;
                                    viewer->OptionChange(1);
                                }
                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();

                        }

                        if (ImGui::Button("Add##Materials", ImVec2((w - p), 0)))
                        {
                            viewer->add_matfromfile();
                        }
                    }
                    if (ImGui::CollapsingHeader("Background", ImGuiTreeNodeFlags_DefaultOpen))
                    {

                        if (ImGui::BeginCombo("##background shader combo", background_now.c_str())) {


                            for (int i = 0; i < viewer->all_background.size(); i++) {

                                bool isSelected = background_now == viewer->all_background[i];

                                if (ImGui::Selectable(viewer->all_background[i].c_str(), isSelected)) {
                                    viewer->idGround = i;
                                    background_now = viewer->all_background[i];
                                    viewer->MatSet(0, viewer->idGround);
                                }
                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();

                        }
                    }



                    ///change
                    if (ImGui::CollapsingHeader("Transparent", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        float w = ImGui::GetContentRegionAvailWidth();
                        float p = ImGui::GetStyle().FramePadding.x;


                        if (ImGui::BeginCombo("##transparent shader combo", (std::to_string(viewer->Percent * 100).c_str())))
                        {
                            for (int i = 0; i < viewer->all_100.size(); i++) {

                                bool isSelected = (std::to_string(viewer->Percent * 100).c_str()) == viewer->all_100[i];

                                if (ImGui::Selectable(viewer->all_100[i].c_str(), isSelected)) {
                                    viewer->Percent = std::stof(viewer->all_100[i]) / 100.0;

                                    if (viewer->chooseshapeind != -1) {
                                        viewer->data_list[viewer->MyShapes_map[viewer->chooseshapeind].index_shape]->TranperentPercent = viewer->Percent;
                                    }
                                }
                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();

                        }

                        if (ImGui::Button("transperent ##Tranperent", ImVec2((w - p), 0))) {
                            if (viewer->chooseshapeind != -1) {


                                viewer->data_list[viewer->MyShapes_map[viewer->chooseshapeind].index_shape]->passing = !viewer->data_list[viewer->MyShapes_map[viewer->chooseshapeind].index_shape]->passing;


                                viewer->data_list[viewer->MyShapes_map[viewer->chooseshapeind].index_shape]->TranperentPercent = viewer->Percent;
                            }
                        }
                    }


                    // Viewing options
                    if (ImGui::CollapsingHeader("Viewing Options", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (ImGui::Button("Center object", ImVec2(-1, 0)))
                        {
                            std::cout << "not implemented yet" << std::endl;
                            //      core[1].align_camera_center(viewer->data().V, viewer->data().F); TODO: add function like this to camera
                        }
                        //if (ImGui::Button("Snap canonical view", ImVec2(-1, 0)))
                        //{
                        //  core[1].snap_to_canonical_quaternion();
                        //}

                        // Zoom
                        ImGui::PushItemWidth(80 * menu_scaling());
                        if (camera[0]->_ortho)
                            ImGui::DragFloat("Zoom", &(camera[0]->_length), 0.05f, 0.1f, 20.0f);
                        else
                            ImGui::DragFloat("Fov", &(camera[0]->_fov), 0.05f, 30.0f, 90.0f);

                        // Select rotation type
                        static Eigen::Quaternionf trackball_angle = Eigen::Quaternionf::Identity();
                        static bool orthographic = true;

                        // Orthographic view
                        ImGui::Checkbox("Orthographic view", &(camera[0]->_ortho));
                        if (camera[0]->_ortho) {
                            camera[0]->SetProjection(0, camera[0]->_relationWH);
                        }
                        else {
                            camera[0]->SetProjection(camera[0]->_fov > 0 ? camera[0]->_fov : 45, camera[0]->_relationWH);
                        }

                        ImGui::PopItemWidth();
                    }

                    // Helper for setting viewport specific mesh options
                    auto make_checkbox = [&](const char* label, unsigned int& option)
                    {
                        return ImGui::Checkbox(label,
                            [&]() { return drawInfos[1]->is_set(option); },
                            [&](bool value) { return drawInfos[1]->set(option, value); }
                        );
                    };
                    ImGui::ColorEdit4("Background", drawInfos[1]->Clear_RGBA.data(),
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);

                    // Draw options
                    if (ImGui::CollapsingHeader("Draw Options", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (ImGui::Checkbox("Face-based", &(viewer->data()->face_based)))
                        {
                            viewer->data()->dirty = MeshGL::DIRTY_ALL;
                        }
                        //
                        //    make_checkbox("Show texture", viewer->data().show_texture);
                        //    if (ImGui::Checkbox("Invert normals", &(viewer->data().invert_normals)))
                        //    {
                        //      viewer->data().dirty |= igl::opengl::MeshGL::DIRTY_NORMAL;
                        //    }
                        make_checkbox("Show overlay", viewer->data()->show_overlay);
                        make_checkbox("Show overlay depth", viewer->data()->show_overlay_depth);

                        ImGui::ColorEdit4("Line color", viewer->data()->line_color.data(),
                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
                        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
                        ImGui::DragFloat("Shininess", &(viewer->data()->shininess), 0.05f, 0.0f, 100.0f);
                        ImGui::PopItemWidth();
                    }

                    // Overlays
                    if (ImGui::CollapsingHeader("Overlays", ImGuiTreeNodeFlags_None))
                    {
                        make_checkbox("Wireframe", viewer->data()->show_lines);
                        make_checkbox("Fill", viewer->data()->show_faces);

                    }


                    if (ImGui::CollapsingHeader("animate##Mesh animate", ImGuiTreeNodeFlags_DefaultOpen)) {

                        char* startorpause = "START";
                        if (viewer->IsActive()) {
                            startorpause = "PAUSE";
                        }

                        if (ImGui::CollapsingHeader("time : ", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::InputText("##time", time_now, 30);

                        }

                        if (ImGui::CollapsingHeader("speed : ", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::InputText("##speed", speed_now, 30);

                        }

                        if (ImGui::Button("Set Time", ImVec2(-1, 0)))
                        {
                            viewer->dV = std::stof(time_now) / 10;
                            viewer->OptionChange(2);
                        }

                        if (ImGui::Button("Set Speed", ImVec2(-1, 0)))
                        {
                            viewer->setspeed(std::stof(speed_now) / 1000);
                            viewer->OptionChange(4);
                        }


                        if (ImGui::Button(startorpause, ImVec2(-1, 0)))
                        {

                            if (!viewer->IsActive()) {
                                viewer->Activate();
                            }
                            else {
                                viewer->Deactivate();
                            }
                        }


                    }
                    ImGui::End();
                }

                IGL_INLINE float ImGuiMenu::pixel_ratio()
                {
                    // Computes pixel ratio for hidpi devices
                    int buf_size[2];
                    int win_size[2];
                    GLFWwindow* window = glfwGetCurrentContext();
                    glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
                    glfwGetWindowSize(window, &win_size[0], &win_size[1]);
                    return (float)buf_size[0] / (float)win_size[0];
                }

                IGL_INLINE float ImGuiMenu::hidpi_scaling()
                {
                    // Computes scaling factor for hidpi devices
                    float xscale, yscale;
                    GLFWwindow* window = glfwGetCurrentContext();
                    glfwGetWindowContentScale(window, &xscale, &yscale);
                    return 0.5 * (xscale + yscale);
                }

            } // end namespace
        } // end namespace
    } // end namespace
} // end namespace
