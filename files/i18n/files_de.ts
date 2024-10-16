<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>ConfigWidget</name>
    <message>
        <source>The files plugin provides three extensions to access files. The main extension indexes the file system as specified in the settings and provides a lookup by filename. The other two file browsing extensions are triggered by `/` and `~` and provide a way to browse through the file system using paths. This is handy to access files that are _not_ indexed. In combination with the tab completion this is a nice way to browse the file system.</source>
        <translation>Das Dateien-Plugin bietet drei Erweiterungen um auf Dateien zuzugreifen. Die Haupterweiterung indexiert das Dateisystem gemäß den Einstellungen und ermöglicht eine Suche nach Dateinamen. Die anderen beiden Dateibrowser-Erweiterungen werden durch die Kürzel `/` und `~` ausgelöst und bieten eine Möglichkeit, durch das Dateisystem unter Verwendung von Pfaden zu navigieren. Dies ist praktisch, um auf Dateien zuzugreifen, die _nicht_ indexiert sind. In Kombination mit der Tab-Vervollständigung ist dies eine gute Möglichkeit, das Dateisystem zu durchsuchen.</translation>
    </message>
    <message>
        <source>Path settings</source>
        <translation>Pfad-Einstellungen</translation>
    </message>
    <message>
        <source>Indexing</source>
        <translation>Indexierung</translation>
    </message>
    <message>
        <source>Index hidden files</source>
        <translation>Versteckte Dateien indexieren</translation>
    </message>
    <message>
        <source>Follow links</source>
        <translation>Links folgen</translation>
    </message>
    <message>
        <source>This option should by used with care and only if necessary. It may cause indexing of way more files you wanted when the indexed file tree contains links to directories outside the specified file tree.</source>
        <translation>Diese Option sollte mit Vorsicht und nur bei Bedarf verwendet werden. Sie kann dazu führen, dass mehr Dateien indexiert werden als beabsichtigt, wenn der indexierte Dateibaum Links zu Verzeichnissen außerhalb des angegebenen Dateibaums enthält.</translation>
    </message>
    <message>
        <source>Max depth</source>
        <translation>Maximale Tiefe</translation>
    </message>
    <message>
        <source>Scan interval</source>
        <translation>Scan-Intervall</translation>
    </message>
    <message>
        <source> min</source>
        <extracomment>Abbr. minutes</extracomment>
        <translation> Min</translation>
    </message>
    <message>
        <source>Watch filesystem</source>
        <translation>Dateisystem überwachen</translation>
    </message>
    <message>
        <source>Ignore patterns</source>
        <translation>Ignoriermuster</translation>
    </message>
    <message>
        <source>MIME types</source>
        <translation>MIME-Typen</translation>
    </message>
    <message>
        <source>Directories</source>
        <translation>Verzeichnisse</translation>
    </message>
    <message>
        <source>Documents</source>
        <translation>Dokumente</translation>
    </message>
    <message>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <source>Video</source>
        <translation>Video</translation>
    </message>
    <message>
        <source>Images</source>
        <translation>Bilder</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Erweitert</translation>
    </message>
    <message>
        <source>Choose directory</source>
        <translation>Verzeichnis auswählen</translation>
    </message>
    <message>
        <source>Enabling file system watches comes with caveats. You should only activate this option if you know what you are doing. A lot of file system changes (compilation, installing, etc) while having watches enabled can put your system under high load.</source>
        <translation>Das Aktivieren der Dateisystemüberwachung ist mit Einschränkungen verbunden. Sie sollten diese Option nur aktivieren, wenn Sie wissen, was Sie tun. Viele Dateisystemänderungen (Kompilieren, Installieren usw.) bei aktivierter Überwachung können Ihr System stark belasten.</translation>
    </message>
    <message>
        <source>Index the entire file path</source>
        <translation>Den ganzen Dateipfad indexieren</translation>
    </message>
    <message>
        <source>File browsers</source>
        <translation>Dateibrowser</translation>
    </message>
    <message>
        <source>Match case-sensitive</source>
        <translation>Groß-/Kleinschreibung bei Suche berücksichtigen</translation>
    </message>
    <message>
        <source>Show hidden files</source>
        <translation>Versteckte Dateien anzeigen</translation>
    </message>
    <message>
        <source>Sort entries case-insensitve</source>
        <translation>Groß- und Kleinschreibung bei der Sortierung nicht beachten.</translation>
    </message>
    <message>
        <source>Show dirs first</source>
        <translation>Verzeichnisse zuerst anzeigen</translation>
    </message>
