package npc.backend.fu

import chisel3._
import chisel3.util._
import scala.language.implicitConversions

import utils.EnumUtils.OHEnumeration

object FuType extends OHEnumeration {
  class OHType(i: Int, name: String) extends super.OHVal(i, name)

  final def OHType(i: Int, name: String): OHType = new OHType(i, name)

  implicit class fromOHValToLiteral(x: Value) {
    def ohid: BigInt = x.asInstanceOf[OHType].ohid
    def U: UInt = x.asInstanceOf[OHType].ohid.U
    def U(width: Width): UInt = x.asInstanceOf[OHType].ohid.U(width)
  }

  private var initVal = 0

  private def addType(name: String): OHType = {
    val ohval = OHType(initVal, name)
    initVal += 1
    ohval
  }

  lazy val num = this.values.size
  lazy val width = log2Ceil(maxId + 1)

  val undef = addType(name = "undef")

  // int
  val alu = addType(name = "alu")

  val brh = addType(name = "brh")
  val jmp = addType(name = "jmp")

  // val csr = addType(name = "csr")
  val fence = addType(name = "fence")
  val mul = addType(name = "mul")
  val div = addType(name = "div")

  // val bku = addType(name = "bku")

  // fp
  // val falu = addType(name = "falu")
  // val fmac = addType(name = "fmac")
  // val fcvt = addType(name = "fcvt")
  // val fDivSqrt = addType(name = "fDivSqrt")
  // val fcmp = addType(name = "fcmp")

  // lsu
  val ldu = addType(name = "ldu")
  val stu = addType(name = "stu")
  // val mou = addType(name = "mou")
}
