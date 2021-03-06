// Copyright 2014 eric schkufza
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file contains a listing of instructions that produce buggy behavior in x64asm

BSWAP_R32
, BSWAP_R64
, CRC32_R32_M16
, CRC32_R32_M32
, CRC32_R32_M8
, CRC32_R32_R16
, CRC32_R32_R32
, CRC32_R32_RH
, CRC32_R32_RL
, CRC32_R64_M64
, CRC32_R64_M8
, CRC32_R64_R64
, CRC32_R64_RH
, CRC32_R64_RL
, CALL_FARPTR1616
, CALL_FARPTR1632
, CALL_FARPTR1664
, CALL_M64
, CALL_R64
, CALL_REL32
, CALL_LABEL
, JA_REL32
, JA_REL32_HINT
, JA_REL8
, JA_REL8_HINT
, JAE_REL32
, JAE_REL32_HINT
, JAE_REL8
, JAE_REL8_HINT
, JB_REL32
, JB_REL32_HINT
, JB_REL8
, JB_REL8_HINT
, JBE_REL32
, JBE_REL32_HINT
, JBE_REL8
, JBE_REL8_HINT
, JC_REL32
, JC_REL32_HINT
, JC_REL8
, JC_REL8_HINT
, JE_REL32
, JE_REL32_HINT
, JE_REL8
, JE_REL8_HINT
, JECXZ_REL8
, JECXZ_REL8_HINT
, JG_REL32
, JG_REL32_HINT
, JG_REL8
, JG_REL8_HINT
, JGE_REL32
, JGE_REL32_HINT
, JGE_REL8
, JGE_REL8_HINT
, JL_REL32
, JL_REL32_HINT
, JL_REL8
, JL_REL8_HINT
, JLE_REL32
, JLE_REL32_HINT
, JLE_REL8
, JLE_REL8_HINT
, JMP_REL32
, JMP_REL8
, JNA_REL32
, JNA_REL32_HINT
, JNA_REL8
, JNA_REL8_HINT
, JNAE_REL32
, JNAE_REL32_HINT
, JNAE_REL8
, JNAE_REL8_HINT
, JNB_REL32
, JNB_REL32_HINT
, JNB_REL8
, JNB_REL8_HINT
, JNBE_REL32
, JNBE_REL32_HINT
, JNBE_REL8
, JNBE_REL8_HINT
, JNC_REL32
, JNC_REL32_HINT
, JNC_REL8
, JNC_REL8_HINT
, JNE_REL32
, JNE_REL32_HINT
, JNE_REL8
, JNE_REL8_HINT
, JNG_REL32
, JNG_REL32_HINT
, JNG_REL8
, JNG_REL8_HINT
, JNGE_REL32
, JNGE_REL32_HINT
, JNGE_REL8
, JNGE_REL8_HINT
, JNL_REL32
, JNL_REL32_HINT
, JNL_REL8
, JNL_REL8_HINT
, JNLE_REL32
, JNLE_REL32_HINT
, JNLE_REL8
, JNLE_REL8_HINT
, JNO_REL32
, JNO_REL32_HINT
, JNO_REL8
, JNO_REL8_HINT
, JNP_REL32
, JNP_REL32_HINT
, JNP_REL8
, JNP_REL8_HINT
, JNS_REL32
, JNS_REL32_HINT
, JNS_REL8
, JNS_REL8_HINT
, JNZ_REL32
, JNZ_REL32_HINT
, JNZ_REL8
, JNZ_REL8_HINT
, JO_REL32
, JO_REL32_HINT
, JO_REL8
, JO_REL8_HINT
, JP_REL8
, JP_REL8_HINT
, JPE_REL32
, JPE_REL32_HINT
, JPE_REL8
, JPE_REL8_HINT
, JPO_REL32
, JPO_REL32_HINT
, JPO_REL8
, JPO_REL8_HINT
, JRCXZ_REL8
, JRCXZ_REL8_HINT
, JS_REL32
, JS_REL32_HINT
, JS_REL8
, JS_REL8_HINT
, JZ_REL32
, JZ_REL32_HINT
, JZ_REL8
, JZ_REL8_HINT
, LOOP_REL8
, LOOPE_REL8
, LOOPNE_REL8
, MOVSX_R16_M8
, MOVSX_R16_RH
, MOVSX_R16_RL
, MOVSX_R32_M16
, MOVSX_R32_M8
, MOVSX_R32_R16
, MOVSX_R32_RH
, MOVSX_R32_RL
, MOVSX_R64_M16
, MOVSX_R64_M8
, MOVSX_R64_R16
, MOVSX_R64_RH
, MOVSX_R64_RL
, MOVSXD_R64_M32
, MOVSXD_R64_R32
, MOVZX_R16_M8
, MOVZX_R16_RH
, MOVZX_R16_RL
, MOVZX_R32_M16
, MOVZX_R32_M8
, MOVZX_R32_R16
, MOVZX_R32_RH
, MOVZX_R32_RL
, MOVZX_R64_M16
, MOVZX_R64_M8
, MOVZX_R64_R16
, MOVZX_R64_RH
, MOVZX_R64_RL
, PUSH_IMM16
, VCVTDQ2PD_YMM_M128
, VCVTDQ2PD_YMM_YMM
, VMOVAPD_M128_XMM
, VMOVAPD_M256_YMM
, VMOVAPD_XMM_M128
, VMOVAPD_XMM_XMM
, VMOVAPD_YMM_M256
, VMOVAPD_YMM_YMM
, VMOVAPS_M128_XMM
, VMOVAPS_M256_YMM
, VMOVAPS_XMM_M128
, VMOVAPS_XMM_XMM
, VMOVAPS_YMM_M256
, VMOVAPS_YMM_YMM
, VMOVD_M32_XMM
, VMOVD_R32_XMM
, VMOVD_XMM_M32
, VMOVD_XMM_R32
, VMOVDQA_M128_XMM
, VMOVDQA_M256_YMM
, VMOVDQA_XMM_M128
, VMOVDQA_XMM_XMM
, VMOVDQA_YMM_M256
, VMOVDQA_YMM_YMM
, VMOVDQU_M128_XMM
, VMOVDQU_M256_YMM
, VMOVDQU_XMM_M128
, VMOVDQU_XMM_XMM
, VMOVDQU_YMM_M256
, VMOVDQU_YMM_YMM
, VMOVHLPS_XMM_XMM_XMM
, VMOVHPD_M64_XMM
, VMOVHPD_XMM_XMM_M64
, VMOVHPS_M64_XMM
, VMOVHPS_XMM_XMM_M64
, VMOVLHPS_XMM_XMM_XMM
, VMOVLPD_M64_XMM
, VMOVLPD_XMM_XMM_M64
, VMOVLPS_M64_XMM
, VMOVLPS_XMM_XMM_M64
, VMOVMSKPD_R32_XMM
, VMOVMSKPD_R32_YMM
, VMOVMSKPD_R64_XMM
, VMOVMSKPD_R64_YMM
, VMOVMSKPS_R32_XMM
, VMOVMSKPS_R32_YMM
, VMOVMSKPS_R64_XMM
, VMOVMSKPS_R64_YMM
, VMOVNTDQ_M128_XMM
, VMOVNTDQA_XMM_M128
, VMOVNTPD_M128_XMM
, VMOVNTPD_M256_YMM
, VMOVNTPS_M128_XMM
, VMOVNTPS_M256_YMM
, VMOVQ_M64_XMM
, VMOVQ_R64_XMM
, VMOVQ_XMM_M64
, VMOVQ_XMM_R64
, VMOVQ_XMM_XMM
, VMOVSD_M64_XMM
, VMOVSD_XMM_M64
, VMOVSD_XMM_XMM_XMM
, VMOVSHDUP_XMM_M128
, VMOVSHDUP_XMM_XMM
, VMOVSHDUP_YMM_M256
, VMOVSHDUP_YMM_YMM
, VMOVSLDUP_XMM_M128
, VMOVSLDUP_XMM_XMM
, VMOVSLDUP_YMM_M256
, VMOVSLDUP_YMM_YMM
, VMOVSS_M32_XMM
, VMOVSS_XMM_M32
, VMOVSS_XMM_XMM_XMM
, VMOVUPD_M128_XMM
, VMOVUPD_M256_YMM
, VMOVUPD_XMM_M128
, VMOVUPD_XMM_XMM
, VMOVUPD_YMM_M256
, VMOVUPD_YMM_YMM
, VMOVUPS_M128_XMM
, VMOVUPS_M256_YMM
, VMOVUPS_XMM_M128
, VMOVUPS_XMM_XMM
, VMOVUPS_YMM_M256
, VMOVUPS_YMM_YMM
, XCHG_AX_R16
, XCHG_EAX_R32
, XCHG_M16_R16
, XCHG_M32_R32
, XCHG_M64_R64
, XCHG_M8_RH
, XCHG_M8_RL
, XCHG_R16_AX
, XCHG_R16_M16
, XCHG_R16_R16
, XCHG_R32_EAX
, XCHG_R32_M32
, XCHG_R32_R32
, XCHG_R64_M64
, XCHG_R64_R64
, XCHG_R64_RAX
, XCHG_RAX_R64
, XCHG_RH_M8
, XCHG_RH_RH
, XCHG_RH_RL
, XCHG_RL_M8
, XCHG_RL_RH
, XCHG_RL_RL

