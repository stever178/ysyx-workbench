package utils

import chisel3._
import npc.CoreParameters

object Elaborate extends App {
  val firtoolOptions = Array(
    "--lowering-options=" + List(
      // make yosys happy
      // see https://github.com/llvm/circt/blob/main/docs/VerilogGeneration.md
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=wrapInAtSquareBracket"
    ).reduce(_ + "," + _)
  )
  // circt.stage.ChiselStage.emitSystemVerilogFile(new gcd.GCD(), args, firtoolOptions)
}

// object Elaborate {
//   // 生成普通Module的Verilog
//   def apply[T <: Module](module: => T): Unit = {
//     chisel3.Driver.execute(Array(), () => module)
//   }
  
//   // 生成LazyModule的Verilog
//   def apply[T <: LazyModule](lazyModule: => T): Unit = {
//     chisel3.Driver.execute(Array(), () => LazyModule(lazyModule).module)
//   }
// }
