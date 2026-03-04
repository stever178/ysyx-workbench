package npc

import chisel3._
import chisel3.util._
// import freechips.rocketchip.diplomacy.{LazyModule, LazyModuleImp}
// import org.chipsalliance.diplomacy.lazymodule.{LazyModule, LazyModuleImp}

import utils.Params.defaultParams
import npc._
import npc.frontend.fetch.Fetcher
import npc.backend.decode.Decoder
import npc.backend.exu.ExeUnit
import npc.regs.RiscvRegFileWithPC

class NPCTop(implicit p: CoreParameters) extends Module {
// class NPCTop()(implicit p: CoreParameters) extends LazyModule {

  // 1. 实例化所有子模块
  val fetcher   = Module(new Fetcher())    // 前端取指模块
  val decoder   = Module(new Decoder())   // 译码模块
  val exeUnit   = Module(new ExeUnit()(defaultParams))    // 执行单元
  val regFile   = Module(new RiscvRegFileWithPC()) // 通用寄存器+PC

  // 2. 实现顶层模块的硬件逻辑
  // val module = new LazyModuleImp(this) {
    val io = IO(new Bundle {
      val clk   = Input(Clock())    // 时钟
      val rst_n = Input(Reset())    // 异步复位（低有效）
      val instr = Input(UInt(32.W))

      // val imem_addr = Output(UInt(32.W)) // 指令存储器地址
      // val imem_data = Input(UInt(32.W))  // 指令存储器数据
    })

    // ===================== 时钟/复位连接 =====================
    // 给所有子模块绑定时钟和复位
    withClockAndReset(io.clk, io.rst_n) {
      // ===================== 1. 取指模块（Fetcher）连接 =====================
      // Fetcher从RegFile获取当前PC，作为取指地址
      fetcher.io.pc := regFile.io.pc_out
      fetcher.io.instr_in := io.instr
      // Fetcher输出对接指令存储器接口
      // io.imem_addr := fetcher.io.addr
      // fetcher.io.data_in := io.imem_data

      // Fetcher输出的指令传给译码器
      decoder.io.instr := fetcher.io.instr_out

      // ===================== 2. 译码模块（Decoder）连接 =====================
      // 译码器输出对接执行单元
      exeUnit.io.instr        := fetcher.io.instr_out // 透传原始指令
      // exeUnit.io.fuType       := decoder.io.fuType
      // exeUnit.io.operandTypes := decoder.io.operandTypes
      // exeUnit.io.regIndices   := decoder.io.regIndices
      // exeUnit.io.immValue     := decoder.io.immValue
      exeUnit.io <> decoder.io

      // ===================== 3. 执行单元（ExeUnit）连接 =====================
      // // 1. ExeUnit → GPR：读寄存器请求
      // regFile.io.rs1_addr := exeUnit.io.gpr_rs1_addr
      // regFile.io.rs2_addr := exeUnit.io.gpr_rs2_addr
      // // 2. GPR → ExeUnit：读寄存器数据
      // exeUnit.io.gpr_rs1_data := regFile.io.rs1_data
      // exeUnit.io.gpr_rs2_data := regFile.io.rs2_data
      // // 3. GPR → ExeUnit：PC当前值
      // exeUnit.io.gpr_pc_out := regFile.io.pc_out
      // // 4. ExeUnit → GPR：写寄存器请求
      // regFile.io.rd_addr := exeUnit.io.gpr_rd_addr
      // regFile.io.rd_data := exeUnit.io.gpr_rd_data
      // regFile.io.rd_en   := exeUnit.io.gpr_rd_en
      // // 5. ExeUnit → GPR：PC更新请求
      // regFile.io.pc_in   := exeUnit.io.gpr_pc_in
      // regFile.io.pc_en   := exeUnit.io.gpr_pc_en
      regFile.io <> exeUnit.io

      // ===================== 5. 初始化逻辑 =====================
      // 复位时PC初始化为0x80000000（RISC-V标准复位地址）
      when(!io.rst_n.asBool) {
        regFile.io.pc_in := "h80000000".U
        regFile.io.pc_en := true.B
      }
    }
  // }
}

object TopMain extends App {
  // chisel3.Driver.execute(args, () => LazyModule(new NPCTop()(defaultParams)).module)
  // utils.Elaborate(new NPCTop())
}
