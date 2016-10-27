#ifndef __TYPE_H__
#define __TYPE_H__

#define IO_ADDRESS(x)           (x)

#define mmio_read_32(addr)          (*(volatile unsigned int  *)(addr))
#define mmio_read_16(addr)          (*(volatile unsigned short*)(addr))
#define mmio_read_8(addr)           (*(volatile unsigned char *)(addr))

#define mmio_write_32(addr,data)    (*(volatile unsigned int  *)(addr))  =  ((unsigned int  )(data))
#define mmio_write_16(addr,data)    (*(volatile unsigned short*)(addr))  =  ((unsigned short)(data))
#define mmio_write_8(addr,data)     (*(volatile unsigned char *)(addr))  =  ((unsigned char )(data))

#define mmio_set_32(addr,data)      (*(volatile unsigned int  *)(addr)) |=  ((unsigned int  )(data))
#define mmio_set_16(addr,data)      (*(volatile unsigned short*)(addr)) |=  ((unsigned short)(data))
#define mmio_set_8(addr,data)       (*(volatile unsigned char *)(addr)) |=  ((unsigned char )(data))

#define mmio_clear_32(addr,data)    (*(volatile unsigned int  *)(addr)) &= ~((unsigned int  )(data))
#define mmio_clear_16(addr,data)    (*(volatile unsigned short*)(addr)) &= ~((unsigned short)(data))
#define mmio_clear_8(addr,data)     (*(volatile unsigned char *)(addr)) &= ~((unsigned char )(data))

#define readb(addr)             mmio_read_8(addr)
#define readw(addr)             mmio_read_16(addr)
#define readl(addr)             mmio_read_32(addr)

#define writeb(data, addr)      ({U8  *_v = (U8 *)addr;  mmio_write_8(_v, data);})
#define writew(data, addr)      ({U16 *_v = (U16 *)addr; mmio_write_16(_v, data);})
#define writel(data, addr)      ({U32 *_v = (U32 *)addr; mmio_write_32(_v, data);})

#define u8                      unsigned char
#define u16                     unsigned short
#define u32                     unsigned int
#define u64                     unsigned long long

#define s8                      char
#define s16                     short
#define s32                     int
#define s64                     long long
#endif
