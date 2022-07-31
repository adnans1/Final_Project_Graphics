#pragma once
#include "igl/opengl/glfw/Viewer.h"
#include <stdlib.h>


class Project : public igl::opengl::glfw::Viewer
{

public:
	int choosenpoint = -1;
	bool point_choosed = false;
	Project();
	void Init();
	void Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx);
	void Animate() override;
	void ScaleAllShapes(float amt, int viewportIndx);
	int choosed(float point_x, float point_y);
	void set_the_choosed(float point_x, float point_y);
	void Draw(int draw);
	~Project(void);
	void UnPicked();
};