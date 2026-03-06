package npc.core.fu

import chisel3._
import chisel3.util._

object BRUOpType extends OpTypeBase("bru") {
  val beq = addType(name = "beq")
  val bne = addType(name = "bne")
  val blt = addType(name = "blt")
  val bge = addType(name = "bge")
  val bltu = addType(name = "bltu")
  val bgeu = addType(name = "bgeu")
}
