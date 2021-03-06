/*!
 * @file digest.c
 *
 * @copyright
 * Copyright &copy; 2016, Scott K Logan
 *
 * @copyright
 * All rights reserved.
 *
 * @copyright
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * @copyright
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of OpenELP nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * @copyright
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @copyright
 * EchoLink&reg; is a registered trademark of Synergenics, LLC
 *
 * @author Scott K Logan &lt;logans@cottsay.net&gt;
 *
 * @brief Digest generation implementation
 */

#include "digest.h"
#include "md5.h"

#include <errno.h>
#include <stdio.h>

/*!
 * @brief Converts a 8-bit value to a base 16 string
 *
 * @param[in] data Numeric value to convert
 * @param[out] result Resulting ASCII characters
 */
static inline void digest_to_hex8(uint8_t data, char result[2]);

void digest_get(const uint8_t *data, const unsigned int len, uint8_t result[DIGEST_LEN])
{
	MD5_CTX ctx;

	MD5_Init(&ctx);

	MD5_Update(&ctx, data, len);

	MD5_Final((unsigned char *)result, &ctx);
}

static inline void digest_to_hex8(uint8_t data, char result[2])
{
	static const char lookup[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	result[0] = lookup[data / 16];
	result[1] = lookup[data % 16];
}

void digest_to_hex32(const uint32_t data, char result[8])
{
	const uint8_t *tgt = (const uint8_t *)&data;

	digest_to_hex8(tgt[3], &result[0]);
	digest_to_hex8(tgt[2], &result[2]);
	digest_to_hex8(tgt[1], &result[4]);
	digest_to_hex8(tgt[0], &result[6]);
}

void digest_to_str(const uint8_t md5[DIGEST_LEN], char result[2 * DIGEST_LEN + 1])
{
	size_t i;

	for (i = 0; i < DIGEST_LEN; i++, result += 2)
	{
		digest_to_hex8(md5[i], result);
	}
}
