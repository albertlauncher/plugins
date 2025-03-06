//// Copyright (c) 2022-2024 Manuel Schneider

#include "cast_specialization.hpp"

#include "albert/query.h"
#include "albert/indexqueryhandler.h"
#include "albert/fallbackhandler.h"
#include "albert/action.h"
#include "albert/item.h"
#include "albert/matcher.h"
#include "test.h"
#include <QTest>
using namespace albert;
using namespace std;
QTEST_APPLESS_MAIN(PythonTests)


// Mock a test query class
class MockQuery : public Query
{
public:
    MockQuery(Extension &handler) : handler_(handler) {}

    Extension &handler_;
    bool valid_ = true;
    vector<ResultItem> matches_;
    vector<ResultItem> fallbacks_;

    QString synopsis() const override { return "t"; }
    QString trigger() const override { return "t"; }
    QString string() const override { return "x"; }
    bool isActive() const override { return false; }
    bool isFinished() const override { return false; }
    const bool &isValid() const override { return valid_; }
    bool isTriggered() const override { return true; }
    const vector<ResultItem> &matches() override { return matches_; }
    const vector<ResultItem> &fallbacks() override { return fallbacks_; }
    bool activateMatch(uint, uint) override { return false; }
    bool activateFallback(uint, uint) override { return false; }

    void add(const shared_ptr<Item> &item) override
    {
        matches_.emplace_back(handler_, item);
    }

    void add(shared_ptr<Item> &&item) override
    {
        matches_.emplace_back(handler_, ::move(item));
    }

    void add(const vector<shared_ptr<Item>> &items) override
    {
        for (const auto &item : items)
            matches_.emplace_back(handler_, item);
    }

    void add(vector<shared_ptr<Item>> &&items) override
    {
        for (auto &item : items)
            matches_.emplace_back(handler_, ::move(item));
    }
};



void PythonTests::initTestCase()
{
    PyConfig config;
    PyConfig_InitIsolatedConfig(&config);
    if (auto status = Py_InitializeFromConfig(&config); PyStatus_Exception(status))
        throw runtime_error(QString("Failed initializing the interpreter: %1 %2")
                                .arg(status.func, status.err_msg).toStdString());

    PyConfig_Clear(&config);

    // Dont. Registry does not exist in the test environment.
    // new Plugin();

    py::exec(R"(
import albert

class TQH(albert.TriggerQueryHandler):

    def handleTriggerQuery(self, query):
        if stripped := query.string.strip():
            query.add(Plugin.createItem(stripped))

class GQH(albert.GlobalQueryHandler):

    def handleGlobalQuery(self, query):
        return []

class IQH(albert.IndexQueryHandler):
    pass

class FQH(albert.FallbackHandler):

    def fallbacks(self, s):
        return [Plugin.createItem(s)] if s else []

class Plugin(albert.PluginInstance):

    def __init__(self):
        super().__init__(self)

    def extensions(self):
        return [self.tqh, self.gqh, self.iqh, self.fqh]
)");

}

void PythonTests::testAction()
{
    py::exec(R"(
import albert

test_action_variable = 0

def test_action_callable():
    global test_action_variable
    test_action_variable += 1

test_action = albert.Action(
    id="test_action_id",
    text="test_action_text",
    callable=test_action_callable
)
)");

    auto g = py::globals();

    QVERIFY(g.contains("test_action_variable"));
    QVERIFY(g.contains("test_action_callable"));
    QVERIFY(g.contains("test_action"));

    auto a = g["test_action"].cast<Action>();

    QCOMPARE(a.id, "test_action_id");
    QCOMPARE(a.text, "test_action_text");
    QCOMPARE(g["test_action_variable"].cast<int>(), 0);

    a.function();

    QCOMPARE(g["test_action_variable"].cast<int>(), 1);
}

