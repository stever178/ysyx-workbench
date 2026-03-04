package npc.backend.decode

import chisel3._
import chisel3.util.BitPat
import chisel3.util.experimental.decode._

import npc.backend.fu._

case class InstructionPattern(
  name: String,
  bitPat: BitPat, 
  fuType: FuType.OHType, 
  opType: OpType.IncType,
  immType: ImmType.IncType,
) extends DecodePattern {

}
