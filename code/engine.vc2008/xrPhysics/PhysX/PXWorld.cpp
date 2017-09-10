#include "stdafx.h"
#include "PXWorld.h"
#include "../xrengine/device.h"

//xray allocators and error handling for PhysX

class xrPhysXAllocator : public physx::PxAllocatorCallback
{

public:
    virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override
    {
#ifdef DEBUG_MEMORY_NAME
        return Memory.mem_alloc(size, typeName);
#else
        return Memory.mem_alloc(size);
#endif
    }


    virtual void deallocate(void* ptr) override
    {
        Memory.mem_free(ptr);
    }

};

xrPhysXAllocator gAllocator;

class xrPhysXErrorReporter : public physx::PxErrorCallback
{

public:
    virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
    {
        switch (code)
        {
        case physx::PxErrorCode::eNO_ERROR:
        case physx::PxErrorCode::eDEBUG_INFO:
        case physx::PxErrorCode::eDEBUG_WARNING:
            //#TODO: Supress log
            Msg("# [PX] %s", message);
            break;
        case physx::PxErrorCode::eOUT_OF_MEMORY:
            Msg("! [PX] OUT OF MEMORY %s file: %s line: %d", message, file, line);
            break;
        case physx::PxErrorCode::eINVALID_PARAMETER:
        case physx::PxErrorCode::eINVALID_OPERATION:
        case physx::PxErrorCode::eINTERNAL_ERROR:
        case physx::PxErrorCode::eABORT:
            Msg("! [PX] %s file: %s line: %d", message, file, line);
            break;
        case physx::PxErrorCode::ePERF_WARNING:
            //#TODO: Supress log
            Msg("% [PX PERF] %s", message);
            break;
        case physx::PxErrorCode::eMASK_ALL:
        default:
            break;
        }
    }

};

xrPhysXErrorReporter gErrorCallback;


CPXWorld::CPXWorld()
    : m_Foundation(nullptr), m_Physics(nullptr), m_Dispatcher(nullptr), m_Scene(nullptr), m_Pvd(nullptr)
{
    m_frame_time = 0.0f;
    m_previous_frame_time = 0.0f;
}

void CPXWorld::Create(bool mt, CObjectSpace *os, CObjectList *lo, CRenderDeviceBase *dv)
{
    m_object_space = os;
    m_level_objects = lo;
    m_device = dv;

    //Create basic PhysX state
    using namespace physx;
    m_Foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
    R_ASSERT(m_Foundation);

    //#TODO: Make Pvd optional
    m_Pvd = PxCreatePvd(*m_Foundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
    m_Pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), true, m_Pvd);

    PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -default_world_gravity, 0.0f);
    m_Dispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_Dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    m_Scene = m_Physics->createScene(sceneDesc);

    PxPvdSceneClient* pvdClient = m_Scene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    Device().AddSeqFrame(this, mt);
}

void __stdcall CPXWorld::OnFrame(void)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::OnRender(void)
{
    throw std::logic_error("The method or operation is not implemented.");
}

u32 CPXWorld::CalcNumSteps(u32 dTime)
{
    throw std::logic_error("The method or operation is not implemented.");
}

float CPXWorld::FrameTime()
{
    return m_frame_time;
}

void CPXWorld::Freeze()
{
    throw std::logic_error("The method or operation is not implemented.");
}

float CPXWorld::Gravity()
{
    physx::PxVec3 gravity = m_Scene->getGravity();
    return -gravity.y;
}

u16 CPXWorld::ObjectsNumber()
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool CPXWorld::Processing()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::SetGravity(float g)
{
    m_Scene->setGravity(physx::PxVec3(0.0f, -g, 0.0f));
}

void CPXWorld::SetStep(float s)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::Step()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::StepNumIterations(int num_it)
{
    throw std::logic_error("The method or operation is not implemented.");
}

u64 & CPXWorld::StepsNum()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::UnFreeze()
{
    throw std::logic_error("The method or operation is not implemented.");
}

u16 CPXWorld::UpdateObjectsNumber()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::set_default_character_contact_shotmark(ContactCallbackFun *f)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::set_default_contact_shotmark(ContactCallbackFun *f)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::set_step_time_callback(PhysicsStepTimeCallback* cb)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CPXWorld::set_update_callback(IPHWorldUpdateCallbck* cb)
{
    throw std::logic_error("The method or operation is not implemented.");
}

iphysics_scripted & CPXWorld::get_scripted()
{
    throw std::logic_error("The method or operation is not implemented.");
}

