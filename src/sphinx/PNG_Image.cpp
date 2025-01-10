
// Implements:
#include <sphinx/PNG_Image.hpp>

namespace sphinx {

    bool
    PNG_Image::is_loaded() const {
        return (data != nullptr);
    }

    bool
    PNG_Image::is_ready_to_render() const {
        return (texture_id != -1);
    }

}
