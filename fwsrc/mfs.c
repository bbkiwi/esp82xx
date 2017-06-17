//Copyright 2015 <>< Charles Lohr Under the MIT/x11 License, NewBSD License or
// ColorChord License.  You Choose.

#include "esp82xxutil.h"
#include "mfs.h"
#include "spi_flash.h"
#include "ets_sys.h"

uint32 mfs_at = 0;

void ICACHE_FLASH_ATTR FindMPFS()
{
	uint32 mfs_check[2];
	EnterCritical();
	flashchip->chip_size = 0x01000000;

	spi_flash_read( MFS_START, mfs_check, sizeof( mfs_check ) );
	if( strncmp( "MPFSMPFS", mfs_check, 8 ) == 0 ) { mfs_at = MFS_START; goto done; }
	
	printf( "MFS Not found at regular address %08x (%08x%08x).\n", MFS_START, mfs_check[0], mfs_check[1] );

	spi_flash_read( MFS_ALTERNATIVE_START, mfs_check, sizeof( mfs_check ) );
	if( strncmp( "MPFSMPFS", mfs_check, 8 ) == 0 ) { mfs_at = MFS_ALTERNATIVE_START; goto done; }

	printf( "MFS Not found at alternative address %08x (%08x%08x).\n", MFS_ALTERNATIVE_START, mfs_check[0], mfs_check[1] );

done:
	printf( "MFS Found at: %08x\n", mfs_at );
	flashchip->chip_size = 0x00080000;
	ExitCritical();
}

extern SpiFlashChip * flashchip;

//Returns 0 on succses.
//Returns size of file if non-empty
//If positive, populates mfi.
//Returns -1 if can't find file or reached end of file list.
/*
  connect to AP, use browser on 192.168.4.1/indexd.html  (which doesn't exist)
      close browser, use backdoor to connect to station, 
      should connect to station
      when reset if back to AP then BUG (noticed in this case takes 3-4 to disconnect when switches)
                 if back to Station with remember credentials then no bug (disconnect fast when switch)
  a uncommented, b and c commented out i.e. orginal code bug happens
  a, b, c commented out bug gone
  a commented out and b added, c out bug back
  a commented out and b added and c added bug gone
  a commented out, b added, c out, d skip while by using while(0) bug back
  a commented out, b added, c out, d all of while loop commented out bug back (but with switch fast)
  a commented out, b added, c out, d all of while loop commented out and e (two lines) out bug gone (and switch fast too)
  PROBLEM related to changing chip_size big then small!!!

*/
int8_t ICACHE_FLASH_ATTR MFSOpenFile( const char * fname, struct MFSFileInfo * mfi )
{
	if( mfs_at == 0 )
	{
// a 
		//FindMPFS(); 
// b
		mfs_at = 0x100000; 
		printf( "\nMFSOpenFile after FindMPFS: %08x\n", mfs_at );
	}
	if( mfs_at == 0 )
	{
		printf( "\nMFSOpenFile Returned with mfs_at 0\n");
		return -1;
	}
// c always return with -1 
//	printf( "\nMFSOpenFile debug return -1 mfs_at: %08x\n", mfs_at );
//	return -1;

	EnterCritical();
// e
	flashchip->chip_size = 0x01000000;
	uint32 ptr = mfs_at;
	struct MFSFileEntry e;
// d 
/*
	while(1)
	{
		spi_flash_read( ptr, (uint32*)&e, sizeof( e ) );		
		ptr += sizeof(e);
		if( e.name[0] == 0xff || ets_strlen( e.name ) == 0 ) break;

		if( ets_strcmp( e.name, fname ) == 0 )
		{
			mfi->offset = e.start;
			mfi->filelen = e.len;
			flashchip->chip_size = 0x00080000;
			ExitCritical();
			printf( "\nMFSOpenFile Returned finding file\n");
			return 0;
		}
	}

*/
// e
	flashchip->chip_size = 0x00080000;
	ExitCritical();
	printf( "\nMFSOpenFile Returned outside while\n");
	return -1;
}

int32_t ICACHE_FLASH_ATTR MFSReadSector( uint8_t* data, struct MFSFileInfo * mfi )
{
	 //returns # of bytes left tin file.
	if( !mfi->filelen )
	{
		printf( "\nMFSReadSector Returned  0\n");
		return 0;
	}

	int toread = mfi->filelen;
	if( toread > MFS_SECTOR ) toread = MFS_SECTOR;

	EnterCritical();
	flashchip->chip_size = 0x01000000;
	spi_flash_read( mfs_at+mfi->offset, (uint32*)data, MFS_SECTOR ); //TODO: should we make this toread?  maybe toread rounded up?
	flashchip->chip_size = 0x00080000;
	ExitCritical();

	mfi->offset += toread;
	mfi->filelen -= toread;
	printf( "\nMFSReadSector Returned  %d\n",mfi->filelen );
	return mfi->filelen;
}



