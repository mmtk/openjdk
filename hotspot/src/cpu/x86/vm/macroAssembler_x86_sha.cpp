/*
* Copyright (c) 2016, Intel Corporation.
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* This code is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License version 2 only, as
* published by the Free Software Foundation.
*
* This code is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* version 2 for more details (a copy is included in the LICENSE file that
* accompanied this code).
*
* You should have received a copy of the GNU General Public License version
* 2 along with this work; if not, write to the Free Software Foundation,
* Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*
* Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
* or visit www.oracle.com if you need additional information or have any
* questions.
*
*/

#include "precompiled.hpp"
#include "asm/assembler.hpp"
#include "asm/assembler.inline.hpp"
#include "runtime/stubRoutines.hpp"
#include "macroAssembler_x86.hpp"

// ofs and limit are used for multi-block byte array.
// int com.sun.security.provider.DigestBase.implCompressMultiBlock(byte[] b, int ofs, int limit)
void MacroAssembler::fast_sha1(XMMRegister abcd, XMMRegister e0, XMMRegister e1, XMMRegister msg0,
  XMMRegister msg1, XMMRegister msg2, XMMRegister msg3, XMMRegister shuf_mask,
  Register buf, Register state, Register ofs, Register limit, Register rsp, bool multi_block) {

  Label start, done_hash, loop0;

  address upper_word_mask = StubRoutines::x86::upper_word_mask_addr();
  address shuffle_byte_flip_mask = StubRoutines::x86::shuffle_byte_flip_mask_addr();

  bind(start);
  movdqu(abcd, Address(state, 0));
  pinsrd(e0, Address(state, 16), 3);
  movdqu(shuf_mask, ExternalAddress(upper_word_mask)); // 0xFFFFFFFF000000000000000000000000
  pand(e0, shuf_mask);
  pshufd(abcd, abcd, 0x1B);
  movdqu(shuf_mask, ExternalAddress(shuffle_byte_flip_mask)); //0x000102030405060708090a0b0c0d0e0f

  bind(loop0);
  // Save hash values for addition after rounds
  movdqu(Address(rsp, 0), e0);
  movdqu(Address(rsp, 16), abcd);


  // Rounds 0 - 3
  movdqu(msg0, Address(buf, 0));
  pshufb(msg0, shuf_mask);
  paddd(e0, msg0);
  movdqa(e1, abcd);
  sha1rnds4(abcd, e0, 0);

  // Rounds 4 - 7
  movdqu(msg1, Address(buf, 16));
  pshufb(msg1, shuf_mask);
  sha1nexte(e1, msg1);
  movdqa(e0, abcd);
  sha1rnds4(abcd, e1, 0);
  sha1msg1(msg0, msg1);

  // Rounds 8 - 11
  movdqu(msg2, Address(buf, 32));
  pshufb(msg2, shuf_mask);
  sha1nexte(e0, msg2);
  movdqa(e1, abcd);
  sha1rnds4(abcd, e0, 0);
  sha1msg1(msg1, msg2);
  pxor(msg0, msg2);

  // Rounds 12 - 15
  movdqu(msg3, Address(buf, 48));
  pshufb(msg3, shuf_mask);
  sha1nexte(e1, msg3);
  movdqa(e0, abcd);
  sha1msg2(msg0, msg3);
  sha1rnds4(abcd, e1, 0);
  sha1msg1(msg2, msg3);
  pxor(msg1, msg3);

  // Rounds 16 - 19
  sha1nexte(e0, msg0);
  movdqa(e1, abcd);
  sha1msg2(msg1, msg0);
  sha1rnds4(abcd, e0, 0);
  sha1msg1(msg3, msg0);
  pxor(msg2, msg0);

  // Rounds 20 - 23
  sha1nexte(e1, msg1);
  movdqa(e0, abcd);
  sha1msg2(msg2, msg1);
  sha1rnds4(abcd, e1, 1);
  sha1msg1(msg0, msg1);
  pxor(msg3, msg1);

  // Rounds 24 - 27
  sha1nexte(e0, msg2);
  movdqa(e1, abcd);
  sha1msg2(msg3, msg2);
  sha1rnds4(abcd, e0, 1);
  sha1msg1(msg1, msg2);
  pxor(msg0, msg2);

  // Rounds 28 - 31
  sha1nexte(e1, msg3);
  movdqa(e0, abcd);
  sha1msg2(msg0, msg3);
  sha1rnds4(abcd, e1, 1);
  sha1msg1(msg2, msg3);
  pxor(msg1, msg3);

  // Rounds 32 - 35
  sha1nexte(e0, msg0);
  movdqa(e1, abcd);
  sha1msg2(msg1, msg0);
  sha1rnds4(abcd, e0, 1);
  sha1msg1(msg3, msg0);
  pxor(msg2, msg0);

  // Rounds 36 - 39
  sha1nexte(e1, msg1);
  movdqa(e0, abcd);
  sha1msg2(msg2, msg1);
  sha1rnds4(abcd, e1, 1);
  sha1msg1(msg0, msg1);
  pxor(msg3, msg1);

  // Rounds 40 - 43
  sha1nexte(e0, msg2);
  movdqa(e1, abcd);
  sha1msg2(msg3, msg2);
  sha1rnds4(abcd, e0, 2);
  sha1msg1(msg1, msg2);
  pxor(msg0, msg2);

  // Rounds 44 - 47
  sha1nexte(e1, msg3);
  movdqa(e0, abcd);
  sha1msg2(msg0, msg3);
  sha1rnds4(abcd, e1, 2);
  sha1msg1(msg2, msg3);
  pxor(msg1, msg3);

  // Rounds 48 - 51
  sha1nexte(e0, msg0);
  movdqa(e1, abcd);
  sha1msg2(msg1, msg0);
  sha1rnds4(abcd, e0, 2);
  sha1msg1(msg3, msg0);
  pxor(msg2, msg0);

  // Rounds 52 - 55
  sha1nexte(e1, msg1);
  movdqa(e0, abcd);
  sha1msg2(msg2, msg1);
  sha1rnds4(abcd, e1, 2);
  sha1msg1(msg0, msg1);
  pxor(msg3, msg1);

  // Rounds 56 - 59
  sha1nexte(e0, msg2);
  movdqa(e1, abcd);
  sha1msg2(msg3, msg2);
  sha1rnds4(abcd, e0, 2);
  sha1msg1(msg1, msg2);
  pxor(msg0, msg2);

  // Rounds 60 - 63
  sha1nexte(e1, msg3);
  movdqa(e0, abcd);
  sha1msg2(msg0, msg3);
  sha1rnds4(abcd, e1, 3);
  sha1msg1(msg2, msg3);
  pxor(msg1, msg3);

  // Rounds 64 - 67
  sha1nexte(e0, msg0);
  movdqa(e1, abcd);
  sha1msg2(msg1, msg0);
  sha1rnds4(abcd, e0, 3);
  sha1msg1(msg3, msg0);
  pxor(msg2, msg0);

  // Rounds 68 - 71
  sha1nexte(e1, msg1);
  movdqa(e0, abcd);
  sha1msg2(msg2, msg1);
  sha1rnds4(abcd, e1, 3);
  pxor(msg3, msg1);

  // Rounds 72 - 75
  sha1nexte(e0, msg2);
  movdqa(e1, abcd);
  sha1msg2(msg3, msg2);
  sha1rnds4(abcd, e0, 3);

  // Rounds 76 - 79
  sha1nexte(e1, msg3);
  movdqa(e0, abcd);
  sha1rnds4(abcd, e1, 3);

  // add current hash values with previously saved
  movdqu(msg0, Address(rsp, 0));
  sha1nexte(e0, msg0);
  movdqu(msg0, Address(rsp, 16));
  paddd(abcd, msg0);

  if (multi_block) {
    // increment data pointer and loop if more to process
    addptr(buf, 64);
    addptr(ofs, 64);
    cmpptr(ofs, limit);
    jcc(Assembler::belowEqual, loop0);
    movptr(rax, ofs); //return ofs
  }
  // write hash values back in the correct order
  pshufd(abcd, abcd, 0x1b);
  movdqu(Address(state, 0), abcd);
  pextrd(Address(state, 16), e0, 3);

  bind(done_hash);

}

