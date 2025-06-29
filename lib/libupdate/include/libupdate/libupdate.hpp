#ifndef LIBUPDATE_HPP
#define LIBUPDATE_HPP

#include <atomic>
#include <map>
#include <mutex>
#include <string>

namespace libupdate {
    enum state {
        NONE, DOWNLOAD, UPDATE, COMPILE, PAUSE, ERROR,
    };

    struct progress {
        enum state  state;
        double percentage;
    };

    class update {
        std::mutex _mutex    = {};
        progress   _progress = {};
        std::atomic<bool> _paused = false;
        std::map<std::string, uint32_t> _indexes = {};
        void update_manifest();
    public:
        [[nodiscard]]
        progress get_progress() const noexcept;


        void initiate();
        void terminate();
        void pause(bool val);
    };
} // namespace libupdate

#endif // LIBUPDATE_HPP