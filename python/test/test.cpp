//// Copyright (c) 2022 Manuel Schneider

//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
//#define DOCTEST_CONFIG_COLORS_ANSI
//#include "doctest/doctest.h"
//#include "cast_specialization.h"
//#include "../../src/pluginregistry.h"
//#include "albert/extension/queryhandler/triggerqueryhandler.h"
//#include "pypluginloader.h"
//#include <QDebug>
//#include <QFile>
//#include <QFileInfo>
//#include <QString>
//#include <QTextStream>
//#include <exception>
//using namespace albert;
//using namespace std;

//static void writeFile(const QFileInfo& fileinfo, const QString& content) {
//    QFile file(fileinfo.filePath());
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        // Failed to open the file. Handle the error here.
//        qDebug() << "Failed to open the file for writing:" << file.errorString();
//        return;
//    }
//    QTextStream out(&file);
//    out << content;
//    file.close();
//}


////class PluginLoaderMock : public albert::PluginLoader
////{
////    PluginMetaData metadata;
////public:
////    PluginLoaderMock() :  PluginLoader("") {
////        metadata.id = "test";
////    }
////    virtual const PluginProvider &provider() const;
////    virtual const PluginMetaData &metaData() const { return metadata; }
////    virtual PluginInstance *instance() const { return {}; }
////    virtual QString load() { return {}; }
////    virtual QString unload() { return {}; }
////} pluginLoaderMock;


////class PluginProviderMock : public PluginProvider {
////    QString id() const override  { return {}; }
////    QString name() const override { return {}; }
////    QString description() const override { return {}; }
////    virtual std::vector<PluginLoader*> plugins() override { return {&pluginLoaderMock}; }
////} pluginProviderMock;

////const PluginProvider &PluginLoaderMock::provider() const { return pluginProviderMock; }


//TEST_CASE("Plugin instance")
//{
//    ExtensionRegistry extension_registry;
//    PluginRegistry plugin_registry{extension_registry};
//    Plugin test_plugin;



//    QFileInfo fileinfo("test.py");
//    QString plugin_content = R"(
//from albert import *
//md_iid = '2.0'
//md_version = "0.0"
//md_name = "testname"
//md_description = "testdescription"

//class Plugin(PluginInstance, TriggerQueryHandler):
//    def __init__(self):
//        PluginInstance.__init__(self)
//        TriggerQueryHandler.__init__(
//            self,
//            id=md_id,
//            name=md_name,
//            description=md_description,
//            defaultTrigger='testtrigger',
//            synopsis='testsynopsis'
//        )

//    def handleTriggerQuery(self, query):
//        pass
//)";

//    writeFile(fileinfo, plugin_content);

//    PyPluginLoader loader(*mock_plugin, QFileInfo("test.py"));
//    auto err = loader.load();
//    qCritical() << err;
//    CHECK(err.isNull());

//    auto &plugin = *loader.instance();
//    CHECK(&plugin != nullptr);
//    CHECK(plugin.id() == "test");
//    CHECK(plugin.name() == "testname");
//    CHECK(plugin.description() == "testdescription");

////    auto tqh = dynamic_cast<TriggerQueryHandler*>(plugin.extensions()[0]);

//}

////TEST_CASE("Benchmark new levenshtein")
////{
//////    int test_count = 100000;
//////
//////    srand((unsigned)time(NULL) * getpid());
//////
//////    vector<QString> strings(test_count);
//////    auto lens = {4,8,16,24};
//////    auto divisor=4;
//////    cout << "Randoms"<<endl;
//////    for (int len : lens){
//////        int k = floor(len/divisor);
//////        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//////        for (auto &string : strings)
//////            string = QString::fromStdString(gen_random(len));
//////        levenshtein_compare_benchmarks_and_check_results(strings, k);
//////    }
//////
//////    cout << "Equals"<<endl;
//////    for (int len : lens) {
//////        int k = floor(len/divisor);
//////        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//////        for (auto &string : strings)
//////            string = QString(len, 'a');
//////        levenshtein_compare_benchmarks_and_check_results(strings, k);
//////    }
//////
//////    cout << "Halfhalf equal random"<<endl;
//////    for (int len : lens) {
//////        int k = floor(len/divisor);
//////        cout << "len: "<< setw(2)<<len<<". k: "<<k<<" ";
//////        for (auto &string : strings)
//////            string = QString("%1%2").arg(QString(len/2, 'a'), QString::fromStdString(gen_random(len/2)));
//////        levenshtein_compare_benchmarks_and_check_results(strings, k);
//////    }
////}


////TEST_CASE("Levenshtein")
////{
////    Levenshtein l;

