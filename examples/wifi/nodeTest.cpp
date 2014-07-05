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

  auto s = std::make_shared<Source>(std::string("src1"), std::make_pair(0, 0), at);
  auto ss = static_pointer_cast<Node>(s);
  auto d = std::make_shared<Node>(std::string("dst1"), std::make_pair(0, 1));

  auto s_int = std::make_shared<WifiInterface>("s_int", 1.1, ss);
  auto d_int = std::make_shared<WifiInterface>("d_int", 1.1, d);

  auto s_lnk = std::make_shared<WifiLink>("s_lnk");
  auto d_lnk = std::make_shared<WifiLink>("d_lnk");

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

  auto msg = std::unique_ptr<Message>(new Message(10,
                                                  s.get(),
                                                  d.get(),
                                                  0,
                                                  false));

  d->put(msg);

  REQUIRE( d->consumed() == 1 );

  SECTION( "Node produces the exact number of messages" )
  {
    SIMUL.initSingleRun();

    // First message has to be produced

    REQUIRE( s->produced() == 0 );

    SIMUL.sim_step();

    REQUIRE( s->produced() == 1 );

    // Exactly 100 messages must be produced

    while ( s->produced() < 100)
      SIMUL.sim_step();

    SIMUL.run(500000);

    REQUIRE( s->produced() == 100 );

    SIMUL.endSingleRun();
  }
}
