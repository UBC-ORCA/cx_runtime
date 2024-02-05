// Made by Brandon :)

/* cfu_reg */

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in 
class RVInstR_CFU<dag outs, dag ins, string opcodestr, string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<10> cf_id;
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;
   
  let Inst{31-25} = cf_id{9-3}; //cf_id[9:3]
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = cf_id{2-0}; //cf_id[2:0] 
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_CUSTOM_0.Value;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in 
class RVInstI_CFU<dag outs, dag ins, string opcodestr, string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<8> imm;
  bits<4> cf_id;
  bits<5> rs1;
  bits<5> rd;
  
  let Inst{31-24} = imm;
  let Inst{23-20} = cf_id;
  let Inst{19-15} = rs1;
  let Inst{14-12} = 0b000; 
  let Inst{11-7} = rd;
  let Inst{6-0} = OPC_CUSTOM_1.Value;
}

def uimm4_t : RISCVOp<i32>, TImmLeaf<i32, [{return isUInt<4>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<4>;
  let DecoderMethod = "decodeUImmOperand<4>";
  let OperandType = "OPERAND_CF_ID_4";
}

def uimm8_t : RISCVOp<i32>, TImmLeaf<i32, [{return isUInt<8>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<8>;
  let DecoderMethod = "decodeUImmOperand<8>";
  let OperandType = "OPERAND_CFU_IMM";
}

def uimm10_t : RISCVOp<i32>, TImmLeaf<i32, [{return isUInt<10>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<10>;
  let DecoderMethod = "decodeUImmOperand<10>";
  let OperandType = "OPERAND_CF_ID";
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class CFU_R<string opcodestr,
             bit Commutable = 0>
    : RVInstR_CFU<(outs GPR:$rd), (ins uimm10_t:$cf_id, GPR:$rs1, GPR:$rs2),
              opcodestr, "$cf_id, $rd, $rs1, $rs2"> {
  let isCommutable = Commutable;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class CFU_I<string opcodestr,
             bit Commutable = 0>
    : RVInstI_CFU<(outs GPR:$rd), (ins uimm4_t:$cf_id, GPR:$rs1, uimm8_t:$imm),
              opcodestr, "$cf_id, $rd, $rs1, $imm"> {
  let isCommutable = Commutable;
}

class PatImmGprGpr<SDPatternOperator OpNode, RVInstR_CFU Inst>
    : Pat<(XLenVT (OpNode (XLenVT GPR:$rs1), (XLenVT GPR:$rs2), uimm10_t:$imm)), (Inst uimm10_t:$imm, GPR:$rs1, GPR:$rs2)>;

class PatImmImmGpr<SDPatternOperator OpNode, RVInstI_CFU Inst>
    : Pat<(XLenVT (OpNode uimm4_t:$cf_id, (XLenVT GPR:$rs1), uimm8_t:$imm)), (Inst uimm4_t:$cf_id, GPR:$rs1, uimm8_t:$imm)>;

def CFU_REG   : CFU_R<"cfu_reg">;
def CFU_IMM   : CFU_I<"cfu_imm">;

def : PatImmGprGpr<int_riscv_cfu_reg, CFU_REG>; 
def : PatImmImmGpr<int_riscv_cfu_imm, CFU_IMM>; 