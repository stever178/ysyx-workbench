package npc.backend.decode

import chisel3._
import chisel3.util._
import chisel3.util.experimental.decode._
import freechips.rocketchip.rocket.Instructions._

import utils._
import npc._
import npc.core.interfaces._
import npc.core.fu._
import npc.core.isa._
import npc.core.isa.ImmType._
import npc.core.isa.RiscvFields._
import npc.core.params._

class Decoder(implicit p: CoreParameters) extends Module {
  val io = IO(new Bundle {
    val instr = Input(UInt(32.W))
    val out   = Output(new DecodeInfo)
  })

  val allRV32IPatterns = Seq(
    // ALU
    InstructionPattern("add", ADD, FuType.alu, ALUOpType.add, ImmType.rType),
    InstructionPattern("sub", SUB, FuType.alu, ALUOpType.sub, ImmType.rType),
    InstructionPattern("sll", SLL, FuType.alu, ALUOpType.sll, ImmType.rType),
    InstructionPattern("slt", SLT, FuType.alu, ALUOpType.slt, ImmType.rType),
    InstructionPattern("sltu", SLTU, FuType.alu, ALUOpType.sltu, ImmType.rType),
    InstructionPattern("xor", XOR, FuType.alu, ALUOpType.xor, ImmType.rType),
    InstructionPattern("srl", SRL, FuType.alu, ALUOpType.srl, ImmType.rType),
    InstructionPattern("sra", SRA, FuType.alu, ALUOpType.sra, ImmType.rType),
    InstructionPattern("or", OR, FuType.alu, ALUOpType.or, ImmType.rType),
    InstructionPattern("and", AND, FuType.alu, ALUOpType.and, ImmType.rType),

    InstructionPattern("addi", ADDI, FuType.alu, ALUOpType.addi, ImmType.iType),
    InstructionPattern("slti", SLTI, FuType.alu, ALUOpType.slti, ImmType.iType),
    InstructionPattern("sltiu", SLTIU, FuType.alu, ALUOpType.sltiu, ImmType.iType),
    InstructionPattern("xori", XORI, FuType.alu, ALUOpType.xori, ImmType.iType),
    InstructionPattern("ori", ORI, FuType.alu, ALUOpType.ori, ImmType.iType),
    InstructionPattern("andi", ANDI, FuType.alu, ALUOpType.andi, ImmType.iType),
    InstructionPattern("slli", SLLI, FuType.alu, ALUOpType.slli, ImmType.iType),
    InstructionPattern("srli", SRLI, FuType.alu, ALUOpType.srli, ImmType.iType),
    InstructionPattern("srai", SRAI, FuType.alu, ALUOpType.srai, ImmType.iType),

    InstructionPattern("lui", LUI, FuType.alu, ALUOpType.lui, ImmType.uType),

    // LSU
    InstructionPattern("lb", LB, FuType.ldu, LSUOpType.lb, ImmType.iType),
    InstructionPattern("lh", LH, FuType.ldu, LSUOpType.lh, ImmType.iType),
    InstructionPattern("lw", LW, FuType.ldu, LSUOpType.lw, ImmType.iType),
    InstructionPattern("lbu", LBU, FuType.ldu, LSUOpType.lbu, ImmType.iType),
    InstructionPattern("lhu", LHU, FuType.ldu, LSUOpType.lhu, ImmType.iType),

    InstructionPattern("sb", SB, FuType.stu, LSUOpType.sb, ImmType.sType),
    InstructionPattern("sh", SH, FuType.stu, LSUOpType.sh, ImmType.sType),
    InstructionPattern("sw", SW, FuType.stu, LSUOpType.sw, ImmType.sType),

    // BRU
    InstructionPattern("beq", BEQ, FuType.brh, BRUOpType.beq, ImmType.bType),
    InstructionPattern("bne", BNE, FuType.brh, BRUOpType.bne, ImmType.bType),
    InstructionPattern("blt", BLT, FuType.brh, BRUOpType.blt, ImmType.bType),
    InstructionPattern("bge", BGE, FuType.brh, BRUOpType.bge, ImmType.bType),
    InstructionPattern("bltu", BLTU, FuType.brh, BRUOpType.bltu, ImmType.bType),
    InstructionPattern("bgeu", BGEU, FuType.brh, BRUOpType.bgeu, ImmType.bType),

    // Jump
    InstructionPattern("jal", JAL, FuType.jmp, JumpOpType.jal, ImmType.jType),
    InstructionPattern("jalr", JALR, FuType.jmp, JumpOpType.jalr, ImmType.iType),
    InstructionPattern("auipc", AUIPC, FuType.jmp, JumpOpType.auipc, ImmType.uType),
    
    // Fence
    InstructionPattern("fence", FENCE, FuType.fence, FenceOpType.fence, ImmType.iType),

    // MUL
    InstructionPattern("mul", MUL, FuType.mul, MULOpType.mul, ImmType.rType),
    InstructionPattern("mulh", MULH, FuType.mul, MULOpType.mulh, ImmType.rType),
    InstructionPattern("mulhsu", MULHSU, FuType.mul, MULOpType.mulhsu, ImmType.rType),
    InstructionPattern("mulhu", MULHU, FuType.mul, MULOpType.mulhu, ImmType.rType),

    // DIV
    InstructionPattern("div", DIV, FuType.div, DIVOpType.div, ImmType.rType),
    InstructionPattern("divu", DIVU, FuType.div, DIVOpType.divu, ImmType.rType),
    InstructionPattern("rem", REM, FuType.div, DIVOpType.rem, ImmType.rType),
    InstructionPattern("remu", REMU, FuType.div, DIVOpType.remu, ImmType.rType),

    // todo ebreak
  )

  val allFields = Seq(FuTypeField, OpTypeField, ImmTypeField)

  val decodeTable = new DecodeTable(allRV32IPatterns, allFields)
  val decodeResult = decodeTable.decode(io.instr)

  io.out.fuType := decodeResult(FuTypeField)
  io.out.opType := decodeResult(OpTypeField)

  val immType = decodeResult(ImmTypeField)
  io.out.imm := MuxCase(
    0.S,
    Seq(
      (immType === iType.U) -> immI32(io.instr),
      (immType === sType.U) -> immS32(io.instr),
      (immType === bType.U) -> immB32(io.instr),
      (immType === uType.U) -> immU32(io.instr),
      (immType === jType.U) -> immJ32(io.instr)
    )
  )

  io.out.rs1 := rs1(io.instr)
  io.out.rs2 := rs2(io.instr)
  io.out.rd := rd(io.instr)
}

// mill npc-backend.runMain npc.backend.decode.DecoderElaborate
object DecoderElaborate extends App {
  val firtoolOptions = Array(
    "--lowering-options=" + List(
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=wrapInAtSquareBracket"
    ).reduce(_ + "," + _)
  )

  _root_.circt.stage.ChiselStage.emitSystemVerilogFile(
    new Decoder()(Params.defaultParams),
    args ++ Array("--target-dir", "out/npc-backend/elaborate"),
    firtoolOptions
  )
}
