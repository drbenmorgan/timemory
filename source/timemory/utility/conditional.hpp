// MIT License
//
// Copyright (c) 2020, The Regents of the University of California,
// through Lawrence Berkeley National Laboratory (subject to receipt of any
// required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/** \file timemory/utility/conditional.hpp
 * \headerfile timemory/utility/conditional.hpp "timemory/utility/conditional.hpp"
 * Provides structure for conditionally generate a timemory component configuration
 *
 */

#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

namespace tim
{
///
/// \brief conditional<_Category, _EvalArgs...>
/// This provides a conditional generator
///
/// \example Profiling Categories
///
///     struct G4ProfileType
///     {
///         enum : size_t
///         {
///             Track = 0
///         };
///     };
///
///     struct geant4_tag {};
///
///     using G4TrackProfileBundle =
///             tim::component::user_bundle<G4ProfileType::Track, geant4_tag>;
///
///     using G4TrackProfiler =
///             tim::conditional<G4ProfileType::Track, const G4Track*>;
///
///
///     class G4Track
///     {
///     public:
///         G4Track(G4DynamicParticle*);
///         ~G4Track();
///
///     private:
///         tim::auto_tuple<G4TrackProfileBundle>* fTrackProfiler = nullptr;
///     };
///
///
///     G4Track::G4Track(G4DynamicParticle* p)
///     : fTrackProfiler(nullptr)
///     {
///         if(Profiler_t::enable(this))
///         {
///             auto label = Profiler_t::label(this);
///             if(label.length() == 0)
///                 label = p->GetParticleDefinition()->GetParticleName();
///             fTrackProfiler = new ProfilerHandle_t(label);
///         }
///     }
///
///
///     ~G4Track::G4Track()
///     {
///         delete fTrackProfiler;
///     }
///
template <size_t _Category, typename... _EvalArgs>
struct conditional
{
public:
    using this_type       = conditional<_Category, _EvalArgs...>;
    using eval_functor_t  = std::function<bool(_EvalArgs...)>;
    using label_functor_t = std::function<std::string(_EvalArgs...)>;

    // invokes the functor that determines whether to enable profiling
    static bool query(_EvalArgs... _args) { return get_evaluator()(_args...); }
    // invokes the functor for generating a label when profiling is enabled
    static std::string label(_EvalArgs... _args) { return get_labeler()(_args...); }

    ///
    /// \brief get_evaluator
    /// This is the functor that determines whether to activate the profiler.
    /// By default, returns false.
    /// Use a lambda to customize the activation of the profiler for the category
    ///
    /// \example Profile only G4Electron
    ///
    ///     using TrackProfiler = G4Profiler<G4ProfileType::Track, const G4Track*>;
    ///
    ///     TrackProfiler::get_evaluator() = [](const G4Track* t)
    ///     {
    ///         auto* pdef = t->GetDynamicParticle()->GetParticleDefinition();
    ///         return (pdef == G4Electron::ElectronDefinition());
    ///     };
    ///
    ///     TrackProfiler::get_labeler() = [](const Track* t)
    ///     {
    ///         auto* p = t->GetDynamicParticle();
    ///
    ///         G4double energy = p->GetKineticEnergy() / CLHEP::MeV;
    ///         const G4double eBin = 10 * CLHEP::MeV;
    ///         G4int eMin = energy - fmod(energy, eBin);
    ///         G4int eMax = energy + eBin - fmod(energy + eBin, eBin);
    ///
    ///         std::stringstream ss;
    ///         ss << p->GetParticleDefinition()->GetParticleName() << "_"
    ///            << eMin << "," << eMax << "_MeV";
    ///         return ss.str();
    ///     };
    ///
    static eval_functor_t& get_evaluator()
    {
        static eval_functor_t _instance = [](_EvalArgs...) { return false; };
        return _instance;
    }

    ///
    /// \brief get_labeler
    /// This is the functor that generates a label for a profiling instance.
    /// By default, it returns an empty string.
    /// Use a lambda to customize the label for the profiling instance
    ///
    /// \example Create labels based on particle energy bins of 10 MeV
    ///
    ///     TrackProfiler::get_labeler() = [](const Track* t)
    ///     {
    ///         auto* p = t->GetDynamicParticle();
    ///
    ///         G4double energy = p->GetKineticEnergy() / CLHEP::MeV;
    ///         const G4double eBin = 10 * CLHEP::MeV;
    ///         G4int eMin = energy - fmod(energy, eBin);
    ///         G4int eMax = energy + eBin - fmod(energy + eBin, eBin);
    ///
    ///         std::stringstream ss;
    ///         ss << p->GetParticleDefinition()->GetParticleName() << "_"
    ///            << eMin << "," << eMax << "_MeV";
    ///         return ss.str();
    ///     };
    ///
    static label_functor_t& get_labeler()
    {
        static label_functor_t _instance = [](_EvalArgs...) { return std::string(""); };
        return _instance;
    }
};

}  // namespace tim
