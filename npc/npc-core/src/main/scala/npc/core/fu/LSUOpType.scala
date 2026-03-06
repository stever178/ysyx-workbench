package npc.core.fu

import chisel3._
import chisel3.util._

object LSUOpType extends OpTypeBase("lsu") { 
  val lb = addType(name = "lb")
  val lh = addType(name = "lh")
  val lw = addType(name = "lw")
  val lbu = addType(name = "lbu")
  val lhu = addType(name = "lhu")

  val sb = addType(name = "sb")
  val sh = addType(name = "sh")
  val sw = addType(name = "sw")
}
