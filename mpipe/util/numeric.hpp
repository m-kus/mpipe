#pragma once

#include <cinttypes>
#include <cmath>

#define npmax(a, b)		((a) > (b) ? (a) : (b))
#define npmin(a, b)		((a) < (b) ? (a) : (b))
#define npabs(a)		((a) > 0 ? (a) : (-(a)))
#define npsign(a)		((a) < 0 ? -1 : ((a) > 0 ? 1 : 0))
