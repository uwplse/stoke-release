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

#include "src/cfg/cfg.h"

using namespace cpputil;
using namespace std;
using namespace stoke;
using namespace x64asm;

namespace stoke {

Cfg::loc_type Cfg::get_loc(size_t idx) const {
  assert(idx < code_.size());
  for (auto i = get_exit() - 1; i > get_entry(); --i)
    if (idx >= blocks_[i]) {
      return loc_type(i, idx - blocks_[i]);
    }

  assert(false);
  return loc_type(0, 0);
}

bool Cfg::performs_undef_read() const {
	// No need to check the entry; this will consider the exit, but it will be a nop.
  for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
    for (size_t j = 0, je = num_instrs(*i); j < je; ++j) {
      const auto idx = get_index({*i, j});
      const auto r = code_[idx].maybe_read_set();
      const auto di = def_ins_[idx];

      if ((r & di) != r) {
        return true;
      }
    }
  }
  return false;
}

void Cfg::forward_topo_sort() {
  block_sort_.clear();

	remaining_preds_.resize(num_blocks());
	for ( size_t i = get_entry(), ie = get_exit(); i <= ie; ++i ) {
		remaining_preds_[i] = 0;
		for (auto p = pred_begin(i), pe = pred_end(i); p != pe; ++p ) {
			if (is_reachable(*p)) {
				remaining_preds_[i]++;
			}
		}
	}

	block_sort_.push_back(get_entry());
  for (size_t i = 0; i < block_sort_.size(); ++i) {
    const auto next = block_sort_[i];
    for (auto s = succ_begin(next), se = succ_end(next); s != se; ++s) {
      if (--remaining_preds_[*s] == 0) {
        block_sort_.push_back(*s);
      }
    }
  }
}

void Cfg::recompute_blocks() {
  blocks_.clear();

  boundaries_.resize_for_bits(code_.size() + 1);
  boundaries_.reset();

  // We know a-priori that these are boundaries
  boundaries_[0] = true;
  boundaries_[code_.size()] = true;

  // Labels define the beginning of blocks; jumps and returns define the ends. */
  for (size_t i = 0, ie = code_.size(); i < ie; ++i) {
    const auto& instr = code_[i];
    if (instr.is_label_defn()) {
      boundaries_[i] = true;
    } else if (instr.is_jump() || instr.is_return()) {
      boundaries_[i + 1] = true;
    }
  }

  // Add sentinels for entry and exit blocks along with boundaries
  blocks_.push_back(0);
  for (auto i = boundaries_.set_bit_index_begin(), ie = boundaries_.set_bit_index_end(); i != ie; ++i) {
    blocks_.push_back(*i);
  }
  blocks_.push_back(code_.size());
}

void Cfg::recompute_labels() {
  labels_.clear();
  for (auto i = get_entry() + 1, ie = get_exit(); i < ie; ++i) {
    if (num_instrs(i) > 0) {
      const auto& instr = code_[get_index({i, 0})];
      if (instr.is_label_defn()) {
        labels_[instr.get_operand<Label>(0)] = i;
      }
    }
  }
}

void Cfg::recompute_succs() {
  succs_.resize(num_blocks());
  for (auto& s : succs_) {
    s.clear();
  }

  for (auto i = get_entry(), ie = get_exit(); i < ie; ++i) {
    // Control passes from empty blocks to the next.
    if (num_instrs(i) == 0) {
      succs_[i].push_back(i + 1);
      continue;
    }
    // Control passes from return statements to the exit.
    const auto& instr = code_[get_index({i, num_instrs(i) - 1})];
    if (instr.is_return()) {
      succs_[i].push_back(get_exit());
      continue;
    }
    // Conditional jump targets are always listed second in succs_.
    const auto itr = labels_.find(instr.get_operand<Label>(0));
    const auto dest = itr == labels_.end() ? get_exit() : itr->second;
    if (instr.is_uncond_jump()) {
      succs_[i].push_back(dest);
    } else {
      succs_[i].push_back(i + 1);
      if (instr.is_cond_jump()) {
        succs_[i].push_back(dest);
      }
    }
  }
}

