/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <entt/entt.hpp>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>

struct A { std::uint32_t v; };
struct S { std::uint32_t v; };
struct Fat { std::uint32_t v; std::uint8_t rest[4092]; };
struct G { std::uint32_t gravity, wind; };
template<int N> struct Mark {};
template<int N> struct Word { std::uint32_t on; };

constexpr int ROWS = 204'800;
constexpr unsigned ROUNDS = 8;

static std::uint64_t now()
{
    return (std::uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

static std::uint32_t state = 2463534242u;
static std::uint32_t random32()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

#ifdef _WIN32
struct held_counters {
    std::uint32_t cb, faults;
    std::size_t peak, now, paged_peak, paged, nonpaged_peak, nonpaged, pagefile, pagefile_peak;
};
extern "C" __declspec(dllimport) void *GetCurrentProcess();
extern "C" __declspec(dllimport) int K32GetProcessMemoryInfo(void *process, held_counters *counters, std::uint32_t size);
static std::size_t held()
{
    held_counters c{};
    c.cb = sizeof c;
    return K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof c) ? c.now >> 10 : 0;
}
#else
static std::size_t held() { return 0; }
#endif

template<typename Carried> static void change(unsigned bytes, int count)
{
    entt::registry r;
    std::vector<entt::entity> ids(count);
    std::uint64_t on = 0, off = 0, t;
    for (int i = 0; i < count; i ++) r.emplace<Carried>(ids[i] = r.create());
    for (unsigned round = 0; round < ROUNDS; round ++) {
        t = now();
        for (int i = 0; i < count; i ++) r.emplace<S>(ids[i], 100u + (std::uint32_t)i);
        t = now() - t;
        if (!round || t < on) on = t;
        t = now();
        for (int i = 0; i < count; i ++) r.erase<S>(ids[i]);
        t = now() - t;
        if (!round || t < off) off = t;
    }
    std::printf("EnTT, an entity of %4u bytes: a component given %6.1f ns, taken %6.1f ns\n", bytes,
        (double)on / count, (double)off / count);
}

static void basics()
{
    constexpr int COUNT = 100'000;
    entt::registry r;
    std::vector<entt::entity> ids(COUNT);
    std::uint64_t first, again, gone, t;
    for (int i = 0; i < COUNT; i ++) ids[i] = r.create();
    t = now();
    for (int i = 0; i < COUNT; i ++) r.emplace<A>(ids[i], 1u);
    first = now() - t;
    t = now();
    for (int i = 0; i < COUNT; i ++) r.emplace_or_replace<A>(ids[i], 1u);
    again = now() - t;
    t = now();
    for (int i = 0; i < COUNT; i ++) r.destroy(ids[i]);
    gone = now() - t;
    std::printf("EnTT: an entity's first component %6.1f ns, one it has %6.1f ns, destroyed %6.1f ns\n",
        (double)first / COUNT, (double)again / COUNT, (double)gone / COUNT);
}

static void reads()
{
    entt::registry r;
    std::uint64_t best[3] = {}, t;
    std::uint32_t total = 0;
    for (int i = 0; i < ROWS; i ++) {
        auto e = r.create();
        r.emplace<A>(e, 1u);
        r.emplace<S>(e, 1u);
    }
    for (unsigned round = 0; round < ROUNDS * 4; round ++) {
        t = now();
        r.view<S>().each([&](const S &s) { total += s.v; });
        t = now() - t;
        if (!round || t < best[0]) best[0] = t;
        t = now();
        r.view<A, S>().each([&](const A &a, const S &s) { total += a.v + s.v; });
        t = now() - t;
        if (!round || t < best[1]) best[1] = t;
        t = now();
        for (const S &s : r.storage<S>()) total += s.v;
        t = now() - t;
        if (!round || t < best[2]) best[2] = t;
    }
    std::printf("EnTT, %d rows: a component walked alone %6.1f us, and its storage walked %6.1f us; two "
        "read together %6.1f us, %.3f ns a row (%u)\n", ROWS, (double)best[0] / 1000.0,
        (double)best[2] / 1000.0, (double)best[1] / 1000.0, (double)best[1] / ROWS, total & 1);
}

static void reach()
{
    entt::registry r;
    std::vector<entt::entity> ids(ROWS), mixed(ROWS);
    std::uint64_t near = 0, far = 0, t;
    std::uint32_t total = 0;
    for (int i = 0; i < ROWS; i ++) r.emplace<A>(ids[i] = r.create(), 1u);
    for (int i = 0; i < ROWS; i ++) mixed[i] = ids[random32() % ROWS];
    for (unsigned round = 0; round < ROUNDS * 4; round ++) {
        t = now();
        for (int i = 0; i < ROWS; i ++) if (const A *a = r.try_get<A>(ids[i])) total += a->v;
        t = now() - t;
        if (!round || t < near) near = t;
        t = now();
        for (int i = 0; i < ROWS; i ++) if (const A *a = r.try_get<A>(mixed[i])) total += a->v;
        t = now() - t;
        if (!round || t < far) far = t;
    }
    std::printf("EnTT: %d values reached by id, in the order they lie %6.3f ns each, in no order "
        "%6.3f ns each (%u)\n", ROWS, (double)near / ROWS, (double)far / ROWS, total & 1);
}

static void singleton()
{
    entt::registry r;
    std::uint64_t best = 0, t;
    std::uint32_t total = 0;
    r.ctx().emplace<G>(10u, 1u);
    for (unsigned round = 0; round < ROUNDS * 4; round ++) {
        t = now();
        for (int i = 0; i < 1'000'000; i ++) total += r.ctx().get<G>().gravity;
        t = now() - t;
        if (!round || t < best) best = t;
    }
    std::printf("EnTT, a singleton read: %6.3f ns each time (%u)\n", (double)best / 1'000'000, total & 1);
}

template<template<int> class M, int... N>
static void marked(entt::registry &r, entt::entity e, unsigned bits, std::integer_sequence<int, N...>)
{
    ((bits >> N & 1 ? (void)r.emplace<M<N>>(e) : (void)0), ...);
}

template<template<int> class M> static void markers(const char *as)
{
    std::size_t before = held();
    entt::registry r;
    for (int i = 0; i < ROWS; i ++) {
        auto e = r.create();
        r.emplace<A>(e, 1u);
        marked<M>(r, e, (unsigned)i % 4096, std::make_integer_sequence<int, 12>{});
    }
    std::printf("EnTT, 204800 entities with six markers each, as %s: the process holds %zu KB more\n", as,
        held() - before);
}

int main()
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    basics();
    change<A>(4, ROWS);
    change<Fat>(4096, 10'000);
    reads();
    reach();
    singleton();
    markers<Word>("components of four bytes");
    markers<Mark>("types with no bytes");
    return 0;
}
