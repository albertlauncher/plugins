<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
<context>
    <name>ConfigWidget</name>
    <message>
        <location filename="../src/configwidget.ui" line="29"/>
        <source>The files plugin provides three extensions to access files. The main extension indexes the file system as specified in the settings and provides a lookup by filename. The other two file browsing extensions are triggered by `/` and `~` and provide a way to browse through the file system using paths. This is handy to access files that are _not_ indexed. In combination with the tab completion this is a nice way to browse the file system.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="92"/>
        <source>Path settings</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="112"/>
        <source>Indexing</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="118"/>
        <source>Index hidden files</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="132"/>
        <source>Follow links</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="139"/>
        <source>This option should by used with care and only if necessary. It may cause indexing of way more files you wanted when the indexed file tree contains links to directories outside the specified file tree.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="149"/>
        <source>Max depth</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="169"/>
        <source>Scan interval</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="182"/>
        <source> min</source>
        <extracomment>Abbr. minutes</extracomment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="198"/>
        <source>Watch filesystem</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="218"/>
        <source>Ignore patterns</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="228"/>
        <source>MIME types</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="269"/>
        <source>Directories</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="276"/>
        <source>Documents</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="283"/>
        <source>Audio</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="290"/>
        <source>Video</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="297"/>
        <source>Images</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="310"/>
        <source>Advanced</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.ui" line="327"/>
        <source>Case sensitive file browsers</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.cpp" line="35"/>
        <source>Choose directory</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/configwidget.cpp" line="133"/>
        <source>Enabling file system watches comes with caveats. You should only activate this option if you know what you are doing. A lot of file system changes (compilation, installing, etc) while having watches enabled can put your system under high load.</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>FileItem</name>
    <message>
        <location filename="../src/fileitems.cpp" line="47"/>
        <source>Open with default application</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="57"/>
        <source>Execute</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="66"/>
        <source>Reveal in file browser</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="74"/>
        <source>Open terminal here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="84"/>
        <source>Copy file to clipboard</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/fileitems.cpp" line="113"/>
        <source>Copy path to clipboard</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>FilePathBrowser</name>
    <message>
        <location filename="../src/filebrowsers.cpp" line="63"/>
        <source>Root browser</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/filebrowsers.cpp" line="69"/>
        <source>Browse root directory by path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/filebrowsers.cpp" line="88"/>
        <source>Home browser</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/filebrowsers.cpp" line="94"/>
        <source>Browse home directory by path</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>FsIndexPath</name>
    <message numerus="yes">
        <location filename="../src/fsindexpath.cpp" line="57"/>
        <source>Indexed %n directories in %1.</source>
        <translation>
            <numerusform>Indexed %n directory in %1.</numerusform>
            <numerusform>Indexed %n directories in %1.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>MimeFilterDialog</name>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="14"/>
        <source>MIME filters</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="32"/>
        <source>MIME filters are used while indexing to filter files by their MIME type. Files whose MIME type is not matched by any of the MIME filter patterns below are not indexed. The filter patterns support wildcard characters (*). Utilize the MIME types list on the left to find supported MIME types.</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="47"/>
        <source>Available MIME types</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="78"/>
        <source>Filter the available MIME types</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/mimefilterdialog.ui" line="88"/>
        <source>MIME filter patterns</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>NameFilterDialog</name>
    <message>
        <location filename="../src/namefilterdialog.ui" line="14"/>
        <location filename="../src/namefilterdialog.ui" line="47"/>
        <source>Ignore patterns</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/namefilterdialog.ui" line="32"/>
        <source>Ignore patterns are Perl-compatible regular expressions that can be utilized to exclude files from indexing. The filepath matched is relative to the root directory. Prepending &apos;!&apos; makes the pattern including. Filenames are matched in the order given by the filter list below.</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Plugin</name>
    <message>
        <location filename="../src/plugin.cpp" line="73"/>
        <source>Update index</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="74"/>
        <source>Update the file index</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="76"/>
        <source>Scan</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="134"/>
        <source>Trash</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="135"/>
        <source>Your trash folder</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="140"/>
        <location filename="../src/plugin.cpp" line="149"/>
        <source>Open trash</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugin.cpp" line="144"/>
        <source>Empty trash</source>
        <translation></translation>
    </message>
</context>
</TS>
