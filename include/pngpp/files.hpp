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

} // namespace pngpp

/**************************************************************************************************/

#endif // PNGPP_FILES_HPP__

/**************************************************************************************************/