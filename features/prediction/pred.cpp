#include "../../includes.h"

InputPrediction g_inputpred{};;

void InputPrediction::update( ) {
	if ( !g_csgo.m_cl || !g_csgo.m_prediction )
		return;

	bool        valid{ g_csgo.m_cl->m_delta_tick > 0 };
	int start = g_csgo.m_cl->m_last_command_ack;
	int stop = g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands;

	// call CPrediction::Update.
	if ( valid )
		g_csgo.m_prediction->Update( g_csgo.m_cl->m_delta_tick, valid, start, stop );

	static bool unlocked_fakelag = false;
	if ( !unlocked_fakelag ) {
		auto cl_move_clamp = pattern::find( g_csgo.m_engine_dll, XOR( "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC" ) ) + 1;
		unsigned long protect = 0;

		g_winapi.VirtualProtect( reinterpret_cast< void* >( cl_move_clamp ), 4, PAGE_EXECUTE_READWRITE, &protect );
		*( std::uint32_t* )cl_move_clamp = 62;
		g_winapi.VirtualProtect( reinterpret_cast< void* >( cl_move_clamp ), 4, protect, &protect );
		unlocked_fakelag = true;
	}
}

void InputPrediction::run( ) {
	if ( !g_csgo.m_move_helper || !g_csgo.m_prediction )
		return;

	if ( !g_cl.m_processing )
		return;

	// backup original tickbase.
	m_prediction_data.m_old_tick = g_cl.m_local->m_nTickBase( );

	// backup globals.
	m_prediction_data.m_curtime = g_csgo.m_globals->m_curtime;
	m_prediction_data.m_frametime = g_csgo.m_globals->m_frametime;

	// backup prediction data.
	m_prediction_data.m_in_prediction = g_csgo.m_prediction->m_in_prediction;
	m_prediction_data.m_first_time_predicted = g_csgo.m_prediction->m_split[ GET_ACTIVE_SPLITSCREEN_SLOT( ) ].m_first_time_predicted;

	// set host player.
	// note: this is called before CPrediction::RunCommand.
	g_csgo.m_move_helper->SetHost( g_cl.m_local );

	g_csgo.m_prediction->m_in_prediction = true;
	g_csgo.m_prediction->m_split[ GET_ACTIVE_SPLITSCREEN_SLOT( ) ].m_first_time_predicted = false;

	/*void CPrediction::StartCommand( C_BasePlayer * player, CUserCmd * cmd ) {
#if !defined( NO_ENTITY_PREDICTION )
		VPROF( "CPrediction::StartCommand" );

#if defined( USE_PREDICTABLEID )
		CPredictableId::ResetInstanceCounters( );
#endif

		player->m_pCurrentCommand = cmd;
		player->m_LastCmd = *cmd;
		C_BaseEntity::SetPredictionRandomSeed( cmd );
		C_BaseEntity::SetPredictionPlayer( player );
#endif
	}*/
	g_cl.m_local->m_pCurrentCommand( ) = g_cl.m_cmd;
	g_cl.m_local->m_PlayerCommand( )   = *g_cl.m_cmd;
	*g_csgo.m_nPredictionRandomSeed = g_cl.m_cmd->m_random_seed; // no need to generate this anymore as we are using the correct CreateMove
	g_csgo.m_pPredictionPlayer      = g_cl.m_local;

	m_prediction_data.m_last_cmd = &g_cl.m_local->m_PlayerCommand( );
	if ( !m_prediction_data.m_last_cmd ||
		 !m_prediction_data.m_last_cmd->m_predicted )
		m_prediction_data.m_sequence_difference = g_cl.m_sequence_number - g_cl.m_local->m_nTickBase( );


	m_prediction_data.m_tick = std::max( g_cl.m_local->m_nTickBase( ),
										 g_cl.m_sequence_number - m_prediction_data.m_sequence_difference );

	return repredict( );
}

void RunPreThink( Player* pPlayer ) {
	if ( !pPlayer )
		return;

	if ( pPlayer->PhysicsRunThink( ) )
		pPlayer->PreThink( );
}

