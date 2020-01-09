#include <config/version_private.hpp>

#include <sstream>

const std::string &version::get()
{
    // only execute this code once
    static const std::string calculated_version_string = ([&]{
        using namespace version;
        std::stringstream v;

        // append base version
        v << version;

        // append channel if not empty
        if (!channel.empty())
        {
            v << "-" << channel;
        }

        // append git version if present
        if (git_has_info)
        {
            // append short SHA1 commit hash
            v << "-" << git_sha1.substr(0, 8);

            // append dirty marker on uncommitted changes
            if (git_is_dirty)
            {
                v << "-dirty";
            }
        }

        return v.str();
    })();

    return calculated_version_string;
}
