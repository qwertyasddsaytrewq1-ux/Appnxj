/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include <coroutine>
#include <exception>

template<typename T>
struct Generator {
    struct promise_type {
        T value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T val) {
            value = val;
            return {};
        }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    struct iterator {
        std::coroutine_handle<promise_type> handle = nullptr;

        iterator() = default;
        explicit iterator(std::coroutine_handle<promise_type> h) : handle(h) {}

        iterator& operator++() {
            if (handle) {
                handle.resume();
                if (handle.done()) {
                    handle = nullptr;
                }
            }
            return *this;
        }

        bool operator!=(const iterator& other) const {
            return handle != other.handle;
        }

        T operator*() const {
            return handle.promise().value;
        }
    };

    std::coroutine_handle<promise_type> handle = nullptr;

    explicit Generator(std::coroutine_handle<promise_type> h) : handle(h) {}

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~Generator() {
        if (handle) handle.destroy();
    }

    iterator begin() {
        if (handle) {
            handle.resume();
            if (handle.done()) return end();
        }
        return iterator{handle};
    }

    iterator end() {
        return iterator{nullptr};
    }
};
