package npc.regs

import chisel3._
import chisel3.util._

/**
 * RISC-V 通用寄存器组 + PC 封装模块
 * 特性：
 * 1. 32 个 32 位通用寄存器（GPR），x0 恒为 0
 * 2. 32 位程序计数器（PC），支持同步更新
 * 3. 双读端口（符合 RISC-V 指令解码需求）、单写端口
 */
class RiscvRegFileWithPC extends Module {
  // 定义模块 IO 接口
  val io = IO(new Bundle {
    // 寄存器读端口（两个独立读端口，支持同时读两个操作数）
    val rs1_addr = Input(UInt(5.W))  // 读寄存器1地址（5位，0-31）
    val rs2_addr = Input(UInt(5.W))  // 读寄存器2地址（5位，0-31）
    val rs1_data = Output(UInt(32.W)) // 读寄存器1数据输出
    val rs2_data = Output(UInt(32.W)) // 读寄存器2数据输出

    // 寄存器写端口
    val rd_addr  = Input(UInt(5.W))   // 写寄存器地址（5位，0-31）
    val rd_data  = Input(UInt(32.W))  // 写寄存器数据
    val rd_en    = Input(Bool())      // 写使能信号（高有效）

    // PC 接口
    val pc_in    = Input(UInt(32.W))  // PC 输入（用于跳转/分支更新）
    val pc_en    = Input(Bool())      // PC 更新使能
    val pc_out   = Output(UInt(32.W)) // PC 当前值输出
  })

  // 1. 声明 32 个通用寄存器（GPR），初始化全 0
  val gpr = RegInit(VecInit(Seq.fill(32)(0.U(32.W))))

  // 2. 通用寄存器写逻辑：x0 禁止写入
  when(io.rd_en && (io.rd_addr =/= 0.U)) {
    gpr(io.rd_addr) := io.rd_data
  }

  // 3. 通用寄存器读逻辑：x0 恒输出 0
  io.rs1_data := Mux(io.rs1_addr === 0.U, 0.U, gpr(io.rs1_addr))
  io.rs2_data := Mux(io.rs2_addr === 0.U, 0.U, gpr(io.rs2_addr))

  // 4. 声明 PC 寄存器，初始化 0（实际可根据需求设为复位地址，如 0x80000000.U）
  val pc = RegInit(0.U(32.W))

  // 5. PC 更新逻辑：使能时更新为输入值
  when(io.pc_en) {
    pc := io.pc_in
  }

  // 6. PC 输出连接
  io.pc_out := pc
}

// 生成 Verilog 代码的主对象
  // Chisel 7.0 生成 Verilog 的标准方式
object RiscvRegFileWithPCMain extends App {
  // chisel3.Driver.execute(args, () => new RiscvRegFileWithPC)
}
