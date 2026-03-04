package utils

import MathUtils.IntToOH

object EnumUtils {
  class OHEnumeration() extends Enumeration {
    protected class OHVal(i: Int, name: String) extends super.Val(i, name) {
      def ohid: BigInt = IntToOH(id)
    }
  }

  // class OHEnumeration() extends Enumeration {
  //   protected class OHVal(i: Int, name: String) extends super.Val(i, name) {
  //     def ohid: BigInt = IntToOH(id)
  //   }

  //   protected def OHVal(i: Int, name: String): OHVal = {
  //     val v = new OHVal(i, name)
  //     super.Value(v) // 注册到Enumeration的values列表中
  //     v
  //   }

  //   protected def OHVal(name: String): OHVal = OHVal(nextId, name)
  //   private def nextId: Int = this.maxId + 1
  //   private def maxId: Int = if (values.isEmpty) 0 else values.map(_.id).max

  //   override def num: Int = super.num
  // }
}
