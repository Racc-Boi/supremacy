#pragma once

// class size is only 4 bytes on x86-32 and 8 bytes on x86-64.
class Address {
protected:
    uintptr_t m_addr;

public:
    // Default constructor and destructor.
    __forceinline Address( ) : m_addr( 0 ) { }
    __forceinline ~Address( ) { }

    // Constructors.
    __forceinline Address( uintptr_t a ) : m_addr( a ) { }
    __forceinline Address( const void* a ) : m_addr( reinterpret_cast< uintptr_t >( a ) ) { }

    // Arithmetic operators for native types.
    __forceinline operator uintptr_t( ) const { return m_addr; }
    __forceinline operator void* ( ) const { return reinterpret_cast< void* >( m_addr ); }
    __forceinline operator const void* ( ) const { return reinterpret_cast< const void* >( m_addr ); }

    // Comparison operators.
    __forceinline bool operator==( const Address& a ) const {
        return m_addr == a.m_addr;
    }

    __forceinline bool operator!=( const Address& a ) const {
        return m_addr != a.m_addr;
    }

    // Cast / add offset and cast.
    template<typename T = Address>
    __forceinline T as( ) const {
        return T( reinterpret_cast< void* >( m_addr ) );
    }

    template<typename T = Address>
    __forceinline T as( size_t offset ) const {
        return T( reinterpret_cast< void* >( m_addr + offset ) );
    }

    template<typename T = Address>
    __forceinline T as( ptrdiff_t offset ) const {
        return T( reinterpret_cast< void* >( m_addr + offset ) );
    }

    // Add offset and dereference.
    template<typename T = Address>
    __forceinline T at( size_t offset ) const {
        return ( m_addr ) ? *reinterpret_cast< T* >( m_addr + offset ) : T{};
    }

    template<typename T = Address>
    __forceinline T at( ptrdiff_t offset ) const {
        return ( m_addr ) ? *reinterpret_cast< T* >( m_addr + offset ) : T{};
    }

    // Add offset.
    template<typename T = Address>
    __forceinline T add( size_t offset ) const {
        return T( reinterpret_cast< void* >( m_addr + offset ) );
    }

    template<typename T = Address>
    __forceinline T add( ptrdiff_t offset ) const {
        return T( reinterpret_cast< void* >( m_addr + offset ) );
    }

    // Subtract offset.
    template<typename T = Address>
    __forceinline T sub( size_t offset ) const {
        return T( reinterpret_cast< void* >( m_addr - offset ) );
    }

    template<typename T = Address>
    __forceinline T sub( ptrdiff_t offset ) const {
        return T( reinterpret_cast< void* >( m_addr - offset ) );
    }

    // Dereference.
    template<typename T = Address>
    __forceinline T to( ) const {
        return *reinterpret_cast< T* >( m_addr );
    }

    // Verify address and dereference n times.
    template<typename T = Address>
    __forceinline T get( size_t n = 1 ) const {
        uintptr_t out;

        if ( !m_addr )
            return T{};

        out = m_addr;

        for ( size_t i = n; i > 0; --i ) {
            // Can't dereference, return null.
            if ( !valid( out ) )
                return T{};

            out = *reinterpret_cast< uintptr_t* >( out );
        }

        return T( reinterpret_cast< void* >( out ) );
    }

    // Follow relative8 and relative16/32 offsets.
    template<typename T = Address>
    __forceinline T rel8( size_t offset ) const {
        uintptr_t out;
        uint8_t r;

        if ( !m_addr )
            return T{};

        out = m_addr + offset;

        // Get relative offset.
        r = *reinterpret_cast< uint8_t* >( out );
        if ( !r )
            return T{};

        // Relative to address of next instruction.
        if ( r < 128 )
            out = ( out + 1 ) + r;
        else
            out = ( out + 1 ) - ( uint8_t )( ~r + 1 );

        return T( reinterpret_cast< void* >( out ) );
    }

    template<typename T = Address>
    __forceinline T rel32( size_t offset ) const {
        uintptr_t out;
        uint32_t r;

        if ( !m_addr )
            return T{};

        out = m_addr + offset;

        // Get rel32 offset.
        r = *reinterpret_cast< uint32_t* >( out );
        if ( !r )
            return T{};

        // Relative to address of next instruction.
        out = ( out + 4 ) + r;

        return T( reinterpret_cast< void* >( out ) );
    }

    // Set.
    template<typename T = uintptr_t>
    __forceinline void set( const T& value ) const {
        if ( !m_addr )
            return;

        *reinterpret_cast< T* >( m_addr ) = value;
    }

    // Checks if address is not null and has correct page protection.
    static __forceinline bool valid( uintptr_t addr ) {
        MEMORY_BASIC_INFORMATION mbi;

        // Check for invalid address.
        if ( !addr )
            return false;

        // Check for invalid page protection.
        if ( !VirtualQuery( reinterpret_cast< const void* >( addr ), &mbi, sizeof( mbi ) ) )
            return false;

        // Check for read/write access.
        if ( ( mbi.Protect & PAGE_NOACCESS ) || ( mbi.Protect & PAGE_GUARD ) )
            return false;

        return true;
    }

    // Relative virtual address.
    template<typename T = Address>
    static __forceinline T RVA( const Address& base, size_t offset ) {
        return base.as<T>( offset );
    }
};