void Cfg::recompute_preds() {
  preds_.resize(num_blocks());
  for (auto& p : preds_) {
    p.clear();
  }

  for (auto i = get_entry(), ie = get_exit(); i < ie; ++i) {
    for (auto s = succ_begin(i), se = succ_end(i); s != se; ++s) {
      preds_[*s].push_back(i);
    }
  }
}

void Cfg::recompute_reachable() {
  reachable_.resize_for_bits(num_blocks());
  reachable_.reset();

	work_list_.clear();

 	work_list_.push_back(get_entry());
	reachable_[get_entry()] = true;

	for ( size_t i = 0; i < work_list_.size(); ++i ) {
		const auto next = work_list_[i];
		for ( auto s = succ_begin(next), se = succ_end(next); s != se; ++s ) {
			if ( !reachable_[*s] ) {
				reachable_[*s] = true;
				work_list_.push_back(*s);
			}
		}
	}
}

void Cfg::recompute_dominators() {
  doms_.resize(num_blocks());

  // Constants
  BitVector universe(num_blocks());
  universe.set();

  // Bounary conditions
  doms_[get_entry()].resize_for_bits(num_blocks());
  doms_[get_entry()].reset();
  doms_[get_entry()][get_entry()] = true;

  // Initial conditions
  for (size_t i = get_entry()+1, ie = get_exit(); i <= ie; ++i) {
    doms_[i] = universe;
  }

  // Iterate until fixed point; always skip entry
  for (auto changed = true; changed;) {
    changed = false;

    for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
      // Meet operator; okay to read unreachable blocks which are fixed to universe
      auto new_out = universe;
      for (auto p = pred_begin(*i), pe = pred_end(*i); p != pe; ++p) {
        new_out &= doms_[*p];
      }

      // Transfer function
      new_out[*i] = true;

      // Check for fixed point
      changed |= (doms_[*i] != new_out);
      doms_[*i] = new_out;
    }
  }
}

void Cfg::recompute_back_edges() {
  loops_.clear();

	// No sense in checking the entry which can't ever be in a loop
  for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
    for (auto s = succ_begin(*i), se = succ_end(*i); s != se; ++s) {
      if (!is_exit(*s) && dom(*s, *i)) {
        loops_[edge_type(*i, *s)] = BitVector(num_blocks());
      }
    }
  }
}

void Cfg::recompute_loop_blocks() {
  for (auto& loop : loops_) {
    const auto& e = loop.first;
    auto& l = loop.second;

    l[e.first] = true;
    l[e.second] = true;

    if (e.first != e.second) {
      assert(block_stack_.empty());
      for (block_stack_.push(e.first); !block_stack_.empty();) {
        const auto m = block_stack_.top();
        block_stack_.pop();
        for (auto p = pred_begin(m), pe = pred_end(m); p != pe; ++p) {
          if (is_reachable(*p) && !l[*p]) {
            l[*p] = true;
            block_stack_.push(*p);
          }
        }
      }
    }
  }
}

void Cfg::recompute_nesting_depth() {
  nesting_depth_.assign(num_blocks(), 0);
  for (auto e = back_edge_begin(), ee = back_edge_end(); e != ee; ++e) {
    for (auto i = loop_begin(*e), ie = loop_end(*e); i != ie; ++i) {
      nesting_depth_[*i]++;
    }
  }
}

