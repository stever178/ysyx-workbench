package npc.core.fu

import chisel3._
import chisel3.util._

object MULOpType extends OpTypeBase("mul") {
  val mul = addType(name = "mul")
  val mulh = addType(name = "mulh")
  val mulhsu = addType(name = "mulhsu")
  val mulhu = addType(name = "mulhu")
}
