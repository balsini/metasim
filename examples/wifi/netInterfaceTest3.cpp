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

  auto interfacesTrace = std::make_shared<TraceAscii>("traces/netInterfacesTrace3.txt");

  std::vector<double> times{10, 50};

  auto at = std::make_shared<DetVar>(times);
  auto period = std::make_shared<DeltaVar>(100);

  auto s1 = std::unique_ptr<Source>(new Source(std::string("S1"),
                                    std::make_pair(1.0, 0.0),
                                    at,
                                    1));
  auto s2 = std::unique_ptr<Source>(new Source(std::string("S2"),
                                    std::make_pair(0.0, 1.0),
                                    at,
                                    1));

  s1->setInterval(period);
  s2->setInterval(period);

  auto d = std::unique_ptr<Node>(new Node(std::string("D"),
                                  std::make_pair(1, 1)));

  auto s1_int = std::unique_ptr<WifiInterface>(new WifiInterface("S1_int", 1.1, s1.get()));
  auto s2_int = std::unique_ptr<WifiInterface>(new WifiInterface("S2_int", 1.1, s2.get()));
  auto d_int = std::unique_ptr<WifiInterface>(new WifiInterface("D_int", 1.1, d.get()));

  s1_int->addTrace(interfacesTrace.get());
  s2_int->addTrace(interfacesTrace.get());
  d_int->addTrace(interfacesTrace.get());

  auto s1_lnk = std::unique_ptr<WifiLink>(new WifiLink("S1_lnk"));
  auto s2_lnk = std::unique_ptr<WifiLink>(new WifiLink("S2_lnk"));
  auto d_lnk = std::unique_ptr<WifiLink>(new WifiLink("D_lnk"));

  s1_lnk->addInterface(d_int.get());
  s2_lnk->addInterface(d_int.get());
  d_lnk->addInterface(s1_int.get());
  d_lnk->addInterface(s2_int.get());

  s1_int->link(std::move(s1_lnk));
  s2_int->link(std::move(s2_lnk));
  d_int->link(std::move(d_lnk));

  s1->netInterface(std::move(s1_int));
  s2->netInterface(std::move(s2_int));
  d->netInterface(std::move(d_int));

  s1->addDest(d.get());
  s2->addDest(d.get());

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( s1->netInterface()->status() == IDLE );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( s2->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s2->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.run_to(10000);

    REQUIRE( s1->netInterface()->status() == IDLE );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

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