#define TICK_NEVER_THINK		(-1)
void RunThink( Player* pPlayer ) {
	if ( !pPlayer )
		return;

	if ( pPlayer->m_nNextThinkTick( ) <= 0 || pPlayer->m_nNextThinkTick( ) > pPlayer->m_nTickBase( ) )
		return;

	pPlayer->m_nNextThinkTick( ) = TICK_NEVER_THINK;
	pPlayer->CheckHasThinkFunction( false );
	pPlayer->Think( );
}

// repredict if prediction goes coocoo.
void InputPrediction::repredict( ) {
	if ( !g_csgo.m_move_helper || !g_csgo.m_prediction || !g_csgo.m_game_movement )
		return;

	if ( !g_cl.m_processing || !g_cl.m_cmd )
		return;

	update( );

	// set global variables.
	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME( m_prediction_data.m_tick );
	g_csgo.m_globals->m_frametime = g_csgo.m_prediction->m_engine_paused ? 0.f : g_csgo.m_globals->m_interval;

	//// Add and subtract buttons we're forcing on the player
	//ucmd->buttons |= player->m_afButtonForced;
	//// ucmd->buttons &= ~player->m_afButtonDisabled; // MAY WANT TO DO THIS LATER!!!
	g_cl.m_cmd->m_buttons |= g_cl.m_local->m_afButtonForced( );

	// start track prediction errors
	g_csgo.m_game_movement->StartTrackPredictionErrors( g_cl.m_local );

	//// Do weapon selection
	//if ( ucmd->weaponselect != 0 ) {
	//	C_BaseCombatWeapon* weapon = ToBaseCombatWeapon( CBaseEntity::Instance( ucmd->weaponselect ) );
	//	if ( weapon ) {
	//		player->SelectItem( weapon->GetName( ), ucmd->weaponsubtype );
	//	}
	//}
	if ( g_cl.m_cmd->m_weapon_select != 0 ) {
		Weapon* weapon = g_csgo.m_entlist->GetClientEntity< Weapon* >( g_cl.m_cmd->m_weapon_select );
		if ( WeaponInfo* weaponData = weapon->GetWpnData( );
			 weapon ) {
			g_cl.m_local->SelectItem( weaponData->m_weapon_name, g_cl.m_cmd->m_weapon_subtype );
		}
	}

	//// Latch in impulse.
	//IClientVehicle* pVehicle = player->GetVehicle( );
	//if ( ucmd->impulse ) {
	//	// Discard impulse commands unless the vehicle allows them.
	//	// FIXME: UsingStandardWeapons seems like a bad filter for this. 
	//	// The flashlight is an impulse command, for example.
	//	if ( !pVehicle || player->UsingStandardWeaponsInVehicle( ) ) {
	//		player->m_nImpulse = ucmd->impulse;
	//	}
	//}
	if ( g_cl.m_cmd->m_impulse )
		g_cl.m_local->m_nImpulse( ) = g_cl.m_cmd->m_impulse;

	//// Get button states
	//player->UpdateButtonState( ucmd->buttons );
	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	g_cl.m_local->m_afButtonLast( ) = g_cl.m_local->m_nButtons( );

	// Get button states
	g_cl.m_local->m_nButtons( ) = g_cl.m_cmd->m_buttons;
	int buttonsChanged = g_cl.m_local->m_afButtonLast( ) ^ g_cl.m_local->m_nButtons( );

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	g_cl.m_local->m_afButtonPressed( ) = buttonsChanged & g_cl.m_local->m_nButtons( );		// The changed ones still down are "pressed"
	g_cl.m_local->m_afButtonReleased( ) = buttonsChanged & ( ~g_cl.m_local->m_nButtons( ) );	// The ones not down are "released"

	g_csgo.m_prediction->CheckMovingGround( g_cl.m_local, g_csgo.m_globals->m_frametime );

	//// Copy from command to player unless game .dll has set angle using fixangle
	//// if ( !player->pl.fixangle )
	//{
	//	player->SetLocalViewAngles( ucmd->viewangles );
	//}
	g_cl.m_local->SetLocalViewAngles( g_cl.m_cmd->m_view_angles );

	// Call standard client pre-think
	RunPreThink( g_cl.m_local );

	// Call Think if one is set
	RunThink( g_cl.m_local );

	// Setup input.
	memset( &m_prediction_data.data, 0, sizeof( m_prediction_data.data ) );
	g_csgo.m_prediction->SetupMove( g_cl.m_local, g_cl.m_cmd, g_csgo.m_move_helper, &m_prediction_data.data );

	// modify move data to use non interpolated data.
	ang_t m_angle;
	g_csgo.m_prediction->GetLocalViewAngles( m_angle );
	m_prediction_data.data.m_vecOldAngles = m_angle;
	m_prediction_data.data.SetAbsOrigin( g_cl.m_local->m_vecOrigin( ) );
	m_prediction_data.data.m_vecVelocity = g_cl.m_local->m_vecVelocity( );

	//{
	//	VPROF_BUDGET( "CPrediction::ProcessMovement", "CPrediction::ProcessMovement" );

	//	// RUN MOVEMENT
	//	if ( !pVehicle ) {
	//		Assert( g_pGameMovement );
	//		g_pGameMovement->ProcessMovement( player, g_pMoveData );
	//	}
	//	else {
	//		pVehicle->ProcessMovement( player, g_pMoveData );
	//	}
	//}
	g_csgo.m_game_movement->ProcessMovement( g_cl.m_local, &m_prediction_data.data );

	g_csgo.m_prediction->FinishMove( g_cl.m_local, g_cl.m_cmd, &m_prediction_data.data );

	//// Let server invoke any needed impact functions
	//VPROF_SCOPE_BEGIN( "moveHelper->ProcessImpacts(cl)" );
	//moveHelper->ProcessImpacts( );
	//VPROF_SCOPE_END( );
	//g_csgo.m_move_helper->ProcessImpacts( );

	g_cl.m_local->PostThink( );

	g_csgo.m_game_movement->FinishTrackPredictionErrors( g_cl.m_local );
}

