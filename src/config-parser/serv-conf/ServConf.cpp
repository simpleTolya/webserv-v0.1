#include "ServConf.hpp"
#include <set>
#include <fstream>
#include <streambuf>
#include <iostream>
namespace ft {

std::list<std::string> ServConf::tokenize(const std::string &s) {
    std::string str;
    std::list<std::string> tokens;

    std::set<char> keychars = {' ', '\n', '\t', '#', '{', '}', ';'};

    auto add_token = [&str, &tokens](){
        if (not str.empty())
            tokens.push_back(std::move(str));
    };
    for (size_t i = 0; i < s.size(); ++i) {
        if (not keychars.contains(s[i])) {
            str.push_back(s[i]);
        }
        else if (s[i] == ' ' or s[i] == '\n' or s[i] == '\t') {
            add_token();
        }
        else if (s[i] == '#') {
            add_token();
            while (i < s.size() and s[i] != '\n') {
                str += s[i];
                i++;
            }
            add_token();
        }
        else { // s[i] in ['{', '}', ';'] 
            add_token();
            tokens.push_back(std::string{s[i]});
        }
    }
    add_token();
    return tokens;
}

std::map<std::string, std::vector<ServConf::Node> >
        ServConf::parse(std::list<std::string> &tokens, int depth) {
    std::list<std::string> curr_tokens;
    std::map<std::string, std::vector<ServConf::Node> > nodes;

    auto it = tokens.begin();
    while (it != tokens.end()) {
        if ((*it)[0] == '#') {
            it = tokens.erase(it);
            continue; // skip comment
        }
        
        else if ((*it)[0] == '{') {
            if (curr_tokens.empty()) {
                throw ParseErr::BLOCK_HAS_NO_KEY;
            }
            
            it = tokens.erase(it);
            auto res = parse(tokens, depth + 1);
            it = tokens.begin();

            if (res.empty()) {
                throw ParseErr::BLOCK_IS_EMPTY;
            }

            std::string key = std::move(curr_tokens.front());
            curr_tokens.pop_front();
            std::vector<std::string> args(
                        curr_tokens.begin(), curr_tokens.end());
            curr_tokens.clear();
            nodes[std::move(key)].push_back(
                    Node(std::move(args), std::move(res)));
        }

        else if ((*it)[0] == '}') {
            if (depth == 0) {
                throw ParseErr::EXTRA_BRACKET;
            }
            it = tokens.erase(it);
            return nodes;
        }

        else if ((*it)[0] == ';') {
            it = tokens.erase(it);
            if (curr_tokens.empty()) {
                throw ParseErr::OPERATOR_IS_EMPTY;
            }
            std::string key = std::move(curr_tokens.front());
            curr_tokens.pop_front();
            std::vector<std::string> args(
                        curr_tokens.begin(), curr_tokens.end());
            curr_tokens.clear();
            nodes[std::move(key)].push_back(Node(std::move(args)));
        }
        else { // is word
            curr_tokens.splice(curr_tokens.end(), tokens, it++);
        }
    }
    if (not curr_tokens.empty()) {
        throw ParseErr::OPERATOR_NOT_CLOSED;
    }
    return nodes;
}

Res<ServConf, ServConf::ParseErr> ServConf::from(const std::string &str) {
    using _Result = Res<ServConf, ServConf::ParseErr>;

    auto tokens = tokenize(str);
    try {
        return _Result(std::move(ServConf::parse(tokens)));
    } catch (ServConf::ParseErr err) {
        return _Result(err);
    }
}

Res<ServConf, ServConf::ParseErr> ServConf::from(std::istream &stream) {
    std::string str((std::istreambuf_iterator<char>(stream)),
                 std::istreambuf_iterator<char>());
    return ServConf::from(str);
}

const char* ServConf::err_description(ParseErr e) {
    static const char* description[] = {
        "extra bracket",
        "operator not closed",
        "block is empty",
        "operator is empty",
        "block has no key"
    };
    return description[static_cast<int>(e)];
}

} // namespace ft
