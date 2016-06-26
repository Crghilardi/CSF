#include "Cloth.h"


Cloth::Cloth(double width, double height, int num_particles_width, int num_particles_height, Vec3 origin_pos1, double smoothThreshold, double heightThreshold, int rigidness, double time_step, double cloth_resolution) : num_particles_width(num_particles_width), num_particles_height(num_particles_height),
origin_pos1(origin_pos1), smoothThreshold(smoothThreshold), heightThreshold(heightThreshold), rigidness(rigidness), time_step(time_step), cloth_resolution(cloth_resolution)
{
	constraint_iterations = rigidness;
	particles.resize(num_particles_width*num_particles_height); //I am essentially using this vector as an array with room for num_particles_width*num_particles_height particles

	double time_step2 = time_step*time_step;

	// creating particles in a grid of particles from (0,0,0) to (width,-height,0)
	for (int x = 0; x<num_particles_width; x++)
	{
		for (int y = 0; y<num_particles_height; y++)
		{
			Vec3 pos = Vec3(width * (x / (double)num_particles_width),
				0,
				height * (y / (double)num_particles_height));
			pos = pos + origin_pos1;
			particles[y*num_particles_width + x] = Particle(pos, time_step2); // insert particle in column x at y'th row
			particles[y*num_particles_width + x].pos_x = x;
			particles[y*num_particles_width + x].pos_y = y;
		}
	}

	// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
	for (int x = 0; x<num_particles_width; x++)
	{
		for (int y = 0; y<num_particles_height; y++)
		{
			if (x<num_particles_width - 1) makeConstraint(getParticle(x, y), getParticle(x + 1, y));
			if (y<num_particles_height - 1) makeConstraint(getParticle(x, y), getParticle(x, y + 1));
			if (x<num_particles_width - 1 && y<num_particles_height - 1) makeConstraint(getParticle(x, y), getParticle(x + 1, y + 1));
			if (x<num_particles_width - 1 && y<num_particles_height - 1) makeConstraint(getParticle(x + 1, y), getParticle(x, y + 1));
		}
	}


	// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
	for (int x = 0; x<num_particles_width; x++)
	{
		for (int y = 0; y<num_particles_height; y++)
		{
			if (x<num_particles_width - 2) makeConstraint(getParticle(x, y), getParticle(x + 2, y));
			if (y<num_particles_height - 2) makeConstraint(getParticle(x, y), getParticle(x, y + 2));
			if (x<num_particles_width - 2 && y<num_particles_height - 2) makeConstraint(getParticle(x, y), getParticle(x + 2, y + 2));
			if (x<num_particles_width - 2 && y<num_particles_height - 2) makeConstraint(getParticle(x + 2, y), getParticle(x, y + 2));
		}
	}
}


double Cloth::timeStep()
{

int particleCount = static_cast<int>(particles.size());
#pragma omp parallel for
for (int i = 0; i < particleCount; i++)
	{
		particles[i].timeStep();
	}

for (int i = 0; i<constraint_iterations; i++) // iterate over all constraints several times
{
#pragma omp parallel for
	for (int j = 0; j < constraints.size(); j++)
	{
		constraints[j].satisfyConstraint();
	}
}

//#pragma omp parallel for
//for (int j = 0; j < particleCount; j++)
//		{
//			particles[j].satisfyConstraintSelf(constraint_iterations);
//		}

	double maxDiff = 0;
#pragma omp parallel for
	for (int i = 0; i < particleCount; i++)
	{
		if (particles[i].isMovable())
		{
			double diff = fabs(particles[i].old_pos.f[1] - particles[i].pos.f[1]);
			if (diff > maxDiff)
				maxDiff = diff;
		}
	}
	return maxDiff;
}


void Cloth::addForce(const Vec3 direction)
{
	for (int i = 0; i < particles.size(); i++)
	{
		particles[i].addForce(direction);
	}

}



//��Ⲽ���Ƿ��������ײ
void Cloth::terrCollision(vector<double> &heightvals,Terrian * terr)
{
#pragma omp parallel for
	for (int i = 0; i < particles.size(); i++)
	{
		Vec3 v = particles[i].getPos();
		if (v.f[1] < heightvals[i]) 
		{
			particles[i].offsetPos(Vec3(0, heightvals[i] - v.f[1], 0)); // 
			particles[i].makeUnmovable();
		}
	}
}



