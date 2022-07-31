#pragma once
#include <Eigen/Core>
#include <Eigen/Geometry>

class MyShape
{
public:
	Eigen::Vector2d* get(int part);
	double time;
	double sp;
	double lag_count;
	int part;


	Eigen::Vector2d all_dots[7];
	int index_shape;
	int index_mat;
	int shape_layer;
	bool* choosed;
	float lag;


	void MyShape::time_positive();


	Eigen::Vector3d time_neg();
	MyShape(int index_shape, int shape_layer);

	Eigen::Vector3d time_between_one_and_zero1();

	Eigen::Vector2d shape_bez(double time, int part);

	Eigen::Vector3d position_animate;


	Eigen::Vector3d steps();

	void again();

};

class point
{
public:
	point(double x, double y);
	double get_x();
	double get_y();

private:
	double x;
	double y;




};