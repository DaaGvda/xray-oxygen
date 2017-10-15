#pragma once
#include "game_cl_base.h"
#include "../xrScripts/export/script_export_space.h"

class game_cl_Single: public game_cl_GameState, DLL_Pure
{
public: /* bad */
	virtual DLL_Pure*		    _construct() { return this; }
public:
								game_cl_Single			();
	virtual	CUIGameCustom*		createGameUI			();
	virtual	void				SetGameUI(CUIGameCustom*) { };

	virtual	u64					GetStartGameTime		();
	virtual	u64					GetGameTime				();	
	virtual	float				GetGameTimeFactor		();	
	virtual	void				SetGameTimeFactor		(const float fTimeFactor);
	virtual	void				SetGameTimeFactor		(u64 GameTime, const float fTimeFactor);

	virtual	u64					GetEnvironmentGameTime		();
	virtual	float				GetEnvironmentGameTimeFactor();
	virtual	void				SetEnvironmentGameTimeFactor(const float fTimeFactor);
	virtual	 void				SetEnvironmentGameTimeFactor(u64 GameTime, const float fTimeFactor);

	void						OnDifficultyChanged		();
	void						SendPickUpEvent(u16 ID_who, u16 ID_what);

private:
	u64 m_qwStartGameTime;
	u64 m_qwStartProcessorTime;
	u64 m_fTimeFactor;

	u64 m_qwEStartGameTime;
	u64 m_qwEStartProcessorTime;
	u64 m_fETimeFactor;
};


// game difficulty
enum ESingleGameDifficulty{
	egdNovice			= 0,
	egdStalker			= 1,
	egdVeteran			= 2,
	egdMaster			= 3,
	egdCount,
	egd_force_u32		= u32(-1)
};

extern ESingleGameDifficulty g_SingleGameDifficulty;
xr_token		difficulty_type_token	[ ];

typedef enum_exporter<ESingleGameDifficulty> CScriptGameDifficulty;
add_to_type_list(CScriptGameDifficulty)
#undef script_type_list
#define script_type_list save_type_list(CScriptGameDifficulty)




