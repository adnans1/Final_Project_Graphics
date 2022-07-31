#include "Project.h"
#include <iostream>


static void printMat(const Eigen::Matrix4d& mat)
{
	std::cout << " matrix:" << std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout << mat(j, i) << " ";
		std::cout << std::endl;
	}
}

Project::Project() {
}

//Project::Project(float angle ,float relationWH, float near, float far) : Scene(angle,relationWH,near,far)
//{ 	
//}

void Project::Init()
{

	unsigned int texIDs[5] = { 0 , 1, 2, 3, 4 };
	unsigned int slots[5] = { 0 , 1, 2 ,3, 4 };

	AddShader("shaders/pickingShader");
	AddShader("shaders/NewShader");
	AddShader("shaders/basicShaderTex");
	AddShader("shaders/basicShader");
	AddShader("shaders/cubemapShader");


	AddTexture("textures/bricks.jpg.bmp", 2);
	AddTexture("textures/grass.bmp", 2);
	AddTexture("textures/plane.png", 2);
	AddTexture("textures/box0.bmp", 2);
	AddTexture("textures/device 2.bmp", 2);

	AddMaterial(texIDs, slots, 1);
	AddMaterial(texIDs + 1, slots + 1, 1);
	AddMaterial(texIDs + 2, slots + 2, 1);
	AddMaterial(texIDs + 3, slots + 3, 1);
	AddMaterial(texIDs + 4, slots + 4, 1);


	unsigned int cubeTexIDs[6] = { 0 , 1, 2 ,3 ,4 ,5 };
	unsigned int cubeSlots[6] = { 0 , 1, 2 ,3 ,4 ,5 };
	TexAdd("textures/cubemaps/Daylight Box_");
	TexAdd("textures/cubemaps/overworld/overworld_");
	TexAdd("textures/cubemaps/sky/sky_");
	TexAdd("textures/cubemaps/Japanese/Japanese_");
	TexAdd("textures/cubemaps/Space/Space_");
	TexAdd("textures/cubemaps/Sunset/Sunset_");
	MatAdd(cubeTexIDs, cubeSlots, 1);
	MatAdd(cubeTexIDs + 1, cubeSlots + 1, 1);
	MatAdd(cubeTexIDs + 2, cubeSlots + 2, 1);
	MatAdd(cubeTexIDs + 3, cubeSlots + 3, 1);
	MatAdd(cubeTexIDs + 4, cubeSlots + 4, 1);
	MatAdd(cubeTexIDs + 5, cubeSlots + 5, 1);




	AddShape(Cube, -2, TRIANGLES);
	MatSet(0, 0);
	SetShapeShader(0, 4);
	selected_data_index = 0;
	float s = 60;
	ShapeTransformation(scaleAll, s, 0);
	SetShapeStatic(0);

	AddShape(Axis, -1, TRIANGLES, 1);
	SetShapeViewport(1, 1);

	SetShapeShader(1, 0);
	selected_data_index = 1;
	ShapeTransformation(scaleAll, 0, 1);
	SetShapeStatic(1);
	AddShape(Plane, -1, TRIANGLES, 1);
	SetShapeShader(2, 1);
	SetShapeMaterial(2, 1);
	selected_data_index = 2;
	ShapeTransformation(scaleAll, 60, 1);
	SetShapeStatic(2);
	SetShapeViewport(2, 1);

	data_list[1]->clear();
	if ((chooseshapeind != -1)) {
		Draw(0);
		Draw(1);
	}


	else return;
}



void Project::Draw(int part) {
	auto shape = data_list[1];
	Eigen::RowVector3d vec1((MyShapes_map[chooseshapeind]).shape_bez(0, part)[0], (MyShapes_map[chooseshapeind]).shape_bez(0, part)[1], 0);
	double st = 0.005;
	while (st < 1) {
		Eigen::Vector2d temp = (MyShapes_map[chooseshapeind]).shape_bez((float)st, part);
		Eigen::RowVector3d vec2 = Eigen::RowVector3d(temp[0], temp[1], 0);
		shape->add_edges(vec1, vec2, Eigen::RowVector3d(0.5, 0.5, 0.5));
		vec1 = vec2;
		st += 0.005;

	}
}


