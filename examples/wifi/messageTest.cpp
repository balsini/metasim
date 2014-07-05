#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <memory>
#include <string>

#include <metasim.hpp>
#include <simul.hpp>

#include "message.hpp"

using namespace MetaSim;

TEST_CASE( "Message", "[message]" )
{
  Node * n_src = (Node *) 0x1;
  Node * n_dst = (Node *) 0x2;
  NetInterface * i_src = (NetInterface *)0x11;
  NetInterface * i_dst = (NetInterface *)0x22;
  int id = 0x3;
  int len = 100;
  Tick trans(128);

  Message m(len, n_src, n_dst, id);
  Message m_ack(len, n_dst, n_src, id, true);

  SECTION( "Message data is correctly set" )
  {
    REQUIRE( m.isACK() == false );
    REQUIRE( m_ack.isACK() == true );

    m.transTime(trans);

    m.sourceInterface(i_src);
    m_ack.sourceInterface(i_dst);

    m.destInterface(i_dst);
    m_ack.destInterface(i_src);

    m.destInterface(i_dst);
    m_ack.destInterface(i_src);

    REQUIRE( m.transTime() == trans );
    REQUIRE( m.length() == len );
    REQUIRE( m.id() == id );
    REQUIRE( m_ack.id() == id );
    REQUIRE( m.sourceInterface() == i_src );
    REQUIRE( m.destInterface() == i_dst );
    REQUIRE( m.sourceNode() == n_src );
    REQUIRE( m.destNode() == n_dst );
  }
}
