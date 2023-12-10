/*
 * Emulateur 6809e
 *
 * (c) Samuel Devulder 31/12/97
 */


/* types */
typedef char  t_8;
typedef short t_16;
typedef long  t_32;

typedef unsigned char  t_u8;
typedef unsigned short t_u16;
typedef unsigned long  t_u32;

typedef union {
   t_u16 val;
   struct {
   	t_u8  hi; /* inverser si byteorder == little-endian */
	t_u8  lo;
   } hi_lo;
} two_part_reg;

struct registers {
	two_part_reg dp;
	two_part_reg ab;
	t_u16        x;
	t_u16        y;
	t_u16        u;
	t_u16        s;
	t_u16        pc;
} regs;

#define dpr regs.dp.hi_lo.hi
#define ar  regs.ab.hi_lo.hi
#define br  regs.ab.hi_lo.lo
#define xr  regs.x
#define yr  regs.y
#define ur  regs.u
#define sr  regs.s
#define pcr regs.pc

t_u8 ccr;
#define BitC 0x01
#define BitV 0x02
#define BitZ 0x04
#define BitN 0x08


/* helpers function */
#define get_short(p) (*((t_u16*)(p)))

/* mode d'adressage */
t_u8 *op;

t_u16 immed8(void)
{
  return *op;
}

t_u16 immed16(void)
{
  return get_short(op);
}

t_u16 direc(void)
{
  return (*op)|regs.dp.val;
}

t_u16 etend(void)
{
  return get_short(op);
}

/* helper for index */
#define IND_I(x,reg) \
 case (0x00 + x): return  0x00 + reg; break;\
 case (0x01 + x): return  0x01 + reg; break;\
 case (0x02 + x): return  0x02 + reg; break;\
 case (0x03 + x): return  0x03 + reg; break;\
 case (0x04 + x): return  0x04 + reg; break;\
 case (0x05 + x): return  0x05 + reg; break;\
 case (0x06 + x): return  0x06 + reg; break;\
 case (0x07 + x): return  0x07 + reg; break;\
 case (0x08 + x): return  0x08 + reg; break;\
 case (0x09 + x): return  0x09 + reg; break;\
 case (0x0a + x): return  0x0a + reg; break;\
 case (0x0b + x): return  0x0b + reg; break;\
 case (0x0c + x): return  0x0c + reg; break;\
 case (0x0d + x): return  0x0d + reg; break;\
 case (0x0e + x): return  0x0e + reg; break;\
 case (0x0f + x): return  0x0f + reg; break;\
 case (0x10 + x): return -0x10 + reg; break;\
 case (0x11 + x): return -0x0f + reg; break;\
 case (0x12 + x): return -0x0e + reg; break;\
 case (0x13 + x): return -0x0d + reg; break;\
 case (0x14 + x): return -0x0c + reg; break;\
 case (0x15 + x): return -0x0b + reg; break;\
 case (0x16 + x): return -0x0a + reg; break;\
 case (0x17 + x): return -0x09 + reg; break;\
 case (0x18 + x): return -0x08 + reg; break;\
 case (0x19 + x): return -0x07 + reg; break;\
 case (0x1a + x): return -0x06 + reg; break;\
 case (0x1b + x): return -0x05 + reg; break;\
 case (0x1c + x): return -0x04 + reg; break;\
 case (0x1d + x): return -0x03 + reg; break;\
 case (0x1e + x): return -0x02 + reg; break;\
 case (0x1f + x): return -0x01 + reg; break;

t_u16 index(void)
{
  switch(*op++) {
    IND_I(0x00, xr)
    IND_I(0x20, yr)
    IND_I(0x40, ur)
    IND_I(0x60, sr)
  }
}
