// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2014 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#include "Viewer.h"

//#include <chrono>
#include <thread>

#include <Eigen/LU>


#include <cmath>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <cassert>

#include <igl/project.h>
//#include <igl/get_seconds.h>
#include <igl/readOBJ.h>
#include <igl/readOFF.h>
#include <igl/adjacency_list.h>
#include <igl/writeOBJ.h>
#include <igl/writeOFF.h>
#include <igl/massmatrix.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/quat_mult.h>
#include <igl/axis_angle_to_quat.h>
#include <igl/trackball.h>
#include <igl/two_axis_valuator_fixed_up.h>
#include <igl/snap_to_canonical_view_quat.h>
#include <igl/unproject.h>
#include <igl/serialize.h>
#include "../gl.h"


// Internal global variables used for glfw event handling
//static igl::opengl::glfw::Viewer * __viewer;
static double highdpi = 1;
static double scroll_x = 0;
static double scroll_y = 0;




Eigen::Vector2d* MyShape::get(int part)
{
    int add = part * 3;
    Eigen::Vector2d* res = all_dots + add;
    return res;
}

Eigen::Vector3d MyShape::time_neg()
{
    if (part == 0) {
        time = 0;
        sp = sp * -1;
        position_animate = Eigen::Vector3d(0, 0, 0);
        return position_animate - Eigen::Vector3d(position_animate);
    }

    time = 1;
    part--;


}

void MyShape::time_positive()
{
    if (part != 1) {
        part++;
        time = sp;


    }
    else {
        sp = sp * -1;
        time = 1;
    }
}

MyShape::MyShape(int sh_index, int sh_layer) : index_shape(sh_index), index_mat(2), position_animate(Eigen::Vector3d(0, 0, 0)), shape_layer(sh_layer), choosed(new bool(true)), lag(0), part(0)
{
    point points[] = { point(-0.63,-0.04), point(-0.25,0.34),point(-0.06,0.53),point(0.45,1.64),point(0.07,0.9),point(-0.46,0.16), point(-0.86,0.03) };
    int k = 0;
    for (int i = 0; i < 7; i++) {
        all_dots[i] = Eigen::Vector2d(points[k].get_x(), points[k].get_y());
        k++;

    }
    time = 0;
    sp = 0.009;


}

Eigen::Vector3d MyShape::time_between_one_and_zero1()
{
    double minus = sp;
    Eigen::Vector2d first = shape_bez(time, part);
    Eigen::Vector2d second = shape_bez(time + sp, part);
    Eigen::Vector3d vec(second[0] - first[0], second[1] - first[1], 0);




    if (sp < 0) {
        vec = Eigen::Vector3d(vec[0], -1 * vec[1], 0);
        minus = sp * -1;
    }

    position_animate += vec;
    time += sp;
    if (time > 1 - minus && time < 1)
        time = 1;
    if (time < 0 + minus && time > 0)
        time = 0;
    return vec;
}

Eigen::Vector2d MyShape::shape_bez(double time, int part)
{
    Eigen::Vector2d* Dots1 = get(part);
    point* p1 = new point(time, (1 - time));
    point* p2 = new point(time * time, (1 - time) * (1 - time));
    point* p3 = new point(p1->get_x() * p2->get_x(), p1->get_y() * p2->get_y());
    return p3->get_y() * Dots1[0] + 3 * p2->get_y() * p1->get_x() * Dots1[1] + 3 * p1->get_y() * p2->get_x() * Dots1[2] + p3->get_x() * Dots1[3];
}


Eigen::Vector3d MyShape::steps()
{
    double wait = sp / 4;
    Eigen::Vector3d vec(0, 0, 0);
    if (lag_count <= 0) {
        if (time >= 0 && time <= 1)
            return time_between_one_and_zero1();
        if (time >= 1)
            time_positive();
        else if (time <= 0)
            time_neg();
        return vec;

    }
    else {

        lag_count = lag_count - wait;
        return vec;
    }
}



void MyShape::again()
{
    if (sp < 0) {
        sp = sp * -1;
        part = 0;
        lag_count = lag;
        position_animate = Eigen::Vector3d(0, 0, 0);
    }
    else {
        part = 0;
        lag_count = lag;
        position_animate = Eigen::Vector3d(0, 0, 0);
    }
    time = 0;

}

point::point(double x, double y) : x(x), y(y)
{
}

double point::get_x()
{
    return x;
}

double point::get_y()
{
    return y;
}




namespace igl
{
    namespace opengl
    {
        namespace glfw
        {

            void Viewer::Init(const std::string config)
            {


            }

