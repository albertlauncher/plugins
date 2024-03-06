<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
<context>
    <name>ConfigWidget</name>
    <message>
        <location filename="../src/configwidget.ui" line="29"/>
        <source>Math expression evaluator based on the powerful qalculate library. See the [Qalculate! docs](https://qalculate.github.io/manual/index.html).

The global handler evaluates basic math expressions. Use the trigger handler to enable the full feature set.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="49"/>
        <source>Angle unit:</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="59"/>
        <source>The angle unit for trigonometric functions.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="63"/>
        <source>None</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="68"/>
        <source>Radian</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="73"/>
        <source>Degree</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="78"/>
        <source>Gradian</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="86"/>
        <source>Parsing mode:</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="96"/>
        <source>&lt;p&gt;In the &lt;/b&gt;implicit multiplication first&lt;/b&gt; parse mode, implicit multiplication is parsed before explicit multiplication, i.e. 12/2(1+2) = 12/(2*3) = 2. &lt;/p&gt;
&lt;p&gt;In the &lt;/b&gt;conventional&lt;/b&gt; parse mode implicit multiplication does not differ from explicit multiplication, i.e. 12/2(1+2) = 12/2*3 = 18.&lt;/p&gt;
&lt;p&gt;The &lt;/b&gt;adaptive&lt;/b&gt; parse mode works as the implicit multiplication first mode, unless spaces are found. I.e. 1/5x = 1/(5*x), but 1/5 x = (1/5)*x. In the adaptive mode unit expressions are parsed separately 5 m/5 m/s = (5*m)/(5*(m/s)) = 1 s.&lt;/p&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="102"/>
        <source>Adaptive</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="107"/>
        <source>Implicit multiplication first</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="112"/>
        <source>Conventional</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="120"/>
        <source>Precision:</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="130"/>
        <source>Precision for approximate calculations.</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Plugin</name>
    <message>
        <location filename="../src/plugin.cpp" line="64"/>
        <source>&lt;math expression&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="109"/>
        <source>Copy result to clipboard</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="110"/>
        <source>Copy equation to clipboard</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="111"/>
        <source>Result of %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="112"/>
        <source>Approximate result of %1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="201"/>
        <source>Evaluation error.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="202"/>
        <source>Visit documentation</source>
        <translation></translation>
    </message>
</context>
</TS>
