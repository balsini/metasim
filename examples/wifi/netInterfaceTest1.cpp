#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <vector>
#include <string>

#include <metasim.hpp>
#include <simul.hpp>
#include <trace.hpp>

#include "link.hpp"
#include "netinterface.hpp"

using namespace MetaSim;

TEST_CASE( "netInterface Test, simple communication", "[netInterfaceCommunication]" )
{
  /*
   * Network Organization:
   *
   *
   * S <---> D
   *
   *
   * Source sends messages to Node. Node receives the message and
   * consumes it.
   */

  auto interfacesTrace = std::unique_ptr<WifiTrace>(new WifiTrace("traces/netInterfacesTrace1"));

  auto period = std::make_shared<DeltaVar>(100);
  std::vector<double> times{10};

  auto at = std::make_shared<DetVar>(times);

  auto s = std::unique_ptr<Source>(new Source(std::string("S"),
                                    std::make_pair(0.0, 0.0),
                                    at,
                                    1));

  s->setInterval(period);

  auto d = std::unique_ptr<Node>(new Node(std::string("D"),
                                  std::make_pair(0, 1)));

  auto s_int = std::unique_ptr<WifiInterface>(new WifiInterface("S_int", 1.1, s.get()));
  auto d_int = std::unique_ptr<WifiInterface>(new WifiInterface("D_int", 1.1, d.get()));

  s->netInterface(std::move(s_int));
  d->netInterface(std::move(d_int));

  s->netInterface()->addTrace(interfacesTrace.get());
  d->netInterface()->addTrace(interfacesTrace.get());

  auto s_lnk = std::unique_ptr<WifiLink>(new WifiLink("S_lnk"));
  auto d_lnk = std::unique_ptr<WifiLink>(new WifiLink("D_lnk"));

  s->netInterface()->link(std::move(s_lnk));
  d->netInterface()->link(std::move(d_lnk));

  s->netInterface()->link()->addInterface(d->netInterface());
  d->netInterface()->link()->addInterface(s->netInterface());

  s->addDest(d.get());

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( d->netInterface()->status() == IDLE );
    REQUIRE( s->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == WAITING_FOR_SIFS );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d->netInterface()->status() == SENDING_ACK );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    // No more events in queue

    REQUIRE_THROWS( SIMUL.getNextEventTime() );

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}