            IGL_INLINE Viewer::Viewer() :
                data_list(1),
                selected_data_index(0),
                next_data_id(1),
                next_shader_id(1),
                isActive(false)
            {
                data_list.front() = new ViewerData();
                data_list.front()->id = 0;
                staticScene = 0;
                overlay_point_shader = nullptr;
                overlay_shader = nullptr;


                // Temporary variables initialization
               // down = false;
              //  hack_never_moved = true;
                scroll_position = 0.0f;
                SetShader_overlay("shaders/overlay");
                SetShader_point_overlay("shaders/overlay_points");

                // Per face
                data()->set_face_based(false);

                Layers_checkbox.push_back(new bool(true));

                //#ifndef IGL_VIEWER_VIEWER_QUIET
                //    const std::string usage(R"(igl::opengl::glfw::Viewer usage:
                //  [drag]  Rotate scene
                //  A,a     Toggle animation (tight draw loop)
                //  F,f     Toggle face based
                //  I,i     Toggle invert normals
                //  L,l     Toggle wireframe
                //  O,o     Toggle orthographic/perspective projection
                //  T,t     Toggle filled faces
                //  [,]     Toggle between cameras
                //  1,2     Toggle between models
                //  ;       Toggle vertex labels
                //  :       Toggle face labels)"
                //);
                //    std::cout<<usage<<std::endl;
                //#endif
            }

            IGL_INLINE Viewer::~Viewer()
            {
            }
            IGL_INLINE bool
                Viewer::load_mesh_from_data(const Eigen::MatrixXd& V,
                    const Eigen::MatrixXi& F,
                    const Eigen::MatrixXd& UV_V,
                    const Eigen::MatrixXi& UV_F) {
                if (!(data()->F.rows() == 0 && data()->V.rows() == 0))
                {
                    append_mesh();
                }
                data()->clear();
                data()->set_mesh(V, F);
                if (UV_V.rows() > 0)
                {
                    data()->set_uv(UV_V, UV_F);
                }
                else
                {
                    data()->grid_texture();
                }
                data()->compute_normals();
                data()->uniform_colors(Eigen::Vector3d(255.0 / 255.0, 255.0 / 255.0, 0.0 / 255.0),
                    Eigen::Vector3d(255.0 / 255.0, 228.0 / 255.0, 58.0 / 255.0),
                    Eigen::Vector3d(255.0 / 255.0, 235.0 / 255.0, 80.0 / 255.0));
                return true;
            }
            IGL_INLINE bool Viewer::load_mesh_from_file(
                const std::string& mesh_file_name_string)
            {
                bool normal_read = false;
                // Create new data slot and set to selected
                if (!(data()->F.rows() == 0 && data()->V.rows() == 0))
                {
                    append_mesh();
                }
                data()->clear();

                size_t last_dot = mesh_file_name_string.rfind('.');
                if (last_dot == std::string::npos)
                {
                    std::cerr << "Error: No file extension found in " <<
                        mesh_file_name_string << std::endl;
                    return false;
                }

                std::string extension = mesh_file_name_string.substr(last_dot + 1);

                if (extension == "off" || extension == "OFF")
                {
                    Eigen::MatrixXd V;
                    Eigen::MatrixXi F;
                    if (!igl::readOFF(mesh_file_name_string, V, F))
                        return false;
                    data()->set_mesh(V, F);
                }
                else if (extension == "obj" || extension == "OBJ")
                {
                    Eigen::MatrixXd corner_normals;
                    Eigen::MatrixXi fNormIndices;
                    Eigen::MatrixXd UV_V;
                    Eigen::MatrixXi UV_F;
                    Eigen::MatrixXd V, N;
                    Eigen::MatrixXi F;


                    if (!(
                        igl::readOBJ(
                            mesh_file_name_string,
                            V, UV_V, corner_normals, F, UV_F, fNormIndices)))
                    {
                        return false;
                    }
                    else
                        if (corner_normals.rows() > 0)
                        {
                            //std::cout << "normals: \n" << corner_normals << std::endl;
                            //std::cout << "indices: \n" << fNormIndices << std::endl;
                            N = Eigen::RowVector3d(0, 0, 1).replicate(fNormIndices.rows(), 1);
                            for (size_t k = 0; k < N.rows(); k++)
                            {
                                N.row(k) = corner_normals.row(fNormIndices(k, 0));
                                //std::cout << "faces normals:  " << corner_normals.row(fNormIndices(k, 0)) << std::endl;
                            }

                            std::cout << "faces normals: \n" << N << std::endl;

                            normal_read = true;
                        }
                    data()->set_mesh(V, F);
                    if (normal_read)
                        data()->set_normals(N);
                    if (UV_V.rows() > 0)
                    {
                        data()->set_uv(UV_V, UV_F);
                    }

                }
                else
                {
                    // unrecognized file type
                    printf("Error: %s is not a recognized file type.\n", extension.c_str());
                    return false;
                }
                if (!normal_read)
                    data()->compute_normals();
                data()->uniform_colors(Eigen::Vector3d(255.0 / 255.0, 255.0 / 255.0, 0.0 / 255.0),
                    Eigen::Vector3d(255.0 / 255.0, 228.0 / 255.0, 58.0 / 255.0),
                    Eigen::Vector3d(255.0 / 255.0, 235.0 / 255.0, 80.0 / 255.0));

                // Elik: why?
                if (data()->V_uv.rows() == 0)
                {
                    data()->grid_texture();
                }


                //for (unsigned int i = 0; i<plugins.size(); ++i)
                //  if (plugins[i]->post_load())
                //    return true;

                return true;
            }

