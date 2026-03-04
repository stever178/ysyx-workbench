package npc.backend.decode

import chisel3._
import chisel3.util.BitPat
import chisel3.util.experimental.decode._
import freechips.rocketchip.rocket.Instructions._

import npc.backend.fu._

object FuTypeField extends DecodeField[InstructionPattern, UInt] {
  override def name: String = "FuTypeField"
  override def chiselType: UInt = UInt(FuType.num.W)

  override def genTable(pattern: InstructionPattern): BitPat = {
    val fuTypeUInt = pattern.fuType.U(FuType.num.W)
    BitPat(fuTypeUInt)
  }
}

object OpTypeField extends DecodeField[InstructionPattern, UInt] {
  override def name: String = "OpTypeField"
  override def chiselType: UInt = UInt(OpType.width.W)

  override def genTable(pattern: InstructionPattern): BitPat = {
    val opTypeUInt = pattern.opType.U(OpType.width.W)
    BitPat(opTypeUInt)
  }
}

object ImmTypeField extends DecodeField[InstructionPattern, UInt] {
  override def name: String = "ImmTypeField"
  override def chiselType: UInt = UInt(ImmType.width.W)

  override def genTable(pattern: InstructionPattern): BitPat = {
    val immTypeUInt = pattern.immType.U(ImmType.width.W)
    BitPat(immTypeUInt)
  }
}
