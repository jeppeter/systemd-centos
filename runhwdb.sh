#! /bin/bash

source extargsparse4sh

srcfile=`readlink -f $0`
srcdir=`dirname $srcfile`

verbose=0

INFO_LEVEL=2
DEBUG_LEVEL=3
WARN_LEVEL=1
ERROR_LEVEL=0
ECHO=/usr/bin/echo

function __Debug()
{
        local _fmt=$1
        local _osname=`uname -s | tr [:upper:] [:lower:]`
        shift
        local _backstack=0
        if [ $# -gt 0 ]
                then
                _backstack=$1
        fi
        
        _fmtstr=""
        if [ $verbose -gt $INFO_LEVEL ]
                then
                local _filestack=`expr $_backstack \+ 1`
                _fmtstr="${BASH_SOURCE[$_filestack]}:${BASH_LINENO[$_backstack]} "
        fi

        _fmtstr="$_fmtstr$_fmt"
        if [ "$_osname" = "darwin" ]
                then
                ${ECHO} "$_fmtstr" >&2
        else
                ${ECHO} -e "$_fmtstr" >&2
        fi
}

function Debug()
{
        local _fmt=$1
        shift
        local _backstack=0
        if [ $# -gt 0 ]
                then
                _backstack=$1
        fi
        _backstack=`expr $_backstack \+ 1`
        
        if [ $verbose -ge $DEBUG_LEVEL ]
                then
                __Debug "$_fmt" "$_backstack"
        fi
        return
}

function Info()
{
        local _fmt=$1
        shift
        local _backstack=0
        if [ $# -gt 0 ]
                then
                _backstack=$1
        fi
        _backstack=`expr $_backstack \+ 1`
        
        if [ $verbose -ge $INFO_LEVEL ]
                then
                __Debug "$_fmt" "$_backstack"
        fi
        return
}

function Warn()
{
        local _fmt=$1
        shift
        local _backstack=0
        if [ $# -gt 0 ]
                then
                _backstack=$1
        fi
        _backstack=`expr $_backstack \+ 1`
        
        if [ $verbose -ge $WARN_LEVEL ]
                then
                __Debug "$_fmt" "$_backstack"
        fi
        return
}



read -r -d '' OPTIONS <<EOFM
{
	"verbose|v" : "+",
	"usr|U" : "/mnt/zdisk/hwdb.bin",
	"root|R" : "/mnt/zdisk/cc/",
	"binexe|B" : "$srcdir/build/udevadm",
	"sopaths|S" : "$srcdir/build/src/shared:$srcdir/build:$srcdir/build/src/udev"
}
EOFM

parse_command_line "$OPTIONS" "$@"

Debug "sopaths [$sopaths]"
Debug "binexe [$binexe]"
Debug "usr [$usr]"
Debug "root [$root]"
export LD_LIBRARY_PATH=$sopaths
Debug "LD_LIBRARY_PATH [$LD_LIBRARY_PATH]"


if [ ! -d "$root/usr/lib/udev/hwdb.d" ]
then
	mkdir -p "$root/usr/lib/udev/hwdb.d"
fi

if [ $verbose -ge 3 ]
then
export SYSTEMD_LOG_LEVEL=debug
export SYSTEMD_LOG_TARGET=console
else
export SYSTEMD_LOG_LEVEL=error
export SYSTEMD_LOG_TARGET=console
fi

cp -f /usr/lib/udev/hwdb.d/* "$root/usr/lib/udev/hwdb.d/"
Debug "run [$binexe hwdb --update --root \"$root\" --usr \"$usr\"]"
$binexe hwdb --update --root "$root" --usr "$usr"

