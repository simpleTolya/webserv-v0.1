#include "stream_wrappers.hpp"

std::recursive_mutex ft::cout_wrapper::lock_cout;
std::recursive_mutex ft::cerr_wrapper::lock_cerr;
