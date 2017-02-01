#ifndef _STDTYPES_H_
#define _STDTYPES_H_

typedef unsigned char  B1;
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned long  U32;
typedef signed char    I8;
typedef signed short   I16;
typedef signed long    I32;

typedef unsigned char       byte;
typedef unsigned short      word;
typedef unsigned long       dword;

typedef union
{
  U8 u8;
  I8 i8;
  struct
  {
    U8 b0:1, b1:1, b2:1, b3:1, b4:1, b5:1, b6:1, b7:1;
  } bits;
} UU8;

typedef union
{
  U16 u16;
  U8  u8[2];
  I16 i16;
  I8  i8[2];
  struct
  {
    U8 b0 :1, b1 :1, b2 :1, b3 :1, b4 :1, b5 :1, b6 :1, b7 :1,
       b8 :1, b9 :1, b10:1, b11:1, b12:1, b13:1, b14:1, b15:1;
  } bits;
} UU16;

typedef union
{
  U32 u32;
  U16 u16[2];
  U8  u8[4];
  I32 i32;
  I16 i16[2];
  I8  i8[4];
  struct
  {
    U8 b0 :1, b1 :1, b2 :1, b3 :1, b4 :1, b5 :1, b6 :1, b7 :1,
       b8 :1, b9 :1, b10:1, b11:1, b12:1, b13:1, b14:1, b15:1,
       b16:1, b17:1, b18:1, b19:1, b20:1, b21:1, b22:1, b23:1,
       b24:1, b25:1, b26:1, b27:1, b28:1, b29:1, b30:1, b31:1;
  } bits;
} UU32;

#define HIGH(x) ((U8)(x >> 8))
#define LOW(x)  ((U8)(x & 0xFF))


#define byte0(x)        (*((unsigned char *)(((unsigned char *)&x)+0)))
#define byte1(x)        (*((unsigned char *)(((unsigned char *)&x)+1)))
#define byte2(x)        (*((unsigned char *)(((unsigned char *)&x)+2)))
#define byte3(x)        (*((unsigned char *)(((unsigned char *)&x)+3)))




#ifndef false
  #define false 0
#endif
#ifndef FALSE
  #define FALSE 0
#endif

#ifndef true
  #define true 1
#endif
#ifndef TRUE
  #define TRUE 1
#endif

#ifndef null
  #define null 0
#endif
#ifndef NULL
  #define NULL 0
#endif

#endif  /* _STDTYPES_H_ */
