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

#include "ryu/ryu.h"
#include "spirv_common.hpp"

std::string spirv_cross::convert_to_string(float t, char locale_radix_point)
{
	(void)locale_radix_point; // for compatiable
	char buf[64];
	f2s_buffered(t, buf);
	return buf;
}

std::string spirv_cross::convert_to_string(double t, char locale_radix_point)
{
	(void)locale_radix_point; // for compatiable
	char buf[64];
	d2s_buffered(t, buf);
	return buf;
}