// xmm0 (msg) is used as an implicit argument to sh256rnds2
// and state0 and state1 can never use xmm0 register.
// ofs and limit are used for multi-block byte array.
// int com.sun.security.provider.DigestBase.implCompressMultiBlock(byte[] b, int ofs, int limit)
#ifdef _LP64
void MacroAssembler::fast_sha256(XMMRegister msg, XMMRegister state0, XMMRegister state1, XMMRegister msgtmp0,
  XMMRegister msgtmp1, XMMRegister msgtmp2, XMMRegister msgtmp3, XMMRegister msgtmp4,
  Register buf, Register state, Register ofs, Register limit, Register rsp,
  bool multi_block, XMMRegister shuf_mask) {
#else
void MacroAssembler::fast_sha256(XMMRegister msg, XMMRegister state0, XMMRegister state1, XMMRegister msgtmp0,
  XMMRegister msgtmp1, XMMRegister msgtmp2, XMMRegister msgtmp3, XMMRegister msgtmp4,
  Register buf, Register state, Register ofs, Register limit, Register rsp,
  bool multi_block) {
#endif
  Label start, done_hash, loop0;

  address K256 = StubRoutines::x86::k256_addr();
  address pshuffle_byte_flip_mask = StubRoutines::x86::pshuffle_byte_flip_mask_addr();

  bind(start);
  movdqu(state0, Address(state, 0));
  movdqu(state1, Address(state, 16));

  pshufd(state0, state0, 0xB1);
  pshufd(state1, state1, 0x1B);
  movdqa(msgtmp4, state0);
  palignr(state0, state1, 8);
  pblendw(state1, msgtmp4, 0xF0);

#ifdef _LP64
  movdqu(shuf_mask, ExternalAddress(pshuffle_byte_flip_mask));
#endif
  lea(rax, ExternalAddress(K256));

  bind(loop0);
  movdqu(Address(rsp, 0), state0);
  movdqu(Address(rsp, 16), state1);

  // Rounds 0-3
  movdqu(msg, Address(buf, 0));
#ifdef _LP64
  pshufb(msg, shuf_mask);
#else
  pshufb(msg, ExternalAddress(pshuffle_byte_flip_mask));
#endif
  movdqa(msgtmp0, msg);
  paddd(msg, Address(rax, 0));
  sha256rnds2(state1, state0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);

  // Rounds 4-7
  movdqu(msg, Address(buf, 16));
#ifdef _LP64
  pshufb(msg, shuf_mask);
#else
  pshufb(msg, ExternalAddress(pshuffle_byte_flip_mask));
#endif
  movdqa(msgtmp1, msg);
  paddd(msg, Address(rax, 16));
  sha256rnds2(state1, state0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp0, msgtmp1);

  // Rounds 8-11
  movdqu(msg, Address(buf, 32));
#ifdef _LP64
  pshufb(msg, shuf_mask);
#else
  pshufb(msg, ExternalAddress(pshuffle_byte_flip_mask));
#endif
  movdqa(msgtmp2, msg);
  paddd(msg, Address(rax, 32));
  sha256rnds2(state1, state0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp1, msgtmp2);

  // Rounds 12-15
  movdqu(msg, Address(buf, 48));
#ifdef _LP64
  pshufb(msg, shuf_mask);
#else
  pshufb(msg, ExternalAddress(pshuffle_byte_flip_mask));
#endif
  movdqa(msgtmp3, msg);
  paddd(msg, Address(rax, 48));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp3);
  palignr(msgtmp4, msgtmp2, 4);
  paddd(msgtmp0, msgtmp4);
  sha256msg2(msgtmp0, msgtmp3);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp2, msgtmp3);

  // Rounds 16-19
  movdqa(msg, msgtmp0);
  paddd(msg, Address(rax, 64));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp0);
  palignr(msgtmp4, msgtmp3, 4);
  paddd(msgtmp1, msgtmp4);
  sha256msg2(msgtmp1, msgtmp0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp3, msgtmp0);

  // Rounds 20-23
  movdqa(msg, msgtmp1);
  paddd(msg, Address(rax, 80));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp1);
  palignr(msgtmp4, msgtmp0, 4);
  paddd(msgtmp2, msgtmp4);
  sha256msg2(msgtmp2, msgtmp1);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp0, msgtmp1);

  // Rounds 24-27
  movdqa(msg, msgtmp2);
  paddd(msg, Address(rax, 96));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp2);
  palignr(msgtmp4, msgtmp1, 4);
  paddd(msgtmp3, msgtmp4);
  sha256msg2(msgtmp3, msgtmp2);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp1, msgtmp2);

  // Rounds 28-31
  movdqa(msg, msgtmp3);
  paddd(msg, Address(rax, 112));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp3);
  palignr(msgtmp4, msgtmp2, 4);
  paddd(msgtmp0, msgtmp4);
  sha256msg2(msgtmp0, msgtmp3);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp2, msgtmp3);

  // Rounds 32-35
  movdqa(msg, msgtmp0);
  paddd(msg, Address(rax, 128));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp0);
  palignr(msgtmp4, msgtmp3, 4);
  paddd(msgtmp1, msgtmp4);
  sha256msg2(msgtmp1, msgtmp0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp3, msgtmp0);

  // Rounds 36-39
  movdqa(msg, msgtmp1);
  paddd(msg, Address(rax, 144));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp1);
  palignr(msgtmp4, msgtmp0, 4);
  paddd(msgtmp2, msgtmp4);
  sha256msg2(msgtmp2, msgtmp1);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp0, msgtmp1);

  // Rounds 40-43
  movdqa(msg, msgtmp2);
  paddd(msg, Address(rax, 160));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp2);
  palignr(msgtmp4, msgtmp1, 4);
  paddd(msgtmp3, msgtmp4);
  sha256msg2(msgtmp3, msgtmp2);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp1, msgtmp2);

  // Rounds 44-47
  movdqa(msg, msgtmp3);
  paddd(msg, Address(rax, 176));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp3);
  palignr(msgtmp4, msgtmp2, 4);
  paddd(msgtmp0, msgtmp4);
  sha256msg2(msgtmp0, msgtmp3);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp2, msgtmp3);

  // Rounds 48-51
  movdqa(msg, msgtmp0);
  paddd(msg, Address(rax, 192));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp0);
  palignr(msgtmp4, msgtmp3, 4);
  paddd(msgtmp1, msgtmp4);
  sha256msg2(msgtmp1, msgtmp0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  sha256msg1(msgtmp3, msgtmp0);

  // Rounds 52-55
  movdqa(msg, msgtmp1);
  paddd(msg, Address(rax, 208));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp1);
  palignr(msgtmp4, msgtmp0, 4);
  paddd(msgtmp2, msgtmp4);
  sha256msg2(msgtmp2, msgtmp1);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);

  // Rounds 56-59
  movdqa(msg, msgtmp2);
  paddd(msg, Address(rax, 224));
  sha256rnds2(state1, state0);
  movdqa(msgtmp4, msgtmp2);
  palignr(msgtmp4, msgtmp1, 4);
  paddd(msgtmp3, msgtmp4);
  sha256msg2(msgtmp3, msgtmp2);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);

  // Rounds 60-63
  movdqa(msg, msgtmp3);
  paddd(msg, Address(rax, 240));
  sha256rnds2(state1, state0);
  pshufd(msg, msg, 0x0E);
  sha256rnds2(state0, state1);
  movdqu(msg, Address(rsp, 0));
  paddd(state0, msg);
  movdqu(msg, Address(rsp, 16));
  paddd(state1, msg);

  if (multi_block) {
    // increment data pointer and loop if more to process
    addptr(buf, 64);
    addptr(ofs, 64);
    cmpptr(ofs, limit);
    jcc(Assembler::belowEqual, loop0);
    movptr(rax, ofs); //return ofs
  }

  pshufd(state0, state0, 0x1B);
  pshufd(state1, state1, 0xB1);
  movdqa(msgtmp4, state0);
  pblendw(state0, state1, 0xF0);
  palignr(state1, msgtmp4, 8);

  movdqu(Address(state, 0), state0);
  movdqu(Address(state, 16), state1);

  bind(done_hash);

}