////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefg", "ab_efghij", 0) == 1);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefg", "ab_efghij", 2) == 2);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefg", "ab_efgh", 2) == 2);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcde__h", "abcdefghij", 1) == 2);

////    // plain
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "test", 0) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "test", 1) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "test", 2) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "test_", 0) == 0);

////    //fuzzy substitution
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "_est____", 1) == 1);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "__st____", 1) == 2);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "_est____", 2) == 1);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "__st____", 2) == 2);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "___t____", 2) == 3);

////    //fuzzy deletion
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "ttest____", 1) == 1);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "tttest____", 1) == 2);

////    //fuzzy insertion
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "est____", 1) == 1);
////    CHECK(l.computePrefixEditDistanceWithLimit("test", "st____", 1) > 1);

////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 1) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 2) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 3) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 4) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 5) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 6) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 7) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abcdefghij", "abcdefghij", 8) == 0);

////    // Bug 2022-11-20 string is smaller than prefix
////    CHECK(l.computePrefixEditDistanceWithLimit("abc", "abc", 1) == 0);
////    CHECK(l.computePrefixEditDistanceWithLimit("abc", "ab", 1) == 1);
////    CHECK(l.computePrefixEditDistanceWithLimit("abc", "a", 1) == 2);
////    CHECK(l.computePrefixEditDistanceWithLimit("abc", "", 1) == 2);
////}

////TEST_CASE("Index")
////{
////    auto match = [&](const QStringList& item_strings, const QString& search_string, bool case_sesitivity, int q, int fuzzy){

////        auto index = ItemIndex("[ ]+", case_sesitivity, q, fuzzy);
////        vector<IndexItem> index_items;
////        for (auto &string : item_strings)
////            index_items.emplace_back(make_shared<StandardItem>(string), string);
////        index.setItems(::move(index_items));
////        return index.search(search_string, true);
////    };

////    // case sensitivity
////    CHECK(match({"a","A"}, "a", true, 0, 0).size() == 1);
////    CHECK(match({"a","A"}, "A", true, 0, 0).size() == 1);

////    // intersection
////    CHECK(match({"a b","b c","c d"}, "a", false, 0, 0).size() == 1);
////    CHECK(match({"a b","b c","c d"}, "b", false, 0, 0).size() == 2);
////    CHECK(match({"a b","b c","c d"}, "c", false, 0, 0).size() == 2);
////    CHECK(match({"a b","b c","c d"}, "d", false, 0, 0).size() == 1);
////    CHECK(match({"a b","b c","c d"}, "b c", false, 0, 0).size() == 1);

////    // sequence
////    CHECK(match({"a b","b a","a b"}, "a b", false, 0, 0).size() == 2);
////    CHECK(match({"a b","b a","a b"}, "b a", false, 0, 0).size() == 1);

////    // fuzzy
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "ab_", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "a__", false, 2, 3).size() == 0);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdef", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_e_", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "a_c_e_", false, 2, 3).size() == 0);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefghi", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefg_i", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcde_g_i", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_e_g_i", false, 2, 3).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "a_c_e_g_i", false, 2, 3).size() == 0);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcd", false, 2, 4).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_", false, 2, 4).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "ab__", false, 2, 4).size() == 0);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefgh", false, 2, 4).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcdefg_", false, 2, 4).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abcde_g_", false, 2, 4).size() == 1);
////    CHECK(match({"abcdefghijklmnopqrstuvwxyz"}, "abc_e_g_", false, 2, 4).size() == 0);

////    // score
////    CHECK(qFuzzyCompare(match({"a","ab","abc"}, "a", false, 2, 3).size(), 3.0f));
////    CHECK(qFuzzyCompare(match({"a","ab","abc"}, "a", false, 2, 3)[0].score, 1.0f/3.0f));
////    CHECK(qFuzzyCompare(match({"a","ab","abc"}, "a", false, 2, 3)[1].score, 1.0f/2.0f));
////    CHECK(qFuzzyCompare(match({"a","ab","abc"}, "a", false, 2, 3)[2].score, 1.0f));
////    CHECK(qFuzzyCompare(match({"abc","abd"}, "abe", false, 2, 3)[0].score, 2.0f/3.0f));
////    CHECK(qFuzzyCompare(match({"abc","abd"}, "abe", false, 2, 3)[1].score, 2.0f/3.0f));

////    std::vector<albert::RankItem> M = match({"abc","abd","abcdef"}, "abc", false, 2, 3);
////    sort(M.begin(), M.end(), [](auto &a, auto &b){ return a.item->id() < b.item->id(); });
////    CHECK(qFuzzyCompare(M[0].score, 3.0f/3.0f));
////    CHECK(qFuzzyCompare(M[1].score, 3.0f/6.0f));
////    CHECK(qFuzzyCompare(M[2].score, 2.0f/3.0f));
////}


