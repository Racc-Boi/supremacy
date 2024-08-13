#include "crc32.h"
#define LittleLong( val )	( val )

#define CRC32_INIT_VALUE 0xFFFFFFFFUL
#define CRC32_XOR_VALUE  0xFFFFFFFFUL

void CRC32::init( CRC32_t* pulCRC ) {
	*pulCRC = CRC32_INIT_VALUE;
}

void CRC32::final( CRC32_t* pulCRC ) {
	*pulCRC ^= CRC32_XOR_VALUE;
}

CRC32_t CRC32::getTableEntry( unsigned int nSlot ) {
	return pulCRCTable[ ( unsigned char )nSlot ];
}

void CRC32::processBuffer( CRC32_t* pulCRC, const void* pBuffer, int nBuffer ) {
	CRC32_t ulCrc = *pulCRC;
	unsigned char* pb = ( unsigned char* )pBuffer;
	unsigned int nFront;
	int nMain;

JustAfew:

	switch ( nBuffer ) {
		case 7:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );

		case 6:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );

		case 5:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );

		case 4:
			ulCrc ^= LittleLong( *( CRC32_t* )pb );
			ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
			ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
			ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
			ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
			*pulCRC = ulCrc;
			return;

		case 3:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );

		case 2:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );

		case 1:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );

		case 0:
			*pulCRC = ulCrc;
			return;
	}

	// We may need to do some alignment work up front, and at the end, so that
	// the main loop is aligned and only has to worry about 8 byte at a time.
	//
	// The low-order two bits of pb and nBuffer in total control the
	// upfront work.
	//
	nFront = ( ( unsigned int )pb ) & 3;
	nBuffer -= nFront;
	switch ( nFront ) {
		case 3:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		case 2:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		case 1:
			ulCrc = pulCRCTable[ *pb++ ^ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
	}

	nMain = nBuffer >> 3;
	while ( nMain-- ) {
		ulCrc ^= LittleLong( *( CRC32_t* )pb );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc ^= LittleLong( *( CRC32_t* )( pb + 4 ) );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		ulCrc = pulCRCTable[ ( unsigned char )ulCrc ] ^ ( ulCrc >> 8 );
		pb += 8;
	}

	nBuffer &= 7;
	goto JustAfew;
}