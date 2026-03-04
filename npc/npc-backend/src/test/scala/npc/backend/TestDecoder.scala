package npc.backend.decode

import chisel3._
import chisel3.experimental.BundleLiterals._
import chisel3.simulator.EphemeralSimulator._
import chisel3.simulator.scalatest.ChiselSim
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.freespec.AnyFreeSpec
import org.scalatest.matchers.must.Matchers

import utils.Params._

// class Ex1Tester extends AnyFlatSpec {
//   "Ex1 Module" should "select correct input based on Y" in {
// class Ex1Tester extends AnyFreeSpec with Matchers with ChiselSim {
//   "Ex1 Module should select correct input based on Y" in {

class DecoderTest extends AnyFreeSpec with Matchers {
  "Decoder" - {
    // inst = ff010113, rd = 2, rs1 = 2, rs2 = 16, imm = fffffff0
    "should decode addi" in {
      simulate(new Decoder()(defaultParams)) { dut =>
        dut.io.instr.poke("hff010113".U)
        dut.clock.step()

        dut.io.regIndice3.expect(2.U)
        dut.io.regIndice1.expect(2.U)
        dut.io.regIndice2.expect(16.U)
        dut.io.immValue.expect(BigInt("ffffffff0", 16).toInt)
      }
    }

    // inst = ffc10113, rd = 2, rs1 = 2, rs2 = 28, imm = fffffffc
    "should decode addi(2)" in {
      simulate(new Decoder()(defaultParams)) { dut =>
        dut.io.instr.poke("hffc10113".U)
        dut.clock.step()

        dut.io.regIndice3.expect(2.U)
        dut.io.regIndice1.expect(2.U)
        dut.io.regIndice2.expect(28.U)
        dut.io.immValue.expect(BigInt("fffffffc", 16).toInt)
      }
    }

    // inst = 0a4000ef, rd = 1, rs1 = 0, rs2 = 4, imm = 000000a4
    "should decode jal" in {
      simulate(new Decoder()(defaultParams)) { dut =>
        dut.io.instr.poke("h0a4000ef".U)
        dut.clock.step()

        dut.io.regIndice3.expect(1.U)
        dut.io.regIndice1.expect(0.U)
        dut.io.regIndice2.expect(4.U)
        dut.io.immValue.expect(BigInt("000000a4", 16).toInt)
      }
    }

    // inst = 00009117, rd = 2, rs1 = 1, rs2 = 0, imm = 00009000
    "should decode auipc" in {
      simulate(new Decoder()(defaultParams)) { dut =>
        dut.io.instr.poke("h00009117".U)
        dut.clock.step()

        dut.io.regIndice3.expect(2.U)
        dut.io.regIndice1.expect(1.U)
        dut.io.regIndice2.expect(0.U)
        dut.io.immValue.expect(BigInt("00009000", 16).toInt)
      }
    }

    // inst = 00050463, rd = 8, rs1 = 10, rs2 = 0, imm = 00000008
    "should decode beqz" in {
      simulate(new Decoder()(defaultParams)) { dut =>
        dut.io.instr.poke("h00050463".U)
        dut.clock.step()

        dut.io.regIndice3.expect(8.U)
        dut.io.regIndice1.expect(10.U)
        dut.io.regIndice2.expect(0.U)
        dut.io.immValue.expect(BigInt("00000008", 16).toInt)
      }
    }
  }
}
