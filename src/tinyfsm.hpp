/*
 * TinyFSM - Tiny Finite State Machine Processor
 *
 * Copyright (c) 2012-2018 Axel Burri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* ---------------------------------------------------------------------
 * Version: 0.3.1
 *
 * API documentation: see "../doc/50-API.md"
 *
 * The official TinyFSM website is located at:
 * https://digint.ch/tinyfsm/
 *
 * Author:
 * Axel Burri <axel@tty0.ch>
 * ---------------------------------------------------------------------
 */

#ifndef TINYFSM_HPP_INCLUDED
#define TINYFSM_HPP_INCLUDED

#include <type_traits>
#include <cassert>
#include <utility>

#include "debug.h"

namespace tinyfsm
{

struct Event {
};

template<typename S>
struct _state_instance {
    using value_type = S;
    using type = _state_instance<S>;
    static S value;
};

template<typename S>
typename _state_instance<S>::value_type _state_instance<S>::value;

template<typename F>
class Fsm {
public:
    using fsmtype = Fsm<F>;
    using state_ptr_t = F*;

    static state_ptr_t previous_state_ptr;
    static state_ptr_t current_state_ptr;
    static state_ptr_t resume_state_ptr;

    static F& current() {
        assert(current_state_ptr != nullptr );
        return *current_state_ptr;
    }

    static void resume_at_back() {
        resume_state_ptr = previous_state_ptr;
        // logf(LogLevels::TRACE, "FSM", "Will resume: %s", resume_state_ptr->name());
    }

    bool transitioned() {
        return current_state_ptr != this;
    }

    // public, leaving ability to access state instance (e.g. on reset)
    template<typename S>
    static constexpr S &state() {
        static_assert(std::is_same<fsmtype, typename S::fsmtype>::value, "accessing state of different state machine");
        return _state_instance<S>::value;
    }

    template<typename S>
    static constexpr bool is_in_state() {
        return current_state_ptr == &_state_instance<S>::value;
    }

    struct deferred_t {
        state_ptr_t next_state_ptr{ nullptr };
        const char *name{ nullptr };

        deferred_t() {
        }

        deferred_t(state_ptr_t ptr, const char *name) : next_state_ptr(ptr), name(name) {
        }

        operator bool() const {
            return next_state_ptr != nullptr;
        }
    };

    template<typename S>
    static deferred_t deferred() {
        return { &_state_instance<S>::value, "" };
    }

public:
    // explicitely specialized in FSM_INITIAL_STATE macro
    static void set_initial_state();

    static void reset() {
    }

    static void enter() {
        current_state_ptr->entry();
    }

    static void start() {
        set_initial_state();
        enter();
    }

    template<typename E>
    static void dispatch(E const & event) {
        current_state_ptr->react(event);
    }

    static void back() {
        auto temp = previous_state_ptr;
        // logf(LogLevels::TRACE, "FSM", "Back -> %s", temp->name());
        current_state_ptr->exit();
        previous_state_ptr = current_state_ptr;
        current_state_ptr = temp;
        current_state_ptr->entry();
    }

    static void resume() {
        auto ptr = resume_state_ptr;
        // logf(LogLevels::TRACE, "FSM", "Resume -> %s", ptr->name());
        resume_state_ptr = nullptr;
        transit(ptr);
    }

private:
    static void transit(state_ptr_t state_ptr) {
        current_state_ptr->exit();
        previous_state_ptr = current_state_ptr;
        current_state_ptr = state_ptr;
        current_state_ptr->entry();
    }

protected:
    void transit(deferred_t deferred) {
        transit(deferred.next_state_ptr);
    }

    template<typename S, typename ...Args>
    void transit_into(Args... args) {
        _state_instance<S>::value = S{ std::forward<Args>(args)... };
        transit(&_state_instance<S>::value);
    }

    template<typename S>
    void transit() {
        transit(&_state_instance<S>::value);
    }

    template<typename S, typename ActionFunction
             /*
             typename = typename std::enable_if<
                 std::is_function<
                     typename std::remove_pointer<ActionFunction>::type
                     >::value
                 >::type
             */
    >
    void transit(ActionFunction action_function) {
        static_assert(std::is_void<typename std::result_of<ActionFunction()>::type >::value,
                      "result type of 'action_function()' is not 'void'");

        current_state_ptr->exit();
        // NOTE: we get into deep trouble if the action_function sends a new event.
        // TODO: implement a mechanism to check for reentrancy
        action_function();
        previous_state_ptr = current_state_ptr;
        current_state_ptr = &_state_instance<S>::value;
        current_state_ptr->entry();
    }

    template<typename S, typename ActionFunction, typename ConditionFunction>
    void transit(ActionFunction action_function, ConditionFunction condition_function) {
        static_assert(std::is_same<typename std::result_of<ConditionFunction()>::type, bool>::value,
                      "result type of 'condition_function()' is not 'bool'");

        if (condition_function()) {
            transit<S>(action_function);
        }
    }
};

template<typename F>
typename Fsm<F>::state_ptr_t Fsm<F>::previous_state_ptr{ nullptr };

template<typename F>
typename Fsm<F>::state_ptr_t Fsm<F>::current_state_ptr{ nullptr };

template<typename F>
typename Fsm<F>::state_ptr_t Fsm<F>::resume_state_ptr{ nullptr };

template<typename... FF>
struct FsmList;

template<> struct FsmList<> {
    static void set_initial_state() {
    }

    static void reset() {
    }

    static void enter() {
    }

    template<typename E>
    static void dispatch(E const &) {
    }
};

template<typename F, typename... FF>
struct FsmList<F, FF...> {
    using fsmtype = Fsm<F>;

    static void set_initial_state() {
        fsmtype::set_initial_state();
        FsmList<FF...>::set_initial_state();
    }

    static void reset() {
        F::reset();
        FsmList<FF...>::reset();
    }

    static void enter() {
        fsmtype::enter();
        FsmList<FF...>::enter();
    }

    static void start() {
        set_initial_state();
        enter();
    }

    template<typename E>
    static void dispatch(E const & event) {
        fsmtype::template dispatch<E>(event);
        FsmList<FF...>::template dispatch<E>(event);
    }
};

template<typename... SS> struct StateList;
template<> struct StateList<> {
    static void reset() {
    }
};

template<typename S, typename... SS>
struct StateList<S, SS...> {
    static void reset() {
        _state_instance<S>::value = S();
        StateList<SS...>::reset();
    }
};

}

#define FSM_INITIAL_STATE(_FSM, _STATE)                   \
    namespace tinyfsm {                                   \
template<> void Fsm< _FSM >::set_initial_state() {        \
    current_state_ptr = &_state_instance<_STATE>::value;  \
}                                                         \
    }

#endif /* TINYFSM_HPP_INCLUDED */
