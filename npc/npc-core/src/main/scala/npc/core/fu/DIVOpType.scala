package npc.core.fu

import chisel3._
import chisel3.util._

object DIVOpType extends OpTypeBase("div") {
  val div = addType(name = "div")
  val divu = addType(name = "divu")
  val rem = addType(name = "rem")
  val remu = addType(name = "remu")
}