void PythonTests::testItem()
{
    py::exec(R"(
import albert

class TestItem(albert.Item):

    def id(self):
        return "item_id"

    def text(self):
        return "item_text"

    def subtext(self):
        return "item_subtext"

    def inputActionText(self):
        return "item_input_action_text"

    def iconUrls(self):
        return ["i1", "i2"]

    def actions(self):
        return [test_action]

class InvalidTestItem(albert.Item):
    pass

test_item = TestItem()
invalid_test_item = InvalidTestItem()
)");

    auto g = py::globals();

    QVERIFY(g.contains("TestItem"));
    QVERIFY(g.contains("test_item"));
    QVERIFY(g.contains("invalid_test_item"));
    QVERIFY(g.contains("InvalidTestItem"));

    auto i = g["test_item"].cast<shared_ptr<Item>>();

    QCOMPARE(i->id(), "item_id");
    QCOMPARE(i->text(), "item_text");
    QCOMPARE(i->subtext(), "item_subtext");
    QCOMPARE(i->inputActionText(), "item_input_action_text");
    QCOMPARE(i->iconUrls(), QStringList({"i1", "i2"}));
    QCOMPARE(i->actions().size(), 1);

    i = g["invalid_test_item"].cast<shared_ptr<Item>>();

    QCOMPARE(i->id(), "");
    QCOMPARE(i->text(), "");
    QCOMPARE(i->subtext(), "");
    QCOMPARE(i->inputActionText(), "");
    QCOMPARE(i->iconUrls(), QStringList());
    QCOMPARE(i->actions().size(), 0);
}

void PythonTests::testStandardItem()
{
    py::exec(R"(
test_standard_item = albert.StandardItem(
    id="item_id",
    text="item_text",
    subtext="item_subtext",
    inputActionText="item_input_action_text",
    iconUrls=["i1", "i2"],
    actions=[test_action]
)
)");

    auto g = py::globals();
    QVERIFY(g.contains("test_standard_item"));
    auto i = g["test_standard_item"].cast<shared_ptr<Item>>();

    // Test C++ getter

    QCOMPARE(i->id(), "item_id");
    QCOMPARE(i->text(), "item_text");
    QCOMPARE(i->subtext(), "item_subtext");
    QCOMPARE(i->inputActionText(), "item_input_action_text");
    QCOMPARE(i->iconUrls(), QStringList({"i1", "i2"}));
    QCOMPARE(i->actions().size(), 1);

    // Test Python getter

    py::exec(R"(t = test_standard_item.id)");
    QCOMPARE(g["t"].cast<QString>(), "item_id");

    py::exec(R"(t = test_standard_item.text)");
    QCOMPARE(g["t"].cast<QString>(), "item_text");

    py::exec(R"(t = test_standard_item.subtext)");
    QCOMPARE(g["t"].cast<QString>(), "item_subtext");

    py::exec(R"(t = test_standard_item.inputActionText)");
    QCOMPARE(g["t"].cast<QString>(), "item_input_action_text");

    py::exec(R"(t = test_standard_item.iconUrls)");
    QCOMPARE(g["t"].cast<QStringList>(), QStringList({"i1", "i2"}));

    py::exec(R"(t = test_standard_item.actions)");
    QCOMPARE(g["t"].cast<py::list>().size(), 1);


    // Test Python setter

    py::exec(R"(test_standard_item.id = "x_item_id")");
    QCOMPARE(i->id(), "x_item_id");

    py::exec(R"(test_standard_item.text = "x_item_text")");
    QCOMPARE(i->text(), "x_item_text");

    py::exec(R"(test_standard_item.subtext = "x_item_subtext")");
    QCOMPARE(i->subtext(), "x_item_subtext");

    py::exec(R"(test_standard_item.inputActionText = "x_item_input_action_text")");
    QCOMPARE(i->inputActionText(), "x_item_input_action_text");

    py::exec(R"(test_standard_item.iconUrls = ["x1", "x2"])");
    QCOMPARE(i->iconUrls(), QStringList({"x1", "x2"}));

    py::exec(R"(test_standard_item.actions = [])");
    QCOMPARE(i->actions().size(), 0);
}

