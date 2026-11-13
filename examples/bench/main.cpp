// Loopback benchmark: round-trip latency and one-way throughput between two
// Juntos clients on 127.0.0.1. Run it before and after a change to compare.
//
// Usage: juntos_bench [rtt_iterations] [throughput_packets]

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include <client.h>

using Clock = std::chrono::steady_clock;

namespace
{
constexpr int kPortA = 40101;
constexpr int kPortB = 40102;
constexpr auto kRecvTimeout = std::chrono::milliseconds(100);

// Spin on update() until an application payload arrives or the deadline hits.
bool receiveOne(Client& client, Clock::time_point deadline)
{
  while (Clock::now() < deadline) {
    if (auto bytes = client.update()) {
      return true;
    }
  }
  return false;
}

double percentile(std::vector<double>& sorted, double p)
{
  auto idx = static_cast<size_t>(p * static_cast<double>(sorted.size() - 1));
  return sorted[idx];
}
}  // namespace

int main(int argc, char* argv[])
{
  const int rttIterations = argc > 1 ? std::atoi(argv[1]) : 100000;
  const int throughputPackets = argc > 2 ? std::atoi(argv[2]) : 1000000;

  Client a;
  Client b;
  if (!a.init("127.0.0.1", kPortA, kRecvTimeout)
      || !b.init("127.0.0.1", kPortB, kRecvTimeout))
  {
    std::puts("Failed to bind benchmark sockets");
    return 1;
  }

  // Handshake: each side PINGs the other, update() answers/absorbs PING/PONG.
  (void)a.addPeer("127.0.0.1", kPortB);
  (void)b.addPeer("127.0.0.1", kPortA);
  for (int i = 0; i < 5; ++i) {
    (void)a.update();
    (void)b.update();
  }

  // --- Round-trip latency -------------------------------------------------
  std::vector<uint8_t> payload(32, 0xAB);
  std::vector<double> rttUs;
  rttUs.reserve(static_cast<size_t>(rttIterations));
  int lost = 0;

  for (int i = 0; i < rttIterations; ++i) {
    auto start = Clock::now();
    auto deadline = start + std::chrono::seconds(1);
    (void)a.send(payload);
    if (!receiveOne(b, deadline)) {
      ++lost;
      continue;
    }
    (void)b.send(payload);
    if (!receiveOne(a, deadline)) {
      ++lost;
      continue;
    }
    rttUs.push_back(
        std::chrono::duration<double, std::micro>(Clock::now() - start)
            .count());
  }

  std::sort(rttUs.begin(), rttUs.end());
  double mean = 0;
  for (double v : rttUs) {
    mean += v;
  }
  mean /= static_cast<double>(std::max<size_t>(rttUs.size(), 1));

  std::printf("RTT (%d x 32B): p50 %.2f us | p99 %.2f us | mean %.2f us | "
              "lost %d\n",
              rttIterations,
              rttUs.empty() ? 0.0 : percentile(rttUs, 0.50),
              rttUs.empty() ? 0.0 : percentile(rttUs, 0.99),
              mean,
              lost);

  // --- One-way throughput -------------------------------------------------
  std::atomic<bool> senderDone {false};
  std::atomic<long> received {0};

  std::thread receiver([&] {
    Clock::time_point idleSince = Clock::now();
    while (true) {
      if (auto bytes = b.update()) {
        received.fetch_add(1, std::memory_order_relaxed);
        idleSince = Clock::now();
      } else if (senderDone.load()
                 && Clock::now() - idleSince > std::chrono::milliseconds(300))
      {
        break;
      }
    }
  });

  std::vector<uint8_t> small(64, 0xCD);
  auto start = Clock::now();
  for (int i = 0; i < throughputPackets; ++i) {
    (void)a.send(small);
  }
  auto sendEnd = Clock::now();
  senderDone = true;
  receiver.join();

  double sendSecs = std::chrono::duration<double>(sendEnd - start).count();
  long got = received.load();
  std::printf("Throughput (%d x 64B): sent %.0f pkt/s | received %ld (%.1f%%)\n",
              throughputPackets,
              throughputPackets / sendSecs,
              got,
              100.0 * static_cast<double>(got) / throughputPackets);

  return 0;
}