            IGL_INLINE bool Viewer::save_mesh_to_file(
                const std::string& mesh_file_name_string)
            {
                // first try to load it with a plugin
                //for (unsigned int i = 0; i<plugins.size(); ++i)
                //  if (plugins[i]->save(mesh_file_name_string))
                //    return true;

                size_t last_dot = mesh_file_name_string.rfind('.');
                if (last_dot == std::string::npos)
                {
                    // No file type determined
                    std::cerr << "Error: No file extension found in " <<
                        mesh_file_name_string << std::endl;
                    return false;
                }
                std::string extension = mesh_file_name_string.substr(last_dot + 1);
                if (extension == "off" || extension == "OFF")
                {
                    return igl::writeOFF(
                        mesh_file_name_string, data()->V, data()->F);
                }
                else if (extension == "obj" || extension == "OBJ")
                {
                    Eigen::MatrixXd corner_normals;
                    Eigen::MatrixXi fNormIndices;

                    Eigen::MatrixXd UV_V;
                    Eigen::MatrixXi UV_F;

                    return igl::writeOBJ(mesh_file_name_string,
                        data()->V,
                        data()->F,
                        corner_normals, fNormIndices, UV_V, UV_F);
                }
                else
                {
                    // unrecognized file type
                    printf("Error: %s is not a recognized file type.\n", extension.c_str());
                    return false;
                }
                return true;
            }

            IGL_INLINE bool Viewer::load_scene()
            {
                std::string fname = igl::file_dialog_open();
                if (fname.length() == 0)
                    return false;
                return load_scene(fname);
            }

            IGL_INLINE bool Viewer::load_scene(std::string fname)
            {
                // igl::deserialize(core(),"Core",fname.c_str());
                igl::deserialize(*data(), "Data", fname.c_str());
                return true;
            }

            IGL_INLINE bool Viewer::save_scene()
            {
                std::string fname = igl::file_dialog_save();
                if (fname.length() == 0)
                    return false;
                return save_scene(fname);
            }

            IGL_INLINE bool Viewer::save_scene(std::string fname)
            {
                //igl::serialize(core(),"Core",fname.c_str(),true);
                igl::serialize(data(), "Data", fname.c_str());

                return true;
            }

            IGL_INLINE void Viewer::open_dialog_load_mesh()
            {
                const std::string fname = igl::file_dialog_open();

                if (fname.length() == 0)
                    return;

                this->load_mesh_from_file(fname.c_str());

            }


            IGL_INLINE void Viewer::open_dialog_save_mesh()
            {
                std::string fname = igl::file_dialog_save();

                if (fname.length() == 0)
                    return;

                this->save_mesh_to_file(fname.c_str());
            }

            IGL_INLINE ViewerData* Viewer::data(int mesh_id /*= -1*/)
            {
                assert(!data_list.empty() && "data_list should never be empty");
                int index;
                if (mesh_id == -1)
                    index = selected_data_index;
                else
                    index = mesh_index(mesh_id);

                assert((index >= 0 && index < data_list.size()) &&
                    "selected_data_index or mesh_id should be in bounds");
                return data_list[index];
            }

            IGL_INLINE const ViewerData* Viewer::data(int mesh_id /*= -1*/) const
            {
                assert(!data_list.empty() && "data_list should never be empty");
                int index;
                if (mesh_id == -1)
                    index = selected_data_index;
                else
                    index = mesh_index(mesh_id);

                assert((index >= 0 && index < data_list.size()) &&
                    "selected_data_index or mesh_id should be in bounds");
                return data_list[index];
            }

