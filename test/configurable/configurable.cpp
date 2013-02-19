/* vim:set ts=3 sw=3 sts=3 et: */
/**
 * Copyright © 2008-2013 Last.fm Limited
 *
 * This file is part of libmoost.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <string>

#include "../../include/moost/configurable.hpp"

using namespace moost::configurable;

BOOST_AUTO_TEST_SUITE( binder_test )

struct SimpleConfigurable : public binder
{
  int MyInt;
  float MyFloat;
  std::string MyString;

  SimpleConfigurable()
  {
    bind("MyInt", MyInt, 0);
    bind("MyFloat", MyFloat, 0.0F);
    bind<std::string>("MyString", MyString, "");
  }
};

struct AddressConfigurable : public binder
{
  std::string Host;
  int Port;
  AddressConfigurable()
  {
    bind<std::string>("Host", Host, "");
    bind("Port", Port, 0);
  }
};

struct ComplexConfigurable : public binder
{
  SimpleConfigurable Simple;
  indexed_binder<AddressConfigurable> Addresses;

  int MyInt;
  float SomeFloat;

  ComplexConfigurable()
  {
    child("Simple", Simple);
    child("Addresses", Addresses);
    bind("MyInt", MyInt, 0);
    bind("SomeFloat", SomeFloat, 0.0F);
  }
};

struct BoolConfigurable : public binder
{
  bool MyBool;

  BoolConfigurable()
  {
    bind("MyBool", MyBool, false);
  }
};

struct Fixture
{
   Fixture()
   {
   }
   ~Fixture()
   {
   }

   SimpleConfigurable sc;
   ComplexConfigurable cc;
   indexed_binder<SimpleConfigurable> ic;
   BoolConfigurable bc;
};

BOOST_FIXTURE_TEST_CASE( test_binding_in, Fixture )
{
  sc.set("MyInt", "3");
  BOOST_CHECK_EQUAL(sc.MyInt, 3);
  sc.set("MyFloat", "3.1");
  BOOST_CHECK_EQUAL(sc.MyFloat, 3.1F);
  sc.set("MyString", "hey dude bro");
  BOOST_CHECK_EQUAL(sc.MyString, "hey dude bro");
}

BOOST_FIXTURE_TEST_CASE( test_binding_out, Fixture )
{
  std::string value;
  sc.MyInt = 3;
  sc.get("MyInt", value);
  BOOST_CHECK_EQUAL(value, "3");
  sc.MyFloat = 3.1F;
  sc.get("MyFloat", value);
  BOOST_CHECK_EQUAL(value, "3.1");
  sc.MyString = "hey dude bro";
  sc.get("MyString", value);
  BOOST_CHECK_EQUAL(value, "hey dude bro");
}

BOOST_FIXTURE_TEST_CASE( test_readwrite, Fixture )
{
  sc.MyInt = 3;
  sc.MyFloat = 3.1F;
  sc.MyString = "hey dude bro";

  std::ostringstream out;

  sc.write(out);

  SimpleConfigurable sc2;

  std::istringstream in(out.str());

  sc2.read(in);
  BOOST_CHECK_EQUAL(sc2.MyInt, 3);
  BOOST_CHECK_EQUAL(sc2.MyFloat, 3.1F);
  BOOST_CHECK_EQUAL(sc2.MyString, "hey dude bro");
}

BOOST_FIXTURE_TEST_CASE( test_list, Fixture )
{
  sc.MyInt = 3;
  sc.MyFloat = 3.1F;
  sc.MyString = "hey dude bro";

  std::vector< std::pair<std::string, std::string> > items;
  sc.list(items);

  std::pair<std::string, std::string> item1("MyInt", "3");
  std::pair<std::string, std::string> item2("MyFloat", "3.1");
  std::pair<std::string, std::string> item3("MyString", "hey dude bro");

  BOOST_CHECK_EQUAL(items.size(), 3);
  BOOST_CHECK(std::find(items.begin(), items.end(), item1) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item2) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item3) != items.end());
}

BOOST_FIXTURE_TEST_CASE( test_index_size, Fixture )
{
  ic.resize(7);
  BOOST_CHECK_EQUAL(ic.size(), 7);
  std::string value;
  ic.get("size", value);
  BOOST_CHECK_EQUAL(value, "7");
  ic.set("size", "9");
  BOOST_CHECK_EQUAL(ic.size(), 9);
}

BOOST_FIXTURE_TEST_CASE( test_index_getset, Fixture )
{
  ic.resize(7);
  ic[1].MyFloat = 3.1;

  std::string value;
  ic.get("1.MyFloat", value);
  BOOST_CHECK_EQUAL(value, "3.1");

  ic.set("3.MyInt", "17");
  BOOST_CHECK_EQUAL(ic[3].MyInt, 17);
}

BOOST_FIXTURE_TEST_CASE( test_index_read, Fixture )
{
  ic.resize(2);
  ic[0].MyFloat = 3.1F;
  ic[0].MyInt = 30;
  ic[0].MyString = "yabba dabba";
  ic[1].MyFloat = 3.9F;
  ic[1].MyInt = 40;
  ic[1].MyString = "doooo!";

  std::ostringstream out;
  ic.write(out);

  std::istringstream in(out.str());
  indexed_binder<SimpleConfigurable> ic2;

  ic2.read(in);

  BOOST_CHECK_EQUAL(ic.size(), ic2.size());
  BOOST_CHECK_EQUAL(ic[0].MyFloat, ic2[0].MyFloat);
  BOOST_CHECK_EQUAL(ic[0].MyInt, ic2[0].MyInt);
  BOOST_CHECK_EQUAL(ic[0].MyString, ic2[0].MyString);
  BOOST_CHECK_EQUAL(ic[1].MyFloat, ic2[1].MyFloat);
  BOOST_CHECK_EQUAL(ic[1].MyInt, ic2[1].MyInt);
  BOOST_CHECK_EQUAL(ic[1].MyString, ic2[1].MyString);
}

BOOST_FIXTURE_TEST_CASE( test_complex_binding_in, Fixture )
{
  cc.set("MyInt", "3");
  BOOST_CHECK_EQUAL(cc.MyInt, 3);
  cc.set("SomeFloat", "3.1");
  BOOST_CHECK_EQUAL(cc.SomeFloat, 3.1F);
  cc.set("Simple.MyInt", "4");
  BOOST_CHECK_EQUAL(cc.Simple.MyInt, 4);
  cc.set("Simple.MyFloat", "9.2");
  BOOST_CHECK_EQUAL(cc.Simple.MyFloat, 9.2F);
  cc.set("Simple.MyString", "hey well hello");
  BOOST_CHECK_EQUAL(cc.Simple.MyString, "hey well hello");
  cc.set("Addresses.size", "2");
  BOOST_CHECK_EQUAL(cc.Addresses.size(), 2);
  cc.set("Addresses.0.Host", "hosty host");
  BOOST_CHECK_EQUAL(cc.Addresses[0].Host, "hosty host");
  cc.set("Addresses.0.Port", "123");
  BOOST_CHECK_EQUAL(cc.Addresses[0].Port, 123);
  cc.set("Addresses.1.Host", "another host");
  BOOST_CHECK_EQUAL(cc.Addresses[1].Host, "another host");
  cc.set("Addresses.1.Port", "345");
  BOOST_CHECK_EQUAL(cc.Addresses[1].Port, 345);
}

BOOST_FIXTURE_TEST_CASE( test_complex_binding_out, Fixture )
{
  std::string value;
  cc.MyInt = 3;
  cc.get("MyInt", value);
  BOOST_CHECK_EQUAL(value, "3");
  cc.SomeFloat = 3.1F;
  cc.get("SomeFloat", value);
  BOOST_CHECK_EQUAL(value, "3.1");
  cc.Simple.MyInt = 4;
  cc.get("Simple.MyInt", value);
  BOOST_CHECK_EQUAL(value, "4");
  cc.Simple.MyFloat = 9.2F;
  cc.get("Simple.MyFloat", value);
  BOOST_CHECK_EQUAL(value, "9.2");
  cc.Simple.MyString = "hey well hello";
  cc.get("Simple.MyString", value);
  BOOST_CHECK_EQUAL(value, "hey well hello");
  cc.Addresses.resize(2);
  cc.get("Addresses.size", value);
  BOOST_CHECK_EQUAL(value, "2");
  cc.Addresses[0].Host = "hosted the mosted";
  cc.get("Addresses.0.Host", value);
  BOOST_CHECK_EQUAL(value, "hosted the mosted");
  cc.Addresses[0].Port = 789;
  cc.get("Addresses.0.Port", value);
  BOOST_CHECK_EQUAL(value, "789");
  cc.Addresses[1].Host = "one more host";
  cc.get("Addresses.1.Host", value);
  BOOST_CHECK_EQUAL(value, "one more host");
  cc.Addresses[1].Port = 12;
  cc.get("Addresses.1.Port", value);
  BOOST_CHECK_EQUAL(value, "12");
}

BOOST_FIXTURE_TEST_CASE( test_complex_readwrite, Fixture )
{
  cc.MyInt = 3;
  cc.SomeFloat = 3.1F;
  cc.Simple.MyInt = 4;
  cc.Simple.MyFloat = 9.2F;
  cc.Simple.MyString = "yes here we go";
  cc.Addresses.resize(2);
  cc.Addresses[0].Host = "first host";
  cc.Addresses[0].Port = 111;
  cc.Addresses[1].Host = "second host";
  cc.Addresses[1].Port = 222;

  std::ostringstream out;

  cc.write(out);

  ComplexConfigurable cc2;

  std::istringstream in(out.str());

  cc2.read(in);
  BOOST_CHECK_EQUAL(cc2.MyInt, 3);
  BOOST_CHECK_EQUAL(cc2.SomeFloat, 3.1F);
  BOOST_CHECK_EQUAL(cc2.Simple.MyInt, 4);
  BOOST_CHECK_EQUAL(cc2.Simple.MyFloat, 9.2F);
  BOOST_CHECK_EQUAL(cc2.Simple.MyString, "yes here we go");
  BOOST_CHECK_EQUAL(cc2.Addresses.size(), 2);
  BOOST_CHECK_EQUAL(cc2.Addresses[0].Host, "first host");
  BOOST_CHECK_EQUAL(cc2.Addresses[0].Port, 111);
  BOOST_CHECK_EQUAL(cc2.Addresses[1].Host, "second host");
  BOOST_CHECK_EQUAL(cc2.Addresses[1].Port, 222);
}

BOOST_FIXTURE_TEST_CASE( test_complex_list, Fixture )
{
  cc.MyInt = 3;
  cc.SomeFloat = 3.1F;
  cc.Simple.MyInt = 4;
  cc.Simple.MyFloat = 9.2F;
  cc.Simple.MyString = "yes here we go";
  cc.Addresses.resize(2);
  cc.Addresses[0].Host = "first host";
  cc.Addresses[0].Port = 111;
  cc.Addresses[1].Host = "second host";
  cc.Addresses[1].Port = 222;

  std::vector< std::pair<std::string, std::string> > items;
  cc.list(items);

  std::pair<std::string, std::string> item1("MyInt", "3");
  std::pair<std::string, std::string> item2("SomeFloat", "3.1");
  std::pair<std::string, std::string> item3("Simple.MyInt", "4");
  std::pair<std::string, std::string> item4("Simple.MyFloat", "9.2");
  std::pair<std::string, std::string> item5("Simple.MyString", "yes here we go");
  std::pair<std::string, std::string> item6("Addresses.size", "2");
  std::pair<std::string, std::string> item7("Addresses.0.Host", "first host");
  std::pair<std::string, std::string> item8("Addresses.0.Port", "111");
  std::pair<std::string, std::string> item9("Addresses.1.Host", "second host");
  std::pair<std::string, std::string> item10("Addresses.1.Port", "222");

  BOOST_CHECK_EQUAL(items.size(), 10);
  BOOST_CHECK(std::find(items.begin(), items.end(), item1) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item2) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item3) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item4) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item5) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item6) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item7) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item8) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item9) != items.end());
  BOOST_CHECK(std::find(items.begin(), items.end(), item10) != items.end());
}

BOOST_FIXTURE_TEST_CASE( test_bool, Fixture )
{
  std::string bool_conf = "{\n  MyBool true\n}";
  std::istringstream iss(bool_conf);
  bc.read(iss);

  BOOST_CHECK_EQUAL(bc.MyBool, true);
}

BOOST_AUTO_TEST_SUITE_END()
