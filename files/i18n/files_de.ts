<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
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
    <name>FileItem</name>
    <message>
        <location filename="../src/fileitems.cpp" line="47"/>
        <source>Open with default application</source>
        <translation>Mit Standardanwendung öffnen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="57"/>
        <source>Execute</source>
        <translation>Ausführen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="66"/>
        <source>Reveal in file browser</source>
        <translation>Im Dateibrowser anzeigen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="74"/>
        <source>Open terminal here</source>
        <translation>Terminal hier öffnen</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="84"/>
        <source>Copy file to clipboard</source>
        <translation>Datei in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="113"/>
        <source>Copy path to clipboard</source>
        <translation>Pfad in die Zwischenablage kopieren</translation>
    </message>
</context>
<context>
    <name>FilePathBrowser</name>
    <message>
        <location filename="../src/filebrowsers.cpp" line="62"/>
        <source>Root browser</source>
        <translation>Root-Browser</translation>
    </message>
    <message>
        <location filename="../src/filebrowsers.cpp" line="68"/>
        <source>Browse root directory by path</source>
        <translation>Durchsuche das Stammverzeichnis anhand eines Pfades</translation>
    </message>
    <message>
        <location filename="../src/filebrowsers.cpp" line="87"/>
        <source>Home browser</source>
        <translation>Home-Browser</translation>
    </message>
    <message>
        <location filename="../src/filebrowsers.cpp" line="93"/>
        <source>Browse home directory by path</source>
        <translation>Durchsuche das Nutzerverzeichnis anhand eines Pfades</translation>
    </message>
</context>
<context>
    <name>FsIndexPath</name>
    <message numerus="yes">
        <location filename="../src/fsindexpath.cpp" line="57"/>
        <source>Indexed %n directories in %1.</source>
        <translation>
            <numerusform>%n Verzeichnis in %1 indexiert.</numerusform>
            <numerusform>%n Verzeichnisse in %1 indexiert.</numerusform>
        </translation>
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
        <source>MIME filters are used while indexing to filter files by their MIME type. Files whose MIME type is not matched by any of the MIME filter patterns below are not indexed. The filter patterns support wildcard characters (*). Utilize the MIME types list on the left to find supported MIME types.</source>
        <translation>MIME-Filter werden während der Indexierung verwendet, um Dateien anhand ihres MIME Typs zu filtern. Dateien, deren MIME-Typ mit keinem der folgenden MIME-Filtermuster übereinstimmt, werden nicht indexiert. Die Filtermuster unterstützen Platzhalterzeichen (*). Nutzen Sie die verfügbare Liste der MIME-Typen auf der linken Seite, um unterstützte MIME-Typen zu finden.</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="47"/>
        <source>Available MIME types</source>
        <translation>Verfügbare MIME-Filter</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="78"/>
        <source>Filter the available MIME types</source>
        <translation>Verfügbare MIME-Filter filtern</translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="88"/>
        <source>MIME filter patterns</source>
        <translation>MIME-Filtermuster</translation>
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
</TS>
