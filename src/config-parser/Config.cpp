#include "Config.hpp"
#include "serv-conf/ServConf.hpp"

namespace ft {

static Res<std::string, Config::Error> get_operator_single_val(
        const std::string &key, const ft::ServConf::Node &node, 
        std::string default_val) {
    using _Error = Config::Error;
    using _Result = Res<std::string, _Error>;

    auto value_opt = node.get_by_key(key);
    if (not value_opt.has_value())
        return _Result(std::move(default_val));

    auto &val_blocks = *value_opt.value();
    if (val_blocks.size() != 1)
        return _Result(_Error("several blocks " + key));
    
    auto &val = val_blocks[0];
    if (not val.is_operator())
        return _Result(_Error(key + " must be an operator"));
    
    auto val_args = val.get_args();
    if (val_args.size() != 1)
        return _Result(_Error(key + " has more than one arg"));

    return _Result(std::move(val_args[0]));
}

static Res<std::vector<std::string>, Config::Error> get_operator_args(
        const std::string &key, const ft::ServConf::Node &node) {
    using _Error = Config::Error;
    using _Result = Res<std::vector<std::string>, _Error>;

    auto value_opt = node.get_by_key(key);
    if (not value_opt.has_value())
        return _Result(std::vector<std::string>{});

    auto &val_blocks = *value_opt.value();
    if (val_blocks.size() != 1)
        return _Result(_Error("several blocks " + key));
    
    auto &val = val_blocks[0];
    if (not val.is_operator())
        return _Result(_Error(key + " must be an operator"));

    return _Result(std::move(val.get_args()));
}

static Res< std::map<std::string, Config::Server::Location>, 
        Config::Error> get_location(const ft::ServConf::Node &node) {
    using _Error = Config::Error;
    using _Result = Res<std::map<std::string, 
                        Config::Server::Location>, _Error>;

    auto location_opt = node.get_by_key("location");
    if (not location_opt.has_value())
        return _Result(_Error("no location in server block"));

    std::map<std::string, Config::Server::Location> locs;
    auto &locations = *location_opt.value();
    for (const auto &location : locations) {
        if (location.is_operator())
            return _Result(_Error("location is not operator"));
        
        auto args = location.get_args();
        if (args.size() != 1)
            return _Result(_Error("location arg count error"));
        
        Config::Server::Location loc;
        auto root = get_operator_single_val("root", location, "");
        if (root.is_err())
            return _Result(root.get_err());
        loc.root = std::move(root.get_val());

        auto proxy_pass =get_operator_single_val("proxy_pass", location, "");
        if (proxy_pass.is_err())
            return _Result(proxy_pass.get_err());
        loc.proxy_pass = std::move(proxy_pass.get_val());

        auto cgi = get_operator_single_val("cgi", location, "");
        if (cgi.is_err())
            return _Result(cgi.get_err());
        loc.cgi_path = std::move(cgi.get_val());

        auto methods = get_operator_args("accepted_method", location);
        if (methods.is_err())
            return _Result(methods.get_err());
        auto v = std::move(methods.get_val());
        loc.accepted_methods = std::set<std::string>(v.begin(), v.end());

        locs[args[0]] = std::move(loc);
    }
    return _Result(locs);
}

Res<Config, Config::Error> Config::from(const ServConf &serv_conf) {
    using _Result = Res<Config, Config::Error>;
    using _Error = Config::Error;

    Config conf;
    auto http_opt = serv_conf.get_by_key("http");
    if (not http_opt.has_value())
        return _Result(_Error("no http block"));
    
    auto& http_blocks = *http_opt.value();
    for (const auto &http : http_blocks) {

        auto servers_opt = http.get_by_key("server");
        if (not servers_opt.has_value())
            continue;
        
        auto &servers = *servers_opt.value();

        for (const auto &server : servers) {
            Config::Server conf_server;

            auto _listen_res = get_operator_single_val(
                                "listen", server, "80");
            if (_listen_res.is_err())
                return _Result(_listen_res.get_err());
            conf_server._listen = std::move(_listen_res.get_val());

            auto _server_name_res = get_operator_args(
                                "server_name", server);
            if (_server_name_res.is_err())
                return _Result(_server_name_res.get_err());
            conf_server.server_name = std::move(_server_name_res.get_val());

            auto _location_res = get_location(server);
            if (_location_res.is_err())
                return _Result(_location_res.get_err());
            conf_server.locations = std::move(_location_res.get_val());

            conf.servers[conf_server._listen].push_back(std::move(conf_server));
        }
    }
    if (conf.servers.empty())
        return _Result(_Error("no server block in http block"));
    return _Result(std::move(conf));
}


} // namespace ft
