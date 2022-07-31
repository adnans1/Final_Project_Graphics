#include "MyShape.h"
#include <iostream>
using namespace std;

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
	Eigen::Vector2d* Dots = get(part);
	point* p1 = new point(time, (1 - time));
	point* p2 = new point(time * time, (1 - time) * (1 - time));
	point* p3 = new point(p1->get_x() * p2->get_x(), p1->get_y() * p2->get_y());
	return p3->get_y() * Dots[0] + 3 * p2->get_y() * p1->get_x() * Dots[1] + 3 * p1->get_y() * p2->get_x() * Dots[2] + p3->get_x() * Dots[3];
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


