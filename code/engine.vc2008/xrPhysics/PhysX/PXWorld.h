#pragma once
#include "Physics.h"
#include "IPHWorld.h"
#include "physics_scripted.h"
#include "../xrEngine/pure.h"

class CObjectSpace;
class CObjectList;
class CRenderDeviceBase;

namespace physx
{
    class PxFoundation;
    class PxPhysics;
    class PxDefaultCpuDispatcher;
    class PxScene;
    class PxPvd;
}
			

class CPXWorld : public pureFrame,
    public pureRender,
    public IPHWorld,
    public cphysics_scripted
{

public:

    CPXWorld();
    CPXWorld(const CPXWorld& other) = delete;
    CPXWorld& operator=(const CPXWorld& other) = delete;
    virtual						~CPXWorld() {};

    void Create(bool mt, CObjectSpace *os, CObjectList *lo, CRenderDeviceBase *dv);

    virtual void __stdcall OnFrame(void) override;
    virtual void OnRender(void) override;
    virtual u32 CalcNumSteps(u32 dTime) override;
    virtual float FrameTime() override;
    virtual void Freeze() override;
    virtual float Gravity() override;
    virtual u16 ObjectsNumber() override;
    virtual bool Processing() override;
    virtual void SetGravity(float g) override;
    virtual void SetStep(float s) override;
    virtual void Step() override;
    virtual void StepNumIterations(int num_it) override;
    virtual u64 & StepsNum() override;
    virtual void UnFreeze() override;
    virtual u16 UpdateObjectsNumber() override;
    virtual void set_default_character_contact_shotmark(ContactCallbackFun *f) override;
    virtual void set_default_contact_shotmark(ContactCallbackFun *f) override;
    virtual void set_step_time_callback(PhysicsStepTimeCallback* cb) override;
    virtual void set_update_callback(IPHWorldUpdateCallbck* cb) override;
    virtual iphysics_scripted & get_scripted() override;
    CRenderDeviceBase			&Device() { VERIFY(m_device); return *m_device; }
    CObjectSpace				&ObjectSpace() { VERIFY(m_object_space); return *m_object_space; }
    CObjectList					&LevelObjects() { VERIFY(m_level_objects); return *m_level_objects; }


private:

    physx::PxFoundation*			m_Foundation;
    physx::PxPhysics*				m_Physics;

    physx::PxDefaultCpuDispatcher*	m_Dispatcher;
    physx::PxScene*				    m_Scene;
    physx::PxPvd*                   m_Pvd;

    CRenderDeviceBase* m_device;
    CObjectSpace* m_object_space;
    CObjectList* m_level_objects;

    float m_frame_time;
    float m_previous_frame_time;
};