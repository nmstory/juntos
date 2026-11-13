#include <chrono>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include <client.h>
#include <gtest/gtest.h>
#include <network_handler.h>

using namespace std::chrono_literals;

namespace
{
std::span<const uint8_t> asBytes(std::string_view s)
{
  return {reinterpret_cast<const uint8_t*>(s.data()), s.size()};
}

// Wait for and return the next application payload (copied, since the view
// returned by update() is only valid until the next call)
std::optional<std::vector<uint8_t>> receive(Client& client)
{
  auto deadline = std::chrono::steady_clock::now() + 2s;
  while (std::chrono::steady_clock::now() < deadline) {
    if (auto bytes = client.update()) {
      return std::vector<uint8_t>(bytes->begin(), bytes->end());
    }
  }
  return std::nullopt;
}

// Two solo clients on loopback that know about each other
struct LoopbackPair : ::testing::Test
{
  void SetUp() override
  {
    ASSERT_TRUE(a.init("127.0.0.1", kPortA, 50ms));
    ASSERT_TRUE(b.init("127.0.0.1", kPortB, 50ms));
    ASSERT_TRUE(a.addPeer("127.0.0.1", kPortB));
    ASSERT_TRUE(b.addPeer("127.0.0.1", kPortA));
    // Absorb the handshake Ping/Pong traffic
    for (int i = 0; i < 3; ++i) {
      (void)a.update();
      (void)b.update();
    }
  }

  static constexpr int kPortA = 40201;
  static constexpr int kPortB = 40202;
  Client a;
  Client b;
};
}  // namespace

TEST(ClientTest, DefaultConstruct)
{
  Client client;
}

TEST(ClientTest, InitLocal)
{
  Client client;
  // initSessionSolo: just binds a UDP socket, no STUN/network calls
  EXPECT_TRUE(client.init("127.0.0.1", 0));
}

TEST(ClientTest, InitLocalSocketValid)
{
  Client client;
  ASSERT_TRUE(client.init("127.0.0.1", 0));
  EXPECT_GE(client.getSocketFD(), 0);
}

TEST(AddressTest, ParsesEndpoint)
{
  auto addr = parseEndpoint("203.0.113.7:4242");
  ASSERT_TRUE(addr.has_value());
  EXPECT_EQ(ntohs(addr->sin_port), 4242);
  char ip[INET_ADDRSTRLEN] = {};
  inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
  EXPECT_STREQ(ip, "203.0.113.7");
}

TEST(AddressTest, RejectsMalformedEndpoints)
{
  EXPECT_FALSE(parseEndpoint("").has_value());
  EXPECT_FALSE(parseEndpoint("EMPTY").has_value());
  EXPECT_FALSE(parseEndpoint("1.2.3.4").has_value());
  EXPECT_FALSE(parseEndpoint("1.2.3.4:").has_value());
  EXPECT_FALSE(parseEndpoint("1.2.3.4:70000").has_value());
  EXPECT_FALSE(parseEndpoint("not.an.ip:80").has_value());
}

TEST_F(LoopbackPair, HandshakeDiscoversPeers)
{
  EXPECT_EQ(a.peerCount(), 1u);
  EXPECT_EQ(b.peerCount(), 1u);
}

TEST_F(LoopbackPair, PayloadArrivesIntact)
{
  ASSERT_TRUE(a.send(asBytes("hello juntos")));
  auto got = receive(b);
  ASSERT_TRUE(got.has_value());
  EXPECT_EQ(std::string_view(reinterpret_cast<const char*>(got->data()),
                             got->size()),
            "hello juntos");
}

// Regression: with the old text protocol an app payload of "PING" was eaten
// as a handshake message and answered with "PONG".
TEST_F(LoopbackPair, ProtocolKeywordsAreDeliveredAsData)
{
  for (std::string_view word : {"PING", "PONG", "NEW_PEER:1.2.3.4:5"}) {
    ASSERT_TRUE(a.send(asBytes(word)));
    auto got = receive(b);
    ASSERT_TRUE(got.has_value()) << word;
    EXPECT_EQ(std::string_view(reinterpret_cast<const char*>(got->data()),
                               got->size()),
              word);
  }
}

TEST_F(LoopbackPair, EmptyPayloadIsDelivered)
{
  ASSERT_TRUE(a.send({}));
  auto got = receive(b);
  ASSERT_TRUE(got.has_value());
  EXPECT_TRUE(got->empty());
}

TEST_F(LoopbackPair, RejectsOversizedPayload)
{
  std::vector<uint8_t> tooBig(kMaxAppPayload + 1, 0);
  EXPECT_FALSE(a.send(tooBig));
}

TEST_F(LoopbackPair, WaitForDataTimesOutThenWakes)
{
  EXPECT_FALSE(b.waitForData(20ms));
  ASSERT_TRUE(a.send(asBytes("wake up")));
  EXPECT_TRUE(b.waitForData(1000ms));
}
