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

void kvmsg::set_UUID()
{
	m_present[FRAME_UUID] = true;
	m_frame[FRAME_UUID] = sg::random_factor{}.ramdom_uuid<std::string>();
}

std::string kvmsg::get_prop(std::string const& name) const
{
	return m_props.at(name);
}

void kvmsg::set_prop(std::string const& name, std::string const& val)
{
	m_props[name] = val;
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