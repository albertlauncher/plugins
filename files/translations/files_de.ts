<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>AbstractBrowser</name>
    <message>
        <location filename="../src/filebrowsers.cpp" line="15"/>
        <source>Browse files by path</source>
        <translation>Dateien anhand eines Pfades durchsuchen</translation>
    </message>
</context>
<context>
    <name>AbstractFileItem</name>
    <message>
        <location filename="../src/fileitems.cpp" line="49"/>
        <source>Open with default application</source>
        <translation>Mit Standardanwendung öffnen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="54"/>
        <source>Execute</source>
        <translation>Ausführen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="58"/>
        <source>Reveal in file browser</source>
        <translation>Im Dateibrowser anzeigen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="63"/>
        <source>Open terminal here</source>
        <translation>Terminal hier öffnen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="68"/>
        <source>Copy file to clipboard</source>
        <translation>Datei in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="94"/>
        <source>Copy path to clipboard</source>
        <translation>Pfad in die Zwischenablage kopieren</translation>
    </message>
</context>
<context>
    <name>ConfigWidget</name>
    <message>
        <location filename="../src/configwidget.ui" line="29"/>
        <source>The files plugin provides three extensions to access files. The main extension indexes the file system as specified in the settings and provides a lookup by filename. The other two file browsing extensions are triggered by `/` and `~` and provide a way to browse through the file system using paths. This is handy to access files that are _not_ indexed. In combination with the tab completion this is a nice way to browse the file system.</source>
        <translation>Das Dateien-Plugin bietet drei Erweiterungen um auf Dateien zuzugreifen. Die Haupterweiterung indexiert das Dateisystem gemäß den Einstellungen und ermöglicht eine Suche nach Dateinamen. Die anderen beiden Dateibrowser-Erweiterungen werden durch die Kürzel `/` und `~` ausgelöst und bieten eine Möglichkeit, durch das Dateisystem unter Verwendung von Pfaden zu navigieren. Dies ist praktisch, um auf Dateien zuzugreifen, die _nicht_ indexiert sind. In Kombination mit der Tab-Vervollständigung ist dies eine gute Möglichkeit, das Dateisystem zu durchsuchen.</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="92"/>
        <source>Path settings</source>
        <translation>Pfad-Einstellungen</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="112"/>
        <source>Indexing</source>
        <translation>Indexierung</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="118"/>
        <source>Index hidden files</source>
        <translation>Versteckte Dateien indexieren</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="132"/>
        <source>Follow links</source>
        <translation>Links folgen</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="139"/>
        <source>This option should by used with care and only if necessary. It may cause indexing of way more files you wanted when the indexed file tree contains links to directories outside the specified file tree.</source>
        <translation>Diese Option sollte mit Vorsicht und nur bei Bedarf verwendet werden. Sie kann dazu führen, dass mehr Dateien indexiert werden als beabsichtigt, wenn der indexierte Dateibaum Links zu Verzeichnissen außerhalb des angegebenen Dateibaums enthält.</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="149"/>
        <source>Max Depth</source>
        <translation>Maximale Tiefe</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="169"/>
        <source>Scan interval</source>
        <extracomment>Abbr. minutes</extracomment>
        <translation>Scan-Intervall</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="182"/>
        <source> min</source>
        <extracomment>Abbr. minutes</extracomment>
        <translation> Min</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="198"/>
        <source>Watch filesystem</source>
        <translation>Dateisystem überwachen</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="218"/>
        <source>Ignore patterns</source>
        <translation>Ignoriermuster</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="228"/>
        <source>MIME types</source>
        <translation>MIME-Typen</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="269"/>
        <source>Directories</source>
        <translation>Verzeichnisse</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="276"/>
        <source>Documents</source>
        <translation>Dokumente</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="283"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="290"/>
        <source>Video</source>
        <translation>Video</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="297"/>
        <source>Images</source>
        <translation>Bilder</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="310"/>
        <source>Advanced</source>
        <translation>Erweitert</translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="327"/>
        <source>Case sensitive file browsers</source>
        <translation>Groß-/Kleinschreibung in Dateibrowsern beachten</translation>
    </message>
    <message>
        <location filename="../src/configwidget.cpp" line="35"/>
        <source>Choose directory</source>
        <translation>Verzeichnis auswählen</translation>
    </message>
    <message>
        <location filename="../src/configwidget.cpp" line="133"/>
        <source>Enabling file system watches comes with caveats. You should only activate this option if you know what you are doing. A lot of file system changes (compilation, installing, etc) while having watches enabled can put your system under high load.</source>
        <translation>Das Aktivieren der Dateisystemüberwachung ist mit Einschränkungen verbunden. Sie sollten diese Option nur aktivieren, wenn Sie wissen, was Sie tun. Viele Dateisystemänderungen (Kompilieren, Installieren usw.) bei aktivierter Überwachung können Ihr System stark belasten.</translation>
    </message>
