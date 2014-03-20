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

#ifndef STOKE_SRC_SEARCH_MOVE_H
#define STOKE_SRC_SEARCH_MOVE_H

namespace stoke {

enum class Move {
  INSTRUCTION = 0,
  OPCODE,
  OPERAND,
  RESIZE,
  LOCAL_SWAP,
  GLOBAL_SWAP,

  NUM_MOVES
};

} // namespace stoke

#endif
