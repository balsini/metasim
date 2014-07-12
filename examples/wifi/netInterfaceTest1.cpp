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

  auto interfacesTrace = std::make_shared<TraceAscii>("netInterfacesTrace1.txt");

  auto period = std::make_shared<DeltaVar>(100);
  std::vector<double> times{10};

  auto at = std::make_shared<DetVar>(times);

  auto s = std::make_shared<Source>(std::string("S"),
                                    std::make_pair(0.0, 0.0),
                                    at,
                                    1);

  s->setInterval(period);

  auto ss = static_pointer_cast<Node>(s);
  auto d = std::make_shared<Node>(std::string("D"),
                                  std::make_pair(0, 1));

  auto s_int = std::make_shared<WifiInterface>("S_int", 1.1, ss);
  auto d_int = std::make_shared<WifiInterface>("D_int", 1.1, d);

  s_int->addTrace(interfacesTrace);
  d_int->addTrace(interfacesTrace);

  auto s_lnk = std::make_shared<WifiLink>("S_lnk");
  auto d_lnk = std::make_shared<WifiLink>("D_lnk");

  s_int->link(s_lnk);
  d_int->link(d_lnk);

  s_lnk->addInterface(d_int);
  d_lnk->addInterface(s_int);

  s->netInterface(s_int);
  d->netInterface(d_int);

  s->addDest(d);

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( d_int->status() == IDLE );
    REQUIRE( s_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == WAITING_FOR_DIFS );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == SENDING_MESSAGE );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d_int->status() == SENDING_ACK );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    // No more events in queue

    REQUIRE_THROWS( SIMUL.getNextEventTime() );

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}
