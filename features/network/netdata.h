#pragma once

class NetData {
private:
	class StoredData_t {
	public:
		int    m_tickbase;
		ang_t  m_punch;
		ang_t  m_punch_vel;
		vec3_t m_view_offset;

	public:
		__forceinline StoredData_t( ) : m_tickbase{}, m_punch{}, m_punch_vel{}, m_view_offset{} {};
	};

	std::array< StoredData_t, MULTIPLAYER_BACKUP > m_data;

public:
	void store( int m_command_number );
	void apply( int m_command_number );
	void reset( );
};

extern NetData g_netdata;