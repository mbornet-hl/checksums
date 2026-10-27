# Checksums command
This command computes **MD5**, **SHA256** and **SHA512** checksums and stores
them in **trusted extended attributes**.

## Compilation
To compile **checksums** from the source files, type :
``` bash
$ make
```
> ** :warning: SECURITY WARNING**
>
> Accessing **trusted xattrs** requires **root** privileges. Do **not** set
> the SUID bit or grant **CAP_SYS_ADMIN** to the `checksums` binary. The current
> implementation does not enforce file access permission checks for the invoking user.


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

The **hl** colorization configuration for **checksums** is available in the **hl** project
(https://github.com/mbornet-hl/hl).

It may look like this one :
```bash
$ hl -vop checksums
# /DATA3/projets/hl/config_files/hl_sums.cfg
checksums                                                    :
      -e
      -3c   '(trusted\.((md5|sha256|sha512)_sum|date))'
      -3g   'timestamps are consistent'
      -3g   'checksums are retrieved from xattr'
      -3y   'checksums are computed'
      -3g   'all checksums are in xattr'
      -3g   'tag \[.*\] found in xattr'
      -3y   'timestamps are NOT CONSISTENT.*'
      -3y   '....-..-.. ..:..:.. \(xattr\) != ....-..-.. ..:..:.. \(ctime\)'
      -3y   'xattr need update'
      -3r   'no checksums in xattr'
      -3r   'tag \[.*\] NOT FOUND in xattr'
      -3w   '.*\[DEBUG\].*'
      -3g   '.*: all xattr are OK'
      -3m   '.*: missing checksums in xattr'
      -3y   '.*: needs timestamp update'
      -3r   '.*: no checksums in xattr'
      -3y   '\<[0-9a-f]{32}\>'
      -3g   '\<[0-9a-f]{64}\>'
      -3c   '\<[0-9a-f]{128}\>'

```