void InputPrediction::restore( ) {
	if ( !g_csgo.m_globals || !g_csgo.m_prediction )
		return;

	if ( !g_cl.m_processing )
		return;

	/*void CPrediction::FinishCommand( C_BasePlayer * player ) {
#if !defined( NO_ENTITY_PREDICTION )
		VPROF( "CPrediction::FinishCommand" );

		player->m_pCurrentCommand = NULL;
		C_BaseEntity::SetPredictionRandomSeed( NULL );
		C_BaseEntity::SetPredictionPlayer( NULL );
#endif
	}*/
	g_cl.m_local->m_pCurrentCommand( ) = NULL;
	*g_csgo.m_nPredictionRandomSeed = NULL;
	g_csgo.m_pPredictionPlayer = NULL;

	g_csgo.m_game_movement->Reset( );  // fixes a crash: when loading highlights twice or after previously loaded map, there was a dirty player pointer in game movement

	if ( !g_csgo.m_prediction->m_engine_paused && g_csgo.m_globals->m_frametime > 0 ) {
		g_cl.m_local->m_nTickBase( )++;
	}

	// reset host player.
	// note: this is called after CPrediction::RunCommand.
	g_csgo.m_move_helper->SetHost( nullptr );

	// restore globals.
	g_csgo.m_globals->m_curtime   = m_prediction_data.m_curtime;
	g_csgo.m_globals->m_frametime = m_prediction_data.m_frametime;

	// restore prediction data.
	g_csgo.m_prediction->m_in_prediction = m_prediction_data.m_in_prediction;
	g_csgo.m_prediction->m_split[ GET_ACTIVE_SPLITSCREEN_SLOT( ) ].m_first_time_predicted = m_prediction_data.m_first_time_predicted;

	// restore move data.
	m_prediction_data.data.SetAbsOrigin( g_cl.m_local->GetAbsOrigin( ) );
	m_prediction_data.data.m_vecVelocity = g_cl.m_local->m_vecAbsVelocity( );
	m_prediction_data.data.m_vecOldAngles = m_prediction_data.data.m_vecAngles;
}