#ifndef Z80ANA_H_INCLUDED
#define Z80ANA_H_INCLUDED

#define __z88dk_fastcall
#define __naked

#define __z80ana
#define __z80ana_macro
#define Z80ANA_DEF_VARS volatile int A, B, C, D, E, F, H, L, IXH, IXL, IYH, IYL, AF, BC, DE, HL, PC, SP, I, R, IX, IY,\
    z, eq, nz, ne, c, lt, nc, ge, p, m, v, nv, pe, po,\
    z_er, eq_er, nz_er, ne_er, c_er, lt_er, nc_er, ge_er, p_er, m_er, v_er, nv_er, pe_er, po_er,\
    z_r, eq_r, nz_r, ne_r, c_r, lt_r, nc_r, ge_r,\
    z_rr, eq_rr, nz_rr, ne_rr, c_rr, lt_rr, nc_rr, ge_rr, \
    *mem, *port, asmpc
#define Z80ANA_DEF_DUMMY_VARS(...) volatile int __VA_ARGS__; // 任意の使用しない変数を定義
#define Z80ANA_LOCAL(...)
#define Z80ANA_IF(expr)
#define Z80ANA_ELIF(expr)
#define Z80ANA_ELSE
#define Z80ANA_ENDIF
#define Z80ANA_REPT(expr)
#define Z80ANA_REPTC(var, expr)
#define Z80ANA_REPTI(var, expr, ...)
#define Z80ANA_ENDR
#define Z80ANA_DB(...)
#define Z80ANA_DW(...)
#define Z80ANA_ENDM
#define Z80ANA_NO_RETURN
#define Z80ANA_FALL_THROUGH

void defb(int, ...);
void defw(int, ...);
void ld(int, int);
void ldi(void);
void ldd(void);
void ldir(void);
void lddr(void);
void ex(int, int);
void exx(void);
void push(int, ...);
void pop(int, ...);
void and(int, int);
void or(int, int);
void xor(int, int);
void cpl(int);
void not(int);
void cp(int, int);
void cpi(void);
void cpd(void);
void cpir(void);
void cpdr(void);
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
void jp_eq(int);    // ==
void jp_nz(int);
void jp_ne(int);    // !=
void jp_c(int);
void jp_lt(int);    // <
void jp_nc(int);
void jp_ge(int);    // >=
void jp_p(int);
void jp_m(int);
void jp_v(int);
void jp_nv(int);
void jp_pe(int);
void jp_po(int);
void jr(int);
void jr_z(int);
void jr_eq(int);    // ==
void jr_nz(int);
void jr_ne(int);    // !=
void jr_c(int);
void jr_lt(int);    // <
void jr_nc(int);
void jr_ge(int);    // >=
void djnz(int, int);
void call(int);
void call_z(int);
void call_eq(int);  // ==
void call_nz(int);
void call_ne(int);  // !=
void call_c(int);
void call_lt(int);  // <
void call_nc(int);
void call_ge(int);  // >=
void call_p(int);
void call_m(int);
void call_v(int);
void call_nv(int);
void call_pe(int);
void call_po(int);
void rst(int);
void ret(void);
void ret_z(void);
void ret_eq(void);  // ==
void ret_nz(void);
void ret_ne(void);  // !=
void ret_c(void);
void ret_lt(void);  // <
void ret_nc(void);
void ret_ge(void);  // >
void ret_p(void);
void ret_m(void);
void ret_v(void);
void ret_nv(void);
void ret_pe(void);
void ret_po(void);
void reti(void);
void retn(void);
void nop(void);
void halt(void);
void di(void);
void ei(void);
void im0(void);
void im1(void);
void im2(void);
void scf(int);
void ccf(int);
void in(int, int);
void out(int, int);
void ini(void);
void ind(void);
void inir(void);
void indr(void);
void outi(void);
void outd(void);
void otir(void);
void otdr(void);

#endif // Z80ANA_H_INCLUDED
