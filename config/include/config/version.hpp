#ifndef JIMAKU_EDITOR_CONFIG_VERSION_HPP
#define JIMAKU_EDITOR_CONFIG_VERSION_HPP

// include this to access application version

#include <string>

namespace version
{
    // current project version
    extern const std::string version;
    extern const std::string channel;

    // git version information
    extern const bool git_has_info;
    extern const bool git_is_dirty;
    extern const std::string git_sha1;

    // get full version string, calculated on first execution, than cached
    extern const std::string &get();
}

#endif // JIMAKU_EDITOR_CONFIG_VERSION_HPP
