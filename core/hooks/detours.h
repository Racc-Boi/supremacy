#pragma once

namespace originals {
	inline CDetourHook CL_Move;
	inline CDetourHook CreateMoveProxy;
}

namespace Detour {
	void init( );

	void __cdecl CL_Move( float m_accumulated_samples, bool bFinalTick );
	void __fastcall CreateMoveProxy( CHLClient* thisptr, int edx, int seq_number, float input_sample_frame_time, bool active );
};
