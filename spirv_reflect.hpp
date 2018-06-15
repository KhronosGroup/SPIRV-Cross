/*
 * Copyright 2015-2018 ARM Limited
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

#ifndef SPIRV_CROSS_REFLECT_HPP
#define SPIRV_CROSS_REFLECT_HPP

#include "spirv_cross.hpp"
#include "spirv_glsl.hpp"
#include <utility>
#include <vector>

namespace spirv_cross
{
class CompilerReflection : public CompilerGLSL
{
    using Parent = CompilerGLSL;


public:
    CompilerReflection(std::vector<uint32_t> spirv_)
        : Parent(move(spirv_))
    {
    }

    CompilerReflection(const uint32_t *ir, size_t word_count)
        : Parent(ir, word_count)
    {
    }

    void set_format(const std::string& format);
    std::string compile() override;

private:
    static const char *execution_model_to_str(spv::ExecutionModel model);
    
    void emit_entry_points();
    void emit_types();
    void emit_resources();
    void emit_extensions();

    void emit_type(const SPIRType &type, bool& emitted_open_tag);
    void emit_type_member(const SPIRType &type, uint32_t index);
    void emit_type_member_qualifiers(const SPIRType &type, uint32_t index);
    void emit_type_array(const SPIRType &type);
    void emit_resources(const char *tag, const std::vector<Resource> &resources);

    void begin_json_object();
    void end_json_object();
    void emit_json_key(const std::string& key);
    // FIXME switch to templated types with specialization for string?
    void emit_json_key_value(const std::string& key, const std::string& value);
    void emit_json_key_value(const std::string& key, bool value);
    void emit_json_key_value(const std::string& key, uint32_t value);
    void emit_json_key_object(const std::string& key);
    void emit_json_key_array(const std::string& key);
    void emit_json_array_value(const std::string& value);
    void emit_json_array_value(uint32_t value);
    void begin_json_array();
    void end_json_array();

    template <typename... Ts>
    inline void statement_no_return(Ts &&... ts) {
        if (force_recompile) {
            // Do not bother emitting code while force_recompile is active.
            // We will compile again.
            statement_count++;
            return;
        }

        if (redirect_statement)
            redirect_statement->push_back(join(std::forward<Ts>(ts)...));
        else {
            statement_indent();
            statement_inner(std::forward<Ts>(ts)...);
        }
    }

    enum class JsonType {
        Object,
        Array,
    };

    using JsonState = std::pair<JsonType, bool>;
    std::stack<JsonState> json_state;
};

} // namespace spirv_cross

#endif
