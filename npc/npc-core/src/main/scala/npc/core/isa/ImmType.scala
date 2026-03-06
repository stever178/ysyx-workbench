package npc.core.isa

import chisel3._
import chisel3.util._
import scala.language.implicitConversions

object ImmType extends Enumeration {
  case class IncType(i: Int, name: String) extends super.Val(i, name)

  implicit class fromValToLiteral(x: Value) {
    def U: UInt = x.asInstanceOf[IncType].id.U
    def U(width: Width): UInt = x.asInstanceOf[IncType].id.U(width)
  }

  private var initVal = 0

  private def addType(name: String): IncType = {
    val incval = IncType(initVal, name)
    initVal += 1
    incval
  }

  lazy val num = this.values.size
  lazy val width = log2Ceil(maxId + 1)

  // 0=R,1=I,2=S,3=B,4=U,5=J
  val rType = addType(name = "rType")
  val iType = addType(name = "iType")
  val sType = addType(name = "sType")
  val bType = addType(name = "bType")
  val uType = addType(name = "uType")
  val jType = addType(name = "jType")

  // I型立即数：instr[31:20]
  def immI32(instr: UInt) = Cat(Fill(20, instr(31)), instr(31, 20)).asSInt
  // S型立即数：instr[31:25] + instr[11:7]
  def immS32(instr: UInt) = Cat(Fill(20, instr(31)), instr(31, 25), instr(11, 7)).asSInt
  // B型立即数：instr[31] + instr[7] + instr[30:25] + instr[11:8] + 0
  def immB32(instr: UInt) = Cat(Fill(19, instr(31)), instr(31), instr(7), instr(30, 25), instr(11, 8), 0.U(1.W)).asSInt
  // U型立即数：instr[31:12] + 0*12
  def immU32(instr: UInt) = Cat(instr(31, 12), 0.U(12.W)).asSInt
  // J型立即数：instr[31] + instr[19:12] + instr[20] + instr[30:21] + 0
  def immJ32(instr: UInt) = Cat(Fill(11, instr(31)), instr(31), instr(19, 12), instr(20), instr(30, 21), 0.U(1.W)).asSInt
}
