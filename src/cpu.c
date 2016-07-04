#include "cpu.h"

#include <stdint.h>
#include <syslog.h>


uint32_t reg[16] = { 0 };
#define A  (reg[0])
#define B  (reg[1])
#define C  (reg[2])
#define D  (reg[3])
#define E  (reg[4])
#define F  (reg[5])
#define G  (reg[6])
#define H  (reg[7])
#define I  (reg[8])
#define J  (reg[9])
#define K  (reg[10])
#define L  (reg[11])
#define FP (reg[12])
#define SP (reg[13])
#define PC (reg[14])
#define FL (reg[15])


void cpu_init()
{
}


// {{{ STEP

void cpu_step()
{
/*
  step() {
    const n = this._stepFunction[this._mb.get(this.PC)](this.PC + 1);
    if (n) {
      this.PC += n + 1;
    }
  }

  
  checkInterrupts() {
    if (this.T && this._interruptsPending.length > 0) {
      let n = this._interruptsPending.shift();
      this._push32(this.PC);
      this.T = false;
      this.PC = this._interruptVector[n];
      this.systemHalted = false;
    }
  }


  _affectFlags(value) {
    this.Z = ((value & 0xFFFFFFFF) === 0);
    this.P = ((value % 2) === 0);
    this.S = ((value >> 31) & 0x1 ? true : false);
    this.V = false;
    this.Y = false;
    this.GT = false;
    this.LT = false;
    return value & 0xFFFFFFFF;
  }


  // 
  // STACK
  //

  _push(value) {
    this._mb.set(this.SP, value);
    this.SP -= 1;
  }


  _push16(value) {
    this.SP -= 1;
    this._mb.set16(this.SP, value);
    this.SP -= 1;
  }


  _push32(value) {
    this.SP -= 3;
    this._mb.set32(this.SP, value);
    this.SP -= 1;
  }
  

  _pop() {
    this.SP += 1;
    return this._mb.get(this.SP);
  }


  _pop16() {
    this.SP += 1;
    const r = this._mb.get16(this.SP);
    this.SP += 1;
    return r;
  }


  _pop32() {
    this.SP += 1;
    const r = this._mb.get32(this.SP);
    this.SP += 3;
    return r;
  }


  //
  // GETTERS / SETTERS
  //

  get A() { return this._reg[0]; }
  get B() { return this._reg[1]; }
  get C() { return this._reg[2]; }
  get D() { return this._reg[3]; }
  get E() { return this._reg[4]; }
  get F() { return this._reg[5]; }
  get G() { return this._reg[6]; }
  get H() { return this._reg[7]; }
  get I() { return this._reg[8]; }
  get J() { return this._reg[9]; }
  get K() { return this._reg[10]; }
  get L() { return this._reg[11]; }
  get FP() { return this._reg[12]; }
  get SP() { return this._reg[13]; }
  get PC() { return this._reg[14]; }
  get FL() { return this._reg[15]; }

  set A(v) { this._reg[0] = v; }
  set B(v) { this._reg[1] = v; }
  set C(v) { this._reg[2] = v; }
  set D(v) { this._reg[3] = v; }
  set E(v) { this._reg[4] = v; }
  set F(v) { this._reg[5] = v; }
  set G(v) { this._reg[6] = v; }
  set H(v) { this._reg[7] = v; }
  set I(v) { this._reg[8] = v; }
  set J(v) { this._reg[9] = v; }
  set K(v) { this._reg[10] = v; }
  set L(v) { this._reg[11] = v; }
  set FP(v) { this._reg[12] = v; }
  set SP(v) { this._reg[13] = v & 0xFFFFFFFF; }
  set PC(v) { this._reg[14] = v; }
  set FL(v) { this._reg[15] = v; }

  get Y() { return (this._reg[15] & 0x1) ? true : false; }
  get V() { return ((this._reg[15] >> 1) & 0x1) ? true : false; }
  get Z() { return ((this._reg[15] >> 2) & 0x1) ? true : false; }
  get S() { return ((this._reg[15] >> 3) & 0x1) ? true : false; }
  get GT() { return ((this._reg[15] >> 4) & 0x1) ? true : false; }
  get LT() { return ((this._reg[15] >> 5) & 0x1) ? true : false; }
  get P() { return ((this._reg[15] >> 6) & 0x1) ? true : false; }
  get T() { return ((this._reg[15] >> 7) & 0x1) ? true : false; }

  // jscs:disable validateIndentation
  set Y(v) { if (v) this._reg[15] |= (1 << 0); else this._reg[15] &= ~(1 << 0); }
  set V(v) { if (v) this._reg[15] |= (1 << 1); else this._reg[15] &= ~(1 << 1); }
  set Z(v) { if (v) this._reg[15] |= (1 << 2); else this._reg[15] &= ~(1 << 2); }
  set S(v) { if (v) this._reg[15] |= (1 << 3); else this._reg[15] &= ~(1 << 3); }
  set GT(v) { if (v) this._reg[15] |= (1 << 4); else this._reg[15] &= ~(1 << 4); }
  set LT(v) { if (v) { this._reg[15] |= (1 << 5); } else { this._reg[15] &= ~(1 << 5); } }
  set P(v) { if (v) { this._reg[15] |= (1 << 6); } else { this._reg[15] &= ~(1 << 6); } }
  set T(v) { if (v) { this._reg[15] |= (1 << 7); } else { this._reg[15] &= ~(1 << 6); } }
  // jscs:enable validateIndentation


  //
  // INSTRUCTIONS
  //

  initStepFunctions() {

    // add invalid opcodes
    let f = [];
    for (let i = 0; i < 256; ++i) {
      f.push((pos) => {
        this.fireInterrupt();
        this.invalidUpcode = true;
        return 0;
      });
    }

    //
    // MOV
    //
    f[0x01] = pos => {  // mov R, R
      let [reg, mb] = [this._reg, this._mb];
      const r = reg[mb.get(pos + 1)];
      reg[mb.get(pos)] = this._affectFlags(r);
      return 2;
    };
    f[0x02] = pos => {  // mov R, v8
      let [reg, mb] = [this._reg, this._mb];
      const r = mb.get(pos + 1);
      reg[mb.get(pos)] = this._affectFlags(r);
      return 2;
    };
    f[0x03] = pos => {  // mov R, v16
      let [reg, mb] = [this._reg, this._mb];
      const r = mb.get16(pos + 1);
      reg[mb.get(pos)] = this._affectFlags(r);
      return 3;
    };
    f[0x04] = pos => {  // mov R, v32
      let [reg, mb] = [this._reg, this._mb];
      const r = mb.get32(pos + 1);
      reg[mb.get(pos)] = this._affectFlags(r);
      return 5;
    };

    //
    // MOVB
    //

    f[0x05] = pos => {  // movb R, [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = mb.get(reg[p2]);
      reg[p1] = this._affectFlags(r);
      return 2;
    };

    f[0x06] = pos => {  // movb R, [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = mb.get(p2);
      reg[p1] = this._affectFlags(r);
      return 5;
    };

    f[0x0B] = pos => {  // movb [R], R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p2] & 0xFF;
      mb.set(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x0C] = pos => {  // movb [R], v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = p2;
      mb.set(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x0D] = pos => {  // movb [R], [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = mb.get(reg[p2]);
      mb.set(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x0E] = pos => {  // movb [R], [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = mb.get(p2);
      mb.set(reg[p1], this._affectFlags(r));
      return 5;
    };

    f[0x21] = pos => {  // movb [v32], R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      const r = reg[p2] & 0xFF;
      mb.set(p1, this._affectFlags(r));
      return 5;
    };

    f[0x22] = pos => {  // movb [v32], v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      const r = p2;
      mb.set(p1, this._affectFlags(r));
      return 5;
    };

    f[0x23] = pos => {  // movb [v32], [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      const r = mb.get(reg[p2]);
      mb.set(p1, this._affectFlags(r));
      return 5;
    };

    f[0x24] = pos => {  // movb [v32], [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get32(pos + 4)];
      const r = mb.get(mb.get32(p2));
      mb.set(p1, this._affectFlags(r));
      return 8;
    };

    //
    // MOVW
    //

    f[0x07] = pos => {  // movw R, [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = mb.get16(reg[p2]);
      reg[p1] = this._affectFlags(r);
      return 2;
    };

    f[0x08] = pos => {  // movw R, [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r =  mb.get16(p2);
      reg[p1] = this._affectFlags(r);
      return 5;
    };

    f[0x0F] = pos => {  // movw [R], R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p2] & 0xFFFF;
      mb.set16(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x1A] = pos => {  // movw [R], v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = p2;
      mb.set16(reg[p1], this._affectFlags(r));
      return 3;
    };

    f[0x1B] = pos => {  // movw [R], [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = mb.get16(reg[p2]);
      mb.set16(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x1C] = pos => {  // movw [R], [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = mb.get16(p2);
      mb.set16(reg[p1], this._affectFlags(r));
      return 5;
    };

    f[0x25] = pos => {  // movw [v32], R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      const r = reg[p2] & 0xFFFF;
      mb.set16(p1, this._affectFlags(r));
      return 5;
    };

    f[0x26] = pos => {  // movw [v32], v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get16(pos + 4)];
      const r = p2;
      mb.set16(p1, this._affectFlags(r));
      return 6;
    };

    f[0x27] = pos => {  // movw [v32], [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      mb.set16(p1, mb.get16(reg[p2]));
      return 5;
    };

    f[0x28] = pos => {  // movw [v32], [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get32(pos + 4)];
      mb.set16(p1, mb.get16(mb.get32(p2)));
      return 8;
    };


    //
    // MOVD
    //

    f[0x09] = pos => {  // movd R, [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r =  mb.get32(reg[p2]);
      reg[p1] = this._affectFlags(r);
      return 2;
    };

    f[0x0A] = pos => {  // movd R, [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r =  mb.get32(p2);
      reg[p1] = this._affectFlags(r);
      return 5;
    };

    f[0x1D] = pos => {  // movd [R], R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p2];
      mb.set32(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x1E] = pos => {  // movd [R], v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = p2;
      mb.set32(reg[p1], this._affectFlags(r));
      return 5;
    };

    f[0x1F] = pos => {  // movd [R], [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = mb.get32(reg[p2]);
      mb.set32(reg[p1], this._affectFlags(r));
      return 2;
    };

    f[0x20] = pos => {  // movd [R], [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = mb.get32(p2);
      mb.set32(reg[p1], this._affectFlags(r));
      return 5;
    };

    f[0x29] = pos => {  // movd [v32], R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      const r = reg[p2];
      mb.set32(p1, this._affectFlags(r));
      return 5;
    };

    f[0x2A] = pos => {  // movd [v32], v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get32(pos + 4)];
      const r = p2;
      mb.set32(p1, this._affectFlags(r));
      return 8;
    };

    f[0x2B] = pos => {  // movd [v32], [R]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get(pos + 4)];
      const r = mb.get32(reg[p2]);
      mb.set32(p1, this._affectFlags(r));
      return 5;
    };

    f[0x2C] = pos => {  // movd [v32], [v32]
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get32(pos), mb.get32(pos + 4)];
      const r = mb.get32(mb.get32(p2));
      mb.set32(p1, this._affectFlags(r));
      return 8;
    };

    //
    // SWAP
    //
    f[0x8A] = pos => {  // swap R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      [reg[p1], reg[p2]] = [reg[p2], reg[p1]]
      this._affectFlags(reg[p1]);
      return 2;
    };


    //
    // OR
    //

    f[0x2D] = pos => {  // or R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] | reg[p2];
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x2E] = pos => {  // or R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] | p2;
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x2F] = pos => {  // or R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] | p2;
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x30] = pos => {  // or R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] | p2;
      reg[p1] = this._affectFlags(r);
      return 5;
    };
    
    //
    // XOR
    //

    f[0x31] = pos => {  // xor R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] ^ reg[p2];
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x32] = pos => {  // xor R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] ^ p2;
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x33] = pos => {  // xor R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] ^ p2;
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x34] = pos => {  // xor R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] ^ p2;
      reg[p1] = this._affectFlags(r);
      return 5;
    };
    
    //
    // AND
    //

    f[0x35] = pos => {  // and R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] & reg[p2];
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x36] = pos => {  // and R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] & p2;
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x37] = pos => {  // and R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] & p2;
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x38] = pos => {  // and R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] & p2;
      reg[p1] = this._affectFlags(r);
      return 5;
    };

    //
    // SHIFT
    //

    f[0x39] = pos => {  // shl R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] << reg[p2];
      reg[p1] = this._affectFlags(r);
      this.Y = ((reg[p1] >> 31) & 1) == 1;
      return 2;
    };

    f[0x3A] = pos => {  // shl R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] << p2;
      reg[p1] = this._affectFlags(r);
      this.Y = ((reg[p1] >> 31) & 1) == 1;
      return 2;
    };

    f[0x3D] = pos => {  // shr R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] >> reg[p2];
      reg[p1] = this._affectFlags(r);
      this.Y = (reg[p1] & 1) == 1;
      return 2;
    };

    f[0x3E] = pos => {  // shr R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] >> p2;
      reg[p1] = this._affectFlags(r);
      this.Y = (reg[p1] & 1) == 1;
      return 2;
    };

    //
    // NOT
    //

    f[0x41] = pos => {  // not
      let [reg, mb] = [this._reg, this._mb];
      const [p1] = [mb.get(pos)];
      const r = ~reg[p1];
      reg[p1] = this._affectFlags(r);
      return 1;
    };

    // 
    // ARITHMETIC
    //

    f[0x42] = pos => {  // add R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] + reg[p2] + (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x43] = pos => {  // add R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] + p2 + (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r > 0xFFFFFFFF);
      return 2;
    };
    
    f[0x44] = pos => {  // add R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] + p2 + (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r > 0xFFFFFFFF);
      return 3;
    };
    
    f[0x45] = pos => {  // add R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] + p2 + (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r > 0xFFFFFFFF);
      return 5;
    };
    
    f[0x46] = pos => {  // sub R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] - reg[p2] - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 2;
    };
    
    f[0x47] = pos => {  // sub R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] - p2 - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 2;
    };
    
    f[0x48] = pos => {  // sub R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] - p2 - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 3;
    };
    
    f[0x49] = pos => {  // sub R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] - p2 - (this.Y ? 1 : 0);
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 5;
    };
    
    f[0x4A] = pos => {  // cmp R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      this._affectFlags(reg[p1] - reg[p2], 32);
      this.LT = reg[p1] < reg[p2];
      this.GT = reg[p1] > reg[p2];
      this.Y = (reg[p1] - reg[p2]) < 0;
      return 2;
    };
    
    f[0x4B] = pos => {  // cmp R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      this._affectFlags(reg[p1] - p2, 8);
      this.LT = reg[p1] < p2;
      this.GT = reg[p1] > p2;
      this.Y = (reg[p1] - p2) < 0;
      return 2;
    };
    
    f[0x4C] = pos => {  // cmp R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      this._affectFlags(reg[p1] - p2, 16);
      this.LT = reg[p1] < p2;
      this.GT = reg[p1] > p2;
      this.Y = (reg[p1] - p2) < 0;
      return 3;
    };
    
    f[0x4D] = pos => {  // cmp R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      this._affectFlags(reg[p1] - p2, 32);
      this.LT = reg[p1] < p2;
      this.GT = reg[p1] > p2;
      this.Y = (reg[p1] - p2) < 0;
      return 5;
    };

    f[0x8B] = pos => {  // cmp R
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      this._affectFlags(reg[p1]);
      this.Y = reg[p1] < 0;
      return 1;
    };
    
    f[0x4E] = pos => {  // mul R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] * reg[p2];
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 2;
    };
    
    f[0x4F] = pos => {  // mul R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] * p2;
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 2;
    };
    
    f[0x50] = pos => {  // mul R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] * p2;
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 3;
    };
    
    f[0x51] = pos => {  // mul R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] * p2;
      reg[p1] = this._affectFlags(r);
      this.V = (r > 0xFFFFFFFF);
      return 5;
    };
    
    f[0x52] = pos => {  // idiv R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = Math.floor(reg[p1] / reg[p2]);
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x53] = pos => {  // idiv R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = Math.floor(reg[p1] / p2);
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x54] = pos => {  // idiv R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = Math.floor(reg[p1] / p2);
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x55] = pos => {  // idiv R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = Math.floor(reg[p1] / p2);
      reg[p1] = this._affectFlags(r);
      return 5;
    };
    
    f[0x56] = pos => {  // mod R, R
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] % reg[p2];
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x57] = pos => {  // mod R, v8
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get(pos + 1)];
      const r = reg[p1] % p2;
      reg[p1] = this._affectFlags(r);
      return 2;
    };
    
    f[0x58] = pos => {  // mod R, v16
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get16(pos + 1)];
      const r = reg[p1] % p2;
      reg[p1] = this._affectFlags(r);
      return 3;
    };
    
    f[0x59] = pos => {  // mod R, v32
      let [reg, mb] = [this._reg, this._mb];
      const [p1, p2] = [mb.get(pos), mb.get32(pos + 1)];
      const r = reg[p1] % p2;
      reg[p1] = this._affectFlags(r);
      return 5;
    };

    f[0x5A] = pos => {  // inc R
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      const r = reg[p1] + 1;
      reg[p1] = this._affectFlags(r);
      this.Y = (r > 0xFFFFFFFF);
      return 1;
    };
    
    f[0x5B] = pos => {  // dec R
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      const r = reg[p1] - 1;
      reg[p1] = this._affectFlags(r);
      this.Y = (r < 0);
      return 1;
    };

    //
    // BRANCHES
    //

    f[0x5C] = pos => {  // bz A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x5D] = pos => {  // bz v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x5E] = pos => {  // bz A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (!this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x5F] = pos => {  // bz v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (!this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x60] = pos => {  // bneg A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.S) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x61] = pos => {  // bneg v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.S) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x62] = pos => {  // bpos A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (!this.S) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x63] = pos => {  // bpos v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (!this.S) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x64] = pos => {  // bgt A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.GT) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x65] = pos => {  // bgt v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.GT) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x66] = pos => {  // bgte A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.GT && this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x67] = pos => {  // bgte v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.GT && this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x68] = pos => {  // blt A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.LT) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x69] = pos => {  // blt v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.LT) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x6A] = pos => {  // blte A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.LT && this.Z) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x6B] = pos => {  // blte v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.LT && this.Z) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x6C] = pos => {  // bv A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (this.V) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x6D] = pos => {  // bv v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (this.V) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x6E] = pos => {  // bv A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get(pos);
      if (!this.V) {
        this.PC = reg[p1];
        return 0;
      }
      return 1;
    };

    f[0x6F] = pos => {  // bv v32
      let [reg, mb] = [this._reg, this._mb];
      const p1 = mb.get32(pos);
      if (!this.V) {
        this.PC = p1;
        return 0;
      }
      return 4;
    };

    f[0x70] = pos => {  // jmp A
      let [reg, mb] = [this._reg, this._mb];
      const p1 = this._mb.get(pos);
      this.PC = this._reg[p1];
      return 0;
    };

    f[0x71] = pos => {  // jmp v32
      const p1 = this._mb.get32(pos);
      this.PC = p1;
      return 0;
    };

    f[0x72] = pos => {  // jsr A
      const p1 = this._mb.get(pos);
      this._push32(this.PC + 2);
      this.PC = this._reg[p1];
      return 0;
    };

    f[0x73] = pos => {  // jsr v32
      const p1 = this._mb.get32(pos);
      this._push32(this.PC + 5);
      this.PC = p1;
      return 0;
    };

    f[0x74] = pos => {  // ret
      this.PC = this._pop32();
      return 0;
    };

    f[0x75] = pos => {  // sys R
      const p1 = this._mb.get(pos);
      // TODO - enter supervisor mode
      this._push32(this.PC + 2);
      this.PC = this._syscallVector[this._reg[p1] & 0xFF];
      return 0;
    };
      
    f[0x76] = pos => {  // sys v8
      const p1 = this._mb.get(pos);
      // TODO - enter supervisor mode
      this._push32(this.PC + 2);
      this.PC = this._syscallVector[p1];
      return 0;
    };
      
    f[0x77] = pos => {  // iret
      this.PC = this._pop32();
      this.T = true;
      return 0;
    };

    f[0x86] = pos => {  // sret
      this.PC = this._pop32();
      // TODO - leave supervisor mode
      return 0;
    };

    f[0x78] = pos => {  // pushb R
      const p1 = this._mb.get(pos);
      this._push(this._reg[p1] & 0xFF);
      return 1;
    };

    f[0x79] = pos => {  // pushb v8
      const p1 = this._mb.get(pos);
      this._push(p1);
      return 1;
    };

    f[0x7A] = pos => {  // pushw R
      const p1 = this._mb.get(pos);
      this._push16(this._reg[p1] & 0xFFFF);
      return 1;
    };

    f[0x7B] = pos => {  // pushw v16
      const p1 = this._mb.get16(pos);
      this._push16(p1);
      return 2;
    };

    f[0x7C] = pos => {  // pushw R
      const p1 = this._mb.get(pos);
      this._push32(this._reg[p1]);
      return 1;
    };

    f[0x7D] = pos => {  // pushw v16
      const p1 = this._mb.get32(pos);
      this._push32(p1);
      return 4;
    };

    f[0x7E] = pos => {  // push.a
      for (let i = 0x0; i <= 0xB; ++i) {
        this._push32(this._reg[i]);
      }
      return 0;
    };

    f[0x7F] = pos => {  // popb R
      const p1 = this._mb.get(pos);
      this._reg[p1] = this._pop();
      return 1;
    };

    f[0x80] = pos => {  // popw R
      const p1 = this._mb.get(pos);
      this._reg[p1] = this._pop16();
      return 1;
    };

    f[0x81] = pos => {  // popw R
      const p1 = this._mb.get(pos);
      this._reg[p1] = this._pop32();
      return 1;
    };

    f[0x82] = pos => {  // pop.a
      for (let i = 0xB; i >= 0x0; --i) {
        this._reg[i] = this._pop32();
      }
      return 0;
    };

    f[0x83] = pos => {  // popx R
      const p1 = this._mb.get(pos);
      for (let i = 0; i < this._reg[p1]; ++i) {
        this._pop();
      }
      return 1;
    };
      
    f[0x84] = pos => {  // popx v8
      const p1 = this._mb.get(pos);
      for (let i = 0; i < p1; ++i) {
        this._pop();
      }
      return 1;
    };
      
    f[0x85] = pos => {  // popx v16
      const p1 = this._mb.get16(pos);
      for (let i = 0; i < p1; ++i) {
        this._pop();
      }
      return 2;
    };

    f[0x87] = pos => {  // nop
      return 1;
    };

    f[0x88] = pos => {  // halt
      this.systemHalted = true;
      return 1;
    };

    f[0x89] = pos => {  // dbg
      this.activateDebugger = true;
      return 1;
    };

    return f;
  }
 */
}

