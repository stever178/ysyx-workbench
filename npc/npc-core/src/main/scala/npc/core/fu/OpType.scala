package npc.core.fu

import chisel3._
import chisel3.util._
import scala.language.implicitConversions

case class OpTypeLit(id: Int, name: String) {
  def U: UInt = id.U
  def U(width: Width): UInt = id.U(width)
}

/*
abstract class OpTypeBase {
  private var nextId = 0

  protected def addType(name: String): OpTypeLit = {
    val t = OpTypeLit(nextId, name)
    nextId += 1
    t
  }

  lazy val num: Int = nextId
  lazy val width: Int = log2Ceil(num max 1)
}

object OpType {
  private val registry = scala.collection.mutable.ArrayBuffer[OpTypeBase]()

  def register(x: OpTypeBase): Unit = {
    registry += x
  }
  // usage: OpType.register(this)

  // IMPORTANT:
  // Every OpType unit must be listed here.
  // Otherwise OpType.width will be wrong.
  private val _init = Seq(
    ALUOpType,
    LSUOpType,
    BRUOpType,
    JumpOpType,
    FenceOpType,
    MULOpType,
    DIVOpType,
  )

  lazy val width: Int = {
    require(registry.nonEmpty, "No OpType registered!")
    registry.map(_.width).max
  }
}
*/

abstract class OpTypeBase(prefix: String) {
  protected def addType(name: String): OpTypeLit = {
    OpType.allocate(s"${prefix}_${name}")
  }
}

object OpType {
  private val _values = scala.collection.mutable.ArrayBuffer[OpTypeLit]()

  private var nextId = 0

  private[core] def allocate(name: String): OpTypeLit = {
    val t = OpTypeLit(nextId, name)
    nextId += 1
    _values += t
    t
  }

  def values: Seq[OpTypeLit] = _values.toSeq

  lazy val num: Int = values.size

  lazy val width: Int = {
    require(values.nonEmpty, "No OpType registered!")
    log2Ceil(num max 1)
  }

  // IMPORTANT:
  // Every OpType unit must be listed here.
  // Otherwise OpType.width will be wrong.
  private val _init = Seq(
    ALUOpType,
    LSUOpType,
    BRUOpType,
    JumpOpType,
    FenceOpType,
    MULOpType,
    DIVOpType,
  )
}
