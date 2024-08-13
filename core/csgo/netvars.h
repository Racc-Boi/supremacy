#pragma once

#define PropsToFile 1
#if PropsToFile
__forceinline void WritePropsToFile( const std::vector<std::string>& m_prop_data, const char* m_file_name ) {
	if ( m_prop_data.empty( ) ) {
		return;
	}

	std::filesystem::create_directory( "C:\\supremacy\\" );

	std::string m_file_path = "C:\\supremacy\\" + std::string( m_file_name ) + XOR( ".txt" );

	std::ofstream m_file( m_file_path.c_str( ) );

	if ( !m_file.is_open( ) ) {
		return;
	}

	for ( const auto& m_props : m_prop_data ) {
		m_file << m_props << std::endl;
	}

	m_file.close( );
}
#endif
class Netvars {
private:
	struct NetvarData {
		bool        m_datamap_var; // we can't do proxies on stuff from datamaps :).
		RecvProp* m_prop_ptr;
		size_t      m_offset;

		__forceinline NetvarData( ) : m_datamap_var{}, m_prop_ptr{}, m_offset{} { }
	};

	std::unordered_map< hash32_t, int > m_client_ids;

	// netvar container.
	std::unordered_map< hash32_t,		// hash key + tables
		std::unordered_map< hash32_t,	// hash key + props
		NetvarData						// prop offset / prop ptr
		> > m_offsets;
#if PropsToFile
	std::vector<std::string> m_netvars;
	std::vector<std::string> m_datamaps;
#endif
public:
	// ctor.
	Netvars( ) : m_offsets{} { }

	void init( ) {
		ClientClass* list;

		// sanity check on client.
		if ( !g_csgo.m_client )
			return;

		// grab linked list.
		list = g_csgo.m_client->GetAllClasses( );
		if ( !list )
			return;

		// iterate list of netvars.
		for ( ; list != nullptr; list = list->m_pNext ) {
			StoreDataTable( list->m_pRecvTable->m_pNetTableName, list->m_pRecvTable );
#if PropsToFile
			// store netvar information to a vector.
			for ( int i{}; i < list->m_pRecvTable->m_nProps; ++i ) {
				RecvProp* prop = &list->m_pRecvTable->m_pProps[ i ];
				m_netvars.push_back( list->m_pRecvTable->m_pNetTableName + std::string( XOR( "->" ) ) + prop->m_pVarName );
			}
#endif
		}
#if PropsToFile
		WritePropsToFile( m_netvars, XOR( "netvars" ) );
#endif
		// find all datamaps
		FindAndStoreDataMaps( );
#if PropsToFile
		WritePropsToFile( m_datamaps, XOR( "datamaps" ) );
#endif
	}

	// dtor.
	~Netvars( ) {
		m_offsets.clear( );
	}

	// gather all classids dynamically on runtime.
	void SetupClassData( ) {
		ClientClass* list;

		// clear old shit.
		m_client_ids.clear( );

		// sanity check on client.
		if ( !g_csgo.m_client )
			return;

		// grab linked list.
		list = g_csgo.m_client->GetAllClasses( );
		if ( !list )
			return;

		// iterate list.
		for ( ; list != nullptr; list = list->m_pNext )
			m_client_ids[ FNV1a::get( list->m_pNetworkName ) ] = list->m_ClassID;
	}

	void StoreDataTable( const std::string& name, RecvTable* table, size_t offset = 0 ) {
		hash32_t	var, base{ FNV1a::get( name ) };
		RecvProp* prop;
		RecvTable* child;

		// iterate props
		for ( int i{}; i < table->m_nProps; ++i ) {
			prop = &table->m_pProps[ i ];
			child = prop->m_pDataTable;

			// we have a child table, that contains props.
			if ( child && child->m_nProps > 0 )
				StoreDataTable( name, child, prop->m_Offset + offset );

			// hash var name.
			var = FNV1a::get( prop->m_pVarName );

			// insert if not present.
			if ( !m_offsets[ base ][ var ].m_offset ) {
				m_offsets[ base ][ var ].m_datamap_var = false;
				m_offsets[ base ][ var ].m_prop_ptr = prop;
				m_offsets[ base ][ var ].m_offset = ( size_t )( prop->m_Offset + offset );
			}
		}
	}