            IGL_INLINE int Viewer::append_mesh(bool visible /*= true*/)
            {
                assert(data_list.size() >= 1);

                data_list.emplace_back(new ViewerData());
                selected_data_index = data_list.size() - 1;
                data_list.back()->id = next_data_id++;
                //if (visible)
                //    for (int i = 0; i < core_list.size(); i++)
                //        data_list.back().set_visible(true, core_list[i].id);
                //else
                //    data_list.back().is_visible = 0;
                return data_list.back()->id;
            }

            IGL_INLINE bool Viewer::erase_mesh(const size_t index)
            {
                assert((index >= 0 && index < data_list.size()) && "index should be in bounds");
                assert(data_list.size() >= 1);
                if (data_list.size() == 1)
                {
                    // Cannot remove last mesh
                    return false;
                }
                data_list[index]->meshgl.free();
                data_list.erase(data_list.begin() + index);
                if (selected_data_index >= index && selected_data_index > 0)
                {
                    selected_data_index--;
                }

                return true;
            }

            IGL_INLINE size_t Viewer::mesh_index(const int id) const {
                for (size_t i = 0; i < data_list.size(); ++i)
                {
                    if (data_list[i]->id == id)
                        return i;
                }
                return 0;
            }

            Eigen::Matrix4d Viewer::CalcParentsTrans(int indx)
            {
                Eigen::Matrix4d prevTrans = Eigen::Matrix4d::Identity();

                for (int i = indx; parents[i] >= 0; i = parents[i])
                {
                    prevTrans = data_list[parents[i]]->MakeTransd() * prevTrans;
                }

                return prevTrans;
            }


            IGL_INLINE void Viewer::Draw(int shaderIndx, const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, int viewportIndx, unsigned int flgs, unsigned int property_id)
            {

                Eigen::Matrix4f Normal;

                if (!(staticScene & (1 << viewportIndx)))
                    Normal = MakeTransScale();
                else
                    Normal = Eigen::Matrix4f::Identity();

                for (int i = 0; i < data_list.size(); i++)
                {
                    auto shape = data_list[i];
                    if (shape->Is2Render(viewportIndx))
                    {

                        Eigen::Matrix4f Model = shape->MakeTransScale();


                        //change
                        if (shape->passing) {
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        }


                        if (!shape->IsStatic())
                        {

                            Model = Normal * GetPriviousTrans(View.cast<double>(), i).cast<float>() * Model;
                        }
                        else if (parents[i] == -2) {
                            Model = View.inverse() * Model;
                        }
                        if (!(flgs & 65536))
                        {
                            Update(Proj, View, Model, shape->GetShader(), i);
                            // Draw fill
                            if (shape->show_faces & property_id)
                                shape->Draw(shaders[shape->GetShader()], true);
                            if (shape->show_lines & property_id) {
                                glLineWidth(shape->line_width);
                                shape->Draw(shaders[shape->GetShader()], false);
                            }
                            // overlay draws
                            if (shape->show_overlay & property_id) {
                                if (shape->show_overlay_depth & property_id)
                                    glEnable(GL_DEPTH_TEST);
                                else
                                    glDisable(GL_DEPTH_TEST);
                                if (shape->lines.rows() > 0)
                                {
                                    Update_overlay(Proj, View, Model, i, false);
                                    glEnable(GL_LINE_SMOOTH);
                                    shape->Draw_overlay(overlay_shader, false);
                                }
                                if (shape->points.rows() > 0)
                                {
                                    Update_overlay(Proj, View, Model, i, true);
                                    shape->Draw_overlay_pints(overlay_point_shader, false);
                                }
                                glEnable(GL_DEPTH_TEST);
                            }
                        }
                        else
                        { //picking
                            if (flgs & 16384)
                            {   //stencil
                                Eigen::Affine3f scale_mat = Eigen::Affine3f::Identity();
                                scale_mat.scale(Eigen::Vector3f(1.1f, 1.1f, 1.1f));
                                Update(Proj, View, Model * scale_mat.matrix(), 0, i);
                            }
                            else
                            {
                                Update(Proj, View, Model, 0, i);
                            }
                            shape->Draw(shaders[0], true);
                        }
                    }
                }
            }

            int Viewer::AddShader(const std::string& fileName) {
                shaders.push_back(new Shader(fileName, next_data_id));
                next_data_id += 1;
                return (shaders.size() - 1);
            }

