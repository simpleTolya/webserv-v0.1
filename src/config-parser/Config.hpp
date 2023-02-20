#ifndef FT_CONFIG_HPP
# define FT_CONFIG_HPP

# include <string>
# include <set>
# include <map>
# include <vector>
# include <async-core/util/Result.hpp>

namespace ft {

struct ServConf;

struct Config {
    struct Error {
        std::string cause;
    };

    struct Server {
        std::string _listen;
        std::vector<std::string> server_name;

        struct Location {
            std::set<std::string> accepted_methods;
            std::string root;
            std::string proxy_pass;
            std::string cgi_path;
        };

        // path -> Location
        std::map<std::string, Location> locations;
    };

    // listen -> server block
    std::map<std::string, std::vector<Server>> servers;

    static Res<Config, Error> from(const ServConf &);
};

} // namespace ft

#endif // FT_CONFIG_HPP