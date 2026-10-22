# Checksums command
This command computes **MD5**, **SHA256** and **SHA512** checksums and stores
them in **trusted extended attributes**.

## Compilation
To compile **checksums** from the source files, type :
``` bash
$ make
```

Only **root** is allowed to access **trusted xattr** but the capability
**CAP_SYS_ADMIN** may be added to this program with the command :
``` bash
# setcap cap_sys_admin=ep checksums
```

This command uses GNUlib to compute **MD5**, **SHA256** and **SHA512** checksums.
These checksums are computed simultaneously using threads and CPU affinity.
**checksums** can :
- compute the checksums for a file, or for several files
- compute the checksums for all files of a directory or several directories
- list files that have no checksums in extended attributes
- list files that have all checksums in extended attributes
- list files that have obsolete checksums in extended attributes
- display output in a format compatible with md5sum, sha256sum or sha512sum
- ...


``` bash
$ checksums -h
checksums : version 1.53
Usage : checksums [-hvDNfM25rsdOCmnulL][-w width] pathname [pathname ...]
  -h : help
  -v : verbose mode (for lists, and to display the origin of checksums)
  -D : display debug information (-D, -DD, -DDD)
  -N : display pathname only
  -f : force checksums computation
  -M : display MD5 checksum
  -2 : display SHA256 checksum
  -5 : display SHA512 checksum
  -r : recursively traverses the directory tree
  -s : silent
  -d : display directory checksum (specific to this program)
  -O : list files that have all checksums
  -C : list files that needs checksum computation
  -n : list files that have no checksums in xattr
  -m : list files that have missing checksums in xattr
  -u : list files that have checksums in xattr but need updates
  -l : list xattr checksums status
  -L : legacy display format
  -w : pathname width

```