/** Instructions that STOKE either can't simulate or verify the behavior of */
, UD2
, MOVNTDQ_M128_XMM
, MOVNTDQ_M256_YMM
, MOVNTDQA_XMM_M128
, MOVNTI_M32_R32
, MOVNTI_M64_R64
, MOVNTPD_M128_XMM
, MOVNTPS_M128_XMM
, MOVNTQ_M64_MM
, VMOVNTDQ_M128_XMM
, VMOVNTDQA_XMM_M128
, VMOVNTDQA_YMM_M256
, VMOVNTPD_M128_XMM
, VMOVNTPD_M256_YMM
, VMOVNTPS_M128_XMM
, VMOVNTPS_M256_YMM
, EMMS
, BT_M16_IMM8
, BT_M16_R16
, BT_M32_IMM8
, BT_M32_R32
, BT_M64_IMM8
, BT_M64_R64
, BTC_M16_IMM8
, BTC_M16_R16
, BTC_M32_IMM8
, BTC_M32_R32
, BTC_M64_IMM8
, BTC_M64_R64
, BTR_M16_IMM8
, BTR_M16_R16
, BTR_M32_IMM8
, BTR_M32_R32
, BTR_M64_IMM8
, BTR_M64_R64
, BTS_M16_IMM8
, BTS_M16_R16
, BTS_M32_IMM8
, BTS_M32_R32
, BTS_M64_IMM8
, BTS_M64_R64
, XGETBV
, PAUSE
, CLFLUSH_M8
, SFENCE
, MFENCE
, LFENCE
, WAIT
, MWAIT
, LOCK
, STD
, CLD
, CPUID
, SWAPGS
, MOV_SREG_M16
, MOV_SREG_M64
, MOV_SREG_R16
, MOV_SREG_R64
, MOV_M16_SREG
, MOV_M64_SREG
, MOV_R16_SREG
, MOV_R64_SREG
, RDFSBASE_R32
, RDFSBASE_R64
, RDGSBASE_R32
, RDGSBASE_R64
, RDRAND_R16
, RDRAND_R32
, RDRAND_R64
, WRFSBASE_R32
, WRFSBASE_R64
, WRGSBASE_R32
, WRGSBASE_R64
, LAR_R16_M16
, LAR_R16_R16
, LAR_R32_M16
, LAR_R32_R32
, LAR_R64_M16
, LAR_R64_R32
, LSL_R16_M16
, LSL_R16_R16
, LSL_R32_M16
, LSL_R32_R32
, LSL_R64_M16
, LSL_R64_R32
, LSS_R16_FARPTR1616
, LSS_R32_FARPTR1632
, LSS_R64_FARPTR1664
, LDMXCSR_M32
, STMXCSR_M32
, VLDMXCSR_M32
, VSTMXCSR_M32
, LFS_R16_FARPTR1616
, LFS_R32_FARPTR1632
, LFS_R64_FARPTR1664
, LGS_R16_FARPTR1616
, LGS_R32_FARPTR1632
, LGS_R64_FARPTR1664
, INVPCID_R64_M128
, PUSH_FS
, PUSH_GS
, PUSH_IMM16
, PUSH_IMM32
, PUSH_IMM8
, PUSH_M16
, PUSH_M64
, PUSH_R16
, PUSH_R64
, PUSHF
, PUSHFQ
, POP_FS
, POP_FS_PREF66
, POP_GS
, POP_GS_PREF66
, POP_M16
, POP_M64
, POP_R16
, POP_R64
, POPF
, POPFQ
, MASKMOVDQU_XMM_XMM
, VMASKMOVDQU_XMM_XMM
, MASKMOVQ_MM_MM
, MONITOR
, LEAVE
, LEAVE_PREF66

, DIV_M16 
, DIV_M32 
, DIV_M64 
, DIV_M8 
, IDIV_M16 
, IDIV_M32 
, IDIV_M64 
, IDIV_M8 

