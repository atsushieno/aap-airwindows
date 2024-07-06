
#include <exception>

namespace cmrc {
class filesystem {
};

namespace awdoc_resources {
cmrc::filesystem get_filesystem() { throw std::exception(/* unsupported platform */); }
}

}

