package npc.core.interfaces

import chisel3._
import chisel3.util._

import npc.core.params._
import npc.core.fu._

class DecodeInfo(implicit p: CoreParameters) extends Bundle {
  val rs1 = UInt(5.W)
  val rs2 = UInt(5.W)
  val rd  = UInt(5.W)

  val fuType = UInt(FuType.width.W)
  val opType = UInt(OpType.width.W)
  val imm = SInt(p.XLEN.W)
}
