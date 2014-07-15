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

  auto s = std::unique_ptr<Source>(new Source(std::string("src1"), std::make_pair(0.0, 0.0), at));

  s->setInterval(period);

  auto d1 = std::unique_ptr<Node>(new Node(std::string("dst1"), std::make_pair(0, 1)));
  auto d2 = std::unique_ptr<Node>(new Node(std::string("dst2"), std::make_pair(1, 0)));

  auto s_int = std::unique_ptr<WifiInterface>(new WifiInterface("s_int", 1.1, s.get()));
  auto d1_int = std::unique_ptr<WifiInterface>(new WifiInterface("d1_int", 1.1, d1.get()));
  auto d2_int = std::unique_ptr<WifiInterface>(new WifiInterface("d2_int", 1.1, d2.get()));

  auto s_lnk = std::unique_ptr<WifiLink>(new WifiLink("s_lnk"));
  auto d1_lnk = std::unique_ptr<WifiLink>(new WifiLink("d1_lnk"));
  auto d2_lnk = std::unique_ptr<WifiLink>(new WifiLink("d2_lnk"));

  REQUIRE_THROWS( s_lnk->getRightInterface() );
  REQUIRE_THROWS( s_lnk->getDownInterface() );

  s_lnk->addInterface(d1_int.get());
  s_lnk->addInterface(d2_int.get());
  d1_lnk->addInterface(s_int.get());
  d1_lnk->addInterface(d2_int.get());
  d2_lnk->addInterface(s_int.get());
  d2_lnk->addInterface(d1_int.get());

  s_int->link(std::move(s_lnk));
  d1_int->link(std::move(d1_lnk));
  d2_int->link(std::move(d2_lnk));

  s->netInterface(std::move(s_int));
  d1->netInterface(std::move(d1_int));
  d2->netInterface(std::move(d2_int));

  s->addDest(d1.get());
  s->addDest(d2.get());

  REQUIRE( s->netInterface()->link()->getRightInterface() == d1->netInterface() );
  REQUIRE( s->netInterface()->link()->getDownInterface() == d2->netInterface() );

  SECTION( "Message is forwarded to its broadcast domain" )
  {
    REQUIRE( d1->netInterface()->status() == IDLE );
    REQUIRE( d2->netInterface()->status() == IDLE );

    auto msg = std::unique_ptr<Message>(new Message(10,
                                                     s.get(),
                                                     d1.get(),
                                                     0,
                                                     false));
    s->netInterface()->link()->send(msg);

    REQUIRE( d1->netInterface()->status() == RECEIVING_MESSAGE );
    REQUIRE( d2->netInterface()->status() == RECEIVING_MESSAGE );
  }
}