void Project::Update(const Eigen::Matrix4f& Proj, const Eigen::Matrix4f& View, const Eigen::Matrix4f& Model, unsigned int  shaderIndx, unsigned int shapeIndx)
{
	Shader* s = shaders[shaderIndx];
	int r = ((shapeIndx + 1) & 0x000000FF) >> 0;
	int g = ((shapeIndx + 1) & 0x0000FF00) >> 8;
	int b = ((shapeIndx + 1) & 0x00FF0000) >> 16;


	s->Bind();
	s->SetUniformMat4f("Proj", Proj);
	s->SetUniformMat4f("View", View);
	s->SetUniformMat4f("Model", Model);
	s->SetUniform4f("coeffs", 1, 1, 1, 1);
	s->SetUniform1i("Dots", Dots);
	Eigen::Vector4f all_dots[Dots];


	//change
	if (data_list[shapeIndx]->passing) {

		s->SetUniform1f("TranperentPercent", data_list[shapeIndx]->TranperentPercent);
	}
	else {

		s->SetUniform1f("TranperentPercent", 1);
	}

	int i = 0;
	while (i < Dots && chooseshapeind != -1) {
		all_dots[i] = Eigen::Vector4f((float)MyShapes_map[chooseshapeind].all_dots[i][0], (float)MyShapes_map[chooseshapeind].all_dots[i][1], 0, 0);
		i++;
	}
	s->SetUniform4fv("all_dots", &(all_dots[0]), Dots);

	if (changingdots || (point_choosed && data_list[shapeIndx]->type == Axis)) {

		if ((chooseshapeind != -1))
		{
			data_list[1]->clear();
			Draw(0);
			Draw(1);
			changingdots = false;
		}
		else {
			data_list[1]->clear();
			changingdots = false;

		}
	}
	if (data_list[shapeIndx]->GetMaterial() >= 0 && !materials.empty())
		BindMaterial(s, data_list[shapeIndx]->GetMaterial(), data_list[shapeIndx]->is_or_not);
	if (shaderIndx != 0) {
		s->SetUniform4f("lightColor", 4 / 100.0f, 60 / 100.0f, 99 / 100.0f, 0.5f);
		s->Unbind();

	}

	else {
		s->SetUniform4f("lightColor", r / 255.0f, g / 255.0f, b / 255.0f, 0.0f);
		s->Unbind();
	}


}

void Project::Animate() {
	int i = 0;
	int size = MyShapes_map.size();
	while (i < size) {
		if (isActive) {
			MyShape* vec = &MyShapes_map[i];
			Eigen::Vector3d vel = vec->steps();
			data_list[vec->index_shape]->MyTranslate(vel, 1);
		}
		else {
			MyShape& s = MyShapes_map[i];
			data_list[s.index_shape]->MyTranslate(Eigen::Vector3d(0, 0, 0) - s.position_animate, 1);
			s.again();
		}

		i++;
	}

}



void Project::ScaleAllShapes(float amt, int viewportIndx)
{
	for (int i = 1; i < data_list.size(); i++)
	{
		if (data_list[i]->Is2Render(viewportIndx))
		{
			data_list[i]->MyScale(Eigen::Vector3d(amt, amt, amt));
		}
	}
}

Project::~Project(void)
{

}

void Project::UnPicked() {

	choosenpoint = -1;
	point_choosed = false;
}


int Project::choosed(float point_x, float point_y) {
	bool flag;
	if ((chooseshapeind != -1) || isActive) {

		choosenpoint = -1;
		point_choosed = false;

		int i = 0;
		while (i < Dots) {
			int a = pow((point_x - 1200) / 80 - MyShapes_map[chooseshapeind].all_dots[i][0], 2);
			int b = pow((point_y - 400) / -80 - MyShapes_map[chooseshapeind].all_dots[i][1], 2);

			flag = a + b < pow(0.22, 2);
			if (flag) {
				point_choosed = true;
				choosenpoint = i;
				break;
			}
			i++;
		}
		return choosenpoint;
	}
	return -1;
}

void Project::set_the_choosed(float point_x, float point_y) {
	if (!point_choosed)
		return;
	MyShapes_map[chooseshapeind].all_dots[choosenpoint][0] = (point_x - 1200) / 80;
	if (choosenpoint % 6 != 0)
		MyShapes_map[chooseshapeind].all_dots[choosenpoint][1] = (point_y - 400) / -80;

}

