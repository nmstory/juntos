#include <client.h>
#include <gtest/gtest.h>

TEST(ClientTest, DefaultConstruct)
{
  Client client;
}

TEST(ClientTest, InitAsServer)
{
  Client client;
  // Port 0 lets the OS assign an available port
  EXPECT_TRUE(client.init(0));
}

TEST(ClientTest, InitAsServerSocketValid)
{
  Client client;
  ASSERT_TRUE(client.init(0));
  EXPECT_GE(client.getSocketFD(), 0);
}
