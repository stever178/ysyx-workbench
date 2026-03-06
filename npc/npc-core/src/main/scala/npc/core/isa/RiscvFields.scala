package npc.core.isa

import chisel3._
import chisel3.util._

object RiscvFields {
  def rs1(instr: UInt) = instr(19,15)
  def rs2(instr: UInt) = instr(24,20)
  def rd(instr: UInt)  = instr(11,7)
}
