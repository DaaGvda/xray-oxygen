#include "stdafx.h"
#include "radioactivezone.h"
#include "level.h"
#include "xrmessages.h"
#include "../xrEngine/bone.h"
#include "actor.h"
#include "Hit.h"
#include "../xrengine/xr_collide_form.h"

CRadioactiveZone::CRadioactiveZone(void) 
{}

CRadioactiveZone::~CRadioactiveZone(void) 
{}

void CRadioactiveZone::Load(LPCSTR section) 
{
	inherited::Load(section);
}

bool  CRadioactiveZone::BlowoutState	()
{
	bool result = inherited::BlowoutState();
	if(!result) UpdateBlowout();
	return result;
}

void CRadioactiveZone::Affect(SZoneObjectInfo* O) 
{
	float one				= 0.1f;
	float tg				= Device.fTimeGlobal;

	if(!O->object || O->f_time_affected+one > Device.fTimeGlobal) 
		return;

	clamp					(O->f_time_affected, tg-(one*3), tg);

	Fvector					pos; 
	XFORM().transform_tiny	(pos,CFORM()->getSphere().P);

	Fvector dir				={0,0,0}; 
	float power				= Power(O->object->Position().distance_to(pos),nearest_shape_radius(O));

	float impulse			= 0.0f;
	if(power < EPS)			
	{
		O->f_time_affected	= tg;
		return;
	}
	
	float send_power		= power*one;

	while(O->f_time_affected+one < tg)
	{
		CreateHit	(	O->object->ID(),
						ID(),
						dir,
						send_power,
						BI_NONE,
						Fvector().set(0.0f,0.0f,0.0f),
						impulse,
						m_eHitTypeBlowout);
#ifdef DEBUG
//		if(bDebug)
/*		Msg			(	"Zone[%s]-hit->[%s] Power=%3.3f Frame=%d Time=%3.3f", 
						cName().c_str(), 
						O->object->cName().c_str(), 
						send_power, 
						Device.dwFrame, 
						tg);*/
///		Msg( "Zone hit ___   damage = %.4f    Frame=%d ", send_power, Device.dwFrame );
#endif
		O->f_time_affected += one;
	}//while
}

void CRadioactiveZone::feel_touch_new					(CObject* O	)
{
	inherited::feel_touch_new(O);
};

#include "actor.h"
BOOL CRadioactiveZone::feel_touch_contact(CObject* O)
{

	CActor* A = smart_cast<CActor*>(O);
	if ( A )
	{ 
		if (!((CCF_Shape*)CFORM())->Contact(O))		return	FALSE;
		return										A->feel_touch_on_contact(this);
	}else
		return										FALSE;
}

void CRadioactiveZone::UpdateWorkload					(u32	dt)
{
	inherited::UpdateWorkload(dt);
}

float CRadioactiveZone::nearest_shape_radius(SZoneObjectInfo* O)
{
	CCF_Shape* Sh		= (CCF_Shape*)CFORM();

	if(Sh->Shapes().size()==1)
	{
		return			Radius();
	}else
	{
		xr_vector<CCF_Shape::shape_def>& Shapes = Sh->Shapes();
		CCF_Shape::shape_def& s = Shapes[0];
		return s.data.sphere.R;
	}
}

