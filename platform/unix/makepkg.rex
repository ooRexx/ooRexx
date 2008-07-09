/*
 * This program builds a platform-specific package of ooRexx binaries
 * It will also build a "standard" tar.gz of binaries
 *
 * Prerequisites:
 *   make DESTDIR=`pwd`/tmp install
 *    has been run before this
 * Invocation:
 *   Should not be executed directly, but via:
 *   make DESTDIR=`pwd`/tmp package
 *    in the directory where ooRexx is built.
 */
Parse Arg type version platform prefix root
/*root = Value( 'DESTDIR', , 'ENVIRONMENT' )*/
origdir = Directory()
If root = '' Then 'cd' prefix
Else 'cd' root||prefix
Interpret Call 'pre'type
'tar cvf - * | gzip >' origdir'/ooRexx-'version'-'platform'.tar.gz'
If root = '' Then 'cd' prefix
Else 'cd' root
Interpret Call 'post'type
Return 0

prenone:
Return

presun:
/*
 * Get g++ library dependencies and copy into our library directory
 */
here = Directory()
tmp = './ldd.txt'
'ldd' './bin/rexx >' tmp
tab = d2c(9)
Call Stream tmp, 'C', 'OPEN READ'
Do While Lines( tmp ) > 0
   line = Strip( Linein( tmp ), 'B', tab )
   Select
      When Left( line, 10 ) = 'libstdc++.' | Left( line, 9 ) = 'libgcc_s.' Then
         Do
            Parse Var line . '=>' file
            'zip -y g++dep.zip' file'*'
         End
      Otherwise Nop
   End
End
Call Stream tmp, 'C', 'CLOSE'
'rm -f' tmp
Call Directory './lib/ooRexx'
'unzip -jo ../../g++dep.zip'
Call Directory here
'rm -f g++dep.zip'
Return

postnone:
Return

postsun:
If Countstr( 'sparc', platform ) \= 0 Then arch = 'sparc'
Else arch = 'x86'
lenprefix = Length( prefix )
/*
 * Get the Solaris version
 */
tmp = './un.txt'
'uname -r >' tmp
Call Stream tmp, 'C', 'OPEN READ'
prodver = Linein( tmp )
Call Stream tmp, 'C', 'CLOSE'
'rm -f' tmp
/*
 * Write the package configuration file
 */
conf = '/tmp/oorexx-build.conf'
Call WriteConf conf, arch, version, prefix, prodver, 'ooRexx', 'Open Object Rexx', 'Open Object Rexx Interpreter'
/*
 * Write the package prototype file
 */
proto = '/tmp/oorexx-build.proto'
Call Stream proto, 'C', 'OPEN WRITE REPLACE'
Call Lineout proto,'i pkginfo='conf
tmp = '/tmp/oorexx-build.tmp'
/* write directory lines */
'find . -type d -name "*" -print >' tmp
dirs = ''
Call Stream tmp, 'C', 'OPEN READ'
Do i = 1 While Lines( tmp ) > 0
   fn = Linein( tmp )
   dir = Substr( fn, 2 )
   dir = Substr( dir, lenprefix+2 )
   If Wordpos( dir, dirs ) = 0 & Strip( dir ) \= ''Then
      Do
         Call Lineout proto,'d none' dir '0755 bin bin'
         Say 'd none' dir '0755 bin bin'
         dirs = dirs dir
      End
End
Call Stream tmp, 'C', 'CLOSE'

here = Directory()
/* write file lines */
'find . -type f -name "*" -exec ls -l {} \; >' tmp
Call Stream tmp, 'C', 'OPEN READ'
Do i = 1 While Lines( tmp ) > 0
   line = Linein( tmp )
   Parse Var line 2 p1 5 p2 8 p3 11 . . . . . . . fn
   perms = '0' || GetPerms( p1 ) || GetPerms( p2 ) || GetPerms( p3 )
   file = Substr( fn, lenprefix+3 )
   Call Lineout proto,'f none' file'='here||prefix'/'file perms 'bin bin'
   Say 'f none' file'='here||prefix'/'file perms 'bin bin'
End
Call Stream tmp, 'C', 'CLOSE'

/* write link lines */
'find . -type l -name "*" -exec ls -l {} \; >' tmp
Call Stream tmp, 'C', 'OPEN READ'
Do i = 1 While Lines( tmp ) > 0
   line = Linein( tmp )
   Parse Var line . . . . . . . . file '->' fn
   file = Strip(Substr( file, lenprefix+3 ))
   fn = Strip( fn )
   pos = Lastpos( '/', file )
   link = Substr( file, 1, pos )
   Call Lineout proto,'s none' file'='fn
   Say 's none' file'='fn
End
Call Stream tmp, 'C', 'CLOSE'

Call Stream proto, 'C', 'CLOSE'
/*
 * Write the package file
 */
packagename = 'ooRexx-'version'-'platform'.pkg'
'rm -rf pkg'
'mkdir pkg'
'pkgmk -o -b `pwd`/'prefix '-d `pwd`/pkg -f' proto
'pkgtrans `pwd`/pkg' origdir'/'packagename 'ooRexx'
'gzip -f' origdir'/'packagename
'rm -rf pkg'
/* clean up working files */
'rm -f' tmp proto conf
Return

WriteProtoLine: Procedure
Parse Arg proto, line
Parse Var line perm .
Select
   When Left( perm, 1 ) = 'l' Then
      Do
         /* we have a symbolic link */
         Parse Var line perms . . . . . . . file '->' fn
      End
End
Return

WriteConf: Procedure
Parse Arg conf, arch, version, prefix, prodver, pkg, name, desc
Call Stream conf, 'C', 'OPEN WRITE REPLACE'
Call Lineout conf,'PKG="ooRexx"'
Call Lineout conf,'NAME="Open Object Rexx"'
Call Lineout conf,'ARCH="'arch'"'
Call Lineout conf,'VERSION="'version'"'
Call Lineout conf,'CATEGORY="application"'
Call Lineout conf,'VENDOR="RexxLA"'
Call Lineout conf,'DESC="Open Object Rexx"'
Call Lineout conf,'BASEDIR="'prefix'"'
Call Lineout conf,'SUNW_PRODNAME="SunOS"'
Call Lineout conf,'SUNW_PRODVERS="'prodver'"'
Call Lineout conf,'SUNW_PKGTYPE="usr"'
Call Lineout conf,'CLASSES=none'
Call Stream conf, 'C', 'CLOSE'
Return

GetPerms: Procedure
Parse Arg p
If Substr( p, 1, 1 ) = 'r' Then o = 4
If Substr( p, 2, 1 ) = 'w' Then o = o + 2
If Substr( p, 3, 1 ) = 'x' Then o = o + 1
Return o
