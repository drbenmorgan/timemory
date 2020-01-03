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
//

/** \file timemory/components/general.hpp
 * \headerfile timemory/components/general.hpp "timemory/components/general.hpp"
 * Defines some short general components
 *
 */

#pragma once

#include "timemory/backends/gperf.hpp"
#include "timemory/components/base.hpp"
#include "timemory/components/types.hpp"
#include "timemory/mpl/apply.hpp"
#include "timemory/mpl/types.hpp"
#include "timemory/settings.hpp"
#include "timemory/variadic/types.hpp"

#include <cassert>
#include <cstdint>

//======================================================================================//

namespace tim
{
namespace component
{
#if defined(TIMEMORY_EXTERN_TEMPLATES) && !defined(TIMEMORY_BUILD_EXTERN_TEMPLATE)

extern template struct base<trip_count>;
extern template struct base<gperf_cpu_profiler, void>;
extern template struct base<gperf_heap_profiler, void>;

#endif

//--------------------------------------------------------------------------------------//
//
//          General Components with no specific category
//
//--------------------------------------------------------------------------------------//
// returns the trip count
//
struct trip_count : public base<trip_count>
{
    using value_type = int64_t;
    using this_type  = trip_count;
    using base_type  = base<this_type, value_type>;

    static std::string label() { return "trip_count"; }
    static std::string description() { return "trip counts"; }
    static value_type  record() { return 1; }

    value_type get() const { return accum; }
    value_type get_display() const { return get(); }

    void start()
    {
        set_started();
        value = record();
    }

    void stop()
    {
        accum += value;
        set_stopped();
    }
};

//--------------------------------------------------------------------------------------//
// start/stop gperftools cpu profiler
//
struct gperf_cpu_profiler : public base<gperf_cpu_profiler, void>
{
    using value_type = void;
    using this_type  = gperf_cpu_profiler;
    using base_type  = base<this_type, value_type>;

    static std::string label() { return "gperf_cpu_profiler"; }
    static std::string description() { return "gperftools cpu profiler"; }
    static value_type  record() {}

    static void thread_init(storage_type*) { gperf::cpu::register_thread(); }

    static void global_finalize(storage_type*)
    {
        if(gperf::cpu::is_running())
        {
            gperf::cpu::profiler_flush();
            gperf::cpu::profiler_stop();
        }
    }

    void start()
    {
        set_started();
        if(!gperf::cpu::is_running())
        {
            index                 = this_type::get_index()++;
            const auto& _dmp_info = get_dmp_info();
            bool        _dmp_init = std::get<0>(_dmp_info);
            int32_t     _dmp_rank = std::get<1>(_dmp_info);
            auto        fname     = settings::compose_output_filename(
                label() + "_" + std::to_string(index), ".dat", _dmp_init, _dmp_rank);
            auto ret = gperf::cpu::profiler_start(fname);
            if(ret == 0)
                fprintf(stderr, "[gperf_cpu_profiler]> Error starting %s...",
                        fname.c_str());
        }
    }

    void stop()
    {
        if(index >= 0)
        {
            gperf::cpu::profiler_flush();
            gperf::cpu::profiler_stop();
        }
        set_stopped();
    }

protected:
    int32_t index = -1;  // if this is >= zero, then we flush and stop

private:
    static std::atomic<int64_t>& get_index()
    {
        static std::atomic<int64_t> _instance(0);
        return _instance;
    }

    using dmp_info_t = std::tuple<bool, int32_t, int32_t>;

    static const dmp_info_t& get_dmp_info()
    {
        static dmp_info_t _info{ dmp::is_initialized(), dmp::rank(), dmp::size() };
        return _info;
    }
};

//--------------------------------------------------------------------------------------//
// start/stop gperftools cpu profiler
//
struct gperf_heap_profiler : public base<gperf_heap_profiler, void>
{
    using value_type = void;
    using this_type  = gperf_heap_profiler;
    using base_type  = base<this_type, value_type>;

    static std::string label() { return "gperf_heap_profiler"; }
    static std::string description() { return "gperftools heap profiler"; }
    static value_type  record() {}

    static void global_finalize(storage_type*)
    {
        if(gperf::heap::is_running())
        {
            gperf::heap::profiler_flush("global_finalize");
            gperf::heap::profiler_stop();
        }
    }

    void start()
    {
        set_started();
        if(!gperf::heap::is_running())
        {
            index      = this_type::get_index()++;
            auto fname = settings::compose_output_filename(label(), ".dat");
            auto ret   = gperf::heap::profiler_start(fname);
            if(ret > 0)
                fprintf(stderr, "[gperf_heap_profiler]> Error starting %s...",
                        prefix.c_str());
        }
    }

    void stop()
    {
        if(index >= 0)
        {
            gperf::heap::profiler_flush(prefix);
            gperf::heap::profiler_stop();
        }
        set_stopped();
    }

    void set_prefix(const std::string& _prefix) { prefix = _prefix; }

protected:
    std::string prefix;
    int32_t     index = -1;  // if this is >= zero, then we flush and stop

private:
    static std::atomic<int64_t>& get_index()
    {
        static std::atomic<int64_t> _instance(0);
        return _instance;
    }
};

//--------------------------------------------------------------------------------------//

}  // namespace component

//--------------------------------------------------------------------------------------//
}  // namespace tim