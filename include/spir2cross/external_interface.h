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

#ifndef SPIR2CROSS_EXTERNAL_INTERFACE_H
#define SPIR2CROSS_EXTERNAL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct spir2cross_shader spir2cross_shader_t;

struct spir2cross_interface
{
    spir2cross_shader_t* (*construct)(void);
    void (*destruct)(spir2cross_shader_t *thiz);
    void (*invoke)(spir2cross_shader_t *thiz);
};

void spir2cross_set_stage_input(spir2cross_shader_t *thiz,
        unsigned location, void *data, size_t size);

void spir2cross_set_stage_output(spir2cross_shader_t *thiz,
        unsigned location, void *data, size_t size);

void spir2cross_set_push_constant(spir2cross_shader_t *thiz,
        void *data, size_t size);

void spir2cross_set_uniform_constant(spir2cross_shader_t *thiz,
        unsigned location,
        void *data, size_t size);

void spir2cross_set_resource(spir2cross_shader_t *thiz,
        unsigned set, unsigned binding,
        void **data, size_t size);

const struct spir2cross_interface* spir2cross_get_interface(void);

typedef enum spir2cross_builtin
{
    SPIR2CROSS_BUILTIN_POSITION = 0,
    SPIR2CROSS_BUILTIN_FRAG_COORD = 1,
    SPIR2CROSS_BUILTIN_WORK_GROUP_ID = 2,
    SPIR2CROSS_BUILTIN_NUM_WORK_GROUPS = 3,
    SPIR2CROSS_NUM_BUILTINS
} spir2cross_builtin;

void spir2cross_set_builtin(spir2cross_shader_t *thiz,
        spir2cross_builtin builtin,
        void *data, size_t size);

#define SPIR2CROSS_NUM_DESCRIPTOR_SETS 4
#define SPIR2CROSS_NUM_DESCRIPTOR_BINDINGS 16
#define SPIR2CROSS_NUM_STAGE_INPUTS 16
#define SPIR2CROSS_NUM_STAGE_OUTPUTS 16
#define SPIR2CROSS_NUM_UNIFORM_CONSTANTS 32

enum spir2cross_format
{
    SPIR2CROSS_FORMAT_R8_UNORM = 0,
    SPIR2CROSS_FORMAT_R8G8_UNORM = 1,
    SPIR2CROSS_FORMAT_R8G8B8_UNORM = 2,
    SPIR2CROSS_FORMAT_R8G8B8A8_UNORM = 3,

    SPIR2CROSS_NUM_FORMATS
};

enum spir2cross_wrap
{
    SPIR2CROSS_WRAP_CLAMP_TO_EDGE = 0,
    SPIR2CROSS_WRAP_REPEAT = 1,

    SPIR2CROSS_NUM_WRAP
};

enum spir2cross_filter
{
    SPIR2CROSS_FILTER_NEAREST = 0,
    SPIR2CROSS_FILTER_LINEAR = 1,

    SPIR2CROSS_NUM_FILTER
};

enum spir2cross_mipfilter
{
    SPIR2CROSS_MIPFILTER_BASE = 0,
    SPIR2CROSS_MIPFILTER_NEAREST = 1,
    SPIR2CROSS_MIPFILTER_LINEAR = 2,

    SPIR2CROSS_NUM_MIPFILTER
};

struct spir2cross_miplevel
{
    const void *data;
    unsigned width, height;
    size_t stride;
};

struct spir2cross_sampler_info
{
    const struct spir2cross_miplevel *mipmaps;
    unsigned num_mipmaps;

    enum spir2cross_format format;
    enum spir2cross_wrap wrap_s;
    enum spir2cross_wrap wrap_t;
    enum spir2cross_filter min_filter;
    enum spir2cross_filter mag_filter;
    enum spir2cross_mipfilter mip_filter;
};

typedef struct spir2cross_sampler_2d spir2cross_sampler_2d_t;
spir2cross_sampler_2d_t *spir2cross_create_sampler_2d(const struct spir2cross_sampler_info *info);
void spir2cross_destroy_sampler_2d(spir2cross_sampler_2d_t *samp);


#ifdef __cplusplus
}
#endif

#endif
