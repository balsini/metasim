#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <string>

#include <metasim.hpp>
#include <simul.hpp>

#include "message.hpp"
#include "node.hpp"
#include "netinterface.hpp"

using namespace MetaSim;

TEST_CASE( "Node Test", "[node]" )
{
  auto interfacesTrace = std::unique_ptr<WifiTrace>(new WifiTrace("traces/netInterfacesTraceNode"));

  auto at = std::make_shared<DeltaVar>(1);
  auto period = std::make_shared<DeltaVar>(1);

  auto s = std::unique_ptr<Source>(new Source(std::string("src1"), std::make_pair(0, 0), at));

  s->setInterval(period);

  auto d = std::unique_ptr<Node>(new Node(std::string("dst1"), std::make_pair(0, 1)));

  auto s_int = std::unique_ptr<WifiInterface>(new WifiInterface("s_int", 1.1, s.get()));
  auto d_int = std::unique_ptr<WifiInterface>(new WifiInterface("d_int", 1.1, d.get()));

  s->netInterface(std::move(s_int));
  d->netInterface(std::move(d_int));

  s->netInterface()->addTrace(interfacesTrace.get());
  d->netInterface()->addTrace(interfacesTrace.get());

  auto s_lnk = std::unique_ptr<WifiLink>(new WifiLink("s_lnk"));
  auto d_lnk = std::unique_ptr<WifiLink>(new WifiLink("d_lnk"));

  s->netInterface()->link(std::move(s_lnk));
  d->netInterface()->link(std::move(d_lnk));

  REQUIRE_THROWS( s->produce() );

  s->netInterface()->link()->addInterface(d->netInterface());
  d->netInterface()->link()->addInterface(s->netInterface());

  s->addDest(d.get());

  // Checks if message is correctly consumed

  auto msg = std::unique_ptr<Message>(new Message(10,
                                                  s.get(),
                                                  d.get(),
                                                  0,
                                                  false));

  REQUIRE( d->consumed() == 0 );

  for (unsigned int i=0; i<128; ++i)
    d->put(msg);

  REQUIRE( d->consumed() == 128 );

  SECTION( "Node produces the exact number of messages" )
  {
    SIMUL.initSingleRun();

    // First message has to be produced

    REQUIRE( s->produced() == 0 );

    SIMUL.sim_step();

    REQUIRE( s->produced() == 1 );

    SIMUL.sim_step();

    REQUIRE( s->produced() == 2 );

    // Exactly 100 messages must be produced

    while ( s->produced() < 100)
      SIMUL.sim_step();

    REQUIRE( s->produced() == 100 );

    SIMUL.run_to(500000);

    REQUIRE( s->produced() == 100 );

    REQUIRE_THROWS( SIMUL.getNextEventTime() );

    REQUIRE( s->produced() == 100 );

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}
