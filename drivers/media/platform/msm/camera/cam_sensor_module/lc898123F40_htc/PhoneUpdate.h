/**
 * @brief		LC898123F40 Global declaration & prototype declaration
 *
 * @author		Copyright (C) 2017, ON Semiconductor, all right reserved.
 * @file		PhoneUpdate.h
 * @date		svn:$Date:: 2017-04-20 10:57:58 +0900#$
 * @version		svn:$Revision: 00 $
 * @attention
 *
 **/

#ifndef PHONEUPDATE_H_
#define PHONEUPDATE_H_

//==============================================================================
//
//==============================================================================
#define	MODULE_VENDOR	0x05
#define	MDL_VER			0x08

#define	FW_VER			0x07

#ifdef DEBUG
 extern void dbg_printf(const char *, ...);
 extern void dbg_Dump(const char *, int);
 #define TRACE_INIT(x)			dbgu_init(x)
 #define TRACE_USB(fmt, ...)	dbg_UsbData(fmt, ## __VA_ARGS__)
 #define TRACE(fmt, ...)		dbg_printf(fmt, ## __VA_ARGS__)
 #define TRACE_DUMP(x,y)		dbg_Dump(x,y)
#else
 #define TRACE_INIT(x)
 #define TRACE(...)
 #define TRACE_DUMP(x,y)
 #define TRACE_USB(...)
#endif

/*HTC_START*/
#if 0
typedef	signed char			 INT_8;
typedef	short				 INT_16;
typedef	long                 INT_32;
typedef	long long            INT_64;
typedef	unsigned char       UINT_8;
typedef	unsigned short      UINT_16;
typedef	unsigned long       UINT_32;
typedef	unsigned long long	UINT_64;
#else
typedef	signed char			 INT_8;
typedef	signed short	 INT_16;
typedef	long                 INT_32;
typedef	long long            INT_64;
typedef	unsigned char       UINT_8;
typedef	unsigned short      UINT_16;
typedef	unsigned int       UINT_32;
typedef	unsigned long long	UINT_64;
#endif
/*HTC_END*/

//****************************************************
//	STRUCTURE DEFINE
//****************************************************
typedef struct {
	UINT_16				Index;
	const UINT_8*		MagicCode;
	UINT_16				SizeMagicCode;
	const UINT_8*		FromCode;
	UINT_16				SizeFromCode;
}	DOWNLOAD_TBL;

typedef struct STRECALIB {
	INT_16	SsFctryOffX ;
	INT_16	SsFctryOffY ;
	INT_16	SsRecalOffX ;
	INT_16	SsRecalOffY ;
	INT_16	SsDiffX ;
	INT_16	SsDiffY ;
} stReCalib ;


/*** caution [little-endian] ***/
#ifdef _BIG_ENDIAN_
typedef union	DWDVAL {
	UINT_32	UlDwdVal ;
	UINT_16	UsDwdVal[ 2 ] ;
	struct {
		UINT_16	UsHigVal ;
		UINT_16	UsLowVal ;
	} StDwdVal ;
	struct {
		UINT_8	UcRamVa3 ;
		UINT_8	UcRamVa2 ;
		UINT_8	UcRamVa1 ;
		UINT_8	UcRamVa0 ;
	} StCdwVal ;
}	UnDwdVal ;
#else	// BIG_ENDDIAN
typedef union	DWDVAL {
	UINT_32	UlDwdVal ;
	UINT_16	UsDwdVal[ 2 ] ;
	struct {
		UINT_16	UsLowVal ;
		UINT_16	UsHigVal ;
	} StDwdVal ;
	struct {
		UINT_8	UcRamVa0 ;
		UINT_8	UcRamVa1 ;
		UINT_8	UcRamVa2 ;
		UINT_8	UcRamVa3 ;
	} StCdwVal ;
}	UnDwdVal ;
#endif	// _BIG_ENDIAN_

#define	WPB_OFF			0x01
#define WPB_ON			0x00
#define	SUCCESS			0x00
#define	FAILURE			0x01

