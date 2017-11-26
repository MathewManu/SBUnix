#include <sys/defs.h>

#define PIC1_CMD	0x20	
#define PIC1_DATA       0x21	

#define PIC2_CMD	0xA0	
#define PIC2_DATA       0xA1	

#define PIC_INIT_CMD	0x11 

#define ICW4_8086	0x01

#define PIC_EOI		0x20

void mask_pins(unsigned char IRQline); 

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void io_wait(void) {
    __asm__ __volatile__( "outb %%al, $0x80" : : "a"(0) );
}

void pic_offset_init(int offset1, int offset2) {
	unsigned char m1, m2;

	m1 = inb(PIC1_DATA);                     
	m2 = inb(PIC2_DATA);

	outb(PIC1_CMD, PIC_INIT_CMD); 
	io_wait();
	outb(PIC2_CMD, PIC_INIT_CMD);
	io_wait();

	outb(PIC1_DATA, offset1);                
	io_wait();
	outb(PIC2_DATA, offset2);                
	io_wait();
	outb(PIC1_DATA, 4);                     
	io_wait();
	outb(PIC2_DATA, 2);                    
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, m1); 
	outb(PIC2_DATA, m2);

///	mask_pins(0);
//	mask_pins(1);
	mask_pins(2);
	mask_pins(3);
	mask_pins(4);
	mask_pins(5);
	mask_pins(6);
	mask_pins(7);
	mask_pins(8);
	mask_pins(9);
	mask_pins(10);
	mask_pins(11);
	mask_pins(12);
	mask_pins(13);
	mask_pins(14);
	mask_pins(15);
}

void send_EOI(/*unsigned char irq*/) {
//	if(irq >= 8)
//		outb(PIC2_CMD, PIC_EOI);
 
	outb(PIC1_CMD, PIC_EOI);
}

void mask_pins(unsigned char irq) {
	uint16_t port;
	uint8_t value;

	if (irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8; /* slave's irq is 8-15 */
	}

	/* when no command is issued, data port returns the interrupt mask */

	value = inb(port) | (1 << irq);
	outb(port, value);
}

