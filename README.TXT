Diffy is a tool for comparing files with identical names contained in differing directory heirarchies.

Use Diffy by passing 2 directories to it (typically, 2 project directories). It will launch "meld" on each pair of files
with identical names that it finds whose contents don't MD5 to the same hash value.

Diffy uses an "diffy_ignores" file in /etc (so, /etc/diffy_ignores). This file should contain a list of file and
directory names (each on their own line) that diffy should ignore (Example: "build").
