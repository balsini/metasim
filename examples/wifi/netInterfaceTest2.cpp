#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <vector>
#include <string>

#include <metasim.hpp>
#include <simul.hpp>
#include <trace.hpp>

#include "node.hpp"
#include "link.hpp"
#include "netinterface.hpp"

using namespace MetaSim;

TEST_CASE( "netInterface Test, multiple access control protocol", "[netInterfaceMAC]" )
{
  /*
   * Network Organization:
   *
   *
   * S_1
   * S_2
   * S_3
   *  |
   *  V
   *  D
   *
   * S_1, S_2 and S_3 are sending messages to D.
   * All the senders are able to sense each other,
   * so they will try to avoid collisions.
   */

  auto interfacesTrace = std::make_shared<TraceAscii>("traces/netInterfacesTrace2.txt");

  std::vector<double> times{10, 20, 70};

  auto at = std::make_shared<DetVar>(times);
  auto period = std::make_shared<DeltaVar>(100);

  auto s1 = std::unique_ptr<Source>(new Source(std::string("S1"),
                                     std::make_pair(0.0, 0.0),
                                     at,
                                     1));
  auto s2 = std::unique_ptr<Source>(new Source(std::string("S2"),
                                     std::make_pair(0.1, 0.0),
                                     at,
                                     1));
  auto s3 = std::unique_ptr<Source>(new Source(std::string("S3"),
                                     std::make_pair(0.2, 0.0),
                                     at,
                                     1));

  s1->setInterval(period);
  s2->setInterval(period);
  s3->setInterval(period);

  auto d = std::unique_ptr<Node>(new Node(std::string("D"),
                                  std::make_pair(0.5, 0.0)));

  auto s1_int = std::unique_ptr<WifiInterface>(new WifiInterface("S1_int", 5, s1.get()));
  auto s2_int = std::unique_ptr<WifiInterface>(new WifiInterface("S2_int", 5, s2.get()));
  auto s3_int = std::unique_ptr<WifiInterface>(new WifiInterface("S3_int", 5, s3.get()));
  auto d_int = std::unique_ptr<WifiInterface>(new WifiInterface("D_int", 5, d.get()));

  s1->netInterface(std::move(s1_int));
  s2->netInterface(std::move(s2_int));
  s3->netInterface(std::move(s3_int));
  d->netInterface(std::move(d_int));

  s1->netInterface()->addTrace(interfacesTrace.get());
  s2->netInterface()->addTrace(interfacesTrace.get());
  s3->netInterface()->addTrace(interfacesTrace.get());
  d->netInterface()->addTrace(interfacesTrace.get());

  auto s1_lnk = std::unique_ptr<WifiLink>(new WifiLink("S1_lnk"));
  auto s2_lnk = std::unique_ptr<WifiLink>(new WifiLink("S2_lnk"));
  auto s3_lnk = std::unique_ptr<WifiLink>(new WifiLink("S3_lnk"));
  auto d_lnk = std::unique_ptr<WifiLink>(new WifiLink("D_lnk"));

  s1_lnk->addInterface(d->netInterface());
  s1_lnk->addInterface(s2->netInterface());
  s1_lnk->addInterface(s3->netInterface());
  s2_lnk->addInterface(d->netInterface());
  s2_lnk->addInterface(s1->netInterface());
  s2_lnk->addInterface(s3->netInterface());
  s3_lnk->addInterface(d->netInterface());
  s3_lnk->addInterface(s1->netInterface());
  s3_lnk->addInterface(s2->netInterface());
  d_lnk->addInterface(s1->netInterface());
  d_lnk->addInterface(s2->netInterface());
  d_lnk->addInterface(s3->netInterface());

  s1->netInterface()->link(std::move(s1_lnk));
  s2->netInterface()->link(std::move(s2_lnk));
  s3->netInterface()->link(std::move(s3_lnk));
  d->netInterface()->link(std::move(d_lnk));

  s1->addDest(d.get());
  s2->addDest(d.get());
  s3->addDest(d.get());

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( s1->netInterface()->status() == IDLE );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( s3->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.run_to(11);

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( s3->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.run_to(21);

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( s3->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.run_to(39);

    REQUIRE( s1->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_DIFS );
    REQUIRE( s3->netInterface()->status() == RECEIVING_MESSAGE);
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.run_to(49);

    REQUIRE( s1->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3->netInterface()->status() == RECEIVING_MESSAGE);
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.run_to(295);

    REQUIRE( s1->netInterface()->status() == WAITING_FOR_ACK );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3->netInterface()->status() == WAITING_FOR_DIFS);
    REQUIRE( d->netInterface()->status() == WAITING_FOR_SIFS );

    SIMUL.run_to(305);

    REQUIRE( s1->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3->netInterface()->status() == WAITING_FOR_DIFS);
    REQUIRE( d->netInterface()->status() == SENDING_ACK );

    SIMUL.run_to(310);

    REQUIRE( s1->netInterface()->status() == IDLE );
    REQUIRE( s2->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3->netInterface()->status() == WAITING_FOR_DIFS);
    REQUIRE( d->netInterface()->status() == IDLE );

    SIMUL.run_to(316);

    REQUIRE( s1->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s3->netInterface()->status() == WAITING_FOR_DIFS);
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.run_to(323);

    REQUIRE( s1->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( s2->netInterface()->status() == SENDING_MESSAGE );
    REQUIRE( s3->netInterface()->status() == WAITING_FOR_BACKOFF );
    REQUIRE( d->netInterface()->status() == RECEIVING_MESSAGE );

    SIMUL.run_to(50000);

    REQUIRE( s1->produced() == 1);
    REQUIRE( s2->produced() == 1);
    REQUIRE( s3->produced() == 1);
    REQUIRE( d->consumed() == 3);

    REQUIRE( s1->netInterface()->status() == IDLE );
    REQUIRE( s2->netInterface()->status() == IDLE );
    REQUIRE( s3->netInterface()->status() == IDLE );
    REQUIRE( d->netInterface()->status() == IDLE );

    // No more events in queue

    REQUIRE_THROWS( SIMUL.getNextEventTime() );

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}
