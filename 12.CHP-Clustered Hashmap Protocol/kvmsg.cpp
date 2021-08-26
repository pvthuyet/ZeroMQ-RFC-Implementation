#include "kvmsg.hpp"
#include "gsl/gsl_assert"
#include <sstream>
#include <charconv>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include "sgutils/random_factor.hpp"

SAIGON_NAMESPACE_BEGIN

kvmsg::kvmsg(size_t seq)
{
	set_sequence(seq);
}

void kvmsg::set_sequence(size_t seq)
{
	auto fmt = std::format("{{0:0{}d\}}", FRAME_SEQ_LEN);
	auto strseq = std::format(fmt, seq);
	m_present[FRAME_SEQ] = true;
	m_frame[FRAME_SEQ] = strseq;
}

size_t kvmsg::get_sequence() const
{
	if (m_present[FRAME_SEQ]) {
		auto& fseq = m_frame[FRAME_SEQ];
		Expects(FRAME_SEQ_LEN == fseq.length());
		size_t rs{};
		auto [ptr, ec] = std::from_chars(std::data(fseq), std::data(fseq) + std::size(fseq), rs);
		return rs;
	}
	return 0;
}

std::string kvmsg::get_key() const
{
	if (m_present[FRAME_KEY]) {
		return m_frame.at(FRAME_KEY);
	}
	return {};
}

void kvmsg::set_key(std::string_view key)
{
	m_frame[FRAME_KEY] = key;
	m_present[FRAME_KEY] = true;
}

std::string kvmsg::get_body() const
{
	if (m_present[FRAME_BODY]) {
		return m_frame[FRAME_BODY];
	}
	return {};
}

void kvmsg::set_body(std::string_view body)
{
	m_frame[FRAME_BODY] = body;
	m_present[FRAME_BODY] = true;
}

size_t kvmsg::size() const
{
	if (m_present[FRAME_BODY]) {
		return m_frame[FRAME_BODY].length();
	}
	return 0;
}

std::string kvmsg::get_UUID() const
{
	if (m_present[FRAME_UUID]) {
		return m_frame[FRAME_UUID];
	}
	return {};
}

void kvmsg::set_UUID(std::string_view uuid)
{
	m_present[FRAME_UUID] = true;
	m_frame[FRAME_UUID] = uuid.empty() ?
		sg::random_factor{}.ramdom_uuid<std::string>() :
		uuid;
}

std::string kvmsg::get_prop(std::string const& name) const
{
	return m_props.at(name);
}

void kvmsg::add_prop(std::string const& name, std::string const& val)
{
	m_props[name] = val;
}

void kvmsg::set_prop(std::string const& prop)
{
	m_frame[FRAME_PROPS] = prop;
	m_present[FRAME_PROPS] = true;
}

kvmsg kvmsg::recv(zmqpp::socket_t& sock)
{
	kvmsg ret(0);
	zmqpp::message_t msg;
	sock.receive(msg);
	Expects(msg.parts() == KVMSG_FRAMES);

	ret.set_key(msg.get<std::string>(FRAME_KEY));
	ret.set_sequence(std::stoull(msg.get<std::string>(FRAME_SEQ)));
	ret.set_UUID(msg.get<std::string>(FRAME_UUID));
	ret.set_prop(msg.get<std::string>(FRAME_PROPS));
	ret.set_body(msg.get<std::string>(FRAME_BODY));
	ret.decode_props();

	return ret;
}

void kvmsg::send(zmqpp::socket_t& sock)
{
	zmqpp::message_t msg;
	encode_props();
	for (auto& f : m_frame) {
		msg.push_back(f);
	}
	sock.send(msg);
}

kvmsg kvmsg::dup() const
{
	kvmsg copy(*this);
	return copy;
}

void kvmsg::dump() const
{
	std::stringstream ss;
	ss << std::format("[seq:{}]\n", get_sequence());
	ss << std::format("[size:{}]\n", size());
	ss << "[";
	for (auto& [k, v] : m_props) {
		ss << std::format("{}={};", k, v);
	}
	ss << "]\n";

	ss << get_body();
	ss << '\n';
	std::cout << ss.str();
}

void kvmsg::encode_props()
{
	std::stringstream ss;
	boost::archive::text_oarchive oa(ss);
	oa << m_props;
	m_present[FRAME_PROPS] = true;
	m_frame[FRAME_PROPS] = ss.str();
}

void kvmsg::decode_props()
{
	m_props.clear();
	if (!m_frame[FRAME_PROPS].empty()) {
		std::stringstream ss(m_frame[FRAME_PROPS]);
		boost::archive::text_iarchive ia(ss);
		ia >> m_props;
	}
}

SAIGON_NAMESPACE_END