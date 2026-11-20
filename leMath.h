#pragma once
#include <math.h>
#define Pi 3.14159265358979323846f
enum leObjectType { lePolygon,leEllipse};
struct leObjectID
{
	unsigned int ID;
	leObjectType type;
	leObjectID(){}
	~leObjectID(){}
	leObjectID(unsigned int id,leObjectType t)
	{
		type = t;
		ID = id;
	}
};
float DtoR(float angle);

struct leVec2f
{	
	float x,y;

	leVec2f(){}
	leVec2f(float _x,float _y){x = _x;y = _y;}
	~leVec2f(){};

	leVec2f operator /(const float &scale) const; //Делить вектор на скалярное число 
	leVec2f operator *(const leVec2f &other) const; //Умножить вектор на вектор
	leVec2f operator +(const leVec2f &other) const; //Прибавить к вектору вектор
	leVec2f operator -(const leVec2f &other) const; //Отнять вектор
	leVec2f operator -() const;

	bool operator==(leVec2f &other) const;
	bool operator>=(leVec2f &other) const;
	bool operator<=(leVec2f &other) const;
	bool operator <(leVec2f &other) const;
	bool operator >(leVec2f &other) const;

	bool operator!=(leVec2f &other) const;
	
	void Scale(float size);//Умножить вектор на скалярное число !
	float Length();// Длинна вектора
	leVec2f Normal();
}; 
struct leEdge
{
	unsigned int ID;
	leVec2f v1;
	leVec2f v2;
	leVec2f LightVec1;
	leVec2f LightVec2;
	leObjectType type;
	leEdge(){}
	leEdge(leVec2f v_1,leVec2f v_2,leVec2f lpos, leObjectType t)
	{
		v1 = v_1;
		v2 = v_2;
		LightVec1 = v1 + (v1 - lpos) * leVec2f(110,110);
		LightVec2 = v2 + (v2 - lpos) * leVec2f(100,100);
		type = t;
	}

	
	~leEdge(){}
};
struct leAABB
{
	leVec2f upperBound;
	leVec2f lowerBound;
};
float dot(leVec2f v1,leVec2f v2);