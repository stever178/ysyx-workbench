package npc.backend.fu

import chisel3._

object TestOpType extends App {
  println(s"OpType.num = ${OpType.num}")
  println(s"OpType.width = ${OpType.width}")

  println("=== 测试 OpType 枚举 ===")
  
  val opTypes = OpType.values.toList
  opTypes.foreach { op =>
    println(s"名称: ${op.toString}, ID: ${op.id}, UInt: ${op.U}")
  }
  
  println(s"\nadd 的 UInt: ${OpType.ALUOpType.add.U(8.W)}")

  println("=== 测试完成 ===")
}
