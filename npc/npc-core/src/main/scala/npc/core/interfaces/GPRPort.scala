package npc.core.interfaces

import chisel3._

class GPRPort extends Bundle {
  val rs1_addr = Output(UInt(5.W))
  val rs2_addr = Output(UInt(5.W))
  val rs1_data = Input(UInt(32.W))
  val rs2_data = Input(UInt(32.W))

  val rd_addr = Output(UInt(5.W))
  val rd_data = Output(UInt(32.W))
  val rd_en   = Output(Bool())

  val pc_in  = Output(UInt(32.W))
  val pc_en  = Output(Bool())
  val pc_out = Input(UInt(32.W))
}