</context>
<context>
    <name>FsIndexPath</name>
    <message numerus="yes">
        <location filename="../src/fsindexpath.cpp" line="57"/>
        <source>Indexed %n directories in %2.</source>
        <translation>
            <numerusform>%1 Verzeichnis in %2 indexiert.</numerusform>
            <numerusform>%1 Verzeichnisse in %2 indexiert.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>HomeBrowser</name>
    <message>
        <location filename="../src/filebrowsers.cpp" line="74"/>
        <source>Home directory browser</source>
        <translation>Home-Verzeichnis-Browser</translation>
    </message>
</context>
<context>
    <name>MimeFilterDialog</name>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="14"/>
        <source>MIME filters</source>
        <translation>MIME-Filter</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="32"/>
        <source>Ignore patterns are Perl-compatible regular expressions that can be utilized to exclude files from indexing. The matched filepath is relative to the root directory. By default, patterns are excluding, but prepending an &apos;!&apos; to a pattern makes it including. Filenames are matched in the order specified by the filter list below.</source>
        <translation>Ignoriermuster sind Perl-kompatible reguläre Ausdrücke, die dazu verwendet werden können, Dateien von der Indexierung auszuschließen. Der abgeglichene Pfad ist relativ zum Stammverzeichnis. Durch das Voranstellen eines &apos;!&apos; wird das Muster einschließend. Dateinamen werden in der Reihenfolge der unten stehenden Filterliste abgeglichen.</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="47"/>
        <source>MIME Types</source>
        <translation>MIME-Typen</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="78"/>
        <source>Filter for the mimetypes…</source>
        <translation>Filter für die MIME-Typen…</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="88"/>
        <source>Filter patterns</source>
        <translation>Filtermuster</translation>
    </message>
</context>
<context>
    <name>NameFilterDialog</name>
    <message>
        <location filename="../src/namefilterdialog.ui" line="32"/>
        <source>Ignore patterns are Perl-compatible regular expressions that can be utilized to exclude files from indexing. The filepath matched is relative to the root directory. Prepending &apos;!&apos; makes the pattern including. Filenames are matched in the order given by the filter list below.</source>
        <translation>Ignoriermuster sind Perl-kompatible reguläre Ausdrücke, die dazu verwendet werden können, Dateien von der Indexierung auszuschließen. Der abgeglichene Pfad ist relativ zum Stammverzeichnis. Durch das Voranstellen eines &apos;!&apos; wird das Muster einschließend. Dateinamen werden in der Reihenfolge der unten stehenden Filterliste abgeglichen.</translation>
    </message>
    <message>
        <location filename="../src/namefilterdialog.ui" line="14"/>
        <location filename="../src/namefilterdialog.ui" line="47"/>
        <source>Ignore patterns</source>
        <translation>Ignoriermuster</translation>
    </message>
</context>
<context>
    <name>Plugin</name>
    <message>
        <location filename="../src/plugin.cpp" line="72"/>
        <source>Update index</source>
        <translation>Index aktualisieren</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="73"/>
        <source>Update the file index</source>
        <translation>Den Datei-Index aktualisieren</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="75"/>
        <source>Scan</source>
        <translation>Scannen</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="129"/>
        <source>Trash</source>
        <translation>Papierkorb</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="130"/>
        <source>Your trash folder</source>
        <translation>Ihr Papierkorb-Verzeichnis</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="134"/>
        <source>Open trash</source>
        <translation>Papierkorb öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="144"/>
        <source>Empty trash</source>
        <translation>Papierkorb leeren</translation>
    </message>
</context>
<context>
    <name>RootBrowser</name>
    <message>
        <location filename="../src/filebrowsers.cpp" line="60"/>
        <source>Root directory browser</source>
        <translation>Stammverzeichnis-Browser</translation>
    </message>
</context>
</TS>
