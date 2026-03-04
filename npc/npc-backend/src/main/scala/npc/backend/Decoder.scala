package npc.backend.decode

import chisel3._
import chisel3.util._
import chisel3.util.experimental.decode._
import freechips.rocketchip.rocket.Instructions._

import utils._
import npc._
import npc.backend.fu._
import npc.backend.fu.OpType._
import npc.backend.fu.ImmType._

class Decoder(implicit p: CoreParameters) extends Module {
  val io = IO(new Bundle {
    val instr = Input(UInt(32.W))

    val regIndice1   = Output(UInt(5.W))
    val regIndice2   = Output(UInt(5.W))
    val regIndice3   = Output(UInt(5.W))
    val fuType       = Output(UInt(FuType.num.W))
    val opType       = Output(UInt(OpType.width.W))
    val immValue     = Output(SInt(p.XLEN.W))
  })

  val allRV32IPatterns = Seq(
    // alu
    InstructionPattern("add", ADD, FuType.alu, OpType.add, ImmType.rType),
    InstructionPattern("sub", SUB, FuType.alu, OpType.sub, ImmType.rType),
    InstructionPattern("sll", SLL, FuType.alu, OpType.sll, ImmType.rType),
    InstructionPattern("slt", SLT, FuType.alu, OpType.slt, ImmType.rType),
    InstructionPattern("sltu", SLTU, FuType.alu, OpType.sltu, ImmType.rType),
    InstructionPattern("xor", XOR, FuType.alu, OpType.xor, ImmType.rType),
    InstructionPattern("srl", SRL, FuType.alu, OpType.srl, ImmType.rType),
    InstructionPattern("sra", SRA, FuType.alu, OpType.sra, ImmType.rType),
    InstructionPattern("or", OR, FuType.alu, OpType.or, ImmType.rType),
    InstructionPattern("and", AND, FuType.alu, OpType.and, ImmType.rType),

    InstructionPattern("addi", ADDI, FuType.alu, OpType.addi, ImmType.iType),
    InstructionPattern("slti", SLTI, FuType.alu, OpType.slti, ImmType.iType),
    InstructionPattern("sltiu", SLTIU, FuType.alu, OpType.sltiu, ImmType.iType),
    InstructionPattern("xori", XORI, FuType.alu, OpType.xori, ImmType.iType),
    InstructionPattern("ori", ORI, FuType.alu, OpType.ori, ImmType.iType),
    InstructionPattern("andi", ANDI, FuType.alu, OpType.andi, ImmType.iType),
    InstructionPattern("slli", SLLI, FuType.alu, OpType.slli, ImmType.iType),
    InstructionPattern("srli", SRLI, FuType.alu, OpType.srli, ImmType.iType),
    InstructionPattern("srai", SRAI, FuType.alu, OpType.srai, ImmType.iType),

    // lsu
    InstructionPattern("lb", LB, FuType.ldu, OpType.lb, ImmType.iType),
    InstructionPattern("lh", LH, FuType.ldu, OpType.lh, ImmType.iType),
    InstructionPattern("lw", LW, FuType.ldu, OpType.lw, ImmType.iType),
    InstructionPattern("lbu", LBU, FuType.ldu, OpType.lbu, ImmType.iType),
    InstructionPattern("lhu", LHU, FuType.ldu, OpType.lhu, ImmType.iType),

    InstructionPattern("sb", SB, FuType.stu, OpType.sb, ImmType.sType),
    InstructionPattern("sh", SH, FuType.stu, OpType.sh, ImmType.sType),
    InstructionPattern("sw", SW, FuType.stu, OpType.sw, ImmType.sType),

    // brh
    InstructionPattern("beq", BEQ, FuType.brh, OpType.beq, ImmType.bType),
    InstructionPattern("bne", BNE, FuType.brh, OpType.bne, ImmType.bType),
    InstructionPattern("blt", BLT, FuType.brh, OpType.blt, ImmType.bType),
    InstructionPattern("bge", BGE, FuType.brh, OpType.bge, ImmType.bType),
    InstructionPattern("bltu", BLTU, FuType.brh, OpType.bltu, ImmType.bType),
    InstructionPattern("bgeu", BGEU, FuType.brh, OpType.bgeu, ImmType.bType),

    // jmp
    InstructionPattern("jal", JAL, FuType.jmp, OpType.jal, ImmType.jType),
    InstructionPattern("jalr", JALR, FuType.jmp, OpType.jalr, ImmType.iType),
    InstructionPattern("auipc", AUIPC, FuType.alu, OpType.addi, ImmType.uType),
    InstructionPattern("lui", LUI, FuType.alu, OpType.addi, ImmType.uType),
    
    // fence
    InstructionPattern("fence", FENCE, FuType.fence, OpType.fence, ImmType.iType),

    // mul
    InstructionPattern("mul", MUL, FuType.mul, OpType.mul, ImmType.rType),
    InstructionPattern("mulh", MULH, FuType.mul, OpType.mulh, ImmType.rType),
    InstructionPattern("mulhsu", MULHSU, FuType.mul, OpType.mulhsu, ImmType.rType),
    InstructionPattern("mulhu", MULHU, FuType.mul, OpType.mulhu, ImmType.rType),

    // div
    InstructionPattern("div", DIV, FuType.div, OpType.div, ImmType.rType),
    InstructionPattern("divu", DIVU, FuType.div, OpType.divu, ImmType.rType),
    InstructionPattern("rem", REM, FuType.div, OpType.rem, ImmType.rType),
    InstructionPattern("remu", REMU, FuType.div, OpType.remu, ImmType.rType),

    // todo ebreak
  )

  val allFields = Seq(FuTypeField, OpTypeField, ImmTypeField)

  val decodeTable = new DecodeTable(allRV32IPatterns, allFields)
  val decodeResult = decodeTable.decode(io.instr)

  io.fuType := decodeResult(FuTypeField)
  io.opType := decodeResult(OpTypeField)

  val immType = decodeResult(ImmTypeField)
  io.immValue := MuxCase(
    0.S,
    Seq(
      (immType === iType.U) -> immI32(io.instr),
      (immType === sType.U) -> immS32(io.instr),
      (immType === bType.U) -> immB32(io.instr),
      (immType === uType.U) -> immU32(io.instr),
      (immType === jType.U) -> immJ32(io.instr)
    )
  )

  io.regIndice1 := rs1(io.instr)
  io.regIndice2 := rs2(io.instr)
  io.regIndice3 := rd(io.instr)
}

object DecoderElaborate extends App {
  val firtoolOptions = Array(
    "--lowering-options=" + List(
      // make yosys happy
      // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=wrapInAtSquareBracket"
    ).reduce(_ + "," + _)
  )

  // _root_.circt.stage.ChiselStage.emitSystemVerilogFile(
  //   new Decoder()(Params.defaultParams), args, firtoolOptions)
  _root_.circt.stage.ChiselStage.emitSystemVerilogFile(
    new Decoder()(Params.defaultParams),
    args ++ Array("--target-dir", "out/npc-backend/elaborate"),
    firtoolOptions
  )
}
