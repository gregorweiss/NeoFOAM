// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 NeoFOAM authors

#pragma once

#include "Kokkos_Core.hpp"

#include "NeoFOAM/core.hpp"
#include "NeoFOAM/finiteVolume/cellCentred/boundary/volumeBoundaryFactory.hpp"
#include "NeoFOAM/mesh/unstructured.hpp"

namespace NeoFOAM::finiteVolume::cellCentred
{

template<typename ValueType>
class Calculated : public VolumeBoundaryFactory<ValueType>
{

public:

    using CalculatedType = Calculated<ValueType>;

    Calculated(const UnstructuredMesh& mesh, std::size_t patchID)
        : VolumeBoundaryFactory<ValueType>(mesh, patchID)
    {
        VolumeBoundaryFactory<ValueType>::template registerClass<CalculatedType>();
    }

    static std::unique_ptr<VolumeBoundaryFactory<ValueType>>
    create(const UnstructuredMesh& mesh, const Dictionary&, std::size_t patchID)
    {
        return std::make_unique<CalculatedType>(mesh, patchID);
    }

    virtual void correctBoundaryCondition(DomainField<ValueType>& domainField) override {}

    static std::string name() { return "calculated"; }
};
}
