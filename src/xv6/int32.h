// define our structure
struct __attribute__ ((packed)) regs16
{
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
};

// Map some lowmem pages for BIOS call purposes. The implementations of these live in vm.c for now. 
pte_t biosmap();
void biosunmap(pte_t original);

// tell compiler our int32 function is external
extern void int32(unsigned char intnum, struct regs16 *regs);