void PythonTests::testMatcher()
{
    // This merely tests the Matcher API.
    // Thourough tests are done in the core tests.


    auto PyMatcher = py::module::import("albert").attr("Matcher");
    auto PyMatchConfig = py::module::import("albert").attr("MatchConfig");

    auto matcher = PyMatcher("x");

    using Score = Match::Score;
    using namespace pybind11::literals;

    auto m = matcher.attr("match")("x");
    QCOMPARE(m.cast<bool>(), true);
    QCOMPARE(m.attr("isMatch")().cast<bool>(), true);
    QCOMPARE(m.attr("isEmptyMatch")().cast<bool>(), false);
    QCOMPARE(m.attr("isExactMatch")().cast<bool>(), true);
    QCOMPARE(m.attr("score").cast<Score>(), 1.0);
    QCOMPARE(m.cast<Score>(), 1.0);

    m = matcher.attr("match")(QStringList({"x y", "y z"}));
    QCOMPARE(m.cast<bool>(), true);
    QCOMPARE(m.attr("isMatch")().cast<bool>(), true);
    QCOMPARE(m.attr("isEmptyMatch")().cast<bool>(), false);
    QCOMPARE(m.attr("isExactMatch")().cast<bool>(), false);
    QCOMPARE(m.attr("score").cast<Score>(), .5);
    QCOMPARE(m.cast<Score>(), .5);

    m = matcher.attr("match")("x y", "y z");
    QCOMPARE(m.cast<bool>(), true);
    QCOMPARE(m.attr("isMatch")().cast<bool>(), true);
    QCOMPARE(m.attr("isEmptyMatch")().cast<bool>(), false);
    QCOMPARE(m.attr("isExactMatch")().cast<bool>(), false);
    QCOMPARE(m.attr("score").cast<Score>(), .5);
    QCOMPARE(m.cast<Score>(), .5);

    auto mc = PyMatchConfig();
    QCOMPARE(mc.attr("fuzzy").cast<bool>(), false);
    QCOMPARE(mc.attr("ignore_case").cast<bool>(), true);
    QCOMPARE(mc.attr("ignore_diacritics").cast<bool>(), true);
    QCOMPARE(mc.attr("ignore_word_order").cast<bool>(), true);
    QCOMPARE(mc.attr("separator_regex").cast<QString>(), default_separator_regex.pattern());

    mc = PyMatchConfig("fuzzy"_a=true);
    QCOMPARE(mc.attr("fuzzy").cast<bool>(), true);
    QCOMPARE(mc.attr("ignore_case").cast<bool>(), true);
    QCOMPARE(mc.attr("ignore_diacritics").cast<bool>(), true);
    QCOMPARE(mc.attr("ignore_word_order").cast<bool>(), true);
    QCOMPARE(mc.attr("separator_regex").cast<QString>(), default_separator_regex.pattern());

    // fuzzy
    QCOMPARE(PyMatcher("tost", PyMatchConfig("fuzzy"_a=false)).attr("match")("test").cast<bool>(), false);
    QCOMPARE(PyMatcher("tost", PyMatchConfig("fuzzy"_a=true)).attr("match")("test").cast<Score>(), 0.75);

    // case
    QCOMPARE(PyMatcher("Test", PyMatchConfig("ignore_case"_a=true)).attr("match")("test").cast<bool>(), true);
    QCOMPARE(PyMatcher("Test", PyMatchConfig("ignore_case"_a=false)).attr("match")("test").cast<bool>(), false);

    // diacritics
    QCOMPARE(PyMatcher("tést", PyMatchConfig("ignore_diacritics"_a=true)).attr("match")("test").cast<bool>(), true);
    QCOMPARE(PyMatcher("tést", PyMatchConfig("ignore_diacritics"_a=false)).attr("match")("test").cast<bool>(), false);

    // order
    QCOMPARE(PyMatcher("b a", PyMatchConfig("ignore_word_order"_a=true)).attr("match")("a b").cast<bool>(), true);
    QCOMPARE(PyMatcher("b a", PyMatchConfig("ignore_word_order"_a=false)).attr("match")("a b").cast<bool>(), false);

    // seps
    QCOMPARE(PyMatcher("a_c", PyMatchConfig("separator_regex"_a="[\\s_]+")).attr("match")("a c").cast<bool>(), true);
    QCOMPARE(PyMatcher("a_c", PyMatchConfig("separator_regex"_a="[_]+")).attr("match")("a c").cast<bool>(), false);

    // contextual conversion in rank item
    auto PyRankItem = py::module::import("albert").attr("RankItem");
    auto PyStdItem = py::module::import("albert").attr("StandardItem");
    m = PyMatcher("x").attr("match")("x y");
    auto ri = PyRankItem(PyStdItem("x"), m);
    QCOMPARE(ri.attr("score").cast<Score>(), .5);

}

