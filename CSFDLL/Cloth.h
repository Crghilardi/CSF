/*
This source code is about a ground filtering algorithm for airborn LiDAR data
based on physical process simulations, specifically cloth simulation.

this code is based on a Cloth Simulation Tutorial at the cg.alexandra.dk blog.
Thanks to Jesper Mosegaard (clothTutorial@jespermosegaard.dk)



When applying the cloth simulation to LIDAR point filtering. A lot of features
have been added to the original source code, including
* configuration file management
* point cloud data read/write
* point-to-point collsion detection
* nearest point search structure from CGAL
* addding a terrain class


*/
//using discrete steps (drop and pull) to approximate the physical process
//test merge ��ÿ�����ϵ���Χ�����ڽ���N���㣬�Ը߳����ֵ��Ϊ���ܵ������͵㡣

#ifndef _CLOTH_H_
#define _CLOTH_H_

#ifdef _WIN32
#include <windows.h> 
#endif
#include <math.h>
#include <vector>
#include <iostream>
#include <omp.h>
#include <iostream>
#include <sstream>
#include <list>
#include <cmath>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <cmath>
#include <list>
using namespace std;
#include "Constraint.h"
#include "Vec3.h"
#include "Terrian.h"
#include "Particle.h"
//#include <boost/progress.hpp>





struct XY{
	XY(int x1, int y1){ x = x1; y = y1; }
	int x;
	int y;
};

class Cloth
{
private:

	int num_particles_width; // number of particles in "width" direction
	int num_particles_height; // number of particles in "height" direction
	// total number of particles is num_particles_width*num_particles_height
	int constraint_iterations;

	int rigidness;
	double time_step;

	//���в��Ͻڵ�
	std::vector<Particle> particles; // all particles that are part of this cloth
	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth

	//�˲����´������
	double smoothThreshold;
	double heightThreshold;

public:
	//��ʼƽ��λ��
	Vec3 origin_pos1; //���ϽǶ���  ƽ���Ϊˮƽ
	double cloth_resolution;//��������ֱ���

	//heightvalues
	vector<double> heightvals;

	Particle* getParticle(int x, int y) { return &particles[y*num_particles_width + x]; }
	void makeConstraint(Particle *p1, Particle *p2) 
	{
		constraints.push_back(Constraint(p1, p2));
		//p1->neighborsList.push_back(p2);
		//p2->neighborsList.push_back(p1);
	}
public:
	
	int getSize()
	{
		return num_particles_width*num_particles_height;
	}

	size_t get1DIndex(int x, int y){ return y*num_particles_width + x; }


	//��ȡ��index��particle
	Particle* getParticle1d(int index) { return &particles[index]; }

public:

	/* This is a important constructor for the entire system of particles and constraints*/
	Cloth(double width, double height, int num_particles_width, int num_particles_height, Vec3 origin_pos1, double smoothThreshold, double heightThreshold, int rigidness, double time_step, double cloth_resolution);

	void setheightvals(vector<double> heightvals)
	{
		this->heightvals = heightvals;
	}

	/* this is an important methods where the time is progressed one time step for the entire cloth.
	This includes calling satisfyConstraint() for every constraint, and calling timeStep() for all particles
	*/
	double timeStep();

	/* used to add gravity (or any other arbitrary vector) to all particles*/
	void addForce(const Vec3 direction);


	//��Ⲽ���Ƿ��������ײ
	void terrCollision(vector<double> &heightvals,Terrian * terr);

	//�Կ��ƶ��ĵ���б��´���
	void movableFilter();
	//�ҵ�ÿ����ƶ��㣬�����ͨ������Χ�Ĳ����ƶ��㡣���������м�ƽ�
	vector<int> findUnmovablePoint(vector<XY> connected,vector<double> &heightvals);
	//ֱ�Ӷ���ͨ�������б��´���
	void handle_slop_connected(vector<int> edgePoints, vector<XY> connected, vector<vector<int> >neibors, vector<double> &heightvals);

	void doConstraint(int x, int y);
	Vec3 GetVec(Particle *p1, Particle *p2);

	//�����ϵ㱣�浽�ļ�
	void saveToFile(string path = "");
	//�����ƶ��㱣�浽�ļ�
	void saveMovableToFile(string path = "");


};


#endif