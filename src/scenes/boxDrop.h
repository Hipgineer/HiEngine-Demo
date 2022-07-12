// Test Scene
// 
class BoxDrop : public Scene
{
public :
    BoxDrop(const char* name) : Scene(name) {}

	virtual void Init()
    {
        SPDLOG_INFO("BoxDrop Initializing");
        // TODO: read obj file

        // TODO: Voxelize 3d object to make particles
        // ex) 
        // int64_t const initParticleNumber = voxelizer->CreateParticles(&g_buffer->m_positions);
 
        g_buffer->m_commonParam.radius  = 0.01f;    
        g_buffer->m_commonParam.diameter= 0.02f;    
        g_buffer->m_commonParam.H       = 0.048f;      
        g_buffer->m_commonParam.dt      = 0.0001f;      

        PhaseParameters Water;
        Water.density = 1000.0f; 
        Water.color   = glm::vec3(1.0f, 0.0f, 0.0f);
        g_buffer->m_phaseParam.push_back(Water); // phase : 0
        
        // Box Generate
        int32_t xNum = 10;
        int32_t yNum = 10;
        int32_t zNum = 10;

        int32_t const initParticleNumber = xNum*yNum*zNum;
        
        g_buffer->m_positions.reserve(initParticleNumber);
        g_buffer->m_velocities.reserve(initParticleNumber);
        g_buffer->m_phases.reserve(initParticleNumber);
        g_buffer->m_colorValues.reserve(initParticleNumber);

        for (int32_t ii = 0 ; ii < xNum ; ++ii)
            for (int32_t jj = 0 ; jj < yNum ; ++jj)
                for (int32_t kk = 0 ; kk < zNum ; ++kk)
                {
                    g_buffer->m_positions.push_back(glm::vec3(2*ii*g_buffer->m_commonParam.radius, 2*jj*g_buffer->m_commonParam.radius, 2*kk*g_buffer->m_commonParam.radius));
                    g_buffer->m_velocities.push_back(glm::vec3(ii*0.01f, jj*0.01f, kk*0.01f));
                    g_buffer->m_phases.push_back(0);
                    g_buffer->m_colorValues.push_back(static_cast<float>(jj));
                }
    }
};