void Cloth::movableFilter()
{
	vector<Particle> tmpParticles;
	for (int x = 0; x < num_particles_width; x++)
	{
		for (int y = 0; y < num_particles_height; y++)
		{

			Particle *ptc = getParticle(x, y);
			if (ptc->isMovable() && !ptc->isVisited)
			{
				queue<int> que; //
				vector<XY> connected;  //store the connected component
				vector<vector<int> >neibors;
				int sum = 1;
				int index = y*num_particles_width + x;
				// visit the init node
				connected.push_back(XY(x,y));
				particles[index].isVisited = true;
				//enqueue the init node
				que.push(index);
				while (!que.empty())
				{
					Particle * ptc_f = &particles[que.front()];
					que.pop();
					int cur_x = ptc_f->pos_x;
					int cur_y = ptc_f->pos_y;
					vector<int> neibor;
					//	cout <<"cur:"<< cur_x << " " << cur_y << endl;
					if (cur_x > 0)
					{
						Particle* ptc_left = getParticle(cur_x - 1, cur_y);
						if (ptc_left->isMovable())
						{
							if (!ptc_left->isVisited){
								sum++;
								ptc_left->isVisited = true;
								connected.push_back(XY(cur_x - 1, cur_y));
								que.push(num_particles_width*cur_y + cur_x - 1);
								neibor.push_back(sum - 1);
								ptc_left->c_pos = sum - 1;
							}
							else{
								neibor.push_back(ptc_left->c_pos);
							}

						}

					}
					if (cur_x<num_particles_width - 1)
					{
						Particle* ptc_right = getParticle(cur_x + 1, cur_y);
						if (ptc_right->isMovable())
						{
							if (!ptc_right->isVisited){
								sum++;
								ptc_right->isVisited = true;
								connected.push_back(XY(cur_x + 1, cur_y));
								que.push(num_particles_width*cur_y + cur_x + 1);
								neibor.push_back(sum - 1);
								ptc_right->c_pos = sum - 1;
							}
							else{
								neibor.push_back(ptc_right->c_pos);
							}
						}

					}
					if (cur_y > 0)
					{
						Particle* ptc_bottom = getParticle(cur_x, cur_y - 1);
						if (ptc_bottom->isMovable())
						{
							if (!ptc_bottom->isVisited){
								sum++;
								ptc_bottom->isVisited = true;
								connected.push_back(XY(cur_x, cur_y - 1));
								que.push(num_particles_width*(cur_y - 1) + cur_x);
								neibor.push_back(sum - 1);
								ptc_bottom->c_pos = sum - 1;
							}
							else{
								neibor.push_back(ptc_bottom->c_pos);
							}

						}

					}
					if (cur_y < num_particles_height - 1)
					{
						Particle* ptc_top = getParticle(cur_x, cur_y + 1);
						if (ptc_top->isMovable())
						{
							if (!ptc_top->isVisited)
							{
								sum++;
								ptc_top->isVisited = true;
								connected.push_back(XY(cur_x, cur_y + 1));
								que.push(num_particles_width*(cur_y + 1) + cur_x);
								neibor.push_back(sum - 1);
								ptc_top->c_pos = sum - 1;
							}
							else{
								neibor.push_back(ptc_top->c_pos);
							}

						}

					}
					neibors.push_back(neibor);
				}
				//�������
				if (sum > 100)
				{
					vector<int> edgePoints = findUnmovablePoint(connected, heightvals);
					handle_slop_connected(edgePoints, connected, neibors, heightvals);
				}

			}
		}
	}
}


