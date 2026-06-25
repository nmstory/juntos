#include <client.h>
#include <gtest/gtest.h>

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