            int Viewer::AddShader(const std::string& Vertex_Shader, const std::string& Fragment_shader) {
                shaders.push_back(new Shader(Vertex_Shader, Fragment_shader, false, next_data_id));
                next_data_id += 1;
                return (shaders.size() - 1);
            }

            void Viewer::SetShader_overlay(const std::string& fileName) {
                overlay_shader = new Shader(fileName, true, next_data_id);
                next_data_id += 1;
            }

            void Viewer::SetShader_point_overlay(const std::string& fileName) {
                overlay_point_shader = new Shader(fileName, true, next_data_id);
                next_data_id += 1;
            }



            int Viewer::AddShapeFromFile(const std::string& fileName, int parent, unsigned int mode, int viewport)
            {
                this->load_mesh_from_file(fileName);
                data()->type = MeshCopy;
                data()->mode = mode;
                data()->shaderID = 1;
                data()->viewports = 1 << viewport;
                /*    data()->is_visible = true;*/
                data()->show_lines = 0;
                data()->hide = false;
                data()->show_overlay = 0;
                this->parents.emplace_back(parent);
                return data_list.size() - 1;
            }


            int Viewer::AddShape(int type, int parent, unsigned int mode, int viewport)
            {
                switch (type) {
                    // Axis, Plane, Cube, Octahedron, Tethrahedron, LineCopy, MeshCopy
                case Plane:
                    this->load_mesh_from_file("./data/plane.obj");
                    break;
                case Cube:
                case Axis:
                    this->load_mesh_from_file("./data/cube.obj");
                    break;
                case Octahedron:
                    this->load_mesh_from_file("./data/octahedron.obj");
                    break;
                case Tethrahedron:
                    this->load_mesh_from_file("./data/Tetrahedron.obj");
                    break;
                case Sphere:
                    this->load_mesh_from_file("./data/sphere.obj");
                    break;
                case xCylinder:
                    this->load_mesh_from_file("./data/xcylinder.obj");
                    break;
                case yCylinder:
                    this->load_mesh_from_file("./data/ycylinder.obj");
                    break;
                case zCylinder:
                    this->load_mesh_from_file("./data/zcylinder.obj");
                    break;
                default:
                    break;

                }
                data()->type = type;
                data()->mode = mode;
                data()->shaderID = 1;
                data()->viewports = 1 << viewport;
                //data()->is_visible = 0x1;
                data()->show_lines = 0;
                data()->show_overlay = 0;
                data()->hide = false;
                if (type == Axis) {
                    // data()->is_visible = 0;
                    data()->show_faces = 0;
                    data()->show_lines = 0;
                    data()->show_overlay = 0xFF;
                }

                this->parents.emplace_back(parent);
                return data_list.size() - 1;
            }



            int Viewer::AddShapeCopy(int shpIndx, int parent, unsigned int mode, int viewport)
            {
                load_mesh_from_data(data_list[shpIndx]->V, data_list[shpIndx]->F, data_list[shpIndx]->V_uv, data_list[shpIndx]->F_uv);
                data()->type = data_list[shpIndx]->type;
                data()->mode = mode;
                data()->shaderID = data_list[shpIndx]->shaderID;
                data()->viewports = 1 << viewport;
                //data()->is_visible = true;
                data()->show_lines = 0;
                data()->show_overlay = 0;
                data()->hide = false;
                this->parents.emplace_back(parent);
                return data_list.size() - 1;
            }

            int Viewer::AddShapeFromData(const Eigen::MatrixXd& V,
                const Eigen::MatrixXi& F,
                const Eigen::MatrixXd& UV_V,
                const Eigen::MatrixXi& UV_F
                , int type, int parent, unsigned int mode, int viewport)
            {
                load_mesh_from_data(V, F, UV_V, UV_F);
                data()->type = type;
                data()->mode = mode;
                data()->shaderID = 1;
                data()->viewports = 1 << viewport;
                // data()->is_visible = true;
                data()->show_lines = 0;
                data()->show_overlay = 0;
                data()->hide = false;
                this->parents.emplace_back(parent);
                return data_list.size() - 1;
            }

            void Viewer::ClearPickedShapes(int viewportIndx)
            {
                for (int pShape : pShapes)
                {
                    data_list[pShape]->RemoveViewport(viewportIndx);
                }
                selected_data_index = 0;
                pShapes.clear();
            }

