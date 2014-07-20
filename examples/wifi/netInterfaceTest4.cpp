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

  auto interfacesTrace = std::unique_ptr<WifiTrace>(new WifiTrace("traces/netInterfacesTrace4"));

  std::vector<double> times{10};

  auto at = std::make_shared<DetVar>(times);
  auto period = std::make_shared<DeltaVar>(100);

  auto s = std::unique_ptr<Source>(new Source(std::string("S"),
                                    std::make_pair(0.0, 0.0),
                                    at,
                                    1));

  s->setInterval(period);

  auto n = std::unique_ptr<Node>(new Node(std::string("N"),
                                  std::make_pair(0, 1)));
  auto d = std::unique_ptr<Node>(new Node(std::string("D"),
                                  std::make_pair(0, 2)));

  auto s_int = std::unique_ptr<WifiInterface>(new WifiInterface("S_int", 1.1, s.get()));
  auto n_int = std::unique_ptr<WifiInterface>(new WifiInterface("N_int", 1.1, n.get()));
  auto d_int = std::unique_ptr<WifiInterface>(new WifiInterface("D_int", 1.1, d.get()));

  s_int->addTrace(interfacesTrace.get());
  n_int->addTrace(interfacesTrace.get());
  d_int->addTrace(interfacesTrace.get());

  auto s_lnk = std::unique_ptr<WifiLink>(new WifiLink("S_lnk"));
  auto d_lnk = std::unique_ptr<WifiLink>(new WifiLink("D_lnk"));
  auto n_lnk = std::unique_ptr<WifiLink>(new WifiLink("N_lnk"));

  s_lnk->addInterface(n_int.get());
  d_lnk->addInterface(n_int.get());
  n_lnk->addInterface(s_int.get());
  n_lnk->addInterface(d_int.get());

  s_int->link(std::move(s_lnk));
  d_int->link(std::move(d_lnk));
  n_int->link(std::move(n_lnk));

  s->netInterface(std::move(s_int));
  d->netInterface(std::move(d_int));
  n->netInterface(std::move(n_int));

  s->addDest(d.get());

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( n->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( n->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( n->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_SIFS );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( n->netInterface()->status() == SENDING_ACK );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( n->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( d->netInterface()->status() == WAITING_FOR_SIFS );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d->netInterface()->status() == SENDING_ACK );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.sim_step();

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.run_to(10000);

    REQUIRE( s->netInterface()->status() == IDLE );
    REQUIRE( n->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

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
