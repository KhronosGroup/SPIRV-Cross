/*
 * Copyright 2016 ARM Limited
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

#include "spirv_cfg.hpp"
#include <algorithm>

using namespace std;

namespace spirv_cross
{
CFG::CFG(Compiler &compiler_, const SPIRFunction &func_)
    : compiler(compiler_), func(func_)
{
	preceding_edges.resize(compiler.get_current_id_bound());
	succeeding_edges.resize(compiler.get_current_id_bound());
	visit_order.resize(compiler.get_current_id_bound());
	immediate_dominators.resize(compiler.get_current_id_bound());

	// Set up all edges between blocks.
	for (auto block_id : func.blocks)
	{
		auto &block = compiler.get<SPIRBlock>(block_id);

		switch (block.terminator)
		{
		case SPIRBlock::Direct:
			add_branch(block.self, block.next_block);
			break;

		case SPIRBlock::Select:
			add_branch(block.self, block.true_block);
			add_branch(block.self, block.false_block);
			break;

		case SPIRBlock::MultiSelect:
			for (auto &target : block.cases)
				add_branch(block.self, target.block);
			if (block.default_block)
				add_branch(block.self, block.default_block);
			break;

		default:
			break;
		}
	}

	build_post_order_visit_order();
	build_immediate_dominators();
}

uint32_t CFG::find_common_dominator(uint32_t a, uint32_t b) const
{
	while (a != b)
	{
		if (visit_order[a] < visit_order[b])
			a = immediate_dominators[a];
		else
			b = immediate_dominators[b];
	}
	return a;
}

uint32_t CFG::update_common_dominator(uint32_t a, uint32_t b)
{
	auto dominator = find_common_dominator(immediate_dominators[a], immediate_dominators[b]);
	immediate_dominators[a] = dominator;
	immediate_dominators[b] = dominator;
	return dominator;
}

void CFG::build_immediate_dominators()
{
	// Traverse the post-order in reverse and build up the immediate dominator tree.
	fill(begin(immediate_dominators), end(immediate_dominators), 0);
	immediate_dominators[func.entry_block] = func.entry_block;

	for (auto i = post_order.size(); i; i--)
	{
		uint32_t block = post_order[i - 1];
		auto &pred = preceding_edges[block];
		if (pred.empty()) // This is for the entry block, but we've already set up the dominators.
			continue;

		for (auto &edge : pred)
		{
			if (!immediate_dominators[block])
				immediate_dominators[block] = edge;
			else if (immediate_dominators[block] && immediate_dominators[edge])
				immediate_dominators[block] = update_common_dominator(block, edge);
		}
	}
}

void CFG::post_order_visit(uint32_t block)
{
	// If we have already branched to this block (back edge), stop recursion.
	if (visit_order[block] >= 0)
		return;

	// Block back-edges from recursively revisiting ourselves.
	visit_order[block] = 0;

	// First visit our branch targets.
	auto &succeeding = succeeding_edges[block];
	for (auto &id : succeeding)
		post_order_visit(id);
	// Then visit ourselves.
	visit_order[block] = visit_count++;
	post_order.push_back(block);
}

void CFG::build_post_order_visit_order()
{
	uint32_t block = func.entry_block;
	visit_count = 0;
	fill(begin(visit_order), end(visit_order), -1);
	post_order.clear();
	post_order_visit(block);
}

void CFG::add_branch(uint32_t from, uint32_t to)
{
	const auto add_unique = [](vector<uint32_t> &l, uint32_t value) {
		auto itr = find(begin(l), end(l), value);
		if (itr == end(l))
			l.push_back(value);
	};
	add_unique(preceding_edges[to], from);
	add_unique(succeeding_edges[from], to);
}

DominatorBuilder::DominatorBuilder(const CFG &cfg_)
	: cfg(cfg_)
{
}

void DominatorBuilder::add_block(uint32_t block)
{
	if (!dominator)
	{
		dominator = block;
		return;
	}

	if (block != dominator)
		dominator = cfg.find_common_dominator(block, dominator);
}
}