            //return coordinates in global system for a tip of arm position is local system
            void Viewer::MouseProccessing(int button, int xrel, int yrel, float movCoeff, Eigen::Matrix4d cameraMat, int viewportIndx)
            {
                Eigen::Matrix4d scnMat = Eigen::Matrix4d::Identity();
                if (selected_data_index <= 0 && !(staticScene & (1 << viewportIndx)))
                    scnMat = MakeTransd().inverse();
                else if (!(staticScene & (1 << viewportIndx)))
                    scnMat = (MakeTransd() * GetPriviousTrans(Eigen::Matrix4d::Identity(), selected_data_index)).inverse();
                else if (selected_data_index > 0)
                    scnMat = (GetPriviousTrans(Eigen::Matrix4d::Identity(), selected_data_index)).inverse();

                if (button == 1)
                {
                    //  for (int pShape : pShapes)
                    //  {
                    //      selected_data_index = pShape;
                    WhenTranslate(scnMat * cameraMat, -xrel / 80.0, yrel / 80.0);
                    //  }

                }
                else
                {
                    movCoeff = 2.0f;

                    if (button == 0)
                    {
                        //            if (selected_data_index > 0 )
                        WhenRotate(scnMat * cameraMat, -((float)xrel / 180) / movCoeff, -((float)yrel / 180) / movCoeff);
                    }
                    else
                    {
                        for (int pShape : pShapes)
                        {
                            selected_data_index = pShape;
                            WhenScroll(scnMat * cameraMat, yrel / movCoeff);
                        }
                    }
                }
            }

            void Viewer::ShapeTransformation(int type, float amt, int mode)
            {
                if (abs(amt) > 1e-5 && selected_data_index >= 0 && !data()->IsStatic())
                {
                    switch (type)
                    {
                    case xTranslate:
                        data()->MyTranslate(Eigen::Vector3d(amt, 0, 0), mode);
                        break;
                    case yTranslate:
                        data()->MyTranslate(Eigen::Vector3d(0, amt, 0), mode);
                        break;
                    case zTranslate:
                        data()->MyTranslate(Eigen::Vector3d(0, 0, amt), mode);
                        break;
                    case xRotate:
                        data()->MyRotate(Eigen::Vector3d(1, 0, 0), amt, mode);
                        break;
                    case yRotate:
                        data()->MyRotate(Eigen::Vector3d(0, 1, 0), amt, mode);
                        break;
                    case zRotate:
                        data()->MyRotate(Eigen::Vector3d(0, 0, 1), amt, mode);
                        break;
                    case xScale:
                        data()->MyScale(Eigen::Vector3d(amt, 1, 1));
                        break;
                    case yScale:
                        data()->MyScale(Eigen::Vector3d(1, amt, 1));
                        break;
                    case zScale:
                        data()->MyScale(Eigen::Vector3d(1, 1, amt));
                        break;
                    case scaleAll:
                        data()->MyScale(Eigen::Vector3d(amt, amt, amt));
                        break;
                    case reset:
                        data()->ZeroTrans();
                        break;
                    default:
                        break;
                    }
                }

            }

            bool Viewer::Picking(unsigned char data[4], int newViewportIndx)
            {
                if (data[0] >= 4 && data[0] < MyShapes_map.size() + 3) {
                    return true;
                }
                else {
                    return false;
                }
            }

            void Viewer::WhenTranslate(const Eigen::Matrix4d& preMat, float dx, float dy)
            {
                for (int i = 0; i < Myshapes_checkbox.size(); i++) {

                    if (*(Myshapes_checkbox[i])) {

                        int x = MyShapes_map[i].index_shape;
                        ViewerData* viewerdata = data_list[x];

                        viewerdata->MyTranslate(Eigen::Vector3d(dx, dy, 0), 1);

                    }
                }

            }


            void Viewer::WhenScroll(const Eigen::Matrix4d& preMat, float dy)
            {
                if (selected_data_index == 0 || data()->IsStatic())
                    this->TranslateInSystem(preMat.block<3, 3>(0, 0), Eigen::Vector3d(0, 0, dy));
                else if (selected_data_index > 0)
                    data()->TranslateInSystem(preMat.block<3, 3>(0, 0), Eigen::Vector3d(0, 0, dy));
                WhenScroll(dy);
            }

            int Viewer::AddMaterial(unsigned int texIndices[], unsigned int slots[], unsigned int size)
            {

                materials.push_back(new Material(texIndices, slots, size));
                return (materials.size() - 1);
            }



            Eigen::Matrix4d Viewer::GetPriviousTrans(const Eigen::Matrix4d& View, unsigned int index)
            {
                Eigen::Matrix4d Model = Eigen::Matrix4d::Identity();
                int p = index >= 0 ? parents[index] : -1;
                for (; p >= 0; p = parents[p])
                    Model = data_list[p]->MakeTransd() * Model;
                if (p == -2)
                    return  View.inverse() * Model;
                else
                    return Model;
            }

