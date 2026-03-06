package npc.core.fu

import chisel3._
import chisel3.util._

object ALUOpType extends OpTypeBase("alu") {
  val add = addType("add")
  val sub = addType("sub")
  val sll = addType("sll")
  val slt = addType("slt")
  val sltu = addType("sltu")
  val xor = addType("xor")
  val srl = addType("srl")
  val sra = addType("sra")
  val or  = addType("or")
  val and = addType("and")

  val addi = addType(name = "addi")
  val slti = addType(name = "slti")
  val sltiu = addType(name = "sltiu")
  val xori = addType(name = "xori")
  val ori = addType(name = "ori")
  val andi = addType(name = "andi")
  val slli = addType(name = "slli")
  val srli = addType(name = "srli")
  val srai = addType(name = "srai")

  val lui = addType(name = "lui")
}
