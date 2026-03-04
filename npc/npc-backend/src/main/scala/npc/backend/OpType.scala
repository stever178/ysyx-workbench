package npc.backend.fu

import chisel3._
import chisel3.util._
import scala.language.implicitConversions

object OpType extends Enumeration {
  def rs1(instr: UInt) = instr(19, 15)
  def rs2(instr: UInt) = instr(24, 20)
  def rd(instr: UInt)  = instr(11, 7)

  class IncType(i: Int, name: String) extends super.Val(i, name)

  final def IncType(i: Int, name: String): IncType = new IncType(i, name)

  implicit class fromValToLiteral(x: Value) {
    def U: UInt = x.asInstanceOf[IncType].id.U
    def U(width: Width): UInt = x.asInstanceOf[IncType].id.U(width)
  }

  private var initVal = 0

  private def addType(name: String): IncType = {
    val incval = IncType(initVal, name)
    initVal += 1
    incval
  }

  lazy val num = this.values.size
  lazy val width = log2Ceil(maxId + 1)

  val undef = addType(name = "undef")

  // ALU
  val add = addType(name = "add")
  val sub = addType(name = "sub")
  val sll = addType(name = "sll")
  val slt = addType(name = "slt")
  val sltu = addType(name = "sltu")
  val xor = addType(name = "xor")
  val srl = addType(name = "srl")
  val sra = addType(name = "sra")
  val or = addType(name = "or")
  val and = addType(name = "and")

  val addi = addType(name = "addi")
  val slti = addType(name = "slti")
  val sltiu = addType(name = "sltiu")
  val xori = addType(name = "xori")
  val ori = addType(name = "ori")
  val andi = addType(name = "andi")
  val slli = addType(name = "slli")
  val srli = addType(name = "srli")
  val srai = addType(name = "srai")

  object ALUOpType {
    val add = OpType.add
    val sub = OpType.sub
    val sll = OpType.sll
    val slt = OpType.slt
    val sltu = OpType.sltu
    val xor = OpType.xor
    val srl = OpType.srl
    val sra = OpType.sra
    val or = OpType.or
    val and = OpType.and

    val addi = OpType.addi
    val slti = OpType.slti
    val sltiu = OpType.sltiu
    val xori = OpType.xori
    val ori = OpType.ori
    val andi = OpType.andi
    val slli = OpType.slli
    val srli = OpType.srli
    val srai = OpType.srai
  }

  // LSU
  val lb = addType(name = "lb")
  val lh = addType(name = "lh")
  val lw = addType(name = "lw")
  val lbu = addType(name = "lbu")
  val lhu = addType(name = "lhu")

  val sb = addType(name = "sb")
  val sh = addType(name = "sh")
  val sw = addType(name = "sw")

  object LSUOpType {
    val lb = OpType.lb
    val lh = OpType.lh
    val lw = OpType.lw
    val lbu = OpType.lbu
    val lhu = OpType.lhu

    val sb = OpType.sb
    val sh = OpType.sh
    val sw = OpType.sw
  }

  // Branch
  val beq = addType(name = "beq")
  val bne = addType(name = "bne")
  val blt = addType(name = "blt")
  val bge = addType(name = "bge")
  val bltu = addType(name = "bltu")
  val bgeu = addType(name = "bgeu")

  object BranchOpType {
    val beq = OpType.beq
    val bne = OpType.bne
    val blt = OpType.blt
    val bge = OpType.bge
    val bltu = OpType.bltu
    val bgeu = OpType.bgeu
  }

  // Jump
  val jal = addType(name = "jal")
  val jalr = addType(name = "jalr")
  val auipc = addType(name = "auipc")

  object JumpOpType {
    val jal = OpType.jal
    val jalr = OpType.jalr
    val auipc = OpType.auipc
  }

  // Fence
  val fence = addType(name = "fence")
  val fencei = addType(name = "fencei")

  object FenceOpType {
    val fence = OpType.fence
    val fencei = OpType.fencei
  }

  // MUL
  val mul = addType(name = "mul")
  val mulh = addType(name = "mulh")
  val mulhsu = addType(name = "mulhsu")
  val mulhu = addType(name = "mulhud")

  object MULOpType {
    val mul = OpType.mul
    val mulh = OpType.mulh
    val mulhsu = OpType.mulhsu
    val mulhu = OpType.mulhu
  }

  // DIV
  val div = addType(name = "div")
  val divu = addType(name = "divu")
  val rem = addType(name = "rem")
  val remu = addType(name = "remu")

  object DIVOpType {
    val div = OpType.div
    val divu = OpType.divu
    val rem = OpType.rem
    val remu = OpType.remu
  }
}
