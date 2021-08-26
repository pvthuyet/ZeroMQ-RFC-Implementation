#include "kvmsg.hpp"
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(kvmsg_class, test_simple)
{
	zmqpp::context_t ctx;
	zmqpp::socket_t output(ctx, zmqpp::socket_type::dealer);
	output.bind("tcp://*:5555");
	zmqpp::socket_t input(ctx, zmqpp::socket_type::dealer);
	input.connect("tcp://localhost:5555");

	// test send/receive simple message
	sg::kvmsg msg1(1);
	msg1.set_key("getKey");
	msg1.set_UUID("");
	msg1.set_body("body");
	msg1.dump();
	msg1.send(output);

	auto msgrecv1 = sg::kvmsg::recv(input);
	msgrecv1.dump();
	ASSERT_EQ(msg1.get_key(), msgrecv1.get_key());
}

TEST(kvmsg_class, test_properties)
{
	zmqpp::context_t ctx;
	zmqpp::socket_t output(ctx, zmqpp::socket_type::dealer);
	output.bind("tcp://*:5555");
	zmqpp::socket_t input(ctx, zmqpp::socket_type::dealer);
	input.connect("tcp://localhost:5555");

	// test send/receive simple message
	sg::kvmsg msg(2);
	msg.add_prop("prop1", "value1");
	msg.add_prop("prop2", "value1");
	msg.add_prop("prop2", "value2");
	msg.set_key("get_key");
	msg.set_UUID("");
	msg.set_body("body");
	ASSERT_EQ(msg.get_prop("prop2"), "value2");
	msg.dump();
	msg.send(output);

	auto rmsg = sg::kvmsg::recv(input);
	rmsg.dump();
	ASSERT_EQ(msg.get_key(), rmsg.get_key());
	ASSERT_EQ(rmsg.get_prop("prop2"), "value2");
}