void PythonTests::testTriggerQueryHandler()
{
    py::exec(R"(
class TQH(albert.TriggerQueryHandler):

    def id(self):
        return "tst_id"

    def name(self):
        return "tst_name"

    def description(self):
        return "tst_desc"

    def synopsis(self, query):
        return "tst_syn" + query

    def defaultTrigger(self):
        return "tst_trigger"

    def allowTriggerRemap(self):
        return False

    def supportsFuzzyMatching(self):
        return True

    def handleTriggerQuery(self, query):
        query.add(albert.Item())
)");

    auto py = py::globals()["TQH"]();
    auto &cpp = py.cast<albert::TriggerQueryHandler&>();

    QCOMPARE(py.attr("id")().cast<QString>(), "tst_id");
    QCOMPARE(cpp.id(), "tst_id");

    QCOMPARE(py.attr("name")().cast<QString>(), "tst_name");
    QCOMPARE(cpp.name(), "tst_name");

    QCOMPARE(py.attr("description")().cast<QString>(), "tst_desc");
    QCOMPARE(cpp.description(), "tst_desc");

    QCOMPARE(py.attr("defaultTrigger")().cast<QString>(), "tst_trigger");
    QCOMPARE(cpp.defaultTrigger(), "tst_trigger");

    QCOMPARE(py.attr("synopsis")("test").cast<QString>(), "tst_syntest");
    QCOMPARE(cpp.synopsis("test"), "tst_syntest");

    QCOMPARE(py.attr("allowTriggerRemap")().cast<bool>(), false);
    QCOMPARE(cpp.allowTriggerRemap(), false);

    QCOMPARE(py.attr("supportsFuzzyMatching")().cast<bool>(), true);
    QCOMPARE(cpp.supportsFuzzyMatching(), true);

    MockQuery query(py.cast<Extension&>());

    py.attr("handleTriggerQuery")(static_cast<Query*>(&query));
    QCOMPARE(query.matches().size(), 1);

    cpp.handleTriggerQuery(query);
    QCOMPARE(query.matches().size(), 2);
}

void PythonTests::testGlobalQueryHandler()
{
    py::exec(R"(
class GQH(albert.GlobalQueryHandler):

    def id(self):
        return "tst_id"

    def name(self):
        return "tst_name"

    def description(self):
        return "tst_desc"

    def synopsis(self, query):
        return "tst_syn" + query

    def defaultTrigger(self):
        return "tst_trigger"

    def allowTriggerRemap(self):
        return False

    def supportsFuzzyMatching(self):
        return True

    def handleGlobalQuery(self, query):
        return [
            albert.RankItem(albert.StandardItem(), 1),
            albert.RankItem(albert.StandardItem(), 0)
        ]
)");

    auto py = py::globals()["GQH"]();
    auto &cpp = py.cast<albert::GlobalQueryHandler&>();

    QCOMPARE(py.attr("id")().cast<QString>(), "tst_id");
    QCOMPARE(cpp.id(), "tst_id");

    QCOMPARE(py.attr("name")().cast<QString>(), "tst_name");
    QCOMPARE(cpp.name(), "tst_name");

    QCOMPARE(py.attr("description")().cast<QString>(), "tst_desc");
    QCOMPARE(cpp.description(), "tst_desc");

    QCOMPARE(py.attr("defaultTrigger")().cast<QString>(), "tst_trigger");
    QCOMPARE(cpp.defaultTrigger(), "tst_trigger");

    QCOMPARE(py.attr("synopsis")("test").cast<QString>(), "tst_syntest");
    QCOMPARE(cpp.synopsis("test"), "tst_syntest");

    QCOMPARE(py.attr("allowTriggerRemap")().cast<bool>(), false);
    QCOMPARE(cpp.allowTriggerRemap(), false);

    QCOMPARE(py.attr("supportsFuzzyMatching")().cast<bool>(), true);
    QCOMPARE(cpp.supportsFuzzyMatching(), true);

    MockQuery query(py.cast<Extension&>());

    QCOMPARE(py.attr("handleGlobalQuery")(static_cast<Query*>(&query)).cast<py::list>().size(), 2);
    QCOMPARE(cpp.handleGlobalQuery(query).size(), 2);

    py.attr("handleTriggerQuery")(static_cast<Query*>(&query));
    QCOMPARE(query.matches().size(), 2);

    cpp.handleTriggerQuery(query);
    QCOMPARE(query.matches().size(), 4);
}

