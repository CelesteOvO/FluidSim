#ifndef MACGrid_H_
#define MACGrid_H_

#pragma warning(disable: 4244 4267 4996)

#include <math.h>
#include <map>
#include <stdio.h>
#include <cstdlib>
#include <fstream> 
#include <Partio.h>
#include "util/custom_output.h" 
#include "util/constants.h" 
#include "util/open_gl_headers.h" 
#include "util/vec.h"
#include "camera.h"
#include "grid_data.h"
#include "grid_data_matrix.h" 
#include "particle.h"
#include <Eigen/Core>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/Sparse>
#include <Eigen/Cholesky>

class Camera;

class MACGrid
{

public:
	MACGrid();
	~MACGrid();
	MACGrid(const MACGrid& orig);
	MACGrid& operator=(const MACGrid& orig);

	void reset();

	void draw(const Camera& c);
	void updateSources();
	void advectVelocity(double dt);
	void addExternalForces(double dt);
	void project(double dt);
	void computeDivergence(GridData &d);
	void advectTemperature(double dt);
	void advectDensity(double dt);
	void advectRenderingParticles(double dt);

	void initMarkerGrid();
	vec3 integrate(const vec3 &pos, const vec3 &vel, const double &dt)
	{
		// RK2
		vec3 newPos = vec3(0,0,0);
		vec3 k1 = vel * pos * dt / 2;
		vec3 k2 = vel * (pos + k1) * dt;
		newPos = pos + vel * dt;

		return clamp(newPos);
	}

	int getID(int i, int j, int k)
	{
		int x = i;
		int y = j * theDim[0];
		int z = k * theDim[0] * theDim[1];

		return x + y + z;
	}

	vec3& clamp(vec3 &pos)
	{
		pos[0] = max(0 - EPSILON, min(theDim[X] * theCellSize, pos[0] + EPSILON));
		pos[1] = max(0 - EPSILON, min(theDim[Y] * theCellSize, pos[1] + EPSILON));
		pos[2] = max(0 - EPSILON, min(theDim[Z] * theCellSize, pos[2] + EPSILON));
	
		return pos;
	}

	// Setup
	void initialize();

	// Simulation
	void computeGravity(double dt);
	void computeBouyancy(double dt);
	void computeVorticityConfinement(double dt);
	void applyVorticityConfinement(vec3 &fConf, int &i, int &j, int &k);

	// Rendering
	struct Cube { vec3 pos; vec4 color; double dist; };
	void drawWireGrid();
	void drawSmokeCubes(const Camera& c);
	void drawSmoke(const Camera& c);
	void drawCube(const MACGrid::Cube& c);
	void drawParticles(const Camera &c);
	void drawFace(const MACGrid::Cube& c);
	void drawVelocities();
	vec4 getRenderColor(int i, int j, int k);
	vec4 getRenderColor(const vec3& pt);
	void drawZSheets(bool backToFront);
	void drawXSheets(bool backToFront);

	// GridData accessors
	enum Direction { X, Y, Z };
	vec3 getVelocity(const vec3& pt);
	double getVelocityX(const vec3& pt);
	double getVelocityY(const vec3& pt);
	double getVelocityZ(const vec3& pt);
	double getTemperature(const vec3& pt);
	double getDensity(const vec3& pt);
	vec3 getCenter(int i, int j, int k);
	
	vec3 getRewoundPosition(const vec3 & currentPosition, const double dt);
	vec3 clipToGrid(const vec3& outsidePoint, const vec3& insidePoint);
	double getSize(int dimension);
	int getCellIndex(int i, int j, int k);
	int getNumberOfCells();
	bool isValidCell(int i, int j, int k);
	bool isValidFace(int dimension, int i, int j, int k);
	vec3 getFacePosition(int dimension, int i, int j, int k);
	void calculateAMatrix();
	bool preconditionedConjugateGradient(const GridDataMatrix & A, GridData & p, const GridData & d, int maxIterations, double tolerance);
	void calculatePreconditioner(const GridDataMatrix & A);
	void applyPreconditioner(const GridData & r, const GridDataMatrix & A, GridData & z);
	double dotProduct(const GridData & vector1, const GridData & vector2);
	void add(const GridData & vector1, const GridData & vector2, GridData & result);
	void subtract(const GridData & vector1, const GridData & vector2, GridData & result);
	void multiply(const double scalar, const GridData & vector, GridData & result);
	double maxMagnitude(const GridData & vector);
	void apply(const GridDataMatrix & matrix, const GridData & vector, GridData & result);


	GridDataX mU; // X component of velocity, stored on X faces, size is (dimX+1)*dimY*dimZ
	GridDataY mV; // Y component of velocity, stored on Y faces, size is dimX*(dimY+1)*dimZ
	GridDataZ mW; // W component of velocity, stored on Z faces, size is dimX*dimY*(dimZ+1)
	GridData mP;  // Pressure, stored at grid centers, size is dimX*dimY*dimZ
	GridData mD;  // Density, stored at grid centers, size is dimX*dimY*dimZ
	GridData mT;  // Temperature, stored at grid centers, size is dimX*dimY*dimZ

	// FLIP - Save a copy of the velocities for FLIP solve
	GridDataX mUcopy; 
	GridDataY mVcopy; 
	GridDataZ mWcopy; 

	std::vector<Particle> particles;
	std::vector<Particle> particlesCopy;
	std::vector<Particle> particlesCopyPIC;
	GridData markerGrid;

	GridDataMatrix AMatrix;
	GridData precon;

public:
	// rendering particles
	std::vector<vec3> rendering_particles;
	std::vector<vec3> rendering_particles_vel;

	enum RenderMode { CUBES, SHEETS, PARTICLES };
	RenderMode theRenderMode;
	static bool theDisplayVel;
	
	void saveSmoke(const char* fileName);
	void saveParticle(std::string filename, bool fluidsim);
	void saveDensity(std::string filename);
};

#endif