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

#include <set>

#include "src/search/transforms.h"

using namespace std;
using namespace x64asm;

namespace {

set<Opcode> unsupported_ {{
#include "src/search/tables/unsupported.h"
  }
};

} // namespace

namespace stoke {

Transforms& Transforms::set_opcode_pool(const FlagSet& flags, size_t nop_percent, bool use_mem_read,
    bool use_mem_write) {
  control_free_.clear();
  for (auto i = (int)LABEL_DEFN, ie = (int)XSAVEOPT64_M64; i != ie; ++i) {
    auto op = (Opcode)i;
    if (is_control_opcode(op) || is_unsupported(op) || !is_enabled(op, flags)) {
      continue;
    } 
   
    if(!use_mem_read) {
      if (!use_mem_write) {
        //no memory allowed
        if(is_mem_opcode(op))
          continue;
      } else {
        //reads disallowed, writes allowed
        if(is_mem_opcode(op) && !is_mem_write_only_opcode(op))
          continue;
      }
    } else if (!use_mem_write) {
      //read allowed, write disallowed
      if (is_mem_opcode(op) && !is_mem_read_only_opcode(op))
        continue;
    } 
    
    control_free_.push_back(op);
  }

  control_free_or_nop_ = control_free_;
  for (size_t i = 0, ie = (nop_percent / 100) * control_free_.size(); i < ie; ++i) {
    control_free_or_nop_.push_back(NOP);
  }

  control_free_type_equiv_.clear();
  control_free_type_equiv_.resize((int)XSAVEOPT64_M64 + 1);
  for (const auto i : control_free_) {
    for (const auto j : control_free_) {
      if (is_type_equiv(i, j)) {
        control_free_type_equiv_[i].push_back(j);
      }
    }
  }

  return *this;
}

Transforms& Transforms::set_operand_pool(const Code& target, bool use_callee_save) {
  rl_pool_.clear();
  for (const auto& r : rls) {
    if (use_callee_save || !is_callee_save(r)) {
      rl_pool_.push_back(r);
    }
  }
  rh_pool_.clear();
  for (const auto& r : rhs) {
    if (use_callee_save || !is_callee_save(r)) {
      rh_pool_.push_back(r);
    }
  }
  rb_pool_.clear();
  for (const auto& r : rbs) {
    if (use_callee_save || !is_callee_save(r)) {
      rb_pool_.push_back(r);
    }
  }
  r16_pool_.clear();
  for (const auto& r : r16s) {
    if (use_callee_save || !is_callee_save(r)) {
      r16_pool_.push_back(r);
    }
  }
  r32_pool_.clear();
  for (const auto& r : r32s) {
    if (use_callee_save || !is_callee_save(r)) {
      r32_pool_.push_back(r);
    }
  }
  r64_pool_.clear();
  for (const auto& r : r64s) {
    if (use_callee_save || !is_callee_save(r)) {
      r64_pool_.push_back(r);
    }
  }
  mm_pool_.assign(mms.begin(), mms.end());
  sreg_pool_.assign(sregs.begin(), sregs.end());
  st_pool_.assign(sts.begin(), sts.end());
  xmm_pool_.assign(xmms.begin(), xmms.end());
  ymm_pool_.assign(ymms.begin(), ymms.end());

  imm_pool_.assign({
    0ul, 1ul, -1ul, 2ul, -2ul, 3ul, -3ul, 4ul, -4ul, 5ul, -5ul, 6ul, -6ul, 7ul, -7ul, 8ul, -8ul,
    16ul, -16ul, 32ul, -32ul, 64ul, -64ul, 128ul, -128ul
  });

  m_pool_.clear();
  for (const auto& instr : target) {
    if (instr.derefs_mem()) {
      const auto& ref = instr.get_operand<M8>(instr.mem_index());
      if (!use_callee_save && ref.contains_base() && is_callee_save(ref.get_base())) {
        continue;
      } else if (!use_callee_save && ref.contains_index() && is_callee_save(ref.get_index())) {
        continue;
      } else if (find(m_pool_.begin(), m_pool_.end(), ref) != m_pool_.end()) {
        continue;
      }
      m_pool_.push_back(ref);
    }
  }

  return *this;
}

bool Transforms::modify(Cfg& cfg, Move type) {
  switch (type) {
    case Move::INSTRUCTION:
      return instruction_move(cfg);
    case Move::OPCODE:
      return opcode_move(cfg);
    case Move::OPERAND:
      return operand_move(cfg);
    case Move::RESIZE:
      return resize_move(cfg);
    case Move::LOCAL_SWAP:
      return local_swap_move(cfg);
    case Move::GLOBAL_SWAP:
      return global_swap_move(cfg);
		case Move::EXTENSION:	
			return extension_move(cfg);
    default:
      assert(false);
      return false;
  }
}

bool Transforms::instruction_move(Cfg& cfg) {
  auto& code = cfg.get_code();

	if ( cfg.num_reachable() < 3 ) {
		return false;
	}
	auto bb = cfg.reachable_begin();
	for ( size_t i = 0, ie = (gen_() % (cfg.num_reachable()-2))+1; i < ie; ++i ) {
		++bb;
	}

  const auto num_instrs = cfg.num_instrs(*bb);
  if (num_instrs == 0) {
    return false;
  }
  const auto idx = gen_() % num_instrs;

  instr_index_ = cfg.get_index({*bb, idx});
  if (is_control_opcode(code[instr_index_].get_opcode())) {
    return false;
  }
  old_instr_ = code[instr_index_];

  auto& instr = code[instr_index_];
  instr.set_opcode(get_control_free_or_nop());
  const auto& rs = cfg.def_ins({*bb, idx});

  for (size_t i = 0, ie = instr.arity(); i < ie; ++i) {
    Operand o = instr.get_operand<R64>(i);
    if (instr.maybe_read(i)) {
      if (!get_read_op(instr.get_opcode(), i, rs, o)) {
        instr = old_instr_;
        return false;
      }
    } else {
      if (!get_write_op(instr.get_opcode(), i, rs, o)) {
        instr = old_instr_;
        return false;
      }
    }
    instr.set_operand(i, o);
  }
  cfg.recompute_defs();

  if (!cfg.is_sound()) {
    undo_instruction_move(cfg);
    return false;
  }
  return true;
}

bool Transforms::opcode_move(Cfg& cfg) {
  auto& code = cfg.get_code();

	if ( cfg.num_reachable() < 3 ) {
		return false;
	}
	auto bb = cfg.reachable_begin();
	for ( size_t i = 0, ie = (gen_() % (cfg.num_reachable()-2))+1; i < ie; ++i ) {
		++bb;
	}

  const auto num_instrs = cfg.num_instrs(*bb);
  if (num_instrs == 0) {
    return false;
  }
  instr_index_ = cfg.get_index({*bb, gen_() % num_instrs});

  auto& instr = code[instr_index_];
  old_opcode_ = instr.get_opcode();
  if (is_control_opcode(old_opcode_)) {
    return false;
  }

  const auto o = get_control_free_type_equiv(old_opcode_);
  instr.set_opcode(o);
  cfg.recompute_defs();

  if (!cfg.is_sound()) {
    undo_opcode_move(cfg);
    return false;
  }
  return true;
}

bool Transforms::operand_move(Cfg& cfg) {
  auto& code = cfg.get_code();

	if ( cfg.num_reachable() < 3 ) {
		return false;
	}
	auto bb = cfg.reachable_begin();
	for ( size_t i = 0, ie = (gen_() % (cfg.num_reachable()-2))+1; i < ie; ++i ) {
		++bb;
	}

  const auto num_instrs = cfg.num_instrs(*bb);
  if (num_instrs == 0) {
    return false;
  }
  const auto idx = gen_() % num_instrs;

  instr_index_ = cfg.get_index({*bb, idx});
  if (code[instr_index_].arity() == 0 || is_control_opcode(code[instr_index_].get_opcode())) {
    return false;
  }

  auto& instr = code[instr_index_];
  operand_index_ = gen_() % instr.arity();
  old_operand_ = instr.get_operand<R64>(operand_index_);
  auto o = old_operand_;

  const auto& rs = cfg.def_ins({*bb, idx});
  if (instr.maybe_read(operand_index_)) {
    if (!get_read_op(instr.get_opcode(), operand_index_, rs, o)) {
      return false;
    }
  } else {
    if (!get_write_op(instr.get_opcode(), operand_index_, rs, o)) {
      return false;
    }
  }
  instr.set_operand(operand_index_, o);
  cfg.recompute_defs();

  if (!cfg.is_sound()) {
    undo_operand_move(cfg);
    return false;
  }
  return true;
}

bool Transforms::resize_move(Cfg& cfg) {
  auto& code = cfg.get_code();
  const auto start = gen_() % code.size();

  move_i_ = 0;
  for (size_t ie = code.size(); move_i_ < ie; ++move_i_) {
    if (code[move_i_].is_nop()) {
      goto found_a_nop;
    }
  }
  return false;
found_a_nop:

  move_j_ = gen_() % code.size();
  if (move_i_ == move_j_) {
    return false;
  }

  move(code, move_i_, move_j_);
  cfg.recompute();

  return true;
}

bool Transforms::local_swap_move(Cfg& cfg) {
  auto& code = cfg.get_code();

  const auto bb = (gen_() % (cfg.num_blocks() - 2)) + 1;
  if (!cfg.is_reachable(bb)) {
    return false;
  }
  const auto num_instrs = cfg.num_instrs(bb);
  if (num_instrs < 2) {
    return false;
  }

  move_i_ = cfg.get_index({bb, gen_() % num_instrs});
  move_j_ = cfg.get_index({bb, gen_() % num_instrs});
  if (move_i_ == move_j_) {
    return false;
  }

  auto& i = code[move_i_];
  if (is_control_opcode(i.get_opcode())) {
    return false;
  }
  auto& j = code[move_j_];
  if (is_control_opcode(j.get_opcode())) {
    return false;
  }

  swap(i, j);
  cfg.recompute_defs();

  if (!cfg.is_sound()) {
    undo_local_swap_move(cfg);
    return false;
  }
  return true;
}

bool Transforms::global_swap_move(Cfg& cfg) {
  auto& code = cfg.get_code();

  move_i_ = gen_() % code.size();
  move_j_ = gen_() % code.size();
  if (move_i_ == move_j_) {
    return false;
  }

  auto& i = code[move_i_];
  if (is_control_opcode(i.get_opcode())) {
    return false;
  }
  auto& j = code[move_j_];
  if (is_control_opcode(j.get_opcode())) {
    return false;
  }

  swap(i, j);
  cfg.recompute_defs();

  if (!cfg.is_sound()) {
    undo_global_swap_move(cfg);
    return false;
  }
  return true;
}

bool Transforms::extension_move(Cfg& cfg) {
	// Add user-defined implementation here ...

	// Invariant 1a:
	// If this method returns true, it should leave this class in a state such that calling
	// undo_extension_move() will revert cfg to its original state.

	// Invariant 1b:
	// If this method returns false, it must leave cfg in its original state.

	return false;
}

void Transforms::undo(Cfg& cfg, Move type) {
  switch (type) {
    case Move::INSTRUCTION:
      undo_instruction_move(cfg);
      break;
    case Move::OPCODE:
      undo_opcode_move(cfg);
      break;
    case Move::OPERAND:
      undo_operand_move(cfg);
      break;
    case Move::RESIZE:
      undo_resize_move(cfg);
      break;
    case Move::LOCAL_SWAP:
      undo_local_swap_move(cfg);
      break;
    case Move::GLOBAL_SWAP:
      undo_global_swap_move(cfg);
      break;
			undo_extension_move(cfg);
			break;
    default:
      assert(false);
      break;
  }
}

void Transforms::undo_extension_move(Cfg& cfg) {
	// Add user-defined implementation here ...

	// Invariant: If the previous invocation of extension_move() returned true, this
	// method must return cfg to its original state. 

	return;
}

bool Transforms::is_rh_opcode(Opcode o) const {
  for (size_t i = 0, ie = arity(o); i < ie; ++i) {
    if (type(o, i) == Type::RH) {
      return true;
    }
  }
  return false;
}

bool Transforms::is_type_equiv(Opcode o1, Opcode o2) const {
  if (arity(o1) != arity(o2)) {
    return false;
  }
  for (size_t i = 0, ie = arity(o1); i < ie; ++i) {
    if (type(o1, i) != type(o2, i)) {
      return false;
    }
  }
  return true;
}

bool Transforms::is_unsupported(Opcode o) const {
  if (is_rh_opcode(o)) {
    return true;
  }
  return unsupported_.find(o) != unsupported_.end();
};

bool Transforms::get_base(const RegSet& rs, M& m) {
  if (gen_() % 2) {
    m.clear_base();
  } else {
    auto r = rax;
    if (get_r64(rs, r)) {
      m.set_base(r);
    } else {
      return false;
    }
  }
  return true;
}

bool Transforms::get_index(const RegSet& rs, M& m) {
  if (gen_() % 2) {
    m.clear_index();
  } else {
    auto r = rax;
    if (get_r64(rs, r)) {
      m.set_index(r);
      return m.get_index() != rsp;
    } else {
      return false;
    }
  }
  return true;
}

bool Transforms::get_m(const RegSet& rs, Opcode c, Operand& o) {
  if (is_lea_opcode(c)) {
    auto m = *((M8*)(&o));
    m.set_rip_offset(false);
    m.clear_seg();
    m.set_scale((Scale)(gen_() % 4));
    m.set_disp((Imm32)(imm_pool_[gen_() % imm_pool_.size()]));

    if (!get_base(rs, m)) {
      return false;
    }
    if (!get_index(rs, m)) {
      return false;
    }
    o = m;
    return true;
  } else {
    if (m_pool_.empty()) {
      return false;
    }
    const auto& m = m_pool_[gen_() % m_pool_.size()];
    if (m.contains_base() && !rs.contains(m.get_base())) {
      return false;
    }
    if (m.contains_index() && !rs.contains(m.get_index())) {
      return false;
    }
    o = m;
    return true;
  }
}

bool Transforms::get_write_op(Opcode o, size_t idx, const RegSet& rs,
                              Operand& op) {
  switch (type(o, idx)) {
    case Type::M_8:
    case Type::M_16:
    case Type::M_32:
    case Type::M_64:
    case Type::M_128:
    case Type::M_256:
    case Type::M_16_INT:
    case Type::M_32_INT:
    case Type::M_64_INT:
    case Type::M_32_FP:
    case Type::M_64_FP:
    case Type::M_80_FP:
    case Type::M_80_BCD:
    case Type::M_2_BYTE:
    case Type::M_28_BYTE:
    case Type::M_108_BYTE:
    case Type::M_512_BYTE:
    case Type::FAR_PTR_16_16:
    case Type::FAR_PTR_16_32:
    case Type::FAR_PTR_16_64: return get_m(rs, o, op);

    case Type::MM: op = mm_pool_[gen_() % mm_pool_.size()]; return true;

    case Type::MOFFS_8:
    case Type::MOFFS_16:
    case Type::MOFFS_32:
    case Type::MOFFS_64: return false;

    case Type::RL: op = rl_pool_[gen_() % rl_pool_.size()]; return true;
    case Type::RH: op = rh_pool_[gen_() % rh_pool_.size()]; return true;
    case Type::RB: op = rb_pool_[gen_() % rb_pool_.size()]; return true;
    case Type::AL: op = al; return true;
    case Type::CL: op = cl; return true;
    case Type::R_16: op = r16_pool_[gen_() % r16_pool_.size()]; return true;
    case Type::AX: op = ax; return true;
    case Type::DX: op = dx; return true;
    case Type::R_32: op = r32_pool_[gen_() % r32_pool_.size()]; return true;
    case Type::EAX: op = eax; return true;
    case Type::R_64: op = r64_pool_[gen_() % r64_pool_.size()]; return true;
    case Type::RAX: op = rax; return true;

    case Type::REL_8:
    case Type::REL_32: return false;

    case Type::SREG: op = sreg_pool_[gen_() % sreg_pool_.size()]; return true;
    case Type::FS: op = fs; return true;
    case Type::GS: op = gs; return true;

    case Type::ST: op = st_pool_[gen_() % st_pool_.size()]; return true;
    case Type::ST_0: op = st0; return true;

    case Type::XMM: op = xmm_pool_[gen_() % xmm_pool_.size()]; return true;
    case Type::XMM_0: op = xmm0; return true;

    case Type::YMM: op = ymm_pool_[gen_() % ymm_pool_.size()]; return true;

    default:
      assert(false); return false;;
  }
}

bool Transforms::get_read_op(Opcode o, size_t idx, const RegSet& rs,
                             Operand& op) {
  switch (type(o, idx)) {
    case Type::HINT: op = gen_() % 2 ? taken : not_taken; return true;

    case Type::IMM_8: get_imm(op); return true;
    case Type::IMM_16: get_imm(op); return true;
    case Type::IMM_32: get_imm(op); return true;
    case Type::IMM_64: get_imm(op); return true;
    case Type::ZERO: op = zero; return true;
    case Type::ONE: op = one; return true;
    case Type::THREE: op = three; return true;

    case Type::LABEL: return false;

    case Type::M_8:
    case Type::M_16:
    case Type::M_32:
    case Type::M_64:
    case Type::M_128:
    case Type::M_256:
    case Type::M_16_INT:
    case Type::M_32_INT:
    case Type::M_64_INT:
    case Type::M_32_FP:
    case Type::M_64_FP:
    case Type::M_80_FP:
    case Type::M_80_BCD:
    case Type::M_2_BYTE:
    case Type::M_28_BYTE:
    case Type::M_108_BYTE:
    case Type::M_512_BYTE:
    case Type::FAR_PTR_16_16:
    case Type::FAR_PTR_16_32:
    case Type::FAR_PTR_16_64: return get_m(rs, o, op);

    case Type::MM: return get_mm(rs, op);

    case Type::PREF_66: op = pref_66; return true;
    case Type::PREF_REX_W: op = pref_rex_w; return true;
    case Type::FAR: op = far; return true;

    case Type::MOFFS_8:
    case Type::MOFFS_16:
    case Type::MOFFS_32:
    case Type::MOFFS_64: return false;

    case Type::RL: return get_rl(rs, op);
    case Type::RH: return get_rh(rs, op);
    case Type::RB: return get_rb(rs, op);
    case Type::AL: op = al; return rs.contains(al);
    case Type::CL: op = cl; return rs.contains(cl);
    case Type::R_16: return get_r16(rs, op);
    case Type::AX: op = ax; return rs.contains(ax);
    case Type::DX: op = dx; return rs.contains(dx);
    case Type::R_32: return get_r32(rs, op);
    case Type::EAX: op = eax; return rs.contains(eax);
    case Type::R_64: return get_r64(rs, op);
    case Type::RAX: op = rax; return rs.contains(rax);

    case Type::REL_8:
    case Type::REL_32: return false;

    case Type::SREG: return get_sreg(rs, op);
    case Type::FS: op = fs; return rs.contains(fs);
    case Type::GS: op = gs; return rs.contains(gs);

    case Type::ST: return get_st(rs, op);
    case Type::ST_0: op = st0; return rs.contains(st0);

    case Type::XMM: return get_xmm(rs, op);
    case Type::XMM_0: op = xmm0; return rs.contains(xmm0);

    case Type::YMM: return get_ymm(rs, op);

    default:
      assert(false); return false;;
  }
}

void Transforms::move(Code& code, size_t i, size_t j) const {
  const auto temp = code[i];
  if (i < j) {
    for (size_t k = i; k < j; ++k) {
      code[k] = code[k + 1];
    }
  } else {
    for (int k = i; k > (int)j; --k) {
      code[k] = code[k - 1];
    }
  }
  code[j] = temp;
}

} // namespace stoke
