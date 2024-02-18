// SPDX-License-Identifier: MPL-2.0
// SPDX-FileCopyrightText: 2023 NeoFOAM authors
#pragma once

// TODO this needs to be implemented in the corresponding cmake file
namespace NeoFOAM {
#ifdef NEOFOAM_DP_SCALAR
typedef double scalar;
#else
typedef float scalar;
#endif

#ifdef NEOFOAM_DP_LABEL
typedef double scalar;
#else
typedef float scalar;
#endif
} // namespace NeoFOAM
