package npc.core.params

import chisel3._
import chisel3.util._

// case class CoreParameters
// (
//   HasPrefetch: Boolean = false,
//   HartId: Int = 0,
//   XLEN: Int = 64,
//   VLEN: Int = 128,
//   ELEN: Int = 64,
//   AddrBits: Int = 64,
// ) {
//   def ISABase = "rv32i"
// }

case class CoreParameters(
  XLEN: Int = 32
)

object Params {
  implicit val defaultParams = CoreParameters(XLEN = 32)
}
