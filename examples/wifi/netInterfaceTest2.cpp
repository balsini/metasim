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

  auto interfacesTrace = std::make_shared<TraceAscii>("netInterfacesTrace2.txt");

  std::vector<double> times{10, 20, 70};

  auto at = std::make_shared<DetVar>(times);

  auto s1 = std::make_shared<Source>(std::string("S1"),
                                     std::make_pair(0.0, 0.0),
                                     at,
                                     1);
  auto s2 = std::make_shared<Source>(std::string("S2"),
                                     std::make_pair(0.1, 0.0),
                                     at,
                                     1);
  auto s3 = std::make_shared<Source>(std::string("S3"),
                                     std::make_pair(0.2, 0.0),
                                     at,
                                     1);

  auto ss1 = static_pointer_cast<Node>(s1);
  auto ss2 = static_pointer_cast<Node>(s2);
  auto ss3 = static_pointer_cast<Node>(s3);

  auto d = std::make_shared<Node>(std::string("D"),
                                  std::make_pair(0.5, 0.0));

  auto s1_int = std::make_shared<WifiInterface>("S1_int", 5, ss1);
  auto s2_int = std::make_shared<WifiInterface>("S2_int", 5, ss2);
  auto s3_int = std::make_shared<WifiInterface>("S3_int", 5, ss3);
  auto d_int = std::make_shared<WifiInterface>("D_int", 5, d);

  s1->netInterface(s1_int);
  s2->netInterface(s2_int);
  s3->netInterface(s3_int);
  d->netInterface(d_int);

  s1_int->addTrace(interfacesTrace);
  s2_int->addTrace(interfacesTrace);
  s3_int->addTrace(interfacesTrace);
  d_int->addTrace(interfacesTrace);

  auto s1_lnk = std::make_shared<WifiLink>("S1_lnk");
  auto s2_lnk = std::make_shared<WifiLink>("S2_lnk");
  auto s3_lnk = std::make_shared<WifiLink>("S3_lnk");
  auto d_lnk = std::make_shared<WifiLink>("D_lnk");

  s1_lnk->addInterface(d_int);
  s1_lnk->addInterface(s2_int);
  s1_lnk->addInterface(s3_int);
  s2_lnk->addInterface(d_int);
  s2_lnk->addInterface(s1_int);
  s2_lnk->addInterface(s3_int);
  s3_lnk->addInterface(d_int);
  s3_lnk->addInterface(s1_int);
  s3_lnk->addInterface(s2_int);
  d_lnk->addInterface(s1_int);
  d_lnk->addInterface(s2_int);
  d_lnk->addInterface(s3_int);

  s1_int->link(s1_lnk);
  s2_int->link(s2_lnk);
  s3_int->link(s3_lnk);
  d_int->link(d_lnk);

  s1->addDest(d);
  s2->addDest(d);
  s3->addDest(d);

  SECTION( "Step by step communication analysis" )
  {
    SIMUL.initSingleRun();

    REQUIRE( s1_int->status() == IDLE );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( s3_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_DIFS );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( s3_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_DIFS );
    REQUIRE( s2_int->status() == WAITING_FOR_DIFS );
    REQUIRE( s3_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_DIFS );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_ACK );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_ACK );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_ACK );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == WAITING_FOR_DIFS);
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == WAITING_FOR_DIFS);
    REQUIRE( d_int->status() == SENDING_ACK );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == WAITING_FOR_DIFS);
    REQUIRE( d_int->status() == IDLE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == IDLE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == WAITING_FOR_DIFS);
    REQUIRE( d_int->status() == IDLE );

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == SENDING_MESSAGE );
    REQUIRE( s3_int->status() == WAITING_FOR_DIFS);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == SENDING_MESSAGE );
    REQUIRE( s3_int->status() == WAITING_FOR_BACKOFF);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_ACK );
    REQUIRE( s3_int->status() == WAITING_FOR_BACKOFF);
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_ACK );
    REQUIRE( s3_int->status() == WAITING_FOR_BACKOFF);
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == IDLE );
    REQUIRE( s2_int->status() == WAITING_FOR_ACK );
    REQUIRE( s3_int->status() == WAITING_FOR_BACKOFF);
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    std::cout << std::endl;
    SIMUL.sim_step();

    SIMUL.run_to(50000);

    REQUIRE( s1_int->status() == IDLE );
    REQUIRE( s2_int->status() == IDLE );
    REQUIRE( s3_int->status() == IDLE );
    REQUIRE( d_int->status() == IDLE );

    /*
    REQUIRE( s1_int->status() == WAITING_FOR_ACK );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == WAITING_FOR_ACK );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == WAITING_FOR_DIFS);
    REQUIRE( d_int->status() == WAITING_FOR_SIFS );

    std::cout << std::endl;
    SIMUL.sim_step();
*/

    /*
    REQUIRE( s1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( s3_int->status() == RECEIVING_MESSAGE);
    REQUIRE( d_int->status() == SENDING_ACK );

    std::cout << std::endl;
    SIMUL.sim_step();
*/
    /*
    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_DIFS );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << std::endl;
    SIMUL.sim_step();

    REQUIRE( s1_int->status() == SENDING_MESSAGE );
    REQUIRE( s2_int->status() == WAITING_FOR_BACKOFF );
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    std::cout << "Finiti test" << std::endl;

    SIMUL.run_to(3000);
    */
    /*
    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();

    std::cout << std::endl;
    SIMUL.sim_step();
*/
    /*
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();

    // No more events in queue

    REQUIRE_THROWS( SIMUL.getNextEventTime() );
*/
    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}
