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

  auto interfacesTrace = std::make_shared<TraceAscii>("netInterfacesTrace.txt");

  std::vector<double> times{1, 1000, 2000};

  auto at = std::make_shared<DetVar>(times);

  auto s = std::make_shared<Source>(std::string("S"),
                                    std::make_pair(0.0, 0.0),
                                    at,
                                    1);
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
    // SHOULD BE SENDING_ACK
    REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();
    SIMUL.sim_step();

    //REQUIRE( s_int->status() == WAITING_FOR_ACK );
    // SHOULD BE SENDING_ACK
    //REQUIRE( d_int->status() == RECEIVING_MESSAGE );

    SIMUL.endSingleRun();
  }

  interfacesTrace->close();
}

TEST_CASE( "netInterface Test, multiple access control protocol", "[netInterfaceMAC]" )
{
  /*
   * Network Organization:
   *
   *
   * Source1 <--.--> Source2
   *            |
   *            V
   *           Node
   *
   * Source1 and Source2 are sending messages to Node.
   * Source1 and Source2 are able to sense each other,
   * so they will try to avoid collisions.
   */

  /*
  auto at = std::make_shared<UniformVar>(1,10);

  auto s = std::make_shared<Source>(std::string("src1"),
                                    std::make_pair(0.0, 0.0), at);
  auto ss = static_pointer_cast<Node>(s);
  auto d_1 = std::make_shared<Node>(std::string("dst1"),
                                    std::make_pair(0, 1));
  auto d_2 = std::make_shared<Node>(std::string("dst2"),
                                    std::make_pair(1, 0));

  auto s_int = std::make_shared<WifiInterface>("s_int", 1.1, ss);
  auto d1_int = std::make_shared<WifiInterface>("d1_int", 1.1, d_1);
  auto d2_int = std::make_shared<WifiInterface>("d2_int", 1.1, d_2);

  auto s_lnk = std::make_shared<WifiLink>("s_lnk");
  auto d1_lnk = std::make_shared<WifiLink>("d1_lnk");
  auto d2_lnk = std::make_shared<WifiLink>("d2_lnk");

  s_int->link(s_lnk);
  d1_int->link(d1_lnk);
  d2_int->link(d2_lnk);

  REQUIRE_THROWS( s_lnk->getRightInterface().get() );
  REQUIRE_THROWS( s_lnk->getDownInterface().get() );

  s_lnk->addInterface(d1_int);
  s_lnk->addInterface(d2_int);
  d1_lnk->addInterface(s_int);
  d1_lnk->addInterface(d2_int);
  d2_lnk->addInterface(s_int);
  d2_lnk->addInterface(d1_int);

  s->netInterface(s_int);
  d_1->netInterface(d1_int);
  d_2->netInterface(d2_int);

  s->addDest(d_1);
  s->addDest(d_2);

  REQUIRE( s_lnk->getRightInterface().get() == d1_int.get() );
  REQUIRE( s_lnk->getDownInterface().get() == d2_int.get() );

  SECTION( "Message is forwarded to its broadcast domain" )
  {
    REQUIRE( d1_int->status() == IDLE );
    REQUIRE( d2_int->status() == IDLE );

    auto msg = std::unique_ptr<Message>(new Message(10,
                                                     s.get(),
                                                     d_1.get(),
                                                     0,
                                                     false));
    s_lnk->send(msg);

    REQUIRE( d1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d2_int->status() == RECEIVING_MESSAGE );
  }
  */
}


TEST_CASE( "netInterface Test, hidden terminal problem", "[netInterfaceHiddenTerminal]" )
{
  /*
   * Network Organization:
   *
   *
   * Source1 <---> Node <---> Source2
   *
   *
   * Source1 and Source 2 are sending messages to Node.
   * Because of the hidden terminal problem, in case of
   * collision, the messages will be corrupted.
   */

  /*
  auto at = std::make_shared<UniformVar>(1,10);

  auto s = std::make_shared<Source>(std::string("src1"),
                                    std::make_pair(0.0, 0.0), at);
  auto ss = static_pointer_cast<Node>(s);
  auto d_1 = std::make_shared<Node>(std::string("dst1"),
                                    std::make_pair(0, 1));
  auto d_2 = std::make_shared<Node>(std::string("dst2"),
                                    std::make_pair(1, 0));

  auto s_int = std::make_shared<WifiInterface>("s_int", 1.1, ss);
  auto d1_int = std::make_shared<WifiInterface>("d1_int", 1.1, d_1);
  auto d2_int = std::make_shared<WifiInterface>("d2_int", 1.1, d_2);

  auto s_lnk = std::make_shared<WifiLink>("s_lnk");
  auto d1_lnk = std::make_shared<WifiLink>("d1_lnk");
  auto d2_lnk = std::make_shared<WifiLink>("d2_lnk");

  s_int->link(s_lnk);
  d1_int->link(d1_lnk);
  d2_int->link(d2_lnk);

  REQUIRE_THROWS( s_lnk->getRightInterface().get() );
  REQUIRE_THROWS( s_lnk->getDownInterface().get() );

  s_lnk->addInterface(d1_int);
  s_lnk->addInterface(d2_int);
  d1_lnk->addInterface(s_int);
  d1_lnk->addInterface(d2_int);
  d2_lnk->addInterface(s_int);
  d2_lnk->addInterface(d1_int);

  s->netInterface(s_int);
  d_1->netInterface(d1_int);
  d_2->netInterface(d2_int);

  s->addDest(d_1);
  s->addDest(d_2);

  REQUIRE( s_lnk->getRightInterface().get() == d1_int.get() );
  REQUIRE( s_lnk->getDownInterface().get() == d2_int.get() );

  SECTION( "Message is forwarded to its broadcast domain" )
  {
    REQUIRE( d1_int->status() == IDLE );
    REQUIRE( d2_int->status() == IDLE );

    auto msg = std::unique_ptr<Message>(new Message(10,
                                                     s.get(),
                                                     d_1.get(),
                                                     0,
                                                     false));
    s_lnk->send(msg);

    REQUIRE( d1_int->status() == RECEIVING_MESSAGE );
    REQUIRE( d2_int->status() == RECEIVING_MESSAGE );
  }
  */
}