void Cfg::recompute_defs_gen_kill() {
  gen_.assign(num_blocks(), RegSet::empty());
  kill_.assign(num_blocks(), RegSet::empty());

	// No sense in checking the entry; we'll consider the exit, but it'll be a nop.
  for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
    for (auto j = instr_begin(*i), je = instr_end(*i); j != je; ++j) {
      gen_[*i] |= j->must_write_set();
      gen_[*i] -= j->maybe_undef_set();

      kill_[*i] |= j->maybe_undef_set();
      kill_[*i] -= j->maybe_write_set();
    }
  }
}
void Cfg::recompute_defs_loops() {
  recompute_defs_gen_kill();

	// Need a little extra room for def_ins_[get_exit()]
	// You'll notice that this function uses blocks_[...] instead of get_index(...)
	// This is to subvert the assertion we'd blow for trying to call get_index(get_exit(),0)

  def_ins_.resize(code_.size()+1, RegSet::empty());
  def_outs_.resize(num_blocks(), RegSet::empty());

  // Boundary conditions; MXCSR[rc] is always defined
  def_outs_[get_entry()] = fxn_def_ins_ + mxcsr_rc;

  // Initial conditions
  for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
    def_outs_[*i] = RegSet::universe();
  }

  // Iterate until fixed point
  for (auto changed = true; changed;) {
    changed = false;

    for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
      // Meet operator
      def_ins_[blocks_[*i]] = RegSet::universe();
      for (auto p = pred_begin(*i), pe = pred_end(*i); p != pe; ++p) {
        if (is_reachable(*p)) {
          def_ins_[blocks_[*i]] &= def_outs_[*p];
        }
      }
      // Transfer function
      const auto new_out = (def_ins_[blocks_[*i]] - kill_[*i]) | gen_[*i];

      // Check for fixed point
      changed |= def_outs_[*i] != new_out;
      def_outs_[*i] = new_out;
    }
  }

  // Compute dataflow values for each instruction
  for (auto i = ++reachable_begin(), ie = reachable_end(); i != ie; ++i) {
    for (size_t j = 1, je = num_instrs(*i); j < je; ++j) {
      const auto idx = blocks_[*i] + j;
      def_ins_[idx] = def_ins_[idx - 1];

      const auto& instr = code_[idx - 1];
      def_ins_[idx] |= instr.must_write_set();
      def_ins_[idx] -= instr.maybe_undef_set();
    }
  }
}

void Cfg::recompute_defs_loop_free() {
	// Need a little extra room for def_ins_[get_exit()]
	// You'll notice that this function uses blocks_[...] instead of get_index(...)
	// This is to subvert the assertion we'd blow for trying to call get_index(get_exit(),0)

  def_ins_.resize(code_.size()+1, RegSet::empty());
  def_outs_.resize(num_blocks(), RegSet::empty());

  // Boundary conditions ; MXCSR[rc] is always defined
  def_outs_[get_entry()] = fxn_def_ins_ + mxcsr_rc;

  // Iterate only once in topological order (skip entry)
	forward_topo_sort();
  for (size_t i = 1, ie = block_sort_.size(); i < ie; ++i) {
		const auto b = block_sort_[i];

    // Initial conditions
    def_ins_[blocks_[b]] = RegSet::universe();

    // Meet operator
    for (auto p = pred_begin(b), pe = pred_end(b); p != pe; ++p) {
      if (is_reachable(*p)) {
        def_ins_[blocks_[b]] &= def_outs_[*p];
      }
    }

    // Transfer function
    for (size_t j = 1, je = num_instrs(b); j < je; ++j) {
      const auto idx = blocks_[b] + j;
      def_ins_[idx] = def_ins_[idx - 1];

      const auto& instr = code_[idx - 1];
      def_ins_[idx] |= instr.must_write_set();
      def_ins_[idx] -= instr.maybe_undef_set();
    }

		// Summarize block
		if (num_instrs(b) == 0) {
			def_outs_[b] = def_ins_[blocks_[b]];
		} else {
			const auto idx = blocks_[b] + num_instrs(b) - 1;
			def_outs_[i] = def_ins_[idx];

			const auto& instr = code_[idx];
			def_outs_[b] |= instr.must_write_set();
			def_outs_[b] -= instr.maybe_undef_set();
		}
  }
}

} // namespace x64
