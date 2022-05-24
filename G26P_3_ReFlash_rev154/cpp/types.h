#ifndef	GF_TYPES_H__10_03_2006__13_23
#define	GF_TYPES_H__10_03_2006__13_23

#include <windows.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ArraySize(x) (sizeof(x)/sizeof(*(x)))

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef unsigned char			byte, uint8, u8;
typedef unsigned short int		word, uint, uint16, u16;
typedef unsigned long int		dword, uint32, u32;
typedef short int				sint, int16, i16;
typedef long int				lint, int32, i32;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union DataCRC
{
	word	w;
	byte	b[2];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union U16u
{
	word w; byte b[2]; 

	U16u(word v) {w = v;}
	operator word() {return w;}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union U32u 
{ 
	dword d; word w[2]; byte b[4]; 

	U32u (dword v) {d = v;}
	operator dword() {return d;}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union F32u
{ 
	float f; dword d; word w[2]; byte b[4]; 

	F32u (float v) {f = v;}
	operator float() {return f;}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct DataPointer
{
	union
	{
		void	*v;
		byte	*b;
		word	*w;
		dword	*d;
		float	*f;
	};

	DataPointer(void *p) { v = p; } 
	DataPointer(byte *p) { b = p; } 
	DataPointer(word *p) { w = p; } 
	DataPointer(dword *p) { d = p; } 
	DataPointer(float *p) { f = p; } 

	void operator=(void *p) { v = p; } 
	void operator=(byte *p) { b = p; } 
	void operator=(word *p) { w = p; } 
	void operator=(dword *p) { d = p; } 
	void operator=(float *p) { f = p; } 

	operator void*() { return v; }
	operator byte*() { return b; }
	operator word*() { return w; }
	operator dword*() { return d; }
	operator float*() { return f; }

	operator void const*() { return v; }
	operator byte const*() { return b; }
	operator word const*() { return w; }
	operator dword const*() { return d; }
	operator float const*() { return f; }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct ConstDataPointer
{
	union
	{
		const void	*v;
		const byte	*b;
		const word	*w;
		const dword	*d;
		const float	*f;
	};

	ConstDataPointer(const void *p) { v = p; } 
	ConstDataPointer(const byte *p) { b = p; } 
	ConstDataPointer(const word *p) { w = p; } 
	ConstDataPointer(const dword *p) { d = p; } 
	ConstDataPointer(const float *p) { f = p; } 

	void operator=(const void *p) { v = p; } 
	void operator=(const byte *p) { b = p; } 
	void operator=(const word *p) { w = p; } 
	void operator=(const dword *p) { d = p; } 
	void operator=(const float *p) { f = p; } 

	operator void const*() { return v; }
	operator byte const*() { return b; }
	operator word const*() { return w; }
	operator dword const*() { return d; }
	operator float const*() { return f; }
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
struct FarPointerCRC
{
	union
	{
		void	far *v;
		byte 	far *b;
		word 	far *w;
		dword 	far *d;
		float 	far *f;
	};

	union { word w; byte b[2]; } CRC;

	FarPointerCRC(void far *p) { v = p; } 
	FarPointerCRC(byte far *p) { b = p; } 
	FarPointerCRC(word far *p) { w = p; } 
	FarPointerCRC(dword far *p) { d = p; } 
	FarPointerCRC(float far *p) { f = p; } 

	void operator=(void far *p) { v = p; } 
	void operator=(byte far *p) { b = p; } 
	void operator=(word far *p) { w = p; } 
	void operator=(dword far *p) { d = p; } 
	void operator=(float far *p) { f = p; } 

	void WriteB(byte var) {*(b++) = var; CRC.w = tableCRC[CRC.b[0] ^ var] ^ CRC.b[1]; }
	void WriteW(WORD var) {*(w++) = var; CRC.w = tableCRC[CRC.b[0] ^ var.b[0]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ var.b[1]] ^ CRC.b[1];}
	void WriteD(DWORD var){*(d++) = var; CRC.w = tableCRC[CRC.b[0] ^ var.b[0]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ var.b[1]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ var.b[2]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ var.b[3]] ^ CRC.b[1]; }
	void WriteF(FLOAT var){*(d++) = var.d; 	CRC.w = tableCRC[CRC.b[0] ^ var.b[0]] ^ CRC.b[1];CRC.w = tableCRC[CRC.b[0] ^ var.b[1]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ var.b[2]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ var.b[3]] ^ CRC.b[1]; }

	byte ReadB() { CRC.w = tableCRC[CRC.b[0] ^ *b] ^ CRC.b[1]; return *(b++); }
	word ReadW() { CRC.w = tableCRC[CRC.b[0] ^ b[0]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[1]] ^ CRC.b[1]; return *(w++); 	}
	dword ReadD(){ CRC.w = tableCRC[CRC.b[0] ^ b[0]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[1]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[2]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[3]] ^ CRC.b[1]; return *(d++); }
	float ReadF(){ CRC.w = tableCRC[CRC.b[0] ^ b[0]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[1]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[2]] ^ CRC.b[1]; CRC.w = tableCRC[CRC.b[0] ^ b[4]] ^ CRC.b[1]; return *(f++); }

	void WriteArrayB(ConstDataPointer p, word count);
	void WriteArrayW(ConstDataPointer p, word count);
	void WriteArrayD(ConstDataPointer p, word count);
	void WriteArrayF(ConstDataPointer p, word count);

	void ReadArrayB(DataPointer p, word count);
	void ReadArrayW(DataPointer p, word count);
	void ReadArrayD(DataPointer p, word count);
	void ReadArrayF(DataPointer p, word count);

};
*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



#endif // GF_TYPES_H__10_03_2006__13_23