#ifdef _LP64
/*
  The algorithm below is based on Intel publication:
  "Fast SHA-256 Implementations on Intelë Architecture Processors" by Jim Guilford, Kirk Yap and Vinodh Gopal.
  The assembly code was originally provided by Sean Gulley and in many places preserves
  the original assembly NAMES and comments to simplify matching Java assembly with its original.
  The Java version was substantially redesigned to replace 1200 assembly instruction with
  much shorter run-time generator of the same code in memory.
*/

void MacroAssembler::sha256_AVX2_one_round_compute(
    Register  reg_old_h,
    Register  reg_a,
    Register  reg_b,
    Register  reg_c,
    Register  reg_d,
    Register  reg_e,
    Register  reg_f,
    Register  reg_g,
    Register  reg_h,
    int iter) {
  const Register& reg_y0     = r13;
  const Register& reg_y1     = r14;
  const Register& reg_y2     = r15;
  const Register& reg_y3     = rcx;
  const Register& reg_T1     = r12;
  //;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; RND iter ;;;;;;;;;;;;;;;;;;;;;;;;;;;
  if (iter%4 > 0) {
    addl(reg_old_h, reg_y2);   // reg_h = k + w + reg_h + S0 + S1 + CH = t1 + S0; --
  }
  movl(reg_y2, reg_f);         // reg_y2 = reg_f                                ; CH
  rorxd(reg_y0, reg_e, 25);    // reg_y0 = reg_e >> 25   ; S1A
  rorxd(reg_y1, reg_e, 11);    // reg_y1 = reg_e >> 11    ; S1B
  xorl(reg_y2, reg_g);         // reg_y2 = reg_f^reg_g                              ; CH

  xorl(reg_y0, reg_y1);        // reg_y0 = (reg_e>>25) ^ (reg_h>>11)  ; S1
  rorxd(reg_y1, reg_e, 6);     // reg_y1 = (reg_e >> 6)    ; S1
  andl(reg_y2, reg_e);         // reg_y2 = (reg_f^reg_g)&reg_e                          ; CH

  if (iter%4 > 0) {
    addl(reg_old_h, reg_y3);   // reg_h = t1 + S0 + MAJ                     ; --
  }

  xorl(reg_y0, reg_y1);       // reg_y0 = (reg_e>>25) ^ (reg_e>>11) ^ (reg_e>>6) ; S1
  rorxd(reg_T1, reg_a, 13);   // reg_T1 = reg_a >> 13    ; S0B
  xorl(reg_y2, reg_g);        // reg_y2 = CH = ((reg_f^reg_g)&reg_e)^reg_g                 ; CH
  rorxd(reg_y1, reg_a, 22);   // reg_y1 = reg_a >> 22    ; S0A
  movl(reg_y3, reg_a);        // reg_y3 = reg_a                                ; MAJA

  xorl(reg_y1, reg_T1);       // reg_y1 = (reg_a>>22) ^ (reg_a>>13)  ; S0
  rorxd(reg_T1, reg_a, 2);    // reg_T1 = (reg_a >> 2)    ; S0
  addl(reg_h, Address(rsp, rdx, Address::times_1, 4*iter)); // reg_h = k + w + reg_h ; --
  orl(reg_y3, reg_c);         // reg_y3 = reg_a|reg_c                              ; MAJA

  xorl(reg_y1, reg_T1);       // reg_y1 = (reg_a>>22) ^ (reg_a>>13) ^ (reg_a>>2) ; S0
  movl(reg_T1, reg_a);        // reg_T1 = reg_a                                ; MAJB
  andl(reg_y3, reg_b);        // reg_y3 = (reg_a|reg_c)&reg_b                          ; MAJA
  andl(reg_T1, reg_c);        // reg_T1 = reg_a&reg_c                              ; MAJB
  addl(reg_y2, reg_y0);       // reg_y2 = S1 + CH                          ; --


  addl(reg_d, reg_h);         // reg_d = k + w + reg_h + reg_d                     ; --
  orl(reg_y3, reg_T1);        // reg_y3 = MAJ = (reg_a|reg_c)&reg_b)|(reg_a&reg_c)             ; MAJ
  addl(reg_h, reg_y1);        // reg_h = k + w + reg_h + S0                    ; --

  addl(reg_d, reg_y2);        // reg_d = k + w + reg_h + reg_d + S1 + CH = reg_d + t1  ; --


  if (iter%4 == 3) {
    addl(reg_h, reg_y2);      // reg_h = k + w + reg_h + S0 + S1 + CH = t1 + S0; --
    addl(reg_h, reg_y3);      // reg_h = t1 + S0 + MAJ                     ; --
  }
}