//==============================================================================
//
//==============================================================================
#define		F40_IO_ADR_ACCESS				0xC000
#define		F40_IO_DAT_ACCESS				0xD000
#define 	SYSDSP_DSPDIV					0xD00014
#define 	SYSDSP_SOFTRES					0xD0006C
#define 	OSCRSEL							0xD00090
#define 	OSCCURSEL						0xD00094
#define 	SYSDSP_REMAP					0xD000AC
#define 	SYSDSP_CVER						0xD00100
#define	 	FLASHROM_123F40					0xE07000
#define 	FLASHROM_F40_RDATL				(FLASHROM_123F40 + 0x00)
#define 	FLASHROM_F40_RDATH				(FLASHROM_123F40 + 0x04)
#define 	FLASHROM_F40_WDATL				(FLASHROM_123F40 + 0x08)
#define 	FLASHROM_F40_WDATH				(FLASHROM_123F40 + 0x0C)
#define 	FLASHROM_F40_ADR				(FLASHROM_123F40 + 0x10)
#define 	FLASHROM_F40_ACSCNT				(FLASHROM_123F40 + 0x14)
#define 	FLASHROM_F40_CMD				(FLASHROM_123F40 + 0x18)
#define 	FLASHROM_F40_WPB				(FLASHROM_123F40 + 0x1C)
#define 	FLASHROM_F40_INT				(FLASHROM_123F40 + 0x20)
#define 	FLASHROM_F40_RSTB_FLA			(FLASHROM_123F40 + 0x4CC)
#define 	FLASHROM_F40_UNLK_CODE1			(FLASHROM_123F40 + 0x554)
#define 	FLASHROM_F40_CLK_FLAON			(FLASHROM_123F40 + 0x664)
#define 	FLASHROM_F40_UNLK_CODE2			(FLASHROM_123F40 + 0xAA8)
#define 	FLASHROM_F40_UNLK_CODE3			(FLASHROM_123F40 + 0xCCC)

#define		READ_STATUS_INI					0x01000000



//==============================================================================
// Prototype
//==============================================================================
extern void		F40_Control( UINT_16 ) ;
extern void		F40_IORead32A( UINT_32, UINT_32 * ) ;
extern void		F40_IOWrite32A( UINT_32, UINT_32 ) ;
extern UINT_8	F40_ReadWPB( void ) ;
extern UINT_8	F40_UnlockCodeSet( void ) ;
extern UINT_8	F40_UnlockCodeClear(void) ;
extern UINT_8	F40_FlashDownload( UINT_8, UINT_8, UINT_8 ) ;
extern UINT_8	F40_FlashUpdate( UINT_8, DOWNLOAD_TBL* ) ;
extern UINT_8	F40_FlashBlockErase( UINT_32 ) ;
extern UINT_8	F40_FlashBurstWrite( const UINT_8 *, UINT_32, UINT_32) ;
extern void		F40_FlashSectorRead( UINT_32, UINT_8 * ) ;
//HTC_START
extern void 	F40_FlashSectorRead_htc( UINT_32 UlAddress, UINT_8 *PucData, UINT_32 version );
extern void 	F40_FlashSectorRead_htc_IME( UINT_32 UlAddress, UINT_8 *PucData, UINT_32 version );
//HTC_END
extern UINT_8	F40_FlashSectorErase( UINT_32 ) ;
extern UINT_8	F40_FlashSectorWrite( UINT_32, UINT_8 * );
extern UINT_8	F40_FlashInt32Read( UINT_32, UINT_32 * ) ;
extern UINT_8	F40_FlashInt32Write( UINT_32, UINT_32 *, UINT_8 ) ;
extern void		F40_CalcChecksum( const UINT_8 *, UINT_32, UINT_32 *, UINT_32 * ) ;
extern void		F40_CalcBlockChksum( UINT_8, UINT_32 *, UINT_32 * ) ;
extern void		F40_ReadCalData( UINT_32 * , UINT_32 *  ) ;
extern UINT_8	F40_GyroReCalib( stReCalib *  ) ;
extern UINT_8	F40_WrGyroOffsetData( void ) ;
extern UINT_8	F40_RdStatus( UINT_8  ) ;
extern UINT_8	F40_WriteCalData( UINT_32 * , UINT_32 *  );

#endif /* #ifndef PHONEUPDATE_H_ */
