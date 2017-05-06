/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

// application
#include <pngpp/files.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

path_t associated_filename(path_t src, const std::string& new_leaf) {
    // takes ("/path/to.png", "extra") and returns "/path/to_extra.png"
    std::string stem(src.stem().string());
    std::string extension(src.extension().string());

    src.remove_filename();

    std::string new_filename(stem + "_" + new_leaf + extension);

    return src / new_filename;
}

/**************************************************************************************************/

path_t derived_filename(path_t src, const std::string& new_leaf) {
    // takes ("/path/to.png", "extra") and returns "/path/extra_to.png"
    std::string leaf(src.leaf().string());

    src.remove_filename();

    std::string new_filename(new_leaf + "_" + leaf);

    return src / new_filename;
}

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/
