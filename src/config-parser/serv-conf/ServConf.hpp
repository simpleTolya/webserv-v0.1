#ifndef FT_SERV_CONFIG_HPP
# define FT_SERV_CONFIG_HPP

# include <map>
# include <string>
# include <optional>
# include <vector>
# include <list>
# include <utility>
# include "../../async-core/util/Result.hpp"

namespace ft {

class ServConf {
public:
    struct Node;
private:
    std::map<std::string, std::vector<Node>> _nodes;

    static std::list<std::string> tokenize(const std::string &s);
    static std::map<std::string, std::vector<ServConf::Node> > parse(
                    std::list<std::string> &tokens, int depth = 0);
    ServConf(std::map<std::string, std::vector<Node>> nodes) : 
                _nodes(std::move(nodes)) {}
public:
    enum class ParseErr {
        EXTRA_BRACKET,
        OPERATOR_NOT_CLOSED,
        BLOCK_IS_EMPTY,
        OPERATOR_IS_EMPTY,
        BLOCK_HAS_NO_KEY
    };
    using Error = ParseErr;

    static const char *err_description(ParseErr);

    static Res<ServConf, ParseErr> from(const std::string &str);
    static Res<ServConf, ParseErr> from(std::istream &stream);

    class Node {
        friend ServConf;

        using ArgCollection = std::vector<std::string>;
        using NodesCollection = std::map<std::string, std::vector<Node>>;
        std::vector<std::string> args;
        std::optional<NodesCollection> _nodes;

        explicit Node(ArgCollection args = {}, 
            std::optional<NodesCollection> nodes = std::nullopt) :
                args(std::move(args)), _nodes(std::move(nodes)) {};
    public:
        bool is_operator() const noexcept {
            return _nodes == std::nullopt;
        }

        const std::vector<std::string> &get_args() const noexcept {
            return args;
        }

        // the lifetime of a returned value is related to config lifetime
        std::optional<const std::vector<Node> *> get_by_key(
                            const std::string &str) const noexcept {
            if (_nodes == std::nullopt) {
                return std::nullopt;
            }

            auto &nodes = _nodes.value();
            auto it = nodes.find(str);
            if (it == nodes.end())
                return std::nullopt;
            return &it->second;
        }
    };

    const std::map<std::string, std::vector<Node>> & global() const {
        return _nodes;
    }

    std::optional<const std::vector<Node> *> get_by_key(
                        const std::string &str) const noexcept {
        auto it = _nodes.find(str);
        if (it == _nodes.end())
            return std::nullopt;
        return &it->second;
    }
};

} // namespace ft

#endif // FT_SERV_CONFIG_HPP