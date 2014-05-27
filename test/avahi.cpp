// Copyright (c) Benjamin Kietzman (github.com/bkietz)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <gtest/gtest.h>
#include <dbus/connection.hpp>
#include <dbus/message.hpp>
#include <dbus/filter.hpp>
#include <dbus/match.hpp>
#include <dbus/functional.hpp>
#include <unistd.h>


class AvahiTest
  : public testing::Test
{
protected:
  static void SetUpTestCase()
  {
  }
  static boost::asio::io_service io;
  static dbus::connection system_bus;
  static dbus::string browser_path;
};
// It seems like these should be non-static,
// but I get a mysterious SEGFAULT for io
//   ¿related: http://stackoverflow.com/questions/18009156/boost-asio-segfault-no-idea-why
// and a C++ exception with description
// "assign: File exists" for system_bus
// (probably indicates I should upgrade connection's constructor)
boost::asio::io_service AvahiTest::io;
dbus::connection AvahiTest::system_bus(io, dbus::bus::system);
dbus::string AvahiTest::browser_path;


TEST_F(AvahiTest, GetHostName)
{
  using namespace boost::asio;
  using namespace dbus;
  using boost::system::error_code;

  string avahi_hostname;
  string unix_hostname;

  {
    // get hostname from a system call
    char c[1024];
    gethostname(c, 1024);
    unix_hostname = c;
  }

  // get hostname from the Avahi daemon
  message m = message::new_call(
    "org.freedesktop.Avahi",
    "/",
    "org.freedesktop.Avahi.Server",
    "GetHostName");

  system_bus.async_send(m, [&](error_code ec, message r){  
    r.unpack(avahi_hostname);

    // this is only usually accurate
    ASSERT_EQ(unix_hostname, avahi_hostname);

    // eventually, connection should stop itself
    io.stop();
  });

  io.run();
}


TEST_F(AvahiTest, ServiceBrowser)
{
  using namespace boost::asio;
  using namespace dbus;
  using boost::system::error_code;

  // create new service browser
  message m = message::new_call(
    "org.freedesktop.Avahi",
    "/",
    "org.freedesktop.Avahi.Server",
    "ServiceBrowserNew");

  m.pack<int32>(-1)
   .pack<int32>(-1)
   .pack<string>("_http._tcp")
   .pack<string>("local")
   .pack<uint32>(0);

  message r = system_bus.send(m);

  r.unpack(browser_path);
  
  // RegEx match browser_path
  // catch a possible exception
}


TEST_F(AvahiTest, BrowseForHttp)
{
  using namespace boost::asio;
  using namespace dbus;
  using boost::system::error_code;

  match m(system_bus, "type='signal',path='" + browser_path + "'");
  filter f(system_bus, [](message& m){
    return m.get_member() == "ItemNew"; });

  function<void(error_code, message)> h;
  h = [&] (error_code ec, message m) {};
  f.async_dispatch(h);
  io.run();
}