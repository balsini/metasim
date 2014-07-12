#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <string>

#include <metasim.hpp>
#include <simul.hpp>

#include "link.hpp"
#include "netinterface.hpp"

using namespace MetaSim;

TEST_CASE( "Link Test", "[link]" )
{
  auto at = std::make_shared<UniformVar>(50,1024);
  auto period = std::make_shared<DeltaVar>(100);

  auto s = std::make_shared<Source>(std::string("src1"), std::make_pair(0.0, 0.0), at);

  s->setInterval(period);

  auto ss = static_pointer_cast<Node>(s);
  auto d_1 = std::make_shared<Node>(std::string("dst1"), std::make_pair(0, 1));
  auto d_2 = std::make_shared<Node>(std::string("dst2"), std::make_pair(1, 0));

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
}