void MacroAssembler::sha256_AVX2_four_rounds_compute_first(int start) {
    sha256_AVX2_one_round_compute(rax, rax, rbx, rdi, rsi,  r8,  r9, r10, r11, start + 0);
    sha256_AVX2_one_round_compute(r11, r11, rax, rbx, rdi, rsi,  r8,  r9, r10, start + 1);
    sha256_AVX2_one_round_compute(r10, r10, r11, rax, rbx, rdi, rsi,  r8,  r9, start + 2);
    sha256_AVX2_one_round_compute(r9,  r9,  r10, r11, rax, rbx, rdi, rsi,  r8, start + 3);
}

void MacroAssembler::sha256_AVX2_four_rounds_compute_last(int start) {
    sha256_AVX2_one_round_compute(r8,  r8,   r9, r10, r11, rax, rbx, rdi, rsi, start + 0);
    sha256_AVX2_one_round_compute(rsi, rsi,  r8,  r9, r10, r11, rax, rbx, rdi, start + 1);
    sha256_AVX2_one_round_compute(rdi, rdi, rsi,  r8,  r9, r10, r11, rax, rbx, start + 2);
    sha256_AVX2_one_round_compute(rbx, rbx, rdi, rsi,  r8,  r9, r10, r11, rax, start + 3);
}

