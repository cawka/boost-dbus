Boost D-Bus
===========

This is a simple D-Bus binding powered by Boost.Asio.
As far as possible, I try to follow Asio's idioms.

Code Sample
-----------

```c++
#include <iostream>

#include <boost/asio.hpp>

#include <boost-dbus/connection.hpp>
#include <boost-dbus/message.hpp>
#include <boost-dbus/endpoint.hpp>
#include <boost-dbus/filter.hpp>
#include <boost-dbus/match.hpp>
#include <boost-dbus/functional.hpp>

int
main()
{
  boost::asio::io_service io;
  dbus::connection bus(io, dbus::bus::system);

  dbus::message msg = dbus::message::new_call(
    dbus::endpoint("org.freedesktop.DBus",
                   "/org/freedesktop/DBus",
                   "org.freedesktop.DBus"),
    "Hello");
  bus.async_send(msg, [&] (const boost::system::error_code& ec, dbus::message& reply) {

      std::cout << reply << std::endl;
      std::string replyContent;
      reply.unpack(replyContent);

      std::cout << replyContent << std::endl;
      io.stop();
    });

  io.run();
  return 0;
}
```

To compile example:

    clang++ -std=c++11 -I include/ `pkg-config dbus-1 --cflags --libs` test/example.cpp -l boost_system
