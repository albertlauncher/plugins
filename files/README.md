The files extension offers ways to access files through Albert.

One way to access files is to use the offline index. The extension indexes files that can be accessed by their filename. In the settings you can set paths the extension recursively scans for files.

Further the files extension provides a way to browse through the file system using paths. This is handy to access files that are _not_ indexed. Queries beginning with either `/` or `~` are interpreted as a path. In combination with the tab completion this is a nice way to browse the file system.

The offline index can be configured in several ways. It is recommended that you configure the extension depending on your needs. Simply indexing all files on your system may be convenient, but on one hand the vast amounts of files you'll never use will clutter the output and on the other hand a large file index may introduce performance penalties, like long indexing and lookup times and high memory usage.

Generally you have to define where the extension should look for files. But you may not want to index all of the files in the directory tree below the paths you specified. You have now two options to exclude files from indexing: File filters and MIME type filters.

## File filters / Ignore files

An ignore file is a simple text file with the name `.albertignore` which specifies which files should be ignored. Each line in an ignore file specifies a pattern which serves as an exclusive filter. Patterns read from an ignore file in the same directory as the path, or in any parent directory, with patterns in the higher level files (up to the toplevel path to be indexed) being overridden by those in lower level files down to the directory containing the file. These patterns match relative to the location of the ignore file.

### Pattern format

A blank line matches no files, so it can serve as a separator for readability.

A line starting with `#` serves as a comment.

The prefix `!`  negates the pattern; any matching file excluded by a previous pattern will become included again. It is not possible to re-include a file if a parent directory of that file is excluded. Put a backslash (`\`) in front of the first `!` for patterns that begin with a literal `!`.

A leading slash `/` marks a relative pattern. This pattern is anchored relative to the directory the ignore file resides in. Without the leading slash a pattern is an absolute pattern and is inherited to sub directories. For example the pattern `/bar` matches the file `bar` but not `foo/bar`, while the pattern `bar` does.

An asterisk `*` in a pattern matches against all characters but the directory separator `/`. Two consecutive asterisks `**` in a pattern match against all characters. More consecutive asterisks collapse to `**`.

#### Example

```
# Ignore eclipse dir
/workspace

# Ignore git projects
/git/*

# Include albert repos though
!/git/albert
!/git/aur-albert

# However in the albert repo dont index source files
/git/albert/**.cpp

# General ignores
node_modules
bower_components
```

## MIME filters

You also have fine-grained control over the MIME types that should be indexed. The <kbd>Advanced</kbd> button in the settings opens a dialog that lets you set a list of patterns that are used to match against the MIME types of the indexed files. The check boxes besides the button are shortcuts that let you add or remove the most popular patterns.

As against the file filters the MIME filters are inclusive, files are only indexed if their MIME type match against one of the patterns. Unlike the file filters which reside in a file the MIME filters have global scope.

The patterns support common [wildcard matching](http://doc.qt.io/qt-5/qregexp.html#wildcard-matching).

#### Example

```
application/*
inode/directory
audio/*
video/*
text/x-python
```
