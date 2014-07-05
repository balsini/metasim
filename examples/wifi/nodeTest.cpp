#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <string>

#include <metasim.hpp>
#include <simul.hpp>

#include "message.hpp"
#include "node.hpp"

using namespace MetaSim;

TEST_CASE( "Node Test", "[node]" )
{
  auto at = std::make_shared<UniformVar>(50,1024);

  Source * s = new Source(std::string("src1"), std::make_pair(0.0, 0.0), at);
  Node * d = new Node(std::string("dst1"), std::make_pair(0.0, 1.0));

  WifiInterface * s_int = new WifiInterface("s_int", 1.1, s);
  WifiInterface * d_int = new WifiInterface("d_int", 1.1, d);

  WifiLink * s_lnk = new WifiLink("s_lnk");
  WifiLink * d_lnk = new WifiLink("s_lnk");

  s_int->link(s_lnk);
  d_int->link(d_lnk);

  s_lnk->addInterface(d_int);
  d_lnk->addInterface(s_int);

  REQUIRE_THROWS( s->produce() );

  s->netInterface(s_int);
  d->netInterface(d_int);

  REQUIRE_THROWS( s->produce() );

  s->addDest(d);

  // Checks if message is correctly consumed
  REQUIRE( d->consumed() == 0 );

  Message * msg = new Message(10, s, d, 0, false);
  auto msg_unique = std::unique_ptr<Message>(msg);

  d->put(msg_unique);

  REQUIRE( d->consumed() == 1 );

  SECTION( "Node produces the exact number of messages" )
  {
    SIMUL.initSingleRun();

    // First message has to be produced

    REQUIRE( s->produced() == 0 );

    SIMUL.sim_step();

    REQUIRE( s->produced() == 1 );

    // Exactly 100 messages must be produced

    while ( s->produced() < 100) {
      SIMUL.sim_step();
    }

    for (unsigned int i=0; i<50000; i++) {
      SIMUL.sim_step();
    }

    REQUIRE( s->produced() == 100 );

    SIMUL.endSingleRun();
  }
}