            float Viewer::AddPickedShapes(const Eigen::Matrix4d& PV, const Eigen::Vector4i& viewport, int viewportIndx, int left, int right, int up, int bottom, int newViewportIndx)
            {
                //not correct when the shape is scaled
                Eigen::Matrix4d MVP = PV * MakeTransd();
                std::cout << "picked shapes  ";
                bool isFound = false;
                for (int i = 1; i < data_list.size(); i++)
                { //add to pShapes if the center in range
                    Eigen::Matrix4d Model = data_list[i]->MakeTransd();
                    Model = CalcParentsTrans(i) * Model;
                    Eigen::Vector4d pos = MVP * Model * Eigen::Vector4d(0, 0, 0, 1);
                    float xpix = (1 + pos.x() / pos.z()) * viewport.z() / 2;
                    float ypix = (1 + pos.y() / pos.z()) * viewport.w() / 2;
                    if (data_list[i]->Is2Render(viewportIndx) && xpix < right && xpix > left && ypix < bottom && ypix > up)
                    {
                        pShapes.push_back(i);
                        data_list[i]->AddViewport(newViewportIndx);
                        std::cout << i << ", ";
                        selected_data_index = i;
                        isFound = true;
                    }
                }
                std::cout << std::endl;
                if (isFound)
                {
                    Eigen::Vector4d tmp = MVP * GetPriviousTrans(Eigen::Matrix4d::Identity(), selected_data_index) * data()->MakeTransd() * Eigen::Vector4d(0, 0, 1, 1);
                    return (float)tmp.z();
                }
                else
                    return 0;
            }











            int Viewer::AddTexture(const std::string& textureFileName, int dim)
            {

                string name_file = textureFileName.substr(textureFileName.find_last_of("/\\") + 1);
                name_file = name_file.substr(0, name_file.find('.'));
                all_material.push_back(name_file);

                textures.push_back(new Texture(textureFileName, dim));
                return(textures.size() - 1);
            }



            void Viewer::BindMaterial(Shader* s, unsigned int materialIndx, bool isfalse = false)
            {


                if (isfalse) {
                    auto& textures1 = newtex;
                    auto& materials1 = newMat;

                    for (int i = 0; i < materials1[materialIndx]->GetNumOfTexs(); i++)
                    {
                        materials1[materialIndx]->Bind(textures1, i);
                        s->SetUniform1i("sampler" + std::to_string(i + 1), materials1[materialIndx]->GetSlot(i));
                    }
                }
                else {
                    auto& textures1 = textures;
                    auto& materials1 = materials;

                    for (int i = 0; i < materials1[materialIndx]->GetNumOfTexs(); i++)
                    {
                        materials1[materialIndx]->Bind(textures1, i);
                        s->SetUniform1i("sampler" + std::to_string(i + 1), materials1[materialIndx]->GetSlot(i));
                    }

                }

            }

