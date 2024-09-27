#!/bin/sh
#
# shUnit2 example for mocking files.

MOCK_PASSWD=''  # This will be overridden in oneTimeSetUp().

test_mount_once() {
  result=`chroot_enter`
  assertEquals 'unexpected root uid' '' "${result}"
}

oneTimeSetUp() {
  # Provide a mock passwd file for testing. This will be cleaned up
  # automatically by shUnit2.

  # Load script under test.
  . './../../bin/shell/v2c_env.sh'
}

# Load and run shUnit2.
[ -n "${ZSH_VERSION:-}" ] && SHUNIT_PARENT=$0
. ../shunit2
