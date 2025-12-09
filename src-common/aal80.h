#ifndef AAL_H_INCLUDED
#define AAL_H_INCLUDED

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#define __z88dk_fastcall
#define __naked

#define __aal
#define __aal_macro

#define AAL_DEF_VARS volatile int \
    A, B, C, D, E, H, L, IXH, IXL, IYH, IYL, AF, BC, DE, HL, PC, SP, I, R, IX, IY,\
    A_, B_, C_, D_, E_, H_, L_, AF_, BC_, DE_, HL_, \
    z, eq, nz, ne, c, lt, nc, ge, p, m, v, nv, pe, po,\
    z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr, \
    z_else_jr, eq_else_jr, nz_else_jr, ne_else_jr, c_else_jr, lt_else_jr, nc_else_jr, ge_else_jr, p_else_jr, m_else_jr, v_else_jr, nv_else_jr, pe_else_jr, po_else_jr,\
    z_jr_else_jr, eq_jr_else_jr, nz_jr_else_jr, ne_jr_else_jr, c_jr_else_jr, lt_jr_else_jr, nc_jr_else_jr, ge_jr_else_jr, \
    true_jr,\
    *mem, *port, asmpc
#define AAL_DEF_DUMMY_VARS(...) volatile int __VA_ARGS__; // 任意の使用しない変数を定義
#define AAL_LOCAL(...)
#define AAL_IF(expr)
#define AAL_ELIF(expr)
#define AAL_ELSE
#define AAL_ENDIF
#define AAL_REPT(expr)
#define AAL_REPTC(var, expr)
#define AAL_REPTI(var, expr, ...)
#define AAL_ENDR
#define AAL_DB(...)
#define AAL_DW(...)
#define AAL_ENDM
#define AAL_NO_RETURN
#define AAL_FALL_THROUGH
#define AAL_GLOBAL(...)

// 以下は今のところ没のシフト/ローテート演算子
// Python では >>>= が「符号なし右シフト」だが, aal では >>= が「符号なし右シフト」
//#define s>>= >>=        // 符号付き右シフト(sra)
//#define r>>= >>=        // 右ローテート(rrc)
//#define r<<= >>=        // 左ローテート(rlc)
//#define rc>>= >>=       // キャリー付右ローテート(rr)
//#define rc<<= >>=       // キャリー付左ローテート(rl)

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

#endif // AAL_H_INCLUDED
