/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#pragma once

#include <sstream>

#include "base.hpp"


namespace rl
{


typedef void (*event_output_f)(std::ostream& s, void const* ev);
typedef void (*event_dtor_f)(void* ev);

struct history_entry
{
    thread_id_t thread_index_;
    debug_info info_;
    void* ev_;
    event_output_f output_;
    event_dtor_f dtor_;

    history_entry(thread_id_t thread_index, debug_info_param info, void* ev, event_output_f output, event_dtor_f dtor);
};

template<typename T>
void event_output(std::ostream& s, void const* ev)
{
    static_cast<T const*>(ev)->output(s);
}

template<typename T>
void event_dtor(void* ev)
{
    delete static_cast<T*>(ev);
}


struct user_event
{
    char const* desc_;

    void output(std::ostream& s) const;
};

string strip_path(char const* filename);

std::ostream& operator << (std::ostream& ss, debug_info_param info);

class history_mgr
{
public:
    history_mgr(std::ostream& stream, thread_id_t thread_count);

    history_mgr(const history_mgr &) = delete;
    history_mgr &operator=(const history_mgr &) = delete;

    ~history_mgr();

    template<typename event_t>
    void exec_log(thread_id_t th, debug_info_param info, event_t const& ev, bool output_history)
    {
        exec_history_.push_back(history_entry(th, info, new event_t(ev), &event_output<event_t>, &event_dtor<event_t>));
        if (output_history)
        {
            output(exec_history_.size() - 1);
        }
    }

    void print_exec_history(bool output_history);

    void clear();

private:
    rl_vector<history_entry>    exec_history_;
    thread_id_t                 thread_count_;
    std::ostream&               out_stream_;

    void output(size_t i);
};


}
