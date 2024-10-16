/* Copyright 2005 - 2016, The libsigc++ Development Team
 *  Assigned to public domain.  Use as you wish without restriction.
 */

// Write your custom tests here so you don't have to think about how to compile
// and execute your test code against the exact same library that you are
// currently hacking.

#include "testutilities.h"
#include <sigc++/sigc++.h>

namespace
{
std::ostringstream result_stream;

} // end anonymous namespace

int
main(int argc, char* argv[])
{
  auto util = TestUtilities::get_instance();

  if (!util->check_command_args(argc, argv))
    return util->get_result_and_delete_instance() ? EXIT_SUCCESS : EXIT_FAILURE;

  result_stream << "Example result string";
  util->check_result(result_stream, "Example result string");

  return util->get_result_and_delete_instance() ? EXIT_SUCCESS : EXIT_FAILURE;
}