void PythonTests::testIndexQueryHandler()
{
    py::exec(R"(
class IQH(albert.IndexQueryHandler):

    def id(self):
        return "tst_id"

    def name(self):
        return "tst_name"

    def description(self):
        return "tst_desc"

    def synopsis(self, query):
        return "tst_syn" + query

    def defaultTrigger(self):
        return "tst_trigger"

    def allowTriggerRemap(self):
        return False

    def updateIndexItems(self):
        self.setIndexItems([
            albert.IndexItem(albert.StandardItem("x"), "x"),
            albert.IndexItem(albert.StandardItem("y"), "x y"),
            albert.IndexItem(albert.StandardItem("y"), "y"),
            albert.IndexItem(albert.StandardItem("z"), "z")
        ])
)");

    auto py = py::globals()["IQH"]();
    auto &cpp = py.cast<albert::IndexQueryHandler&>();

    cpp.setFuzzyMatching(false);  // builds the index

    QCOMPARE(py.attr("id")().cast<QString>(), "tst_id");
    QCOMPARE(cpp.id(), "tst_id");

    QCOMPARE(py.attr("name")().cast<QString>(), "tst_name");
    QCOMPARE(cpp.name(), "tst_name");

    QCOMPARE(py.attr("description")().cast<QString>(), "tst_desc");
    QCOMPARE(cpp.description(), "tst_desc");

    QCOMPARE(py.attr("defaultTrigger")().cast<QString>(), "tst_trigger");
    QCOMPARE(cpp.defaultTrigger(), "tst_trigger");

    QCOMPARE(py.attr("synopsis")("test").cast<QString>(), "tst_syntest");
    QCOMPARE(cpp.synopsis("test"), "tst_syntest");

    QCOMPARE(py.attr("allowTriggerRemap")().cast<bool>(), false);
    QCOMPARE(cpp.allowTriggerRemap(), false);

    // QCOMPARE(h.attr("supportsFuzzyMatching")().cast<bool>(), true);
    QCOMPARE(cpp.supportsFuzzyMatching(), true);

    MockQuery query(py.cast<Extension&>());

    auto results = py.attr("handleGlobalQuery")(static_cast<Query*>(&query));
    QCOMPARE(results.cast<py::list>().size(), 2);

    py.attr("handleTriggerQuery")(static_cast<Query*>(&query));
    QCOMPARE(query.matches().size(), 2);

}

void PythonTests::testFallbackQueryHandler()
{
    py::exec(R"(
class FQH(albert.FallbackHandler):

    def id(self):
        return "tst_id"

    def name(self):
        return "tst_name"

    def description(self):
        return "tst_desc"

    def fallbacks(self, query):
        return [albert.StandardItem(query)]
)");

    auto py = py::globals()["FQH"]();
    auto &cpp = py.cast<albert::FallbackHandler&>();

    QCOMPARE(py.attr("id")().cast<QString>(), "tst_id");
    QCOMPARE(cpp.id(), "tst_id");

    QCOMPARE(py.attr("name")().cast<QString>(), "tst_name");
    QCOMPARE(cpp.name(), "tst_name");

    QCOMPARE(py.attr("description")().cast<QString>(), "tst_desc");
    QCOMPARE(cpp.description(), "tst_desc");

    QCOMPARE(py.attr("fallbacks")("test").cast<py::list>().size(), 1);
    QCOMPARE(cpp.fallbacks("test").size(), 1);
}
