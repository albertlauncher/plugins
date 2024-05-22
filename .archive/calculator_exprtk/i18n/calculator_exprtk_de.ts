<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>ConfigWidget</name>
    <message>
        <source>Angle unit:</source>
        <translation type="vanished">Winkeleinheit:</translation>
    </message>
    <message>
        <source>The angle unit for trigonometric functions.</source>
        <translation type="vanished">Die Winkeleinheit for trigonometrische Funktionen.</translation>
    </message>
    <message>
        <source>Math expression evaluator based on the powerful qalculate library. See the [Qalculate! docs](https://qalculate.github.io/manual/index.html).

The global handler evaluates basic math expressions. Use the trigger handler to enable the full feature set.</source>
        <translation type="vanished">Mathematischer Ausdrucksauswerter basierend auf der leistungsstarken Qalculate-Bibliothek. Siehe die [Qalculate!-Dokumentation](https://qalculate.github.io/manual/index.html).

Der globale Handler wertet grundlegende mathematische Ausdrücke aus. Verwenden Sie den Trigger-Handler, um den vollständigen Funktionsumfang zu aktivieren.</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Keine</translation>
    </message>
    <message>
        <source>Degree</source>
        <translation type="vanished">Grad</translation>
    </message>
    <message>
        <source>Parsing mode:</source>
        <translation type="vanished">Parse-Modus:</translation>
    </message>
    <message>
        <source>&lt;p&gt;In the &lt;/b&gt;implicit multiplication first&lt;/b&gt; parse mode, implicit multiplication is parsed before explicit multiplication, i.e. 12/2(1+2) = 12/(2*3) = 2. &lt;/p&gt;
&lt;p&gt;In the &lt;/b&gt;conventional&lt;/b&gt; parse mode implicit multiplication does not differ from explicit multiplication, i.e. 12/2(1+2) = 12/2*3 = 18.&lt;/p&gt;
&lt;p&gt;The &lt;/b&gt;adaptive&lt;/b&gt; parse mode works as the implicit multiplication first mode, unless spaces are found. I.e. 1/5x = 1/(5*x), but 1/5 x = (1/5)*x. In the adaptive mode unit expressions are parsed separately 5 m/5 m/s = (5*m)/(5*(m/s)) = 1 s.&lt;/p&gt;</source>
        <translation type="vanished">&lt;p&gt;Im &lt;b&gt;implizite Multiplikation zuerst&lt;/b&gt;-Analysemodus wird die implizite Multiplikation vor der expliziten Multiplikation ausgewertet, d.h. 12/2(1+2) = 12/(2*3) = 2.&lt;/p&gt;
&lt;p&gt;Im &lt;b&gt;konventionellen&lt;/b&gt; Analysemodus unterscheidet sich die implizite Multiplikation nicht von der expliziten Multiplikation, d.h. 12/2(1+2) = 12/2*3 = 18.&lt;/p&gt;
&lt;p&gt;Der &lt;b&gt;adaptive&lt;/b&gt; Analysemodus funktioniert wie der Modus &quot;implizite Multiplikation zuerst&quot;, es sei denn, es werden Leerzeichen gefunden. Das bedeutet, 1/5x = 1/(5*x), aber 1/5 x = (1/5)*x. Im adaptiven Modus werden Einheitenausdrücke separat ausgewertet: 5 m/5 m/s = (5*m)/(5*(m/s)) = 1 s.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Adaptive</source>
        <translation type="vanished">Adaptiv</translation>
    </message>
    <message>
        <source>Implicit multiplication first</source>
        <translation type="vanished">Implizite Multiplikation zuerst</translation>
    </message>
    <message>
        <source>Conventional</source>
        <translation type="vanished">Konventionell</translation>
    </message>
    <message>
        <source>Precision:</source>
        <translation type="vanished">Präzision:</translation>
    </message>
    <message>
        <source>Precision for approximate calculations.</source>
        <translation type="vanished">Präzision für ungefähre Berechnungen.</translation>
    </message>
</context>
<context>
    <name>Plugin</name>
    <message>
        <source>&lt;math expression&gt;</source>
        <translation type="vanished">&lt;Mathematischer Ausdruck&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="57"/>
        <source>Copy result to clipboard</source>
        <translation>Ergebnis in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="61"/>
        <source>Copy equation to clipboard</source>
        <translation>Gleichung in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="77"/>
        <source>This plugin is based on the %1 library and uses double precision floats which have a well known &lt;a href=%2&gt;accuracy problem&lt;/a&gt;.</source>
        <translation>Dieses Plugin basiert auf der Bibliothek %1 und verwendet Gleitkommazahlen mit doppelter Genauigkeit, die ein bekanntes &lt;a href=%2&gt;Genauigkeitsproblem&lt;/a&gt; haben.</translation>
    </message>
    <message>
        <source>Calculator plugin based on the powerful %1 library.</source>
        <translation type="vanished">Rechner-Plugin basierend auf der leistungsstarken Bibliothek %1.</translation>
    </message>
    <message>
        <source>This plugin uses double precision floats which introduce &lt;a href=%1&gt;accuracy problems&lt;/a&gt;.</source>
        <translation type="vanished">Dieses Plugin verwendet Gleitkommazahlen mit doppelter Genauigkeit, die &lt;a href=%1&gt;Genauigkeitsprobleme&lt;/a&gt; einführen.</translation>
    </message>
    <message>
        <source>This plugin uses double precision floats which introduce &lt;a href=%2&gt;accuracy problems&lt;/a&gt;.</source>
        <translation type="vanished">Dieses Plugin verwendet Gleitkommazahlen mit doppelter Genauigkeit, die &lt;a href=%1&gt;Genauigkeitsprobleme&lt;/a&gt; einführen.</translation>
    </message>
    <message>
        <source>Calculator plugin based on the powerful %1 library.&lt;br&gt; This plugin uses double precision floats which introduce &lt;a href=%2&gt;accuracy problems&lt;/a&gt;.</source>
        <translation type="obsolete">Rechner-Plugin basierend auf der leistungsstarken Bibliothek %1.&lt;br&gt;Dieses Plugin verwendet Gleitkommazahlen mit doppelter Genauigkeit, die die bekannten &lt;a href=&quot;https://de.wikipedia.org/wiki/Gleitkomma#Genauigkeitsprobleme&quot;&gt;Genauigkeitsprobleme&lt;/a&gt; einführen.</translation>
    </message>
    <message>
        <source>Calculator plugin based on the powerful &lt;i&gt;Mathematical Expression Toolkit Library &lt;a href=&quot;https://www.partow.net/programming/exprtk/&quot;&gt;exprtk&lt;/a&gt;&lt;/i&gt;.&lt;br&gt;This plugin uses double precision floats which introduce the well known &lt;a href=&quot;https://en.wikipedia.org/wiki/Floating-point_arithmetic#Accuracy_problems&quot;&gt;accuracy problems&lt;/a&gt;.</source>
        <translation type="vanished">Rechner-Plugin basierend auf der leistungsstarken &lt;i&gt;Mathematical Expression Toolkit Library &lt;a href=&quot;https://www.partow.net/programming/exprtk/&quot;&gt;exprtk&lt;/a&gt;&lt;/i&gt;.&lt;br&gt;Dieses Plugin verwendet Gleitkommazahlen mit doppelter Genauigkeit, die die bekannten &lt;a href=&quot;https://de.wikipedia.org/wiki/Gleitkomma#Genauigkeitsprobleme&quot;&gt;Genauigkeitsprobleme&lt;/a&gt; einführen.</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="53"/>
        <source>Result of %1</source>
        <translation>Ergebnis von %1</translation>
    </message>
    <message>
        <source>Approximate result of %1</source>
        <translation type="vanished">Ungefähres Ergebnis von %1</translation>
    </message>
    <message>
        <source>Evaluation error.</source>
        <translation type="vanished">Auswertungsfehler.</translation>
    </message>
    <message>
        <source>Visit documentation</source>
        <translation type="vanished">Dokumentation ansehen</translation>
    </message>
</context>
</TS>
