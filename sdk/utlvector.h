template <class T, class N = int>
class CUtlMemory {
public:
	T& operator[]( N i ) {
		return memory[ i ];
	}

	const T& operator[]( N i ) const {
		return memory[ i ];
	}

	T* Base( ) {
		return memory;
	}

	[[nodiscard]] int NumAllocated( ) const {
		return allocationCount;
	}

	[[nodiscard]] bool IsExternallyAllocated( ) const {
		return growSize < 0;
	}

	void Grow( const int iCount = 1 ) {
		if ( IsExternallyAllocated( ) )
			return;

		int allocationRequested = allocationCount + iCount;
		int newAllocationCount = 0;

		if ( growSize )
			allocationCount = ( ( 1 + ( ( allocationRequested - 1 ) / growSize ) ) * growSize );
		else {
			if ( !allocationCount )
				allocationCount = ( 31 + sizeof( T ) ) / sizeof( T );

			while ( allocationCount < allocationRequested )
				allocationCount <<= 1;
		}

		if ( static_cast< int >( static_cast< N >( newAllocationCount ) ) < allocationRequested ) {
			if ( static_cast< int >( static_cast< N >( newAllocationCount ) ) == 0 && static_cast< int >( static_cast< N >( newAllocationCount - 1 ) ) >= allocationRequested )
				--newAllocationCount;
			else {
				if ( static_cast< int >( static_cast< N >( allocationRequested ) ) != allocationRequested )
					return;

				while ( static_cast< int >( static_cast< N >( newAllocationCount ) ) < allocationRequested )
					newAllocationCount = ( newAllocationCount + allocationRequested ) / 2;
			}
		}

		allocationCount = newAllocationCount;

		if ( memory != nullptr )
			memory = static_cast< T* >( realloc( memory, allocationCount * sizeof( T ) ) );
		else
			memory = static_cast< T* >( malloc( allocationCount * sizeof( T ) ) );
	}

protected:
	T* memory = nullptr;
	int allocationCount;
	int growSize;
};

template <class T>
void Destruct( T* pMemory ) {
	pMemory->~T( );
}

template <class T>
T* Construct( T* pMemory ) {
	return new( pMemory ) T;
}

template< class T >
T* Copy( T* pMemory, T const& src ) {
	return new( pMemory ) T( src );
}

template <class T, class A = CUtlMemory<T>>
class CUtlVector {
	using CAllocator = A;
public:
	auto begin( ) noexcept {
		return pMemory.Base( );
	}

	auto end( ) noexcept {
		return pMemory.Base( ) + iSize;
	}

	auto begin( ) const noexcept {
		return pMemory.Base( );
	}

	auto end( ) const noexcept {
		return pMemory.Base( ) + iSize;
	}

	T& operator[]( int i ) {
		return pMemory[ i ];
	}

	const T& operator[]( int i ) const {
		return pMemory[ i ];
	}

	T& Element( int i ) {
		return pMemory[ i ];
	}

	const T& Element( int i ) const {
		return pMemory[ i ];
	}

	T* Base( ) {
		return pMemory.Base( );
	}

	int Count( ) const {
		return iSize;
	}

	int& Size( ) {
		return iSize;
	}

	void GrowVector( int nCount = 1 ) {
		if ( iSize + nCount > pMemory.NumAllocated( ) )
			pMemory.Grow( iSize + nCount - pMemory.NumAllocated( ) );

		iSize += nCount;
	}

	void ShiftElementsRight( const int nElement, const int nShift = 1 ) {
		const int nToMove = iSize - nElement - nShift;

		if ( nToMove > 0 && nShift > 0 )
			memmove( &Element( nElement + nShift ), &Element( nElement ), nToMove * sizeof( T ) );
	}

	void ShiftElementsLeft( const int nElement, const int nShift = 1 ) {
		const int nToMove = iSize - nElement - nShift;

		if ( nToMove > 0 && nShift > 0 )
			memmove( &Element( nElement ), &Element( nElement + nShift ), nToMove * sizeof( T ) );
	}

	int InsertBefore( const int nElement ) {
		// can insert at the end
		GrowVector( );
		ShiftElementsRight( nElement );
		Construct( &Element( nElement ) );
		return nElement;
	}

	int InsertBefore( int nElement, const T& src ) {
		// reallocate if can't insert something that's in the list
		// can insert at the end
		GrowVector( );
		ShiftElementsRight( nElement );
		Copy( &Element( nElement ), src );
		return nElement;
	}

	int AddToTail( ) {
		return InsertBefore( iSize );
	}

	int AddToTail( const T& src ) {
		return InsertBefore( iSize, src );
	}

	int Find( const T& src ) const {
		for ( int i = 0; i < Count( ); ++i ) {
			if ( Element( i ) == src )
				return i;
		}

		return -1;
	}

	void Remove( const int nElement ) {
		Destruct( &Element( nElement ) );
		ShiftElementsLeft( nElement );
		--iSize;
	}

	void RemoveAll( ) {
		for ( int i = iSize; --i >= 0;)
			Destruct( &Element( i ) );

		iSize = 0;
	}

	bool FindAndRemove( const T& src ) {
		if ( const int nElement = Find( src ); nElement != -1 ) {
			Remove( nElement );
			return true;
		}

		return false;
	}

protected:
	CAllocator pMemory;
	int iSize;
	T* pElements;
};