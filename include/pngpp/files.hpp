/**************************************************************************************************/
// PNGpp copyright 2017 Foster Brereton. See LICENSE.txt for license details.
/**************************************************************************************************/

#ifndef PNGPP_FILES_HPP__
#define PNGPP_FILES_HPP__

// boost
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

/**************************************************************************************************/

namespace pngpp {

/**************************************************************************************************/

typedef boost::filesystem::path path_t;

using boost::filesystem::canonical;

/**************************************************************************************************/

// takes ("/path/to.png", "extra") and returns "/path/to_extra.png"
path_t associated_filename(path_t src, const std::string& new_leaf);

// takes ("/path/to.png", "extra") and returns "/path/extra_to.png"
path_t derived_filename(path_t src, const std::string& new_leaf);

/**************************************************************************************************/

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_FILES_HPP__

/**************************************************************************************************/
