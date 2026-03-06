package npc.core.isa

import chisel3._

/* 
mill npc-core.test.runMain npc.core.isa.PrintImmType
 */
object PrintImmType extends App {
  println(s"ImmType.num = ${ImmType.num}")
  println(s"ImmType.width = ${ImmType.width}")

  println("=== 测试 ImmType 枚举 ===")
  
  val opTypes = ImmType.values.toList
  opTypes.foreach { op =>
    println(s"名称: ${op.toString}, ID: ${op.id}, UInt: ${op.U}")
  }
  
  println(s"\nrType 的 UInt: ${ImmType.rType.U(8.W)}")

  println("=== 测试完成 ===")
}
