/*
 * Copyright 2015-2016 ARM Limited
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

#ifndef SPIR2CROSS_SAMPLER_HPP
#define SPIR2CROSS_SAMPLER_HPP

#include <vector>

namespace spir2cross
{
    struct spir2cross_sampler_2d
    {
        inline virtual ~spir2cross_sampler_2d() {}
    };

    template<typename T>
    struct sampler2DBase : spir2cross_sampler_2d
    {
        sampler2DBase(const spir2cross_sampler_info *info)
        {
            mips.insert(mips.end(), info->mipmaps, info->mipmaps + info->num_mipmaps);
            format = info->format;
            wrap_s = info->wrap_s;
            wrap_t = info->wrap_t;
            min_filter = info->min_filter;
            mag_filter = info->mag_filter;
            mip_filter = info->mip_filter;
        }

        inline virtual T sample(glm::vec2 uv, float bias)
        {
            return sampleLod(uv, bias);
        }

        inline virtual T sampleLod(glm::vec2 uv, float lod)
        {
            if (mag_filter == SPIR2CROSS_FILTER_NEAREST)
            {
                uv.x = wrap(uv.x, wrap_s, mips[0].width);
                uv.y = wrap(uv.y, wrap_t, mips[0].height);
                glm::vec2 uv_full = uv * glm::vec2(mips[0].width, mips[0].height);

                int x = int(uv_full.x);
                int y = int(uv_full.y);
                return sample(x, y, 0);
            }
            else
            {
                return T(0, 0, 0, 1);
            }
        }

        inline float wrap(float v, spir2cross_wrap wrap, unsigned size)
        {
            switch (wrap)
            {
                case SPIR2CROSS_WRAP_REPEAT:
                    return v - glm::floor(v);
                case SPIR2CROSS_WRAP_CLAMP_TO_EDGE:
                {
                    float half = 0.5f / size;
                    return glm::clamp(v, half, 1.0f - half);
                }

                default:
                    return 0.0f;
            }
        }

        std::vector<spir2cross_miplevel> mips;
        spir2cross_format format;
        spir2cross_wrap wrap_s;
        spir2cross_format wrap_t;
        spir2cross_filter min_filter;
        spir2cross_filter mag_filter;
        spir2cross_mipfilter mip_filter;
    };

    typedef sampler2DBase<glm::vec4> sampler2D;
    typedef sampler2DBase<glm::ivec4> isampler2D;
    typedef sampler2DBase<glm::uvec4> usampler2D;

    template<typename T>
    inline T texture(const sampler2DBase<T> &samp, const glm::vec2 &uv, float bias = 0.0f) { return samp.sample(uv, bias); }

}

#endif
