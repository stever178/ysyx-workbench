package utils

import chisel3._
import utils.EnumUtils.OHEnumeration

// mill utils.test.runMain utils.TestFuType
object FuType extends OHEnumeration {
  case class OHType(i: Int, name: String) extends super.OHVal(i, name)

  // def OHType(i: Int, name: String): OHType = new OHType(i, name)

  implicit class fromOHValToLiteral(x: OHType) {
    def U: UInt = x.ohid.U
    def U(width: Width): UInt = x.ohid.U(width)
  }

  private var initVal = 0

  private def addType(name: String): OHType = {
    val ohval = OHType(initVal, name)
    initVal += 1
    ohval
  }

  lazy val num = this.values.size

  val undef = addType(name = "undef")

  // int
  val jmp = addType(name = "jmp")
  val brh = addType(name = "brh")
  val i2f = addType(name = "i2f")
  val i2v = addType(name = "i2v")
  val f2v = addType(name = "f2v")
  val csr = addType(name = "csr")
  val alu = addType(name = "alu")
  val mul = addType(name = "mul")
  val div = addType(name = "div")
  val fence = addType(name = "fence")
  val bku = addType(name = "bku")

  // fp
  // val falu = addType(name = "falu")
  // val fmac = addType(name = "fmac")
  // val fcvt = addType(name = "fcvt")
  // val fDivSqrt = addType(name = "fDivSqrt")
  // val fcmp = addType(name = "fcmp")

  // ldu / stu
  val ldu = addType(name = "ldu")
  val stu = addType(name = "stu")
  // val mou = addType(name = "mou")
}

object TestFuType extends App {
  println(s"FuType.num = ${FuType.num}")
  println(s"FuType.alu.ohid = ${FuType.alu.ohid}")

  // 测试 FuType 的基本功能
  println("=== 测试 FuType 枚举 ===")
  
  // 打印所有 FuType 成员及其属性
  val fuTypes = List(
    FuType.jmp, FuType.brh, FuType.i2f, FuType.i2v, FuType.f2v,
    FuType.csr, FuType.alu, FuType.mul, FuType.div, FuType.fence, FuType.bku
  )
  
  fuTypes.foreach { ft =>
    println(s"名称: ${ft.toString}, ID: ${ft.id}, OHID: ${ft.ohid}, UInt: ${ft.U}")
  }
  
  // 测试隐式转换的宽度指定
  println(s"\nJMP 的 8 位 UInt: ${FuType.jmp.U(8.W)}")
  println("=== 测试完成 ===")
}
