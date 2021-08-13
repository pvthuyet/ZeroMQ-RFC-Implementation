#include "zmqutil.hpp"
#include "logger/logger_define.hpp"
#include <string>
#include <sstream>
#include <fstream>

SAIGON_NAMESPACE_BEGIN
namespace zmqutil
{
	void dump(const zmqpp::message_t& msg)
	{
        std::ostringstream oss;
        oss << "\n--------------------------------------\n";
        for (unsigned int part_nbr = 0; part_nbr < msg.parts(); part_nbr++) {
            auto data = msg.get<std::string>(part_nbr);

            // Dump the message as text or binary
            int is_text = 1;
            for (unsigned int char_nbr = 0; char_nbr < data.size(); char_nbr++)
                if (data[char_nbr] < 32 || data[char_nbr] > 127)
                    is_text = 0;

            oss << "[" << std::setw(3) << std::setfill('0') << (int)data.size() << "] ";
            for (unsigned int char_nbr = 0; char_nbr < data.size(); char_nbr++) {
                if (is_text) {
                    oss << (char)data[char_nbr];
                }
                else {
                    oss << std::hex << std::setw(2) << std::setfill('0') << (short int)data[char_nbr];
                }
            }
            oss << std::endl;
        }
        SPDLOG_DEBUG(oss.str());
	}

    void save(std::string_view path, const zmqpp::message_t& msg)
    {
        std::ofstream ofs(path.data(), std::ios::out);
        for (int i = 0; i < msg.parts(); ++i) {
            ofs << msg.get<std::string>(i) << std::endl;
        }
    }

    zmqpp::message_t load(std::string_view path)
    {
        zmqpp::message_t msg;
        std::ifstream ifs(path.data(), std::ios::in);
        for (std::string line; std::getline(ifs, line);) {
            msg << line;
        }
        return msg;
    }
}
SAIGON_NAMESPACE_END