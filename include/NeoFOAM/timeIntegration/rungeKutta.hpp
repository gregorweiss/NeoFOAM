// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 NeoFOAM authors

#pragma once

#include <functional>
#include <memory>

#include "NeoFOAM/core/parallelAlgorithms.hpp"
#include "NeoFOAM/fields/field.hpp"
#include "NeoFOAM/timeIntegration/timeIntegration.hpp"
#include "NeoFOAM/timeIntegration/sundials.hpp"

namespace NeoFOAM::timeIntegration
{

/**
 * @class RungeKutta
 * @brief Integrates in time, using Sundials, a PDE expression using the Runge-Kutta method.
 * @tparam SolutionFieldType The Solution field type, should be a volume or surface field.
 *
 * @details
 * Implements explicit Runge-Kutta time integration using the Sundials library. The class manages
 * Sundials vectors and memory through RAII principles, handling the conversion between internal
 * field representations and Sundials' N_Vector format. Supports various (at present explicit)
 * Runge-Kutta methods which can be specified through the dictionary configuration. The main
 * interface for a solve is through the `solve` function.
 *
 * @note
 * Useful Sundials documentation below, currently we have implemented only an explicit Runge-Kutta
 * interface, this simplifies things considerably as compared to some of the examples:
 * Initialization (and order thereof):
 * https://sundials.readthedocs.io/en/latest/arkode/Usage/Skeleton.html
 * Sundials-Kokkos:
 * https://sundials.readthedocs.io/en/latest/nvectors/NVector_links.html#the-nvector-kokkos-module
 * Sundials Contexts (scroll to bottom eg, they don't like copying):
 * https://sundials.readthedocs.io/en/latest/sundials/SUNContext_link.html#c.SUNContext_Create
 *
 * @warning For developers:
 * 1. This class uses Sundials-Kokkos vectors for computation, which are immediately wrapped as
 *    Sundials N_Vectors. After initialization, only interact with the N_Vector interface as per
 *    Sundials guidelines.
 * 2. The Sundials context is supposed to only be created and freed once in a program, making
 *    copying less desirable, see above. However we need to copy, so the context is placed in a
 *    shared_ptr to prevent early freeing. Please read the documentation about multiple, concurrent
 *    solves (you must not do them). You have been warned.
 */
template<typename SolutionFieldType>
class RungeKutta :
    public TimeIntegratorBase<SolutionFieldType>::template Register<RungeKutta<SolutionFieldType>>
{
public:

    using Expression = NeoFOAM::dsl::Expression;
    using Base =
        TimeIntegratorBase<SolutionFieldType>::template Register<RungeKutta<SolutionFieldType>>;
    using Base::dict_;

    /**
     * @brief Default constructor.
     */
    RungeKutta() = default;

    /**
     * @brief Default destructor.
     */
    ~RungeKutta() = default;

    /**
     * @brief Constructor that initializes the RungeKutta solver with a dictionary configuration.
     * @param dict The dictionary containing configuration parameters.
     */
    RungeKutta(const Dictionary& dict) : Base(dict) {}

    /**
     * @brief Copy constructor.
     * @param other The RungeKutta instance to copy from.
     * @note Sundials Kokkos vectors have copy constructors, N_Vectors should be constructed from
     * the Kokkos vectors.
     */
    RungeKutta(const RungeKutta& other);
    /**
     * @brief Move Constructor
     * @param other The RungeKutta instance to move from.
     */
    RungeKutta(RungeKutta&& other);

    // deleted because base class method deleted.
    RungeKutta& operator=(const RungeKutta& other) = delete;

    // deleted because base class method deleted.
    RungeKutta& operator=(RungeKutta&& other) = delete;

    /**
     * @brief Returns the name of the class.
     * @return std::string("Runge-Kutta").
     */
    static std::string name() { return "Runge-Kutta"; }

    /**
     * @brief Returns the documentation for the class.
     * @return std::string containing class documentation.
     */
    static std::string doc() { return "Explicit time integration using the Runge-Kutta method."; }

    /**
     * @brief Returns the schema for the class.
     * @return std::string containing the schema definition.
     */
    static std::string schema() { return "none"; }

    /**
     * @brief Solves one (explicit) time step, from n to n+1
     * @param exp The expression to be solved
     * @param solutionField The field containing the solution.
     * @param t The current time
     * @param dt The time step size
     */
    void
    solve(Expression& exp, SolutionFieldType& solutionField, scalar t, const scalar dt) override;

    /**
     * @brief Return a copy of this instantiated class.
     * @return std::unique_ptr to the new copy.
     */
    std::unique_ptr<TimeIntegratorBase<SolutionFieldType>> clone() const override;


private:

    std::unique_ptr<NeoFOAM::sundials::SKVector> solutionnew_;
    std::unique_ptr<NeoFOAM::sundials::SKVector> initialConditionsnew_;
    // VectorType kokkosSolution_ {}; /**< The Sundails, kokkos solution vector (do not use).*/
    // VectorType kokkosInitialConditions_ {
    // }; /**< The Sundails, kokkos initial conditions vector (do not use).*/
    N_Vector solution_ {nullptr
    }; /**< The N_Vector for the solution. (wrapping the kokkos vector). */
    N_Vector initialConditions_ {nullptr
    }; /**< The N_Vector for the initial conditions. (wrapping the kokkos vector). */
    std::shared_ptr<SUNContext_> context_ {nullptr, sundials::SUN_CONTEXT_DELETER}; // see type def
    std::unique_ptr<char> ODEMemory_ {nullptr}; /**< The 'memory' (sundails configuration) for the
                                                   solve. (note void* is not stl compliant). */
    std::unique_ptr<NeoFOAM::dsl::Expression> pdeExpr_ {nullptr
    }; /**< Pointer to the pde system we are integrating in time. */

    /**
     * @brief Initializes the complete Sundials solver setup.
     * @param exp The (DSL) expression being integrated in time
     * @param field The solution field
     * @param t The current time
     * @param dt The time step size
     */
    void initSUNERKSolver(Expression& exp, SolutionFieldType& field, const scalar t);

    /**
     * @brief Initializes the PDE expression to be solved.
     * @param exp The expression to be initialized
     */
    void initExpression(const Expression& exp);

    /**
     * @brief Initializes the Sundials context for the solver.
     */
    void initSUNContext();

    /**
     * @brief Initializes the Sundials vectors for solution storage.
     * @param size The size of the vectors to be initialized
     */
    void initSUNVector(size_t size);

    /**
     * @brief Initializes the initial conditions for the solver.
     * @param solutionField The field containing the initial conditions
     */
    void initSUNInitialConditions(const SolutionFieldType& solutionField);

    /**
     * @brief Initializes the ODE memory and solver parameters.
     * @param t The initial time for the solver
     */
    void initODEMemory(const scalar t);
};

} // namespace NeoFOAM
