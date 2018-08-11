#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>

#include "../../../src/include/poac/core/inference.hpp"
#include "../../../src/include/poac/core/except.hpp"


BOOST_AUTO_TEST_CASE( poac_core_inference_test1 )
{
    using namespace std;
    using namespace poac::core::inference;
    using namespace poac::core::except;

    BOOST_CHECK_THROW(apply("exec", "nothing", vector<string>()), invalid_first_arg);
}
