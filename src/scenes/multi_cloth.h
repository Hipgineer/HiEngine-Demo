// Test Scene
// 
class MultiCloth : public Scene
{
public :
    MultiCloth(const char* name) : Scene(name, StateOfMatter::CLOTH) {}

	virtual void Init()
    {
        SPDLOG_INFO("Cloth Initializing");
        
        g_buffer->m_commonParam.radius  = 0.001f;    
        g_buffer->m_commonParam.diameter= g_buffer->m_commonParam.radius * 2.0f;    
        g_buffer->m_commonParam.H       = g_buffer->m_commonParam.diameter * 2.0f * 1.2f ; // 0.048f;      
        g_buffer->m_commonParam.dt      = 0.001f;    

        g_buffer->m_commonParam.iterationNumber = 30;
	    g_buffer->m_commonParam.relaxationParameter = powf(3.3f/g_buffer->m_commonParam.radius,2.0f);
	    g_buffer->m_commonParam.scorrK              = 0.00001f;
	    g_buffer->m_commonParam.scorrDq             = 0.3f;

        g_buffer->m_commonParam.gravity             = glm::vec3(0.0f, -10.0f, 0.0f);
        g_buffer->m_commonParam.AnalysisBox         = boxPoint(glm::vec3(-10.f, -10.0f, -10.0f), glm::vec3(10.f, 10.f, 10.f));


        PhaseParameters Sweater;
        Sweater.phaseType = StateOfMatter::CLOTH;
        Sweater.density = 1000.0f; 
        Sweater.color   = glm::vec3(0.0f, 0.0f, 0.0f);
        g_buffer->m_phaseParam.push_back(Sweater);

        // Generate Planes
        float size = 0.5;
        glm::vec3 initVel = glm::vec3(0.0, 0.0, 0.0);

        // boxPoint fixedBox1 = boxPoint(glm::vec3(-0.01, 0.99, -0.01), glm::vec3(0.01, 1.01, 0.01));
        // g_buffer->m_commonParam.fixedBox.push_back(fixedBox1);
        glm::vec3 minPoint = glm::vec3(0, 1.0, 0.0);        
        createParticleCloth(minPoint, 0.51, 0.3, 1, initVel, 0);


        // boxPoint fixedBox2 = boxPoint(glm::vec3(0.49, 0.99, -0.01), glm::vec3(0.51, 1.01, 0.01));
        // g_buffer->m_commonParam.fixedBox.push_back(fixedBox2);
        // glm::vec3 minPoint2 = glm::vec3(0.5, 1.0, 0.0);
        // createParticleCloth(minPoint2, size, size, 1, initVel, 0);
    }
};