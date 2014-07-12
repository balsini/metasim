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

TEST_CASE( "netInterface Test, forwarding verification", "[netInterfaceForwarding]" )
{
  /*
   * Network Organization:
   *
   * S <---> N <---> D
   *
   *
   * S sends a messages to D.
   * S cannot directly reach D, so the message
   * has to be sent to N and then forwarded to D.
   */

  auto interfacesTrace = std::make_shared<TraceAscii>("netInterfacesTrace4.txt");

  std::vector<double> times{10};

  auto at = std::make_shared<DetVar>(times);
  auto period = std::make_shared<DeltaVar>(100);

  auto s = std::make_shared<Source>(std::string("S"),
                                    std::make_pair(0.0, 0.0),
                                    at,
                                    1);
  auto ss = static_pointer_cast<Node>(s);

  s->setInterval(period);

  auto n = std::make_shared<Node>(std::string("N"),
                                  std::make_pair(0, 1));
  auto d = std::make_shared<Node>(std::string("D"),
                                  std::make_pair(0, 2));

  auto s_int = std::make_shared<WifiInterface>("S_int", 1.1, ss);
  auto n_int = std::make_shared<WifiInterface>("N_int", 1.1, n);
  auto d_int = std::make_shared<WifiInterface>("D_int", 1.1, d);

  s_int->addTrace(interfacesTrace);
  n_int->addTrace(interfacesTrace);
  d_int->addTrace(interfacesTrace);

  auto s_lnk = std::make_shared<WifiLink>("S_lnk");
  auto d_lnk = std::make_shared<WifiLink>("D_lnk");
  auto n_lnk = std::make_shared<WifiLink>("N_lnk");

  s_int->link(s_lnk);
  d_int->link(d_lnk);
  n_int->link(n_lnk);

  s_lnk->addInterface(n_int);
  d_lnk->addInterface(n_int);
  n_lnk->addInterface(s_int);
  n_lnk->addInterface(d_int);

  s->netInterface(s_int);
  d->netInterface(d_int);
  n->netInterface(n_int);

  s->addDest(d);

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == WAITING_FOR_DIFS );
    REQUIRE( n_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == SENDING_MESSAGE );
    REQUIRE( n_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == WAITING_FOR_ACK );
    REQUIRE( n_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == WAITING_FOR_ACK );
    REQUIRE( n_int->status() == WAITING_FOR_SIFS );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == RECEIVING_MESSAGE );
    REQUIRE( n_int->status() == SENDING_ACK );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == RECEIVING_MESSAGE );
    REQUIRE( n_int->status() == WAITING_FOR_DIFS );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == WAITING_FOR_DIFS );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == WAITING_FOR_DIFS );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == RECEIVING_MESSAGE );
    REQUIRE( n_int->status() == SENDING_MESSAGE );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == RECEIVING_MESSAGE );
    REQUIRE( n_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == WAITING_FOR_ACK );
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d_int->status() == SENDING_ACK );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    SIMUL.run_to(10000);

    REQUIRE( s_int->status() == IDLE );
    REQUIRE( n_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    REQUIRE( s->consumed() == 0 );
    REQUIRE( n->consumed() == 0 );
    REQUIRE( d->consumed() == 1 );

    REQUIRE( s->produced() == 1 );

    REQUIRE_THROWS( SIMUL.getNextEventTime() );

    // No more events in queue

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();

}
