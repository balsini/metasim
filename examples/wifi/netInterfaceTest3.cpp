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

TEST_CASE( "netInterface Test, hidden terminal problem", "[netInterfaceHiddenTerminal]" )
{
  /*
   * Network Organization:
   *
   *
   *         S2
   *          |
   *          V
   * S1 <---> D
   *
   *
   * S1 and S2 are sending messages to D.
   * Because of the hidden terminal problem, in case of
   * collision, the messages will result corrupted and
   * simply lost.
   */

  auto interfacesTrace = std::make_shared<TraceAscii>("netInterfacesTrace3.txt");

  std::vector<double> times{10, 50};

  auto at = std::make_shared<DetVar>(times);
  auto period = std::make_shared<DeltaVar>(100);

  auto s1 = std::make_shared<Source>(std::string("S1"),
                                    std::make_pair(1.0, 0.0),
                                    at,
                                    1);
  auto ss1 = static_pointer_cast<Node>(s1);
  auto s2 = std::make_shared<Source>(std::string("S2"),
                                    std::make_pair(0.0, 1.0),
                                    at,
                                    1);
  auto ss2 = static_pointer_cast<Node>(s2);

  s1->setInterval(period);
  s2->setInterval(period);

  auto d = std::make_shared<Node>(std::string("D"),
                                  std::make_pair(1, 1));

  auto s1_int = std::make_shared<WifiInterface>("S1_int", 1.1, ss1);
  auto s2_int = std::make_shared<WifiInterface>("S2_int", 1.1, ss2);
  auto d_int = std::make_shared<WifiInterface>("D_int", 1.1, d);

  s1_int->addTrace(interfacesTrace);
  s2_int->addTrace(interfacesTrace);
  d_int->addTrace(interfacesTrace);

  auto s1_lnk = std::make_shared<WifiLink>("S1_lnk");
  auto s2_lnk = std::make_shared<WifiLink>("S2_lnk");
  auto d_lnk = std::make_shared<WifiLink>("D_lnk");

  s1_int->link(s1_lnk);
  s2_int->link(s2_lnk);
  d_int->link(d_lnk);

  s1_lnk->addInterface(d_int);
  s2_lnk->addInterface(d_int);
  d_lnk->addInterface(s1_int);
  d_lnk->addInterface(s2_int);

  s1->netInterface(s1_int);
  s2->netInterface(s2_int);
  d->netInterface(d_int);

  s1->addDest(d);
  s2->addDest(d);

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( s1_int->status() == IDLE );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_DIFS );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_DIFS );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == SENDING_MESSAGE );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_ACK );
    REQUIRE( s2_int->status() == SENDING_MESSAGE );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s2_int->status() == SENDING_MESSAGE );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s2_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s2_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.run_to(10000);

    REQUIRE( s1_int->status() == IDLE );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    REQUIRE( s1->consumed() == 0 );
    REQUIRE( s2->consumed() == 0 );
    REQUIRE( d->consumed() == 2 );

    REQUIRE( s1->produced() == 1 );
    REQUIRE( s2->produced() == 1 );

    REQUIRE_THROWS( SIMUL.getNextEventTime() );

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}