</context>
<context>
    <name>FileItem</name>
    <message>
        <source>Open with default application</source>
        <translation>Mit Standardanwendung öffnen</translation>
    </message>
    <message>
        <source>Execute</source>
        <translation>Ausführen</translation>
    </message>
    <message>
        <source>Reveal in file browser</source>
        <translation>Im Dateibrowser anzeigen</translation>
    </message>
    <message>
        <source>Open terminal here</source>
        <translation>Terminal hier öffnen</translation>
    </message>
    <message>
        <source>Copy file to clipboard</source>
        <translation>Datei in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <source>Copy path to clipboard</source>
        <translation>Pfad in die Zwischenablage kopieren</translation>
    </message>
</context>
<context>
    <name>FsIndexPath</name>
    <message numerus="yes">
        <source>Indexed %n directories in %1.</source>
        <translation>
            <numerusform>%n Verzeichnis in %1 indexiert.</numerusform>
            <numerusform>%n Verzeichnisse in %1 indexiert.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>HomeBrowser</name>
    <message>
        <source>Home browser</source>
        <translation>Home-Browser</translation>
    </message>
    <message>
        <source>Browse home directory by path</source>
        <translation>Durchsuche das Nutzerverzeichnis anhand eines Pfades</translation>
    </message>
</context>
<context>
    <name>MimeFilterDialog</name>
    <message>
        <source>MIME filters</source>
        <translation>MIME-Filter</translation>
    </message>
    <message>
        <source>MIME filters are used while indexing to filter files by their MIME type. Files whose MIME type is not matched by any of the MIME filter patterns below are not indexed. The filter patterns support wildcard characters (*). Utilize the MIME types list on the left to find supported MIME types.</source>
        <translation>MIME-Filter werden während der Indexierung verwendet, um Dateien anhand ihres MIME Typs zu filtern. Dateien, deren MIME-Typ mit keinem der folgenden MIME-Filtermuster übereinstimmt, werden nicht indexiert. Die Filtermuster unterstützen Platzhalterzeichen (*). Nutzen Sie die verfügbare Liste der MIME-Typen auf der linken Seite, um unterstützte MIME-Typen zu finden.</translation>
    </message>
    <message>
        <source>Available MIME types</source>
        <translation>Verfügbare MIME-Filter</translation>
    </message>
    <message>
        <source>Filter the available MIME types</source>
        <translation>Verfügbare MIME-Filter filtern</translation>
    </message>
    <message>
        <source>MIME filter patterns</source>
        <translation>MIME-Filtermuster</translation>
    </message>
</context>
<context>
    <name>NameFilterDialog</name>
    <message>
        <source>Ignore patterns</source>
        <translation>Ignoriermuster</translation>
    </message>
    <message>
        <source>Ignore patterns are Perl-compatible regular expressions that can be utilized to exclude files from indexing. The filepath matched is relative to the root directory. Prepending &apos;!&apos; makes the pattern including. Filenames are matched in the order given by the filter list below.</source>
        <translation>Ignoriermuster sind Perl-kompatible reguläre Ausdrücke, die dazu verwendet werden können, Dateien von der Indexierung auszuschließen. Der abgeglichene Pfad ist relativ zum Stammverzeichnis. Durch das Voranstellen eines &apos;!&apos; wird das Muster einschließend. Dateinamen werden in der Reihenfolge der unten stehenden Filterliste abgeglichen.</translation>
    </message>
</context>
<context>
    <name>Plugin</name>
    <message>
        <source>Update index</source>
        <translation>Index aktualisieren</translation>
    </message>
    <message>
        <source>Update the file index</source>
        <translation>Den Datei-Index aktualisieren</translation>
    </message>
    <message>
        <source>Scan</source>
        <translation>Scannen</translation>
    </message>
    <message>
        <source>Trash</source>
        <translation>Papierkorb</translation>
    </message>
    <message>
        <source>Your trash folder</source>
        <translation>Ihr Papierkorb-Verzeichnis</translation>
    </message>
    <message>
        <source>Open trash</source>
        <translation>Papierkorb öffnen</translation>
    </message>
    <message>
        <source>Empty trash</source>
        <translation>Papierkorb leeren</translation>
    </message>
</context>
<context>
    <name>RootBrowser</name>
    <message>
        <source>Root browser</source>
        <translation>Root-Browser</translation>
    </message>
    <message>
        <source>Browse root directory by path</source>
        <translation>Durchsuche das Stammverzeichnis anhand eines Pfades</translation>
    </message>
</context>
</TS>
