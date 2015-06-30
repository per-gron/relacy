/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#include "rmw.hpp"

namespace rl
{

char const* format(rmw_type_e t)
{
    switch (t)
    {
    case rmw_type_swap: return "exchange";
    case rmw_type_add: return "fetch_add";
    case rmw_type_sub: return "fetch_sub";
    case rmw_type_and: return "fetch_and";
    case rmw_type_or: return "fetch_or";
    case rmw_type_xor: return "fetch_xor";
    }
    RL_VERIFY(!"invalid rmw type");
    throw std::logic_error("invalid rmw type");
}

}
