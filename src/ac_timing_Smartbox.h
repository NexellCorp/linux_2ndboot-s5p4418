
// Clock List
#define		DDR3_800	5
#define		DDR3_666	6
#define		DDR3_533	7
#define		DDR3_TEST	8


// User Define
#define		MEM_CLK		DDR3_800
#define		ChipNum		1				// nCS Number : Tablet=1, VTK=2
#define 	BUSWIDTH 	16				// 16bit, 8bit



// Refer to Memory Datasheet
#define		chip_row	3				// ROW address bit : 15bit : 3(Tabletx2), 16bit : 4(Tabletx4, Elcomtech)
//#define		chip_col	3				// Column Address Bit. 2:9bit, 3:10bit, others:Reserved
#define 	chip_mask	0x7C0			// Capacity per nCS: 2G=0x780(Tabletx4, Elcomtech), 1G=0x7C0(Tabletx2, VTK)
#define 	CWL			3				// CAS Write Latency(CWL).
										// 0:5ck(tCK>2.5ns), 1:6ck(2.5ns>tCK>1.875ns), 2:7ck(1.875ns>tCK>2.5ns), 3:8ck(1.5ns>tCK>1.25ns),
										// 4:9ck(1.25ns>tCK>1.07ns), 5:10ck(1.07ns>tCK>0.935ns), 6:11ck(0.935ns>tCK>0.833ns), 7:12ck(0.833ns>tCK>0.75ns)
#define		WL			0x8				// WL=CWL+AL
#define		RL			0xB				// RL=CAS latency + AL.
#define		RDFETCH		0x1				// CONCONTROL.rd_fetch[14:12]

#define   	PHYCON4		0x0C0C0C0C		//- ctrl_offsetr, 0x0C0C0C0C, 0x18181818
#define   	PHYCON6		0x02020202		//- ctrl_offsetw, 0x04040404, 0x02020202



// Option
#define		CONFIG_ODTOFF_GATELEVELINGON		0		// Not support yet.
#define 	MEMTEST		0		// Not support yet.
#define 	REG_MSG		1
#define		QOS1		1



// AC Timing
#if 1	// 800MHz Over
#if (MEM_CLK == DDR3_800)

#define		tREF		0x618	// 7.8us

#define 	tRFC		0x50
#define 	tRRD		0x2
#define		tRP			0x4
#define 	tRCD		0X5		//0x6
#define		tRC			0xe
#define		tRAS		0xd

#define		tWTR		0x2
#define		tWR			0x4
#define		tRTP		0x2
#define		W2W			0x1
#define		R2R			0x1

#define 	tFAW		0xC
#define 	tXSR		0x6C
#define 	tXP			0x3
#define		tCKE		0x3
#define		tMRD		0x4

#endif

#else	// 800MHz Typ
#if (MEM_CLK == DDR3_800)

#define		tREF		0x618

#define 	tRFC		0x68
#define 	tRRD		0x4
#define		tRP			0x6
#define 	tRCD		0X6
#define		tRC			0x14
#define		tRAS		0xF

#define		tWTR		0x4
#define		tWR			0x6
#define		tRTP		0x4
#define		W2W			0x1
#define		R2R			0x1

#define 	tFAW		0xC
#define 	tXSR		0x6C
#define 	tXP			0x3
#define		tCKE		0x3
#define		tMRD		0x4

#endif
#endif

#if (MEM_CLK == DDR3_666)

#define		tREF		0x513

#define 	tRFC		0x57
#define 	tRRD		0x4
#define		tRP			0x5
#define 	tRCD		0x6
#define		tRC			0x10
#define		tRAS		0xC

#define		tWTR		0x4
#define		tWR			0x5
#define		tRTP		0x4
#define		W2W_C2C		0x1
#define		R2R_C2C		0x1

#define 	tFAW		0xA
#define 	tXSR		0x5A
#define 	tXP			0x2
#define		tCKE		0x3
#define		tMRD		0x4

#endif


#if (MEM_CLK == DDR3_533)

#define		tREF		0x40E

#define 	tRFC		0x46
#define 	tRRD		0x4
#define		tRP			0x4
#define 	tRCD		0x5
#define		tRC			0xD
#define		tRAS		0xA

#define		tWTR		0x4
#define		tWR			0x4
#define		tRTP		0x4
#define		W2W_C2C		0x1
#define		R2R_C2C		0x1

#define 	tFAW		0x8
#define 	tXSR		0x48
#define 	tXP			0x2
#define		tCKE		0x3
#define		tMRD		0x4

#endif