// }}}


void cpu_destroy()
{
}


// {{{ TESTS

void cpu_test()
{


    /*
  t.comment('Register movement (mov)');

  s = opc('mov A, B', () => cpu.B = 0x42); 
  t.equal(cpu.A, 0x42, s);
  t.equal(cpu.PC, 3, 'checking PC position');
  */


/*
function encode(line, acceptLabel=false, labelPrefix = '') {

  // separate operand and parameters
  let [operand, ...rest] = line.split(/[\s\t]+/);
  rest = rest.join('');

  // find parameters
  const parameters = rest.split(/[\s\t]*,[\s\t]* /).filter(s => s !== '');
  if (parameters.length > 2) {
    throw new Error('Invalid number of parameters.');
  }

  const pars = parameters.map(p => parseParameter(p, acceptLabel, labelPrefix));
  return parseOpcode(operand, pars, line);
}


// 
// OPCODES
//
let opcodes = [
  // movement
  [ 0x01, 'mov', 'reg', 'reg' ],
  [ 0x02, 'mov', 'reg', 'v8' ],
  [ 0x03, 'mov', 'reg', 'v16' ],
  [ 0x04, 'mov', 'reg', 'v32' ],
  [ 0x05, 'movb', 'reg', 'indreg' ],
  [ 0x06, 'movb', 'reg', 'indv32' ],
  [ 0x07, 'movw', 'reg', 'indreg' ],
  [ 0x08, 'movw', 'reg', 'indv32' ],
  [ 0x09, 'movd', 'reg', 'indreg' ],
  [ 0x0A, 'movd', 'reg', 'indv32' ],

  [ 0x0B, 'movb', 'indreg', 'reg' ],
  [ 0x0C, 'movb', 'indreg', 'v8' ],
  [ 0x0D, 'movb', 'indreg', 'indreg' ],
  [ 0x0E, 'movb', 'indreg', 'indv32' ],
  [ 0x0F, 'movw', 'indreg', 'reg' ],
  [ 0x1A, 'movw', 'indreg', 'v16' ],
  [ 0x1B, 'movw', 'indreg', 'indreg' ],
  [ 0x1C, 'movw', 'indreg', 'indv32' ],
  [ 0x1D, 'movd', 'indreg', 'reg' ],
  [ 0x1E, 'movd', 'indreg', 'v32' ],
  [ 0x1F, 'movd', 'indreg', 'indreg' ],
  [ 0x20, 'movd', 'indreg', 'indv32' ],

  [ 0x21, 'movb', 'indv32', 'reg' ],
  [ 0x22, 'movb', 'indv32', 'v8' ],
  [ 0x23, 'movb', 'indv32', 'indreg' ],
  [ 0x24, 'movb', 'indv32', 'indv32' ],
  [ 0x25, 'movw', 'indv32', 'reg' ],
  [ 0x26, 'movw', 'indv32', 'v16' ],
  [ 0x27, 'movw', 'indv32', 'indreg' ],
  [ 0x28, 'movw', 'indv32', 'indv32' ],
  [ 0x29, 'movd', 'indv32', 'reg' ],
  [ 0x2A, 'movd', 'indv32', 'v32' ],
  [ 0x2B, 'movd', 'indv32', 'indreg' ],
  [ 0x2C, 'movd', 'indv32', 'indv32' ],

  [ 0x8A, 'swap', 'reg', 'reg' ],

  // logic
  [ 0x2D, 'or', 'reg', 'reg' ],
  [ 0x2E, 'or', 'reg', 'v8' ],
  [ 0x2F, 'or', 'reg', 'v16' ],
  [ 0x30, 'or', 'reg', 'v32' ],
  [ 0x31, 'xor', 'reg', 'reg' ],
  [ 0x32, 'xor', 'reg', 'v8' ],
  [ 0x33, 'xor', 'reg', 'v16' ],
  [ 0x34, 'xor', 'reg', 'v32' ],
  [ 0x35, 'and', 'reg', 'reg' ],
  [ 0x36, 'and', 'reg', 'v8' ],
  [ 0x37, 'and', 'reg', 'v16' ],
  [ 0x38, 'and', 'reg', 'v32' ],
  [ 0x39, 'shl', 'reg', 'reg' ],
  [ 0x3A, 'shl', 'reg', 'v8' ],
  [ 0x3D, 'shr', 'reg', 'reg' ],
  [ 0x3E, 'shr', 'reg', 'v8' ],
  [ 0x41, 'not', 'reg' ],

  // arithmetic
  [ 0x42, 'add', 'reg', 'reg' ],
  [ 0x43, 'add', 'reg', 'v8' ],
  [ 0x44, 'add', 'reg', 'v16' ],
  [ 0x45, 'add', 'reg', 'v32' ],
  [ 0x46, 'sub', 'reg', 'reg' ],
  [ 0x47, 'sub', 'reg', 'v8' ],
  [ 0x48, 'sub', 'reg', 'v16' ],
  [ 0x49, 'sub', 'reg', 'v32' ],
  [ 0x4A, 'cmp', 'reg', 'reg' ],
  [ 0x4B, 'cmp', 'reg', 'v8' ],
  [ 0x4C, 'cmp', 'reg', 'v16' ],
  [ 0x4D, 'cmp', 'reg', 'v32' ],
  [ 0x8B, 'cmp', 'reg' ],
  [ 0x4E, 'mul', 'reg', 'reg' ],
  [ 0x4F, 'mul', 'reg', 'v8' ],
  [ 0x50, 'mul', 'reg', 'v16' ],
  [ 0x51, 'mul', 'reg', 'v32' ],
  [ 0x52, 'idiv', 'reg', 'reg' ],
  [ 0x53, 'idiv', 'reg', 'v8' ],
  [ 0x54, 'idiv', 'reg', 'v16' ],
  [ 0x55, 'idiv', 'reg', 'v32' ],
  [ 0x56, 'mod', 'reg', 'reg' ],
  [ 0x57, 'mod', 'reg', 'v8' ],
  [ 0x58, 'mod', 'reg', 'v16' ],
  [ 0x59, 'mod', 'reg', 'v32' ],
  [ 0x5A, 'inc', 'reg' ],
  [ 0x5B, 'dec', 'reg' ],

  // jumps
  [ 0x5C, 'bz', 'reg' ],
  [ 0x5D, 'bz', 'v32' ],
  [ 0x5C, 'beq', 'reg' ],
  [ 0x5D, 'beq', 'v32' ],
  [ 0x5E, 'bnz', 'reg' ],
  [ 0x5F, 'bnz', 'v32' ],
  [ 0x60, 'bneg', 'reg' ],
  [ 0x61, 'bneg', 'v32' ],
  [ 0x62, 'bpos', 'reg' ],
  [ 0x63, 'bpos', 'v32' ],
  [ 0x64, 'bgt', 'reg' ],
  [ 0x65, 'bgt', 'v32' ],
  [ 0x66, 'bgte', 'reg' ],
  [ 0x67, 'bgte', 'v32' ],
  [ 0x68, 'blt', 'reg' ],
  [ 0x69, 'blt', 'v32' ],
  [ 0x6A, 'blte', 'reg' ],
  [ 0x6B, 'blte', 'v32' ],
  [ 0x6C, 'bv', 'reg' ],
  [ 0x6D, 'bv', 'v32' ],
  [ 0x6E, 'bnv', 'reg' ],
  [ 0x6F, 'bnv', 'v32' ],
  [ 0x70, 'jmp', 'reg' ],
  [ 0x71, 'jmp', 'v32' ],
  [ 0x72, 'jsr', 'reg' ],
  [ 0x73, 'jsr', 'v32' ],
  [ 0x74, 'ret' ],
  [ 0x75, 'sys', 'reg' ],
  [ 0x76, 'sys', 'v8' ],
  [ 0x77, 'iret' ],
  [ 0x86, 'sret' ],

  // stack
  [ 0x78, 'pushb', 'reg' ],
  [ 0x79, 'pushb', 'v8' ],
  [ 0x7A, 'pushw', 'reg' ],
  [ 0x7B, 'pushw', 'v16' ],
  [ 0x7C, 'pushd', 'reg' ],
  [ 0x7D, 'pushd', 'v32' ],
  [ 0x7E, 'push.a' ],
  [ 0x7F, 'popb', 'reg' ],
  [ 0x80, 'popw', 'reg' ],
  [ 0x81, 'popd', 'reg' ],
  [ 0x82, 'pop.a' ],
  [ 0x83, 'popx', 'reg' ],
  [ 0x84, 'popx', 'v8' ],
  [ 0x85, 'popx', 'v9' ],

  // other
  [ 0x87, 'nop' ],
  [ 0x88, 'halt' ],
  [ 0x89, 'dbg' ],
];

function parseOpcode(operand, pars, line) {
  // find opcode
  for(let op of opcodes) {
    let ptype = pars.map(p => (p ? p.type : undefined));
    if (operand.toLowerCase() === op[1] && ptype[0] === op[2] && ptype[1] === op[3]) {
      let a = [op[0]];
      if (pars[0]) a = a.concat(pars[0].array);
      if (pars[1]) a = a.concat(pars[1].array);
      return a;
    }
  }

  // if opcode wasn't found, and par is < v32, try to find higher value
  let increasePar = (p) => p.type === 'v16' ? { type: 'v32', array: p.array.concat([0,0]) } : { type: 'v16', array: p.array.concat([0]) };
  let canIncrease = (p) => p && (p.type === 'v8' || p.type === 'v16');

  if (canIncrease(pars[0])) {
    try { return parseOpcode(operand, [increasePar(pars[0]), pars[1]]); } catch(e) {}
  } else if (canIncrease(pars[1])) {
    try { return parseOpcode(operand, [pars[0], increasePar(pars[1])]); } catch(e) {}
  }
  if (canIncrease(pars[0]) && canIncrease(pars[1])) {
    try { return parseOpcode(operand, [increasePar(pars[0]), increasePar(pars[1])]); } catch(e) {}
  }

  throw new Error(`Invalid command ${line}.`);
}


// 
// PARSE PARAMETERS
// 
function parseParameter(p, acceptLabel, labelPrefix) {

  function registerValue(r) {
    switch (r) {
      case 'a': return 0;
      case 'b': return 1;
      case 'c': return 2;
      case 'd': return 3;
      case 'e': return 4;
      case 'f': return 5;
      case 'g': return 6;
      case 'h': return 7;
      case 'i': return 8;
      case 'j': return 9;
      case 'k': return 10;
      case 'l': return 11;
      case 'fp': return 12;
      case 'sp': return 13;
      case 'pc': return 14;
      case 'fl': return 15;
      default: 
        return -1;
    }
  }

  let [type, array] = [0, 'none', []];

  // if indirect, add indirection and return
  if (p.startsWith('[') && p.endsWith(']')) {
    let pp = parseParameter(p.slice(1, p.length - 1), acceptLabel, labelPrefix);
    if (pp.type === 'v8') {
      pp.array = pp.array.concat([0, 0, 0]);
      pp.type = 'v32';
    } else if (pp.type === 'v16') {
      pp.array = pp.array.concat([0, 0]);
      pp.type = 'v32';
    }
    pp.type = 'ind' + pp.type;
    return pp;
  }

  // if binary, convert to number
  if (/0b[01_]+/.test(p)) {
    p = parseInt(p.slice(2).replace('_', ''), 2).toString();
  }

  // is it a number?
  if (/^-?\d+$/.test(p) || /^0[Xx][\dA-Fa-f]+$/.test(p)) {
    let value = parseInt(p);
    if (value < 0) {
      value = value >>> 0;
    }
    if (value <= 0xFF) {
      type = 'v8';
      array = [value];
    } else if (value <= 0xFFFF) {
      type = 'v16';
      array = [value & 0xFF, value >> 8];
    } else if (value <= 0xFFFFFFFF) {
      type = 'v32';
      array = [value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF];
    } else {
      throw new Error('Values higher than 32-bit are unsupported.');
    }
  
  // is a register or label
  } else {
    const value = registerValue(p.toLowerCase());

    // is a register
    if (value >= 0) {
      type = 'reg';
      array = [value];

    // is a label
    } else if (acceptLabel) {
      type = 'v32';
      array = [p.startsWith('.') ? (labelPrefix + p) : p, 0, 0, 0];

    // its niether
    } else {
      throw new Error(`Could not understand expression '${p}'.`);
    }

  }

  return { type, array };
}

function makeCPU() {
  const m = new Motherboard();
  m.addDevice(new MMU(new RAM(256)));
  const c = new CPU(m);
  m.addDevice(c);
  return [m, c];
}


test('CPU: Sanity check', t => {
  let m, c;
  t.doesNotThrow(() => [m, c] = makeCPU(), null, 'CPU is created without errors');
  c.A = 24;
  c.FL = 0b10;
  t.equal(c.A, 24, 'register setter/getter');
  t.pass(c.V, 'flag bits set correctly');
  t.equal(c.PC, 0, 'CPU init address');
  t.end();
});


test('CPU: Get register contents from memory', t => {
  const [mb, cpu] = makeCPU();
  cpu.K = 0xABCDEF01;
  t.equal(cpu.CPU_K, 0xF0002038, 'CPU_K == 0xF0002038');
  t.equal(mb.get32(cpu.CPU_K), 0xABCDEF01, 'read register from memory');
  mb.set32(cpu.CPU_K, 0x12345678);
  t.equal(cpu.K, 0x12345678, 'set register from memory');
  t.equal(mb.get32(cpu.CPU_K), 0x12345678, 'set and then read register from memory');
  t.end();
});


test('CPU: Execute valid basic commands', t => {
  let [mb, cpu] = makeCPU();

  function opc(s, pre) {
    mb.reset();
    if(pre) { 
      pre(); 
    }
    mb.setArray(0, Debugger.encode(s));
    let r = `[0x${mb.get(0) < 0x10 ? '0' : ''}${mb.get(0).toString(16)}] ` + s;
    mb.step();
    return r;
  }

  let s;

  // 
  // MOV
  //
  t.comment('Register movement (mov)');

  s = opc('mov A, B', () => cpu.B = 0x42); 
  t.equal(cpu.A, 0x42, s);
  t.equal(cpu.PC, 3, 'checking PC position');

  s = opc('mov A, 0x34'); 
  t.equal(cpu.A, 0x34, s);
  
  s = opc('mov A, 0x1234'); 
  t.equal(cpu.A, 0x1234, s);
  
  s = opc('mov A, 0xFABC1234'); 
  t.equal(cpu.A, 0xFABC1234, s);

  t.comment('Test movement flags');
  
  s = opc('mov A, 0');
  t.true(cpu.Z, 'cpu.Z = 1');
  t.true(cpu.P, 'cpu.P = 1');
  t.false(cpu.S, 'cpu.S = 0');

  s = opc('mov A, 0xF0000001');
  t.false(cpu.Z, 'cpu.Z = 0');
  t.false(cpu.P, 'cpu.P = 0');
  t.true(cpu.S, 'cpu.S = 1');

  // 
  // MOVB
  //
  
  t.comment('8-bit movement (movb)');

  s = opc('movb A, [B]', () => { cpu.B = 0x1000; mb.set(cpu.B, 0xAB); }); 
  t.equal(cpu.A, 0xAB, s);
  
  s = opc('movb A, [0x1000]', () => mb.set(0x1000, 0xAB));
  t.equal(cpu.A, 0xAB, s);

  s = opc('movb [C], A', () => { cpu.A = 0x64, cpu.C = 0x32 });
  t.equal(mb.get(0x32), 0x64, s);

  s = opc('movb [A], 0xFA', () => cpu.A = 0x64);
  t.equal(mb.get(0x64), 0xFA, s);

  s = opc('movb [A], [B]', () => { cpu.A = 0x32; cpu.B = 0x64; mb.set(0x64, 0xFF); });
  t.equal(mb.get(0x32), 0xFF, s);

  s = opc('movb [A], [0x6420]', () => { cpu.A = 0x32; mb.set(0x6420, 0xFF); });
  t.equal(mb.get(0x32), 0xFF, s);

  s = opc('movb [0x64], A', () => cpu.A = 0xAC32);
  t.equal(mb.get(0x64), 0x32, s);

  s = opc('movb [0x64], 0xF0');
  t.equal(mb.get(0x64), 0xF0, s);
  
  s = opc('movb [0xCC64], [A]', () => { 
    cpu.A = 0xF000; mb.set(0xF000, 0x42); 
  });
  t.equal(mb.get(0xCC64), 0x42, s);
  
  s = opc('movb [0x64], [0xABF0]', () => { 
    mb.set32(0xABF0, 0x1234); mb.set(0x1234, 0x3F);
  });
  t.equal(mb.get(0x64), 0x3F, s);

  // 
  // MOVW
  //
  
  t.comment('16-bit movement (movw)');
  
  s = opc('movw A, [B]', () => { cpu.B = 0x1000; mb.set16(cpu.B, 0xABCD); }); 
  t.equal(cpu.A, 0xABCD, s);
  
  s = opc('movw A, [0x1000]', () => mb.set16(0x1000, 0xABCD));
  t.equal(cpu.A, 0xABCD, s);

  s = opc('movw [A], A', () => cpu.A = 0x6402);
  t.equal(mb.get16(0x6402), 0x6402, s);

  s = opc('movw [A], 0xFABA', () => cpu.A = 0x64);
  t.equal(mb.get16(0x64), 0xFABA, s);

  s = opc('movw [A], [B]', () => { cpu.A = 0x32CC; cpu.B = 0x64; mb.set16(0x64, 0xFFAB); });
  t.equal(mb.get16(0x32CC), 0xFFAB, s);

  s = opc('movw [A], [0x6420]', () => { cpu.A = 0x32; mb.set16(0x6420, 0xFFAC); });
  t.equal(mb.get16(0x32), 0xFFAC, s);

  s = opc('movw [0x64], A', () => cpu.A = 0xAB32AC);
  t.equal(mb.get16(0x64), 0x32AC, s);

  s = opc('movw [0x64], 0xF0FA');
  t.equal(mb.get16(0x64), 0xF0FA, s);
  
  s = opc('movw [0xCC64], [A]', () => { 
    cpu.A = 0xF000; mb.set16(0xF000, 0x4245); 
  });
  t.equal(mb.get16(0xCC64), 0x4245, s);
  
  s = opc('movw [0x64], [0xABF0]', () => { 
    mb.set32(0xABF0, 0x1234); mb.set16(0x1234, 0x3F54);
  });
  t.equal(mb.get16(0x64), 0x3F54, s);

  // 
  // MOVD
  //

  t.comment('32-bit movement (movd)');
  
  s = opc('movd A, [B]', () => { cpu.B = 0x1000; mb.set32(cpu.B, 0xABCDEF01); }); 
  t.equal(cpu.A, 0xABCDEF01, s);
  
  s = opc('movd A, [0x1000]', () => mb.set32(0x1000, 0xABCDEF01));
  t.equal(cpu.A, 0xABCDEF01, s);

  s = opc('movd [A], A', () => cpu.A = 0x16402);
  t.equal(mb.get32(0x16402), 0x16402, s);

  s = opc('movd [A], 0xFABA1122', () => cpu.A = 0x64);
  t.equal(mb.get32(0x64), 0xFABA1122, s);

  s = opc('movd [A], [B]', () => { cpu.A = 0x32CC; cpu.B = 0x64; mb.set32(0x64, 0xFFAB5678); });
  t.equal(mb.get32(0x32CC), 0xFFAB5678, s);

  s = opc('movd [A], [0x6420]', () => { cpu.A = 0x32; mb.set32(0x6420, 0xFFAC9876); });
  t.equal(mb.get32(0x32), 0xFFAC9876, s);

  s = opc('movd [0x64], A', () => cpu.A = 0xAB32AC44);
  t.equal(mb.get32(0x64), 0xAB32AC44, s);

  s = opc('movd [0x64], 0xF0FA1234');
  t.equal(mb.get32(0x64), 0xF0FA1234, s);
  
  s = opc('movd [0xCC64], [A]', () => { 
    cpu.A = 0xF000; mb.set32(0xF000, 0x4245AABB); 
  });
  t.equal(mb.get32(0xCC64), 0x4245AABB, s);
  
  s = opc('movd [0x64], [0xABF0]', () => { 
    mb.set32(0xABF0, 0x1234); mb.set32(0x1234, 0x3F54FABC);
  });
  t.equal(mb.get32(0x64), 0x3F54FABC, s);

  //
  // LOGIC OPERATIONS
  //

  t.comment('Logic operations');

  s = opc('or A, B', () => { cpu.A = 0b1010; cpu.B = 0b1100; });
  t.equal(cpu.A, 0b1110, s);
  t.false(cpu.S, "cpu.S == 0");
  t.true(cpu.P, "cpu.P == 1");
  t.false(cpu.Z, "cpu.Z == 0");
  t.false(cpu.Y, "cpu.Y == 0");
  t.false(cpu.V, "cpu.V == 0");

  s = opc('or A, 0x4', () => { cpu.A = 0b11; });
  t.equal(cpu.A, 0b111, s);

  s = opc('or A, 0x4000', () => { cpu.A = 0b111; });
  t.equal(cpu.A, 0x4007, s);

  s = opc('or A, 0x2A426653', () => { cpu.A = 0x10800000; });
  t.equal(cpu.A, 0x3AC26653, s);

  s = opc('xor A, B', () => { cpu.A = 0b1010; cpu.B = 0b1100; });
  t.equal(cpu.A, 0b110, s);

  s = opc('xor A, 0x4', () => { cpu.A = 0b11; });
  t.equal(cpu.A, 0b111, s);

  s = opc('xor A, 0xFF00', () => { cpu.A = 0xFF0; });
  t.equal(cpu.A, 0xF0F0, s);

  s = opc('xor A, 0x2A426653', () => { cpu.A = 0x148ABD12; });
  t.equal(cpu.A, 0x3EC8DB41, s);

  s = opc('and A, B', () => { cpu.A = 0b11; cpu.B = 0b1100; });
  t.equal(cpu.A, 0, s);
  t.true(cpu.Z, "cpu.Z == 1");

  s = opc('and A, 0x7', () => { cpu.A = 0b11; });
  t.equal(cpu.A, 0b11, s);

  s = opc('and A, 0xFF00', () => { cpu.A = 0xFF0; });
  t.equal(cpu.A, 0xF00, s);

  s = opc('and A, 0x2A426653', () => { cpu.A = 0x148ABD12; });
  t.equal(cpu.A, 0x22412, s);

  s = opc('shl A, B', () => { cpu.A = 0b10101010; cpu.B = 4; });
  t.equal(cpu.A, 0b101010100000, s);

  s = opc('shl A, 4', () => { cpu.A = 0b10101010;});
  t.equal(cpu.A, 0b101010100000, s);

  s = opc('shr A, B', () => { cpu.A = 0b10101010; cpu.B = 4; });
  t.equal(cpu.A, 0b1010, s);

  s = opc('shr A, 4', () => { cpu.A = 0b10101010; });
  t.equal(cpu.A, 0b1010, s);

  s = opc('not A', () => { cpu.A = 0b11001010; });
  t.equal(cpu.A, 0b11111111111111111111111100110101, s);

  //
  // integer math
  //

  t.comment('Integer arithmetic');
  
  s = opc('add A, B', () => { cpu.A = 0x12; cpu.B = 0x20; });
  t.equal(cpu.A, 0x32, s);
  
  s = opc('add A, 0x20', () => cpu.A = 0x12);
  t.equal(cpu.A, 0x32, s);

  s = opc('add A, 0x20', () => { cpu.A = 0x12, cpu.Y = true; });
  t.equal(cpu.A, 0x33, 'add A, 0x20 (with carry)');

  s = opc('add A, 0x2000', () => cpu.A = 0x12);
  t.equal(cpu.A, 0x2012, s);

  s = opc('add A, 0xF0000000', () => cpu.A = 0x10000012);
  t.equal(cpu.A, 0x12, s);
  t.true(cpu.Y, "cpu.Y == 1");

  s = opc('sub A, B', () => { cpu.A = 0x30; cpu.B = 0x20; });
  t.equal(cpu.A, 0x10, s);
  t.false(cpu.S, 'cpu.S == 0');

  s = opc('sub A, B', () => { cpu.A = 0x20; cpu.B = 0x30; });
  t.equal(cpu.A, 0xFFFFFFF0, 'sub A, B (negative)');
  t.true(cpu.S, 'cpu.S == 1');

  s = opc('sub A, 0x20', () => cpu.A = 0x22);
  t.equal(cpu.A, 0x2, s);

  s = opc('sub A, 0x20', () => { cpu.A = 0x22; cpu.Y = true; });
  t.equal(cpu.A, 0x1, 'sub A, 0x20 (with carry)');

  s = opc('sub A, 0x2000', () => cpu.A = 0x12);
  t.equal(cpu.A, 0xFFFFE012, s);
  t.true(cpu.S, 'cpu.S == 1');
  t.true(cpu.Y, 'cpu.Y == 1');

  s = opc('sub A, 0xF0000000', () => cpu.A = 0x10000012);
  t.equal(cpu.A, 0x20000012, s);
  t.true(cpu.Y, 'cpu.Y == 1');

  s = opc('cmp A, B');
  t.true(cpu.Z, s);

  s = opc('cmp A, 0x12');
  t.true(cpu.LT && !cpu.GT, s);

  s = opc('cmp A, 0x1234', () => cpu.A = 0x6000);
  t.true(!cpu.LT && cpu.GT, s);

  s = opc('cmp A, 0x12345678', () => cpu.A = 0xF0000000);
  t.true(!cpu.LT && cpu.GT, s);  // because of the signal!

  s = opc('cmp A', () => cpu.A = 0x0);
  t.true(cpu.Z, s);

  s = opc('mul A, B', () => { cpu.A = 0xF0; cpu.B = 0xF000; });
  t.equal(cpu.A, 0xE10000, s);

  s = opc('mul A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x147A8, s);

  s = opc('mul A, 0x12AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x154198C, s);
  t.false(cpu.V, 'cpu.V == 0');

  s = opc('mul A, 0x12AF87AB', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x233194BC, s);
  t.true(cpu.V, 'cpu.V == 1');

  s = opc('idiv A, B', () => { cpu.A = 0xF000; cpu.B = 0xF0; });
  t.equal(cpu.A, 0x100, s);

  s = opc('idiv A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x102, s);

  s = opc('idiv A, 0x2AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x6, s);

  s = opc('idiv A, 0x12AF', () => cpu.A = 0x123487AB);
  t.equal(cpu.A, 0xF971, s);

  s = opc('mod A, B', () => { cpu.A = 0xF000; cpu.B = 0xF0; });
  t.equal(cpu.A, 0x0, s);
  t.true(cpu.Z, 'cpu.Z == 1');

  s = opc('mod A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x10, s);

  s = opc('mod A, 0x2AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x21A, s);

  s = opc('mod A, 0x12AF', () => cpu.A = 0x123487AB);
  t.equal(cpu.A, 0x116C, s);

  s = opc('inc A', () => cpu.A = 0x42);
  t.equal(cpu.A, 0x43, s);

  s = opc('inc A', () => cpu.A = 0xFFFFFFFF);
  t.equal(cpu.A, 0x0, 'inc A (overflow)');
  t.true(cpu.Y, 'cpu.Y == 1');
  t.true(cpu.Z, 'cpu.Z == 1');

  s = opc('dec A', () => cpu.A = 0x42);
  t.equal(cpu.A, 0x41, s);

  s = opc('dec A', () => cpu.A = 0x0);
  t.equal(cpu.A, 0xFFFFFFFF, 'dec A (underflow)');
  t.false(cpu.Z, 'cpu.Z == 0');

  // 
  // branches
  //

  t.comment('Branch operations');

  s = opc('bz A', () => { cpu.Z = true; cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bz A', () => { cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x2, 'bz A (false)');

  s = opc('bz 0x1000', () => cpu.Z = true);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bnz A', () => cpu.A = 0x1000);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bnz 0x1000');
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bneg A', () => { cpu.S = true; cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bneg A', () => { cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x2, 'bneg A (false)');

  s = opc('bneg 0x1000', () => cpu.S = true);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bpos A', () => cpu.A = 0x1000);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bpos 0x1000');
  t.equal(cpu.PC, 0x1000, s);

  s = opc('jmp 0x12345678');
  t.equal(cpu.PC, 0x12345678, s);
  

  // 
  // stack
  //

  t.comment('Stack operations');

  mb.reset();
  cpu.SP = 0xFFF; 
  cpu.A = 0xABCDEF12;

  mb.setArray(0x0, Debugger.encode('pushb A'));
  mb.setArray(0x2, Debugger.encode('pushb 0x12'));
  mb.setArray(0x4, Debugger.encode('pushw A'));
  mb.setArray(0x6, Debugger.encode('pushd A'));

  mb.setArray(0x8, Debugger.encode('popd B'));
  mb.setArray(0xA, Debugger.encode('popw B'));
  mb.setArray(0xC, Debugger.encode('popb B'));

  mb.setArray(0xE, Debugger.encode('popx 1'));

  mb.step();
  t.equal(mb.get(0xFFF), 0x12, 'pushb A');
  t.equal(cpu.SP, 0xFFE, 'SP = 0xFFE');

  mb.step();
  t.equal(mb.get(0xFFE), 0x12, 'pushb 0x12');
  t.equal(cpu.SP, 0xFFD, 'SP = 0xFFD');

  mb.step();
  t.equal(mb.get16(0xFFC), 0xEF12, s);
  t.equal(mb.get(0xFFD), 0xEF, s);
  t.equal(mb.get(0xFFC), 0x12, s);
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFB');

  mb.step();
  t.equal(mb.get32(0xFF8), 0xABCDEF12);
  t.equal(cpu.SP, 0xFF7, 'SP = 0xFF7');

  mb.step();
  t.equal(cpu.B, 0xABCDEF12, 'popd B');

  mb.step();
  t.equal(cpu.B, 0xEF12, 'popw B');

  mb.step();
  t.equal(cpu.B, 0x12, 'popb B');

  mb.step();
  t.equal(cpu.SP, 0xFFF, 'popx 1');

  // all registers
  s = opc('push.a', () => {
    cpu.SP = 0xFFF;
    cpu.A = 0xA1B2C3E4;
    cpu.B = 0xFFFFFFFF;
  });
  t.equal(cpu.SP, 0xFCF, s);
  t.equal(mb.get32(0xFFC), 0xA1B2C3E4, 'A is saved');
  t.equal(mb.get32(0xFF8), 0xFFFFFFFF, 'B is saved');
  
  s = opc('pop.a', () => {
    cpu.SP = 0xFCF;
    mb.set32(0xFFC, 0xA1B2C3E4);
    mb.set32(0xFF8, 0xFFFFFFFF);
  });
  t.equal(cpu.SP, 0xFFF, s);
  t.equal(cpu.A, 0xA1B2C3E4, 'A is restored');
  t.equal(cpu.B, 0xFFFFFFFF, 'B is restored');

  // others
  t.comment('Others');

  opc('nop');
  
  s = opc('dbg');
  t.true(cpu.activateDebugger, s);

  s = opc('halt');
  t.true(cpu.systemHalted, s);

  s = opc('swap A, B', () => {
    cpu.A = 0xA;
    cpu.B = 0xB;
  });
  t.true(cpu.A == 0xB && cpu.B == 0xA, s);

  t.end();

});


test('CPU: subroutines and system calls', t => {

  let [mb, cpu] = makeCPU();

  // jsr
  mb.reset();
  mb.setArray(0x200, Debugger.encode('jsr 0x1234'));
  mb.setArray(0x1234, Debugger.encode('ret'));
  cpu.PC = 0x200;
  cpu.SP = 0xFFF;
  mb.step();
  t.equal(cpu.PC, 0x1234, 'jsr 0x1234');
  t.equal(mb.get(0xFFC), 0x5, '[FFC] = 0x5');
  t.equal(mb.get(0xFFD), 0x2, '[FFD] = 0x2');
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFB');
  t.equal(mb.get32(0xFFC), 0x200 + 5, 'address in stack'); 

  mb.step();
  t.equal(cpu.PC, 0x205, 'ret');
  t.equal(cpu.SP, 0xFFF, 'SP = 0xFFF');

  // sys
  mb.reset();
  cpu.SP = 0xFFF;
  mb.setArray(0, Debugger.encode('sys 2'));
  mb.set32(cpu.CPU_SYSCALL_VECT + 8, 0x1000);
  t.equal(cpu._syscallVector[2], 0x1000, 'syscall vector');
  mb.setArray(0x1000, Debugger.encode('sret'));

  mb.step();
  t.equal(cpu.PC, 0x1000, 'sys 2');
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFD');
  mb.step();
  t.equal(cpu.PC, 0x2, 'sret');
  t.equal(cpu.SP, 0xFFF, 'SP = 0xFFF');

  t.end();

});


test('CPU: interrupts', t => {

  let [mb, cpu] = makeCPU();
  cpu.T = true;
  cpu.SP = 0x800;
  mb.set32(cpu.CPU_INTERRUPT_VECT + 8, 0x1000);
  mb.setArray(0x0, Debugger.encode('movb A, [0xE0000000]'));
  mb.setArray(0x1000, Debugger.encode('iret'));

  mb.step();  // cause the exception
  t.equal(cpu.PC, 0x1000, 'interrupt called');
  t.true(cpu.T, 'interrupts disabled');

  mb.step();  // iret
  t.equal(cpu.PC, 0x6, 'iret');
  t.true(cpu.T, 'interrupts enabled');

  t.end();

});


test('CPU: invalid opcode', t => {

  let [mb, cpu] = makeCPU();
  cpu.T = true;
  mb.set32(cpu.CPU_INTERRUPT_VECT + 12, 0x1000);
  mb.set(0x0, 0xFF);
  mb.step();
  t.equal(cpu.PC, 0x1000, 'interrupt called');

  t.end();

});
*/
}

// }}}
