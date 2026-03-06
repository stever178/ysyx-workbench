package npc.frontend.fetch

import chisel3._
import chisel3.util._

import utils._
import npc.core._

class Fetcher extends Module {
// class Fetcher(implicit p: CoreParameters) extends LazyModule {
  // val module = new LazyModuleImp(this) {
    val io = IO(new Bundle {
      val pc = Input(UInt(32.W))

      // val addr = Output(UInt(32.W))
      // val data_in = Input(UInt(32.W))

      val instr_in = Input(UInt(32.W))
      val instr_out = Output(UInt(32.W))
    })

    io.instr_out := io.instr_in
  // }
}

// object FetcherMain extends App {
//   chisel3.Driver.execute(args, () => LazyModule(new Fetcher(defaultParams)).module)
// }
object FetcherMain extends App {
  import npc.Params.defaultParams
  // utils.Elaborate(new Fetcher())
}
