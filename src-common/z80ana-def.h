#ifndef Z80ANA_DEF_H_INCLUDED
#define Z80ANA_DEF_H_INCLUDED

#define __z88dk_fastcall
#define __naked

#define __z80ana
#define __z80ana_macro
#define Z80ANA_DEF_VARS volatile int A, B, C, D, E, F, H, L, IXH, IXL, IYH, IYL, AF, BC, DE, HL, PC, SP, I, R, IX, IY, p, m, nz, z, po, pe, nv, v, nc, c, nz_r, z_r, nc_r, c_r, *mem, *port, asmpc;
#define Z80ANA_LOCAL(...)
#define Z80ANA_IF(expr)
#define Z80ANA_ELIF(expr)
#define Z80ANA_ELSE
#define Z80ANA_ENDIF
#define Z80ANA_REPT(expr)
#define Z80ANA_ENDR
#define Z80ANA_DB(...)
#define Z80ANA_DW(...)

void defb(int, ...);
void defw(int, ...);
void ld(int, int);
void ldi();
void ldd();
void ldir();
void lddr();
void ex(int, int);
void exx();
void push(int, ...);
void pop(int, ...);
void and(int, int);
void or(int, int);
void xor(int, int);
void cpl(int);
void not(int);
void cp(int, int);
void cpi();
void cpd();
void cpir();
void cpdr();
void add(int, int);
void adc(int, int);
void sub(int, int);
void sbc(int, int);
void inc(int);
void dec(int);
void neg(int);
void daa(int);
void bit(int, int);
void set(int, int);
void res(int, int);
void bit_3(int, int, int);
void set_3(int, int, int);
void res_3(int, int, int);
void sla(int);
void sla_n(int, int);
void sla_2(int, int);
void sll(int);
void sll_n(int, int);
void sll_2(int, int);
void sra(int);
void sra_n(int, int);
void sra_2(int, int);
void srl(int);
void srl_n(int, int);
void srl_2(int, int);
void rlc(int);
void rlc_n(int, int);
void rlc_2(int, int);
void rlca(int);
void rlca_n(int, int);
void rl(int);
void rl_n(int, int);
void rl_2(int, int);
void rla(int);
void rla_n(int, int);
void rrc(int);
void rrc_n(int, int);
void rrc_2(int, int);
void rrca(int);
void rrca_n(int, int);
void rr(int);
void rr_n(int, int);
void rr_2(int, int);
void rra(int);
void rra_n(int, int);
void rld(int, int);
void rrd(int, int);
void jp(int);
void jp_z(int);
void jp_nz(int);
void jp_c(int);
void jp_nc(int);
void jp_p(int);
void jp_m(int);
void jp_v(int);
void jp_nv(int);
void jp_pe(int);
void jp_po(int);
void jr(int);
void jr_z(int);
void jr_nz(int);
void jr_c(int);
void jr_nc(int);
void djnz(int, int);
void call(int);
void call_z(int);
void call_nz(int);
void call_c(int);
void call_nc(int);
void call_p(int);
void call_m(int);
void call_v(int);
void call_nv(int);
void call_pe(int);
void call_po(int);
void rst(int);
void ret();
void ret_z();
void ret_nz();
void ret_c();
void ret_nc();
void ret_p();
void ret_m();
void ret_v();
void ret_nv();
void ret_pe();
void ret_po();
void reti();
void retn();
void nop();
void halt();
void di();
void ei();
void im0();
void im1();
void im2();
void scf(int);
void ccf(int);
void in(int, int);
void out(int, int);
void ini();
void ind();
void inir();
void indr();
void outi();
void outd();
void otir();
void otdr();

#endif // Z80ANA_DEF_H_INCLUDED
