package npc.core.fu

import chisel3._
import chisel3.util._

object FenceOpType extends OpTypeBase("fence") {
  val fence = addType(name = "fence")
  val fencei = addType(name = "fencei")
}