void MacroAssembler::sha256_AVX2_one_round_and_sched(
        XMMRegister  xmm_0,     /* == ymm4 on 0, 1, 2, 3 iterations, then rotate 4 registers left on 4, 8, 12 iterations */
        XMMRegister  xmm_1,     /* ymm5 */  /* full cycle is 16 iterations */
        XMMRegister  xmm_2,     /* ymm6 */
        XMMRegister  xmm_3,     /* ymm7 */
        Register  reg_a,        /* == rax on 0 iteration, then rotate 8 register right on each next iteration */
        Register  reg_b,        /* rbx */    /* full cycle is 8 iterations */
        Register  reg_c,        /* rdi */
        Register  reg_d,        /* rsi */
        Register  reg_e,        /* r8 */
        Register  reg_f,        /* r9d */
        Register  reg_g,        /* r10d */
        Register  reg_h,        /* r11d */
        int iter)
{
  movl(rcx, reg_a);           // rcx = reg_a               ; MAJA
  rorxd(r13, reg_e, 25);      // r13 = reg_e >> 25    ; S1A
  rorxd(r14, reg_e, 11);      //  r14 = reg_e >> 11    ; S1B
  addl(reg_h, Address(rsp, rdx, Address::times_1, 4*iter));
  orl(rcx, reg_c);            // rcx = reg_a|reg_c          ; MAJA

  movl(r15, reg_f);           // r15 = reg_f               ; CH
  rorxd(r12, reg_a, 13);      // r12 = reg_a >> 13      ; S0B
  xorl(r13, r14);             // r13 = (reg_e>>25) ^ (reg_e>>11)  ; S1
  xorl(r15, reg_g);           // r15 = reg_f^reg_g         ; CH

  rorxd(r14, reg_e, 6);       // r14 = (reg_e >> 6)    ; S1
  andl(r15, reg_e);           // r15 = (reg_f^reg_g)&reg_e ; CH

  xorl(r13, r14);             // r13 = (reg_e>>25) ^ (reg_e>>11) ^ (reg_e>>6) ; S1
  rorxd(r14, reg_a, 22);      // r14 = reg_a >> 22    ; S0A
  addl(reg_d, reg_h);         // reg_d = k + w + reg_h + reg_d                     ; --

  andl(rcx, reg_b);          // rcx = (reg_a|reg_c)&reg_b                          ; MAJA
  xorl(r14, r12);            // r14 = (reg_a>>22) ^ (reg_a>>13)  ; S0

  rorxd(r12, reg_a, 2);      // r12 = (reg_a >> 2)    ; S0
  xorl(r15, reg_g);          // r15 = CH = ((reg_f^reg_g)&reg_e)^reg_g                 ; CH

  xorl(r14, r12);            // r14 = (reg_a>>22) ^ (reg_a>>13) ^ (reg_a>>2) ; S0
  movl(r12, reg_a);          // r12 = reg_a                                ; MAJB
  andl(r12, reg_c);          // r12 = reg_a&reg_c                              ; MAJB
  addl(r15, r13);            // r15 = S1 + CH                          ; --

  orl(rcx, r12);             // rcx = MAJ = (reg_a|reg_c)&reg_b)|(reg_a&reg_c)             ; MAJ
  addl(reg_h, r14);          // reg_h = k + w + reg_h + S0                    ; --
  addl(reg_d, r15);          // reg_d = k + w + reg_h + reg_d + S1 + CH = reg_d + t1  ; --

  addl(reg_h, r15);          // reg_h = k + w + reg_h + S0 + S1 + CH = t1 + S0; --
  addl(reg_h, rcx);          // reg_h = t1 + S0 + MAJ                     ; --

  if (iter%4 == 0) {
    vpalignr(xmm0, xmm_3, xmm_2, 4, AVX_256bit);   // ymm0 = W[-7]
    vpaddd(xmm0, xmm0, xmm_0, AVX_256bit);         // ymm0 = W[-7] + W[-16]; y1 = (e >> 6)     ; S1
    vpalignr(xmm1, xmm_1, xmm_0, 4, AVX_256bit);   // ymm1 = W[-15]
    vpsrld(xmm2, xmm1, 7, AVX_256bit);
    vpslld(xmm3, xmm1, 32-7, AVX_256bit);
    vpor(xmm3, xmm3, xmm2, AVX_256bit);            // ymm3 = W[-15] ror 7
    vpsrld(xmm2, xmm1,18, AVX_256bit);
  } else if (iter%4 == 1 ) {
    vpsrld(xmm8, xmm1, 3, AVX_256bit);             // ymm8 = W[-15] >> 3
    vpslld(xmm1, xmm1, 32-18, AVX_256bit);
    vpxor(xmm3, xmm3, xmm1, AVX_256bit);
    vpxor(xmm3, xmm3, xmm2, AVX_256bit);           // ymm3 = W[-15] ror 7 ^ W[-15] ror 18
    vpxor(xmm1, xmm3, xmm8, AVX_256bit);           // ymm1 = s0
    vpshufd(xmm2, xmm_3, 0xFA, AVX_256bit);        // 11111010b ; ymm2 = W[-2] {BBAA}
    vpaddd(xmm0, xmm0, xmm1, AVX_256bit);          // ymm0 = W[-16] + W[-7] + s0
    vpsrld(xmm8, xmm2, 10, AVX_256bit);            // ymm8 = W[-2] >> 10 {BBAA}
  } else if (iter%4 == 2) {
    vpsrlq(xmm3, xmm2, 19, AVX_256bit);            // ymm3 = W[-2] ror 19 {xBxA}
    vpsrlq(xmm2, xmm2, 17, AVX_256bit);            // ymm2 = W[-2] ror 17 {xBxA}
    vpxor(xmm2, xmm2, xmm3, AVX_256bit);
    vpxor(xmm8, xmm8, xmm2, AVX_256bit);           // ymm8 = s1 {xBxA}
    vpshufb(xmm8, xmm8, xmm10, AVX_256bit);        // ymm8 = s1 {00BA}
    vpaddd(xmm0, xmm0, xmm8, AVX_256bit);          // ymm0 = {..., ..., W[1], W[0]}
    vpshufd(xmm2, xmm0, 0x50, AVX_256bit);         // 01010000b ; ymm2 = W[-2] {DDCC}
  } else if (iter%4 == 3) {
    vpsrld(xmm11, xmm2, 10, AVX_256bit);           // ymm11 = W[-2] >> 10 {DDCC}
    vpsrlq(xmm3, xmm2, 19, AVX_256bit);            // ymm3 = W[-2] ror 19 {xDxC}
    vpsrlq(xmm2, xmm2, 17, AVX_256bit);            // ymm2 = W[-2] ror 17 {xDxC}
    vpxor(xmm2, xmm2, xmm3, AVX_256bit);
    vpxor(xmm11, xmm11, xmm2, AVX_256bit);         // ymm11 = s1 {xDxC}
    vpshufb(xmm11, xmm11, xmm12, AVX_256bit);      // ymm11 = s1 {DC00}
    vpaddd(xmm_0, xmm11, xmm0, AVX_256bit);        // xmm_0 = {W[3], W[2], W[1], W[0]}
  }
}

