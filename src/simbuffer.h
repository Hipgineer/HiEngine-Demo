#ifndef __SIMBUFFER_H__
#define __SIMBUFFER_H__

#include "common.h"

uint64_t const maxParticle = 1'000'000;

// enum OutputType {
// 	DENSITY,
	
// }

struct boxPoint {
	glm::vec3 minPoint;
	glm::vec3 maxPoint;
	boxPoint() {};
	boxPoint(glm::vec3 minP, glm::vec3 maxP)
	{
		minPoint = minP;
		maxPoint = maxP;
	};
};

struct CommonParameters {
	float radius;
	float diameter = 2.0f * radius;
	float H = 1.3f * diameter;
	float dt;
	float relaxationParameter;
	float scorrK;
	float scorrDq;
	glm::vec3 gravity;
	int32_t iterationNumber;
	boxPoint AnalysisBox;
	std::vector<boxPoint> fixedBox;
	
	CommonParameters() : 
		radius(0.1f), 
		diameter(0.2f),
		H(0.48f),
		dt(0.1f),
		relaxationParameter(0.0000001f),
		scorrK(0.01f),
		scorrDq(0.1f),
		gravity(glm::vec3(0.0f, -9.81f, 0.0f)),
		iterationNumber(3),
		AnalysisBox(boxPoint(glm::vec3(0.0f), glm::vec3(1.0f)))
		// fixedBox(nullptr)
		{};
};

enum class StateOfMatter
{
	FLUID,
	CLOTH
};

struct PhaseParameters {
	StateOfMatter phaseType;
	float density;
	glm::vec3 color; 
	PhaseParameters() : 
		phaseType(StateOfMatter::FLUID),
		density(1000.0f), 
		color(glm::vec3(1.0f,0.0f,0.0f))
	{};
};

CLASS_PTR(SimBuffer);
class SimBuffer
{
public :
	static SimBufferPtr Create();
	int32_t GetNumParticles() { return m_positions.size(); }
	int32_t GetNumStretchLines() { return m_stretchID.size()/2; }
	int32_t GetNumBendLines() { return m_bendID.size()/2; }
	int32_t GetNumShearLines() { return m_shearID.size()/2; }
	int32_t GetNumTriangles() { return m_triangleID.size()/3; }

private :
    SimBuffer() {};
	bool Init();

public :
	std::vector<glm::vec3>  m_positions;
	std::vector<glm::vec3>  m_velocities;
	std::vector<int32_t>    m_phases;
	std::vector<float>  	m_colorValues;

	std::vector<int32_t> 	m_stretchID;
	std::vector<int32_t> 	m_bendID;
	std::vector<int32_t> 	m_shearID;
	std::vector<int32_t> 	m_triangleID;

	CommonParameters m_commonParam;
	std::vector<PhaseParameters> m_phaseParam;


private :
};

#endif // __SIMBUFFER_H__