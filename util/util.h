#pragma once

namespace util {
    // Memory copy utility using __movsb intrinsic.
    __forceinline Address copy( Address dst, Address src, size_t size ) {
        __movsb(
            dst.as<uint8_t*>( ),
            src.as<uint8_t*>( ),
            size
        );
        return dst;
    }

    // Memory set utility using __stosb intrinsic.
    __forceinline Address set( Address dst, uint8_t val, size_t size ) {
        __stosb(
            dst.as<uint8_t*>( ),
            val,
            size
        );
        return dst;
    }

    template<typename T>
    struct is_function_pointer: std::integral_constant<bool, std::is_pointer<T>::value&& std::is_function<typename std::remove_pointer<T>::type>::value> { };

    // General case: cast between non-function pointer types
    template<typename To, typename From>
    __forceinline std::enable_if_t<!std::is_function_v<From> && !std::is_pointer_v<From>, To> force_cast( From from ) {
        static_assert( sizeof( To ) == sizeof( From ), "Size of To and From must be the same for force_cast." );
        union {
            From from;
            To to;
        } u{ from };
        return u.to;
    }

    // Specialization for function pointers: disallow casting
    template<typename To, typename From>
    __forceinline std::enable_if_t<std::is_function_v<From> || std::is_pointer_v<From>, To> force_cast( From from ) {
        static_assert( !std::is_function_v<To> && !std::is_pointer_v<To>, "Cannot cast function pointers with force_cast." );
        return reinterpret_cast< To >( from );
    }

    // Retrieve a method pointer from a vtable (virtual method table).
    template<typename T = Address>
    __forceinline T get_method( Address this_ptr, size_t index ) {
        return this_ptr.to<T*>( )[ index ];
    }

    // Get the base pointer (EBP on x86 or RBP on x86-64).
    __forceinline uintptr_t GetBasePointer( ) {
        return reinterpret_cast< uintptr_t >( _AddressOfReturnAddress( ) ) - sizeof( uintptr_t );
    }

    // Convert wide string to multi-byte string (UTF-8).
    __forceinline std::string WideToMultiByte( const std::wstring& str ) {
        if ( str.empty( ) ) return {};

        int str_len = g_winapi.WideCharToMultiByte( CP_UTF8, 0, str.data( ), static_cast< int >( str.size( ) ), nullptr, 0, nullptr, nullptr );
        std::string ret( static_cast< size_t >( str_len ), '\0' );

        g_winapi.WideCharToMultiByte( CP_UTF8, 0, str.data( ), static_cast< int >( str.size( ) ), &ret[ 0 ], str_len, nullptr, nullptr );

        return ret;
    }

    // Convert multi-byte string (UTF-8) to wide string.
    __forceinline std::wstring MultiByteToWide( const std::string& str ) {
        if ( str.empty( ) ) return {};

        int str_len = g_winapi.MultiByteToWideChar( CP_UTF8, 0, str.data( ), static_cast< int >( str.size( ) ), nullptr, 0 );
        std::wstring ret( static_cast< size_t >( str_len ), L'\0' );

        g_winapi.MultiByteToWideChar( CP_UTF8, 0, str.data( ), static_cast< int >( str.size( ) ), &ret[ 0 ], str_len );

        return ret;
    }
};