void MacroAssembler::addm(int disp, Register r1, Register r2) {
  addl(r2, Address(r1, disp));
  movl(Address(r1, disp), r2);
}

void MacroAssembler::sha256_AVX2(XMMRegister msg, XMMRegister state0, XMMRegister state1, XMMRegister msgtmp0,
  XMMRegister msgtmp1, XMMRegister msgtmp2, XMMRegister msgtmp3, XMMRegister msgtmp4,
  Register buf, Register state, Register ofs, Register limit, Register rsp,
  bool multi_block, XMMRegister shuf_mask) {

  Label loop0, loop1, loop2, loop3,
        last_block_enter, do_last_block, only_one_block, done_hash,
        compute_size, compute_size_end,
        compute_size1, compute_size_end1;

  address K256_W = StubRoutines::x86::k256_W_addr();
  address pshuffle_byte_flip_mask = StubRoutines::x86::pshuffle_byte_flip_mask_addr();
  address pshuffle_byte_flip_mask_addr = 0;

const XMMRegister& SHUF_00BA        = xmm10;    // ymm10: shuffle xBxA -> 00BA
const XMMRegister& SHUF_DC00        = xmm12;    // ymm12: shuffle xDxC -> DC00
const XMMRegister& BYTE_FLIP_MASK   = xmm13;   // ymm13

const XMMRegister& X_BYTE_FLIP_MASK = xmm13;   //XMM version of BYTE_FLIP_MASK

const Register& NUM_BLKS = r8;   // 3rd arg
const Register& CTX      = rdx;  // 2nd arg
const Register& INP      = rcx;  // 1st arg

const Register& c        = rdi;
const Register& d        = rsi;
const Register& e        = r8;    // clobbers NUM_BLKS
const Register& y3       = rcx;  // clobbers INP

const Register& TBL      = rbp;
const Register& SRND     = CTX;   // SRND is same register as CTX

const Register& a        = rax;
const Register& b        = rbx;
const Register& f        = r9;
const Register& g        = r10;
const Register& h        = r11;

const Register& T1       = r12;
const Register& y0       = r13;
const Register& y1       = r14;
const Register& y2       = r15;


enum {
  _XFER_SIZE = 2*64*4, // 2 blocks, 64 rounds, 4 bytes/round
#ifndef _WIN64
  _XMM_SAVE_SIZE = 0,
#else
  _XMM_SAVE_SIZE = 8*16,
#endif
  _INP_END_SIZE = 8,
  _INP_SIZE = 8,
  _CTX_SIZE = 8,
  _RSP_SIZE = 8,

  _XFER = 0,
  _XMM_SAVE  = _XFER     + _XFER_SIZE,
  _INP_END   = _XMM_SAVE + _XMM_SAVE_SIZE,
  _INP       = _INP_END  + _INP_END_SIZE,
  _CTX       = _INP      + _INP_SIZE,
  _RSP       = _CTX      + _CTX_SIZE,
  STACK_SIZE = _RSP      + _RSP_SIZE
};

#ifndef _WIN64
  push(rcx);    // linux: this is limit, need at the end
  push(rdx);    // linux: this is ofs
#else
  push(r8);     // win64: this is ofs
  push(r9);     // win64: this is limit, we need them again at the very and
#endif


  push(rbx);
#ifdef _WIN64
  push(rsi);
  push(rdi);
#endif
  push(rbp);
  push(r12);
  push(r13);
  push(r14);
  push(r15);

  movq(rax, rsp);
  subq(rsp, STACK_SIZE);
  andq(rsp, -32);
  movq(Address(rsp, _RSP), rax);

#ifndef _WIN64
  // copy linux params to win64 params, therefore the rest of code will be the same for both
  movq(r9,  rcx);
  movq(r8,  rdx);
  movq(rdx, rsi);
  movq(rcx, rdi);
#endif

  // setting original assembly ABI
  /** message to encrypt in INP */
  lea(INP, Address(rcx, 0));    // rcx == message (buf)     ;; linux: INP = buf = rdi
  /** digest in CTX             */
  movq(CTX, rdx);               // rdx = digest  (state)    ;; linux: CTX = state = rsi

  /** NUM_BLK is the length of message, need to set it from ofs and limit  */
  if (multi_block) {

    // Win64: cannot directly update NUM_BLKS, since NUM_BLKS = ofs = r8
    // on entry r8 = ofs
    // on exit  r8 = NUM_BLKS

    xorq(rax, rax);

    bind(compute_size);
    cmpptr(r8, r9); // assume the original ofs <= limit ;; linux:  cmp rcx, rdx
    jccb(Assembler::aboveEqual, compute_size_end);
    addq(r8, 64);                                          //;; linux: ofs = rdx
    addq(rax, 64);
    jmpb(compute_size);

    bind(compute_size_end);
    movq(NUM_BLKS, rax);  // NUM_BLK (r8)                  ;; linux: NUM_BLK = rdx

    cmpq(NUM_BLKS, 0);
    jcc(Assembler::equal, done_hash);

    } else {
    xorq(NUM_BLKS, NUM_BLKS);
    addq(NUM_BLKS, 64);
  }//if (!multi_block)

  lea(NUM_BLKS, Address(INP, NUM_BLKS, Address::times_1, -64)); // pointer to the last block
  movq(Address(rsp, _INP_END), NUM_BLKS);  //

  cmpptr(INP, NUM_BLKS);                   //cmp INP, NUM_BLKS
  jcc(Assembler::equal, only_one_block);   //je only_one_block

  // load initial digest
  movl(a, Address(CTX, 4*0));
  movl(b, Address(CTX, 4*1));
  movl(c, Address(CTX, 4*2));
  movl(d, Address(CTX, 4*3));
  movl(e, Address(CTX, 4*4));
  movl(f, Address(CTX, 4*5));
  movl(g, Address(CTX, 4*6));
  movl(h, Address(CTX, 4*7));

  pshuffle_byte_flip_mask_addr = pshuffle_byte_flip_mask;
  vmovdqu(BYTE_FLIP_MASK, ExternalAddress(pshuffle_byte_flip_mask_addr +0)); //[PSHUFFLE_BYTE_FLIP_MASK wrt rip]
  vmovdqu(SHUF_00BA, ExternalAddress(pshuffle_byte_flip_mask_addr + 32));     //[_SHUF_00BA wrt rip]
  vmovdqu(SHUF_DC00, ExternalAddress(pshuffle_byte_flip_mask_addr + 64));     //[_SHUF_DC00 wrt rip]

  movq(Address(rsp, _CTX), CTX);           // store

bind(loop0);
  lea(TBL, ExternalAddress(K256_W));

  // assume buffers not aligned

  // Load first 16 dwords from two blocks
  vmovdqu(xmm0, Address(INP, 0*32));
  vmovdqu(xmm1, Address(INP, 1*32));
  vmovdqu(xmm2, Address(INP, 2*32));
  vmovdqu(xmm3, Address(INP, 3*32));

  // byte swap data
  vpshufb(xmm0, xmm0, BYTE_FLIP_MASK, AVX_256bit);
  vpshufb(xmm1, xmm1, BYTE_FLIP_MASK, AVX_256bit);
  vpshufb(xmm2, xmm2, BYTE_FLIP_MASK, AVX_256bit);
  vpshufb(xmm3, xmm3, BYTE_FLIP_MASK, AVX_256bit);

  // transpose data into high/low halves
  vperm2i128(xmm4, xmm0, xmm2, 0x20);
  vperm2i128(xmm5, xmm0, xmm2, 0x31);
  vperm2i128(xmm6, xmm1, xmm3, 0x20);
  vperm2i128(xmm7, xmm1, xmm3, 0x31);

bind(last_block_enter);
  addq(INP, 64);
  movq(Address(rsp, _INP), INP);

  //;; schedule 48 input dwords, by doing 3 rounds of 12 each
  xorq(SRND, SRND);

align(16);
bind(loop1);
  vpaddd(xmm9, xmm4, Address(TBL, SRND, Address::times_1, 0*32), AVX_256bit);
  vmovdqu(Address(rsp, SRND, Address::times_1, _XFER + 0*32), xmm9);
  sha256_AVX2_one_round_and_sched(xmm4, xmm5, xmm6, xmm7, rax, rbx, rdi, rsi, r8,  r9,  r10, r11, 0);
  sha256_AVX2_one_round_and_sched(xmm4, xmm5, xmm6, xmm7, r11, rax, rbx, rdi, rsi, r8,  r9,  r10, 1);
  sha256_AVX2_one_round_and_sched(xmm4, xmm5, xmm6, xmm7, r10, r11, rax, rbx, rdi, rsi, r8,  r9,  2);
  sha256_AVX2_one_round_and_sched(xmm4, xmm5, xmm6, xmm7, r9,  r10, r11, rax, rbx, rdi, rsi, r8,  3);

  vpaddd(xmm9, xmm5, Address(TBL, SRND, Address::times_1, 1*32), AVX_256bit);
  vmovdqu(Address(rsp, SRND, Address::times_1, _XFER + 1*32), xmm9);
  sha256_AVX2_one_round_and_sched(xmm5, xmm6, xmm7, xmm4, r8,  r9,  r10, r11, rax, rbx, rdi, rsi,  8+0);
  sha256_AVX2_one_round_and_sched(xmm5, xmm6, xmm7, xmm4, rsi, r8,  r9,  r10, r11, rax, rbx, rdi,  8+1);
  sha256_AVX2_one_round_and_sched(xmm5, xmm6, xmm7, xmm4, rdi, rsi, r8,  r9,  r10, r11, rax, rbx,  8+2);
  sha256_AVX2_one_round_and_sched(xmm5, xmm6, xmm7, xmm4, rbx, rdi, rsi, r8,  r9,  r10, r11, rax,  8+3);

  vpaddd(xmm9, xmm6, Address(TBL, SRND, Address::times_1, 2*32), AVX_256bit);
  vmovdqu(Address(rsp, SRND, Address::times_1, _XFER + 2*32), xmm9);
  sha256_AVX2_one_round_and_sched(xmm6, xmm7, xmm4, xmm5, rax, rbx, rdi, rsi, r8,  r9,  r10, r11, 16+0);
  sha256_AVX2_one_round_and_sched(xmm6, xmm7, xmm4, xmm5, r11, rax, rbx, rdi, rsi, r8,  r9,  r10, 16+1);
  sha256_AVX2_one_round_and_sched(xmm6, xmm7, xmm4, xmm5, r10, r11, rax, rbx, rdi, rsi, r8,  r9,  16+2);
  sha256_AVX2_one_round_and_sched(xmm6, xmm7, xmm4, xmm5, r9,  r10, r11, rax, rbx, rdi, rsi, r8,  16+3);

  vpaddd(xmm9, xmm7, Address(TBL, SRND, Address::times_1, 3*32), AVX_256bit);
  vmovdqu(Address(rsp, SRND, Address::times_1, _XFER + 3*32), xmm9);

  sha256_AVX2_one_round_and_sched(xmm7, xmm4, xmm5, xmm6, r8,  r9,  r10, r11, rax, rbx, rdi, rsi,  24+0);
  sha256_AVX2_one_round_and_sched(xmm7, xmm4, xmm5, xmm6, rsi, r8,  r9,  r10, r11, rax, rbx, rdi,  24+1);
  sha256_AVX2_one_round_and_sched(xmm7, xmm4, xmm5, xmm6, rdi, rsi, r8,  r9,  r10, r11, rax, rbx,  24+2);
  sha256_AVX2_one_round_and_sched(xmm7, xmm4, xmm5, xmm6, rbx, rdi, rsi, r8,  r9,  r10, r11, rax,  24+3);

  addq(SRND, 4*32);
  cmpq(SRND, 3 * 4*32);
  jcc(Assembler::below, loop1);

bind(loop2);
  // Do last 16 rounds with no scheduling
  vpaddd(xmm9, xmm4, Address(TBL, SRND, Address::times_1, 0*32), AVX_256bit);
  vmovdqu(Address(rsp, SRND, Address::times_1, _XFER + 0*32), xmm9);
  sha256_AVX2_four_rounds_compute_first(0);

  vpaddd(xmm9, xmm5, Address(TBL, SRND, Address::times_1, 1*32), AVX_256bit);
  vmovdqu(Address(rsp, SRND, Address::times_1, _XFER + 1*32), xmm9);
  sha256_AVX2_four_rounds_compute_last(0 + 8);

  addq(SRND, 2*32);

  vmovdqu(xmm4, xmm6);
  vmovdqu(xmm5, xmm7);

  cmpq(SRND, 4 * 4*32);
  jcc(Assembler::below, loop2);

  movq(CTX, Address(rsp, _CTX));
  movq(INP, Address(rsp, _INP));

  addm(4*0, CTX, a);
  addm(4*1, CTX, b);
  addm(4*2, CTX, c);
  addm(4*3, CTX, d);
  addm(4*4, CTX, e);
  addm(4*5, CTX, f);
  addm(4*6, CTX, g);
  addm(4*7, CTX, h);

  cmpq(INP, Address(rsp, _INP_END));
  jcc(Assembler::above, done_hash);

  //Do second block using previously scheduled results
  xorq(SRND, SRND);
align(16);
bind(loop3);
  sha256_AVX2_four_rounds_compute_first(4);
  sha256_AVX2_four_rounds_compute_last(4+8);

  addq(SRND, 2*32);
  cmpq(SRND, 4 * 4*32);
  jcc(Assembler::below, loop3);

  movq(CTX, Address(rsp, _CTX));
  movq(INP, Address(rsp, _INP));
  addq(INP, 64);

  addm(4*0, CTX, a);
  addm(4*1, CTX, b);
  addm(4*2, CTX, c);
  addm(4*3, CTX, d);
  addm(4*4, CTX, e);
  addm(4*5, CTX, f);
  addm(4*6, CTX, g);
  addm(4*7, CTX, h);

  cmpq(INP, Address(rsp, _INP_END));
  jcc(Assembler::below, loop0);
  jccb(Assembler::above, done_hash);

bind(do_last_block);
  lea(TBL, ExternalAddress(K256_W));

  movdqu(xmm4, Address(INP, 0*16));
  movdqu(xmm5, Address(INP, 1*16));
  movdqu(xmm6, Address(INP, 2*16));
  movdqu(xmm7, Address(INP, 3*16));

  vpshufb(xmm4, xmm4, xmm13, AVX_128bit);
  vpshufb(xmm5, xmm5, xmm13, AVX_128bit);
  vpshufb(xmm6, xmm6, xmm13, AVX_128bit);
  vpshufb(xmm7, xmm7, xmm13, AVX_128bit);

  jmp(last_block_enter);

bind(only_one_block);

  // load initial digest ;; table should be preloaded with following values
  movl(a, Address(CTX, 4*0));   // 0x6a09e667
  movl(b, Address(CTX, 4*1));   // 0xbb67ae85
  movl(c, Address(CTX, 4*2));   // 0x3c6ef372
  movl(d, Address(CTX, 4*3));   // 0xa54ff53a
  movl(e, Address(CTX, 4*4));   // 0x510e527f
  movl(f, Address(CTX, 4*5));   // 0x9b05688c
  movl(g, Address(CTX, 4*6));   // 0x1f83d9ab
  movl(h, Address(CTX, 4*7));   // 0x5be0cd19


  pshuffle_byte_flip_mask_addr = pshuffle_byte_flip_mask;
  vmovdqu(BYTE_FLIP_MASK, ExternalAddress(pshuffle_byte_flip_mask_addr + 0)); //[PSHUFFLE_BYTE_FLIP_MASK wrt rip]
  vmovdqu(SHUF_00BA, ExternalAddress(pshuffle_byte_flip_mask_addr + 32));     //[_SHUF_00BA wrt rip]
  vmovdqu(SHUF_DC00, ExternalAddress(pshuffle_byte_flip_mask_addr + 64));     //[_SHUF_DC00 wrt rip]

  movq(Address(rsp, _CTX), CTX);
  jmpb(do_last_block);

bind(done_hash);

  movq(rsp, Address(rsp, _RSP));

  pop(r15);
  pop(r14);
  pop(r13);
  pop(r12);
  pop(rbp);
#ifdef _WIN64
  pop(rdi);
  pop(rsi);
#endif
  pop(rbx);

#ifdef _WIN64
  pop(r9);
  pop(r8);
#else
  pop(rdx);
  pop(rcx);
#endif

  if (multi_block) {
#ifdef _WIN64
const Register& limit_end = r9;
const Register& ofs_end   = r8;
#else
const Register& limit_end = rcx;
const Register& ofs_end   = rdx;
#endif
    movq(rax, ofs_end);

bind(compute_size1);
    cmpptr(rax, limit_end); // assume the original ofs <= limit
    jccb(Assembler::aboveEqual, compute_size_end1);
    addq(rax, 64);
    jmpb(compute_size1);

bind(compute_size_end1);
  }
}
#endif //#ifdef _LP64
