#pragma once

class CMoveData {
public:
	bool        m_bFirstRunOfFunctions : 1;
	bool        m_bGameCodeMovedPlayer : 1;
	bool        m_bNoAirControl : 1;
	CBaseHandle m_nPlayerHandle;
	int         m_nImpulseCommand;
	ang_t       m_vecViewAngles;
	ang_t       m_vecAbsViewAngles;
	int         m_nButtons;
	int         m_nOldButtons;
	float       m_flForwardMove;
	float       m_flSideMove;
	float       m_flUpMove;
	float       m_flMaxSpeed;
	float       m_flClientMaxSpeed;
	vec3_t      m_vecVelocity;
	vec3_t      m_vecOldVelocity;
	float       m_unknown;
	ang_t       m_vecAngles;
	ang_t       m_vecOldAngles;
	float       m_outStepHeight;
	vec3_t      m_outWishVel;
	vec3_t      m_outJumpVel;
	vec3_t      m_vecConstraintCenter;
	float       m_flConstraintRadius;
	float       m_flConstraintWidth;
	float       m_flConstraintSpeedFactor;
	bool        m_bConstraintPastRadius;
	vec3_t      m_vecAbsOrigin;
};

class IMoveHelper {
public:
	// indexes for virtuals and hooks.
	enum indices : size_t {
		SETHOST = 1,
		PROCESSIMPACTS = 4,
	};

	__forceinline void SetHost( Entity* host ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity* ) >( this, SETHOST )( this, host );
	}

	__forceinline void ProcessImpacts( ) {
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, PROCESSIMPACTS )( this );
	}
};

#define MAX_SPLITSCREEN_PLAYERS 1
#define GET_ACTIVE_SPLITSCREEN_SLOT() ( 0 )

class CPrediction {
public:
	// indexes for virtuals and hooks.
	enum indices : size_t {
		UPDATE                  = 3,
        POSTNETWORKDATARECEIVED = 6,
		GETLOCALVIEWANGLES      = 12,
		SETLOCALVIEWANGLES      = 13,
		INPREDICTION            = 14,
		CHECKMOVINGGROUND       = 18,
		RUNCOMMAND              = 19,
		SETUPMOVE               = 20,
		FINISHMOVE              = 21
	};

protected:
	// Last object the player was standing on
	CHandle< Entity > m_last_ground;
public:
	bool			m_in_prediction;
	bool			m_old_cl_predict_value;
	bool			m_engine_paused;

	int				m_previous_startframe;
	int				m_incoming_packet_number;

	float			m_last_server_world_time_stamp;

	// Last network origin for local player
	struct Split_t {
		Split_t( ) {
			m_first_time_predicted = false;
			m_commands_predicted = 0;
			m_server_commands_acknowledged = 0;
			m_previous_ack_had_errors = false;
			m_ideal_pitch = 0.0f;
			m_last_command_acknowledged = 0;
			m_previous_ack_error_triggers_full_latch_reset = false;
		}

		bool			m_first_time_predicted;
		int				m_commands_predicted;
		int				m_server_commands_acknowledged;
		int				m_previous_ack_had_errors;
		float			m_ideal_pitch;
		int				m_last_command_acknowledged;
		bool			m_previous_ack_error_triggers_full_latch_reset;
		CUtlVector< CHandle< Entity > > m_ents_with_prediction_errors_in_last_ack;
		bool			m_performed_tick_shift;
	};

	Split_t				m_split[ MAX_SPLITSCREEN_PLAYERS ];

	CGlobalVarsBase	m_saved_vars;

	bool			m_player_origin_type_description_searched;
	CUtlVector< const typedescription_t* >	m_player_origin_type_description; // A vector in cases where the .x, .y, and .z are separately listed

	void* m_p_dump_panel;
public:
	// virtual methods
	__forceinline void Update( int startframe, bool validframe, int incoming_acknowledged, int outgoing_command ) {
		return util::get_method< void( __thiscall* )( void*, int, bool, int, int ) >( this, UPDATE )( this, startframe, validframe, incoming_acknowledged, outgoing_command );
	}

	__forceinline void CheckMovingGround( Entity* player, double frametime ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, double ) >( this, CHECKMOVINGGROUND )( this, player, frametime );
	}

	__forceinline void GetLocalViewAngles( const ang_t& ang ) {
		return util::get_method< void( __thiscall* )( decltype( this ), const ang_t& ) >( this, GETLOCALVIEWANGLES )( this, ang );
	}

	__forceinline void SetLocalViewAngles( const ang_t& ang ) {
		return util::get_method< void( __thiscall* )( decltype( this ), const ang_t& ) >( this, SETLOCALVIEWANGLES )( this, ang );
	}

	__forceinline void SetupMove( Entity* player, CUserCmd* cmd, IMoveHelper* helper, CMoveData* data ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, CUserCmd*, IMoveHelper*, CMoveData* ) >( this, SETUPMOVE )( this, player, cmd, helper, data );
	}

	__forceinline void FinishMove( Entity* player, CUserCmd* cmd, CMoveData* data ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, CUserCmd*, CMoveData* ) >( this, FINISHMOVE )( this, player, cmd, data );
	}
};

class CGameMovement {
public:
	// indexes for virtuals and hooks
	enum indices : size_t {
		PROCESSMOVEMENT             = 1,
		RESET						= 2,
		STARTTRACKPREDICTIONERRORS  = 3,
		FINISHTRACKPREDICTIONERRORS = 4
	};

	__forceinline void ProcessMovement( Entity* player, CMoveData* data ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, CMoveData* ) >( this, PROCESSMOVEMENT )( this, player, data );
	}

	__forceinline void Reset( ) {
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, RESET )( this );
	}

	__forceinline void StartTrackPredictionErrors( Entity* player ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity* ) >( this, STARTTRACKPREDICTIONERRORS )( this, player );
	}

	__forceinline void FinishTrackPredictionErrors( Entity* player ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity* ) >( this, FINISHTRACKPREDICTIONERRORS )( this, player );
	}
};