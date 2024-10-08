/*
 * Copyright 2015-2021 Arm Limited
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * At your option, you may choose to accept this material under either:
 *  1. The Apache License, Version 2.0, found at <http://www.apache.org/licenses/LICENSE-2.0>, or
 *  2. The MIT License, found at <http://opensource.org/licenses/MIT>.
 */

#ifndef SPIRV_CROSS_SPIRV_HPP
#define SPIRV_CROSS_SPIRV_HPP

#include "spirv_cross.hpp"
#include <vector>
#include <map>
#include <set>
#include <stddef.h>

namespace SPIRV_CROSS_NAMESPACE
{
class CompilerSPIRV : public Compiler
{
public:
	explicit CompilerSPIRV(std::vector<uint32_t> spirv);
	CompilerSPIRV(const uint32_t *ir, size_t word_count);
	explicit CompilerSPIRV(const ParsedIR &ir);
	explicit CompilerSPIRV(ParsedIR &&ir);

	std::string compile() override;

private:
	std::vector<uint32_t> buffer;

	void emit_header();
	void emit_capabilities();
	void emit_extensions();
	void emit_ext_inst_imports();
	void emit_exec_info();

	void emit_opcode(spv::Op opcode, uint32_t arglength = 0);
	void emit_id(uint32_t id);
	void emit_string(std::string str);
};
} // namespace SPIRV_CROSS_NAMESPACE

#endif