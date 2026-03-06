package npc.core.fu

import chisel3._
import chisel3.util._

object JumpOpType extends OpTypeBase("jump") {
  val jal = addType(name = "jal")
  val jalr = addType(name = "jalr")
  val auipc = addType(name = "auipc")
}
