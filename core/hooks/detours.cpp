#include "../../includes.h"

void Detour::init( ) {
	if ( MH_Initialize( ) != MH_OK )
		return;

	// CL_Move hook
	if ( !originals::CL_Move.Create(
		g_csgo.CLMove, &CL_Move
		) )
		return;

	// CreateMoveProxy hook
	if ( !originals::CreateMoveProxy.Create(
		util::get_method( g_csgo.m_client, CHLClient::CREATEMOVE ), &CreateMoveProxy
		) )
		return;
}

void __cdecl Detour::CL_Move( float accumulated_extra_samples, bool bFinalTick ) {
	static auto cl_Move = originals::CL_Move.GetOriginal<decltype( &CL_Move )>( );

	if ( !g_csgo.m_engine->IsInGame( ) )
		return cl_Move( accumulated_extra_samples, bFinalTick );

	// set final packet.
	g_cl.m_final_packet = &bFinalTick;

	// allow original cl_move code to run.
	return cl_Move( accumulated_extra_samples, bFinalTick );
}

static void __stdcall CreateMove( int seq_number, float input_sample_frame_time, bool active, bool& send_packet ) {
	// run original create move.
	static auto CreateMoveProxy = originals::CreateMoveProxy.GetOriginal<decltype( &Detour::CreateMoveProxy )>( );

	// process original CHLClient::CreateMove -> IInput::CreateMove
	CreateMoveProxy( g_csgo.m_client, 0, seq_number, input_sample_frame_time, active );

	CUserCmd* cmd = g_csgo.m_input->GetUserCmd( seq_number );
	CVerifiedUserCmd* verified = g_csgo.m_input->GetVerifiedCmd( seq_number );

	if ( !cmd || !cmd->m_command_number )
		return;

	// get bSendPacket off the stack.
	g_cl.m_packet = &send_packet;

	// set sequence number.
	g_cl.m_sequence_number = seq_number;

	// invoke move function.
	g_cl.OnTick( cmd );

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum( );
}

__declspec( naked ) void __fastcall Detour::CreateMoveProxy( CHLClient* thisptr, int edx, int seq_number, float input_sample_frame_time, bool active ) {
	__asm {
		push ebp
		mov ebp, esp

		push ebx
		push esp
		push dword ptr[ ebp + 16 ]
		push dword ptr[ ebp + 12 ]
		push dword ptr[ ebp + 8 ]

		call CreateMove

		pop ebx

		pop ebp
		retn 12
	}
}