vector<int> Cloth::findUnmovablePoint(vector<XY> connected,vector<double> &heightvals)
{
	vector<int> edgePoints;
	for (size_t i = 0; i < connected.size(); i++)
	{
		int x = connected[i].x;
		int y = connected[i].y;
		int index = y*num_particles_width + x;
		Particle *ptc = getParticle(x, y);
		if (x > 0){
			Particle *ptc_x = getParticle(x - 1, y);
			if (!ptc_x->isMovable())
			{
				int index_ref = y*num_particles_width + x - 1;
				if (fabs(heightvals[index] - heightvals[index_ref]) < smoothThreshold && ptc->getPos().f[1] - heightvals[index]<heightThreshold)
				{
					Vec3 offsetVec = Vec3(0, heightvals[index] - ptc->getPos().f[1], 0);
					particles[index].offsetPos(offsetVec);
					ptc->makeUnmovable();
					edgePoints.push_back(i);
					continue;
				}
			}
		}

		if (x < num_particles_width - 1)
		{
			Particle *ptc_x = getParticle(x + 1, y);
			if (!ptc_x->isMovable()){
				int index_ref = y*num_particles_width + x + 1;
				if (fabs(heightvals[index] - heightvals[index_ref]) < smoothThreshold && ptc->getPos().f[1] - heightvals[index]<heightThreshold)
				{
					Vec3 offsetVec = Vec3(0, heightvals[index] - ptc->getPos().f[1], 0);
					particles[index].offsetPos(offsetVec);
					ptc->makeUnmovable();
					edgePoints.push_back(i);
					continue;
				}
			}
		}

		if (y>0)
		{
			Particle *ptc_y = getParticle(x, y - 1);
			if (!ptc_y->isMovable())
			{
				int index_ref = (y - 1)*num_particles_width + x;
				if (fabs(heightvals[index] - heightvals[index_ref]) < smoothThreshold && ptc->getPos().f[1] - heightvals[index]<heightThreshold)
				{
					Vec3 offsetVec = Vec3(0, heightvals[index] - ptc->getPos().f[1], 0);
					particles[index].offsetPos(offsetVec);
					ptc->makeUnmovable();
					edgePoints.push_back(i);
					continue;
				}
			}

		}

		if (y<num_particles_height - 1)
		{
			Particle *ptc_y = getParticle(x, y + 1);
			if (!ptc_y->isMovable())
			{
				int index_ref = (y + 1)*num_particles_width + x;
				if (fabs(heightvals[index] - heightvals[index_ref]) < smoothThreshold && ptc->getPos().f[1] - heightvals[index]<heightThreshold)
				{
					Vec3 offsetVec = Vec3(0, heightvals[index] - ptc->getPos().f[1], 0);
					particles[index].offsetPos(offsetVec);
					ptc->makeUnmovable();
					edgePoints.push_back(i);
					continue;
				}
			}

		}

	}
	return edgePoints;
}
//ֱ�Ӷ���ͨ�������б��´���
void Cloth::handle_slop_connected(vector<int> edgePoints, vector<XY> connected, vector<vector<int> >neibors, vector<double> &heightvals)
{
	vector<bool> visited;
	for (size_t i = 0; i < connected.size(); i++)
		visited.push_back(false);

	queue<int> que;
	for (size_t i = 0; i < edgePoints.size(); i++){
		que.push(edgePoints[i]);
		visited[edgePoints[i]] = true;
	}

	while (!que.empty()){
		int index = que.front();
		que.pop();
		//�ж��ܱߵ��Ƿ���Ҫ����
		int index_center = connected[index].y*num_particles_width + connected[index].x;
		for (size_t i = 0; i < neibors[index].size(); i++){
			int index_neibor = connected[neibors[index][i]].y*num_particles_width + connected[neibors[index][i]].x;
			if (fabs(heightvals[index_center] - heightvals[index_neibor]) < smoothThreshold && fabs(particles[index_neibor].getPos().f[1] - heightvals[index_neibor])<heightThreshold)
			{
				Vec3 offsetVec = Vec3(0, heightvals[index_neibor] - particles[index_neibor].getPos().f[1] , 0);
				particles[index_neibor].offsetPos(offsetVec);
				particles[index_neibor].makeUnmovable();
				if (visited[neibors[index][i]] == false){
					que.push(neibors[index][i]);
					visited[neibors[index][i]] = true;
				}

			}
		}

	}

}


void Cloth::saveToFile(string path)
{
	string filepath = "cloth_nodes.txt";
	if (path == "")
	{
		filepath = "cloth_nodes.txt";
	}
	else
	{
		filepath = path;
	}
	ofstream f1(filepath);
	if (!f1)return;
	for (size_t i = 0; i < particles.size(); i++)
	{
		//if (!particles[i].isMovable())
		f1 << fixed << setprecision(8) << particles[i].getPos().f[0]  << "	" << particles[i].getPos().f[2]<< "	" << -particles[i].getPos().f[1] << endl;
	}
	f1.close();
}

void Cloth::saveMovableToFile(string path)
{
	string filepath = "cloth_movable.txt";
	if (path == "")
	{
		filepath = "cloth_movable.txt";
	}
	else
	{
		filepath = path;
	}
	ofstream f1(filepath);
	if (!f1)return;
	for (size_t i = 0; i < particles.size(); i++)
	{
		if (particles[i].isMovable())
			f1 << fixed << setprecision(8) << particles[i].getPos().f[0] << "	" << particles[i].getPos().f[2] << "	" << -particles[i].getPos().f[1] << endl;
	}
	f1.close();
}



