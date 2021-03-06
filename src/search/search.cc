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

#include <cassert>
#include <cmath>
#include <csignal>
#include <unistd.h>

#include <chrono>

#include "src/search/search.h"

using namespace cpputil;
using namespace std;
using namespace std::chrono;
using namespace x64asm;

namespace {

bool give_up_now = false;
void handler(int sig, siginfo_t* siginfo, void* context) {
  give_up_now = true;
  cout << "\nTiming Out Early!\n" << endl;
}

} // namespace

namespace stoke {

Search::Search(Transforms* transforms) : transforms_(transforms) {
	set_init(Init::EMPTY, 16);
  set_seed(0);
  set_timeout(0);
  set_beta(1.0);
  set_progress_callback(nullptr, nullptr);
  set_statistics_callback(nullptr, nullptr);
  set_statistics_interval(100000);

  static bool once = false;
  if (!once) {
    once = true;

    struct sigaction term_act;
    memset(&term_act, '\0', sizeof(term_act));
    sigfillset(&term_act.sa_mask);
    term_act.sa_sigaction = handler;
    term_act.sa_flags = SA_ONSTACK;

    sigaction(SIGINT, &term_act, 0);
  }
}

Search& Search::set_mass(Move move, size_t mass) {
  vector<Move> new_moves;
  for (auto m : moves_) {
    if (m != move) {
      new_moves.push_back(m);
    }
  }
  for (size_t i = 0; i < mass; ++i) {
    new_moves.push_back(move);
  }
  moves_ = new_moves;
  int_ = decltype(int_)(0, moves_.size() - 1);

  return *this;
}

Search::result_type Search::run(const Cfg& target, const Cfg& rewrite, CostFunction& fxn) {
  // Make sure target is correct with respect to itself
  assert(fxn(target).first);
  // Make sure target and rewrite are sound to begin with
  assert(target.is_sound());
  assert(initialize(rewrite).is_sound());

  // Progress callback and search variables
  auto current = initialize(rewrite);
  auto current_cost = fxn(current, CostFunction::max_cost - 1).second;
  auto best_yet = current;
  auto best_yet_cost = current_cost;
  auto best_correct = target;
  auto best_correct_cost = fxn(target, CostFunction::max_cost - 1).second;
  auto success = false;

	// Early corner case bailouts
	if ( current_cost == 0 ) {
		return result_type(rewrite, true);
	} else if ( moves_.empty() ) {
		return result_type(target, false);
	}

  // Statistics callback variables
  vector<Statistics> statistics((size_t) Move::NUM_MOVES);
  size_t iterations = 0;
  const auto start = chrono::steady_clock::now();

	give_up_now = false;
	for (; (iterations < timeout_) && (current_cost > 0) && !give_up_now; ++iterations) {
		if ((statistics_cb_ != nullptr) && (iterations % interval_ == 0) && iterations > 0) {
			const auto dur = duration_cast<duration<double>>(steady_clock::now() - start);
			statistics_cb_({statistics, iterations, dur}, statistics_cb_arg_);
		}

		// @todo Check cost function hasn't changed across iterations

		const auto move_type = moves_[int_(gen_)];
		statistics[(size_t) move_type].num_proposed++;

		if (!transforms_->modify(current, move_type)) {
			continue;
		}
		statistics[(size_t) move_type].num_succeeded++;

		const auto p = prob_(gen_);
		const auto max = current_cost - (log(p) / beta_);

		const auto new_res = fxn(current, max + 1);
		const auto is_correct = new_res.first;
		const auto new_cost = new_res.second;

		// @todo Check that cost function hasnt' changed within an iteration

		if (new_cost > max) {
			transforms_->undo(current, move_type);
			continue;
		}
		statistics[(size_t) move_type].num_accepted++;
		current_cost = new_cost;

		const auto new_best_yet = new_cost < best_yet_cost;
		if (new_best_yet) {
			best_yet = current;
			best_yet_cost = new_cost;
		}
		const auto new_best_correct_yet = is_correct && ((new_cost == 0) || (new_cost < best_correct_cost));
		if (new_best_correct_yet) {
			success = true;
			best_correct = current;
			best_correct_cost = new_cost;
		}

		if ((progress_cb_ != nullptr) && (new_best_yet || new_best_correct_yet)) {
			progress_cb_({current, current_cost, best_yet, best_yet_cost, best_correct,
					best_correct_cost, success}, progress_cb_arg_);
		}
	}

  return result_type(success ? best_correct : best_yet, success);
}

Cfg Search::initialize(const Cfg& rewrite) const {
	auto ret = rewrite;
	switch ( init_ ) {
		case Init::EMPTY:
			return empty_init(rewrite);
		case Init::SOURCE:
			return rewrite;
		case Init::EXTENSION:
			return extension_init(rewrite);

		default:
			assert(false);
			return rewrite;
	}
}

Cfg Search::empty_init(const Cfg& rewrite) const {
	auto ret = rewrite;

	ret.get_code().clear();
	for ( size_t i = 0, ie = max_instrs_-1; i < ie; ++i ) {
		ret.get_code().push_back({NOP});
	}
	ret.get_code().push_back({RET});

	ret.recompute();

	return ret;
}

Cfg Search::extension_init(const Cfg& rewrite) const {
	auto ret = rewrite;

	// Add user-defined transformations here ...

	// Invariant 1: ret and rewrite must agree on boundary conditions.
	assert(ret.def_ins() == rewrite.def_ins());
	assert(ret.live_outs() == rewrite.live_outs());

	// Invariant 2: ret must be in a valid state. This function isn't on
	// a critical path, so this can safely be accomplished by calling
	ret.recompute();

	return ret;
}

} // namespace stoke
