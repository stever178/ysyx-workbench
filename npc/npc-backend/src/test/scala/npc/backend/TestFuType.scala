package npc.backend.fu

import chisel3._

object TestFuType extends App {
  println(s"FuType.num = ${FuType.num}")
  println(s"FuType.width = ${FuType.width}")

  println("=== 测试 FuType 枚举 ===")
  println(s"FuType.alu.ohid = ${FuType.alu.ohid}")
  
  val fuTypes = FuType.values.toList
  fuTypes.foreach { ft =>
    println(s"名称: ${ft.toString}, ID: ${ft.id}, OHID: ${ft.ohid}, UInt: ${ft.U}")
  }
  
  println(s"\nJMP 的 8 位 UInt: ${FuType.jmp.U(8.W)}")

  println("=== 测试完成 ===")
}
