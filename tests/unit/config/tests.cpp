#include <config-parser/serv-conf/ServConf.hpp>
#include <config-parser/Config.hpp>
#include <simple_test_lib.hpp>

#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include <streambuf>

using namespace ft;

TEST_CASE(correct) {
    auto file = std::ifstream("/home/anatoliy/projects/cpp/webserv/tests/unit/config/correct.conf");

    ASSERT_TRUE(static_cast<bool>(file))

    auto re = ServConf::from(file);
    ASSERT_TRUE(re.is_ok())

    ServConf conf = std::move(re.get_val());

    auto http_opt = conf.get_by_key("http");

    ASSERT_TRUE(http_opt.has_value());

    auto& http_blocks = *http_opt.value();
    ASSERT_TRUE(http_blocks.size() == 1)

    auto& http_block = http_blocks[0];

    ASSERT_TRUE(http_block.get_args().empty())

    auto server_opt = http_block.get_by_key("server");
    ASSERT_TRUE(server_opt.has_value())

    auto& server_blocks = *server_opt.value();
    ASSERT_TRUE(server_blocks.size() == 3)

    { // first server
        auto& server = server_blocks[0];
        ASSERT_TRUE(server.get_args().empty())
        ASSERT_TRUE(!server.is_operator())

        auto location_opt = server.get_by_key("location");
        ASSERT_TRUE(location_opt.has_value())
        
        auto &locations = *location_opt.value();
        // ASSERT_TRUE(locations.size() == 2)
        {
            auto &location = locations[0];
            ASSERT_TRUE(!server.get_args().size() == 1)
            ASSERT_TRUE(!server.is_operator())
            ASSERT_TRUE(location.get_args() == std::vector<std::string>{"/"});

            auto root_opt = location.get_by_key("root");
            ASSERT_TRUE(root_opt.has_value())

            auto &roots = *root_opt.value();
            ASSERT_TRUE(roots.size() == 1)
            ASSERT_TRUE(roots[0].is_operator())
            ASSERT_TRUE(roots[0].get_args() == std::vector<std::string>{"/data/www"})
        }

        {
            auto &location = locations[1];
            ASSERT_TRUE(!server.get_args().size() == 1)
            ASSERT_TRUE(!server.is_operator())
            ASSERT_TRUE(location.get_args() == std::vector<std::string>{"/images/"});

            auto root_opt = location.get_by_key("root");
            ASSERT_TRUE(root_opt.has_value())

            auto &roots = *root_opt.value();
            ASSERT_TRUE(roots.size() == 1)
            ASSERT_TRUE(roots[0].is_operator())
            ASSERT_TRUE(roots[0].get_args() == std::vector<std::string>{"/data"})
        }
    }


    { // second server
        auto& server = server_blocks[1];
        ASSERT_TRUE(server.get_args().empty())
        ASSERT_TRUE(!server.is_operator())

        auto location_opt = server.get_by_key("location");
        ASSERT_TRUE(location_opt.has_value())
        
        auto &locations = *location_opt.value();
        ASSERT_TRUE(locations.size() == 1)

        {
            auto &location = locations[0];
            ASSERT_TRUE(!server.get_args().size() == 1)
            ASSERT_TRUE(!server.is_operator())
            ASSERT_TRUE(location.get_args() == std::vector<std::string>{"/"});

            auto root_opt = location.get_by_key("proxy_pass");
            ASSERT_TRUE(root_opt.has_value())

            auto &roots = *root_opt.value();
            ASSERT_TRUE(roots.size() == 1)
            ASSERT_TRUE(roots[0].is_operator())
            ASSERT_TRUE(roots[0].get_args() == std::vector<std::string>{"http://localhost:8080"})
        }
    }

    { // third server
        auto& server = server_blocks[2];
        ASSERT_TRUE(server.get_args().empty())
        ASSERT_TRUE(!server.is_operator())

        auto location_opt = server.get_by_key("location");
        ASSERT_TRUE(location_opt.has_value())
        
        auto &locations = *location_opt.value();
        ASSERT_TRUE(locations.size() == 1)

        {
            auto &location = locations[0];
            ASSERT_TRUE(!server.get_args().size() == 1)
            ASSERT_TRUE(!server.is_operator())
            ASSERT_TRUE(location.get_args() == std::vector<std::string>{"/"});

            auto root_opt = location.get_by_key("cgi");
            ASSERT_TRUE(root_opt.has_value())

            auto &roots = *root_opt.value();
            ASSERT_TRUE(roots.size() == 1)
            ASSERT_TRUE(roots[0].is_operator())
            ASSERT_TRUE(roots[0].get_args() == std::vector<std::string>{"/some/path"})
        }
    }

    {
        auto res = Config::from(conf);
        ASSERT_TRUE(res.is_ok())

        auto config = std::move(res.get_val());
        ASSERT_TRUE(config.servers.size() == 3)
    }
}

TEST_CASE(error1) {
    auto file = std::ifstream("/home/anatoliy/projects/cpp/webserv/tests/unit/config/error1.conf");

    ASSERT_TRUE(static_cast<bool>(file))

    auto res = ServConf::from(file);
    ASSERT_TRUE(res.is_err())
    ASSERT_TRUE(res.get_err() == ServConf::Error::BLOCK_IS_EMPTY)
}

TEST_CASE(error2) {
    auto file = std::ifstream("/home/anatoliy/projects/cpp/webserv/tests/unit/config/error2.conf");

    ASSERT_TRUE(static_cast<bool>(file))

    auto res = ServConf::from(file);
    ASSERT_TRUE(res.is_err())
    ASSERT_TRUE(res.get_err() == ServConf::Error::OPERATOR_IS_EMPTY)
}


TEST_CASE(error3) {
    auto file = std::ifstream("/home/anatoliy/projects/cpp/webserv/tests/unit/config/error3.conf");

    ASSERT_TRUE(static_cast<bool>(file))

    auto res = ServConf::from(file);
    ASSERT_TRUE(res.is_err())
    ASSERT_TRUE(res.get_err() == ServConf::Error::EXTRA_BRACKET)
}


TEST_CASE(error4) {
    auto file = std::ifstream("/home/anatoliy/projects/cpp/webserv/tests/unit/config/error4.conf");

    ASSERT_TRUE(static_cast<bool>(file))

    auto res = ServConf::from(file);
    ASSERT_TRUE(res.is_err())
    ASSERT_TRUE(res.get_err() == ServConf::Error::BLOCK_HAS_NO_KEY)
}

TEST_CASE(error5) {
    auto file = std::ifstream("/home/anatoliy/projects/cpp/webserv/tests/unit/config/error5.conf");

    ASSERT_TRUE(static_cast<bool>(file))

    auto res = ServConf::from(file);
    ASSERT_TRUE(res.is_err())
    ASSERT_TRUE(res.get_err() == ServConf::Error::OPERATOR_NOT_CLOSED)
}