            int Viewer::AddTexture(int width, int height, unsigned char* data, int mode)
            {
                textures.push_back(new Texture(width, height));

                if (mode)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                }
                else
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); //note GL_RED internal format, to save memory.
                }
                glBindTexture(GL_TEXTURE_2D, 0);
                return(textures.size() - 1);
            }

            void Viewer::Update_overlay(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int shapeIndx, bool is_points) {
                auto data = data_list[shapeIndx];
                Shader* s = is_points ? overlay_point_shader : overlay_shader;
                if (s != nullptr) {
                    s->Bind();
                    s->SetUniformMat4f("Proj", Proj);
                    s->SetUniformMat4f("View", View);
                    s->SetUniformMat4f("Model", Model);
                }
            }


            void Viewer::SetParent(int indx, int newValue, bool savePosition)
            {
                parents[indx] = newValue;
                if (savePosition)
                {
                    Eigen::Vector4d tmp = data_list[newValue]->MakeTransd() * (data_list[indx]->MakeTransd()).inverse() * Eigen::Vector4d(0, 0, 0, 1);
                    data_list[indx]->ZeroTrans();
                    data_list[indx]->MyTranslate(-tmp.head<3>(), false);
                }
            }




            void Viewer::add_shape_file(std::string nameofshape) {
                int savedIndx = selected_data_index;

                const std::string fname = igl::file_dialog_open();
                if (data_list.size() > parents.size())
                {

                    parents.push_back(-1);
                    SetShapeViewport(selected_data_index, 0);

                    SetShapeMaterial(selected_data_index, 0);

                    SetShapeShader(selected_data_index, 2);

                    data_list.back()->UnHide();

                    data_list.back()->show_faces = 3;
                    data()->mode = TRIANGLES;
                    selected_data_index = savedIndx;
                }

                this->load_mesh_from_file(fname);
                data()->mode = TRIANGLES;

                data()->viewports = 1 << 0;
                data()->show_lines = 0;

                data()->shaderID = 1;
                data()->hide = false;

                data()->show_overlay = 0;

                int shapeIdx = data_list.size();

                shapeIdx = shapeIdx - (size_t)1;

                this->parents.emplace_back(-1);


                SetShapeMaterial(shapeIdx, 2);

                SetShapeShader(shapeIdx, 3);

                MyShapes_map.push_back(MyShape(shapeIdx, idLay));

                MyShape x = MyShapes_map[MyShapes_map.size() - 1];
                bool* isa = x.choosed;
                Myshapes_checkbox.push_back(isa);


                allnames_myshape.push_back(nameofshape);

                for (int i = 0; i < MyShapes_map.size() - 1; i++) {
                    *(MyShapes_map[i].choosed) = false;
                }


                OptionChange(3);

            }

            void Viewer::add_matfromfile()
            {
                const std::string fname = igl::file_dialog_open();

                if (fname.length() == 0)
                    return;



                string name_file = fname.substr(fname.find_last_of("/\\") + 1);
                name_file = name_file.substr(0, name_file.find('.'));
                all_material.push_back(name_file);

                textures.push_back(new Texture(fname, 2));

                unsigned int slots[1];


                unsigned int texIndices[1];


                slots[0] = textures.size() - 1;

                texIndices[0] = textures.size() - 1;


                materials.push_back(new Material(texIndices, slots, 1));

            }


            void Viewer::OptionChange(int x) {
                if (x == 1) {
                    if (chooseshapeind != -1) {
                        int x = MyShapes_map[chooseshapeind].index_shape;
                        SetShapeMaterial(x, idMat);
                        MyShapes_map[chooseshapeind].index_mat = idMat;
                    }
                }
                else if (x == 2) {
                    if (chooseshapeind == -1) {

                    }
                    else {
                        MyShapes_map[chooseshapeind].lag = dV;
                    }
                }
                else if (x == 3)
                {
                    changingdots = true;
                    int notclick = -1;
                    for (int i = 0; i < MyShapes_map.size(); i++) {
                        if (*(MyShapes_map[i].choosed)) {
                            if (notclick != -1) {
                                chooseshapeind = -1;
                                return;
                            }
                            else {
                                notclick = i;
                            }
                        }
                    }
                    if ((notclick == -1)) {
                        choose = false;
                    }
                    else {
                        choose = true;
                    }
                    chooseshapeind = notclick;

                    if (chooseshapeind != -1) {
                        idMat = MyShapes_map[chooseshapeind].index_mat;
                        idLay = MyShapes_map[chooseshapeind].shape_layer;
                        dV = MyShapes_map[chooseshapeind].lag;
                    }
                }
                else if (x == 4) {
                    if (chooseshapeind != -1) {
                        MyShapes_map[chooseshapeind].sp = speedval;
                    }
                }
            }


            void Viewer::setspeed(float x) {
                speedval = x;
            }

            int Viewer::TexAdd(const std::string& texname)
            {
                string name_file = texname.substr(texname.find_last_of("/\\") + 1);

                name_file = name_file.substr(0, name_file.find('.'));

                all_background.push_back(name_file);


                Texture* x = new Texture(texname, 3);

                int lentex = newtex.size();

                newtex.push_back(x);

                lentex = lentex - 1;

                return lentex;
            }

            void Viewer::WhenRotate(const Eigen::Matrix4d& preMat, float dx, float dy)
            {
                int sizeofshape = Myshapes_checkbox.size();
                ViewerData* obj;
                for (int i = 0; i < sizeofshape; i++) {
                    bool* isornot = Myshapes_checkbox[i];
                    if (isornot) {
                        obj = data_list[MyShapes_map[i].index_shape];

                        obj->RotateInSystem(Eigen::Vector3d(1, 0, 0), dy);


                        obj->RotateInSystem(Eigen::Vector3d(0, 1, 0), dx);
                    }
                }
            }


            int Viewer::MatAdd(unsigned int texIndices[], unsigned int slots[], unsigned int size)
            {
                newMat.push_back(new Material(texIndices, slots, size));

                int sizemat = newMat.size();

                sizemat = sizemat - 1;

                return sizemat;
            }

        } // end namespace
    } // end namespace
}
