set(SRCS	src/core.cpp
			src/memory.cpp
			src/boot/apploader.cpp
			src/boot/bootrom.cpp
			src/dvd/dol.cpp
			src/dvd/elf.cpp
			src/dvd/gcm.cpp
#			src/dvd/gcm_dump.cpp # TODO: Needs build fixing
			src/dvd/loader.cpp
			src/dvd/realdvd.cpp
			src/hle/hle_audio.cpp
			src/hle/hle.cpp
			src/hle/hle_dsp.cpp
			src/hle/hle_general.cpp
			src/hle/hle_math.cpp
			src/hw/hw_ai.cpp
			src/hw/hw_cp.cpp
			src/hw/hw.cpp
			src/hw/hw_di.cpp
			src/hw/hw_dsp.cpp
			src/hw/hw_exi.cpp
			src/hw/hw_exi_memorycard.cpp
			src/hw/hw_gx.cpp
			src/hw/hw_mi.cpp
			src/hw/hw_pe.cpp
			src/hw/hw_pi.cpp
			src/hw/hw_si.cpp
			src/hw/hw_vi.cpp
#			src/hw/plugins/plugins.cpp # TODO: Remove?
			src/powerpc/cpu_core.cpp
			src/powerpc/cpu_core_regs.cpp
			src/powerpc/disassembler/ppc_disasm.cpp
			src/powerpc/interpreter/cpu_int.cpp
			src/powerpc/interpreter/cpu_int_opcodes.cpp
#			src/powerpc/recompiler/cpu_rec_assembler.cpp
#			src/powerpc/recompiler/cpu_rec_assembler_fpu.cpp
#			src/powerpc/recompiler/cpu_rec_assembler_jumps.cpp
#			src/powerpc/recompiler/cpu_rec_assembler_loadstore.cpp
#			src/powerpc/recompiler/cpu_rec_il_compiler.cpp
#			src/powerpc/recompiler/cpu_rec_il_registers_fpu.cpp
#			src/powerpc/recompiler/cpu_rec_il_registers_x86.cpp
#			src/powerpc/recompiler/cpu_rec_il_regs.cpp
#			src/powerpc/recompiler/cpu_rec_int.cpp
#			src/powerpc/recompiler/cpu_rec_memory.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_branch.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_compare.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_loadstore.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_loadstore_fpu.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_math.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_math_fpu.cpp
#			src/powerpc/recompiler/cpu_rec_opcodes_ps.cpp
#			src/powerpc/recompiler/cpu_rec_regcache.cpp
			src/video/gx_fifo.cpp
			src/video/gx_states.cpp
			src/video/gx_tev.cpp
			src/video/gx_texture.cpp
			src/video/gx_transform.cpp
			src/video/gx_vertex.cpp
			src/video/opengl.cpp)

add_library(core STATIC ${SRCS})
