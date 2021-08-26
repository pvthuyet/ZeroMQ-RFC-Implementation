#include "kvmsg.hpp"
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(kvmsg_class, contructor)
{
	size_t seq = 1;
	sg::kvmsg msg(seq);
	ASSERT_EQ(seq, msg.get_sequence());
}