	// iterate client module and find all datamaps.
	void FindAndStoreDataMaps( ) {
		pattern::patterns_t matches{};

		// sanity.
		if ( !g_csgo.m_client_dll )
			return;

		matches = pattern::FindAll( g_csgo.m_client_dll, XOR( "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C3 CC" ) );
		if ( matches.empty( ) )
			return;

		for ( auto& m : matches )
			StoreDataMap( m );
	}

	void StoreDataMap( Address ptr ) {
		datamap_t* map;
		hash32_t            base, var;
		typedescription_t* entry;

		// get datamap and verify.
		map = ptr.at( 2 ).sub( 4 ).as< datamap_t* >( );

		if ( !map || !map->m_size || map->m_size > 200 || !map->m_desc || !map->m_name )
			return;

		// hash table name.
		base = FNV1a::get( map->m_name );

		for ( int i{}; i < map->m_size; ++i ) {
			entry = &map->m_desc[ i ];
			if ( !entry->m_name )
				continue;

			// hash var name.
			var = FNV1a::get( entry->m_name );

			// if we dont have this var stored yet.
			if ( !m_offsets[ base ][ var ].m_offset ) {
				m_offsets[ base ][ var ].m_datamap_var = true;
				m_offsets[ base ][ var ].m_offset = ( size_t )entry->m_offset[ TD_OFFSET_NORMAL ];
				m_offsets[ base ][ var ].m_prop_ptr = nullptr;
			}

#if PropsToFile
// store var name to datamap file and offset of the var.
			m_datamaps.push_back( map->m_name + std::string( XOR( "->" ) ) + entry->m_name );
			m_datamaps.push_back( std::to_string( entry->m_offset[ TD_OFFSET_NORMAL ] ) );
#endif
		}
	}

	// get datamap offset.
	__forceinline size_t FindInDataMap( datamap_t* map, const std::string& name ) {
		// sanity.
		if ( !map || !map->m_size || map->m_size > 200 || !map->m_desc || !map->m_name )
			return 0;

		// Hash the datamap name
		hash32_t base = FNV1a::get( map->m_name );

		// Iterate over fields
		for ( int i = 0; i < map->m_size; ++i ) {
			const auto& desc = map->m_desc[ i ];

			// Skip null field names
			if ( !desc.m_name )
				continue;

			// Check if the field name matches.
			if ( desc.m_name == name )
				return desc.m_offset[ TD_OFFSET_NORMAL ];

			// Check if the field type is embedded and recursively search.
			if ( desc.m_type == FIELD_EMBEDDED && desc.m_td ) {
				size_t offset = FindInDataMap( desc.m_td, name );
				if ( offset != 0 )
					return offset;
			}
		}

		// Recursively check the base map if it exists.
		if ( map->m_base ) {
			return FindInDataMap( map->m_base, name );
		}

		// Return 0 if we didn't find the field.
		return 0;
	}

	// get client id.
	__forceinline int GetClientID( hash32_t network_name ) {
		return m_client_ids[ network_name ];
	}

	// get netvar offset.
	__forceinline size_t get( hash32_t table, hash32_t prop ) {
		return m_offsets[ table ][ prop ].m_offset;
	}

	// get netvar proxy.
	__forceinline RecvVarProxy_t GetProxy( hash32_t table, hash32_t prop, RecvVarProxy_t proxy ) {
		return m_offsets[ table ][ prop ].m_prop_ptr->m_ProxyFn;
	}

	// set netvar proxy.
	__forceinline void SetProxy( hash32_t table, hash32_t prop, void* proxy, RecvVarProxy_t& original ) {
		auto netvar_entry = m_offsets[ table ][ prop ];

		// we can't set a proxy on a datamap.
		if ( netvar_entry.m_datamap_var )
			return;

		// save original.
		original = netvar_entry.m_prop_ptr->m_ProxyFn;

		// redirect.
		netvar_entry.m_prop_ptr->m_ProxyFn = ( RecvVarProxy_t )proxy;
	}
};

extern Netvars g_netvars;