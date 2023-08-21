#! /usr/bin/env python

import logging
import extargsparse
import os
import sys
import re


def set_logging(args):
    loglvl= logging.ERROR
    if args.verbose >= 3:
        loglvl = logging.DEBUG
    elif args.verbose >= 2:
        loglvl = logging.INFO
    curlog = logging.getLogger(args.lognames)
    #sys.stderr.write('curlog [%s][%s]\n'%(args.logname,curlog))
    curlog.setLevel(loglvl)
    if len(curlog.handlers) > 0 :
        curlog.handlers = []
    formatter = logging.Formatter('%(asctime)s:%(filename)s:%(funcName)s:%(lineno)d<%(levelname)s>\t%(message)s')
    if not args.lognostderr:
        logstderr = logging.StreamHandler()
        logstderr.setLevel(loglvl)
        logstderr.setFormatter(formatter)
        curlog.addHandler(logstderr)

    for f in args.logfiles:
        flog = logging.FileHandler(f,mode='w',delay=False)
        flog.setLevel(loglvl)
        flog.setFormatter(formatter)
        curlog.addHandler(flog)
    for f in args.logappends:       
        if args.logrotate:
            flog = logging.handlers.RotatingFileHandler(f,mode='a',maxBytes=args.logmaxbytes,backupCount=args.logbackupcnt,delay=0)
        else:
            sys.stdout.write('appends [%s] file\n'%(f))
            flog = logging.FileHandler(f,mode='a',delay=0)
        flog.setLevel(loglvl)
        flog.setFormatter(formatter)
        curlog.addHandler(flog)
    return

def load_log_commandline(parser):
    logcommand = '''
    {
        "verbose|v" : "+",
        "logname" : "root",
        "logfiles" : [],
        "logappends" : [],
        "logrotate" : true,
        "logmaxbytes" : 10000000,
        "logbackupcnt" : 2,
        "lognostderr" : false
    }
    '''
    parser.load_command_line_string(logcommand)
    return parser


def read_file(infile=None):
    fin = sys.stdin
    if infile is not None:
        fin = open(infile,'r+b')
    rets = ''
    for l in fin:
        s = l
        if 'b' in fin.mode:
            if sys.version[0] == '3':
                s = l.decode('utf-8')
        rets += s

    if fin != sys.stdin:
        fin.close()
    fin = None
    return rets

def read_file_split(infile=None):
    s = read_file(infile)
    sarr = re.split('\n',s)
    retarr = []
    for l in sarr:
        l = l.rstrip('\r')
        retarr.append(l)
    return retarr

def write_file(s,outfile=None):
    fout = sys.stdout
    if outfile is not None:
        fout = open(outfile, 'w+b')
    outs = s
    if 'b' in fout.mode:
        outs = s.encode('utf-8')
    fout.write(outs)
    if fout != sys.stdout:
        fout.close()
    fout = None
    return 

def rpathrepl_handler(args,parser):
    set_logging(args)
    buildfile = args.input
    replval = '/usr/lib/systemd'
    if len(args.subnargs) > 0:
        replval = args.subnargs[0]
    sarr = read_file_split(buildfile)
    fexpr = re.compile('\\-pie\\s+\'\\-Wl,\\-rpath,[^\']+\'')
    repv = '-pie \'-Wl,-rpath,%s\''%(replval)
    outs = ''
    lineno = 0
    for l in sarr:
        retl = l
        lineno += 1
        if len(l) > 0:
            cont = True
            while cont:
                cont = False
                m = fexpr.findall(l)
                if m is not None and len(m) > 0:
                    cl = l.replace(m[0],repv)
                    logging.info('[%d]m %s o[%s] n [%s]'%(lineno,m,l,cl))
                    if cl != l:
                        cont = True
                        l = cl
            retl = l
        outs += '%s\n'%(retl)
    write_file(outs,args.output)
    sys.exit(0)
    return


class SimpleFile(object):
    def __init__(self,fname):
        self.fname = fname
        self.bname = os.path.basename(fname)
        return


def TakeSimpleBase(elem):
    logging.info('%s'%(elem.bname))
    return elem.bname

def compact_simple_file(s):
    sarr = re.split('\n',s)
    rsvec = []
    for l in sarr:
        l = l.rstrip('\r')
        if len(l) == 0:
            continue
        rsvec.append(SimpleFile(l))
    return rsvec


def rpmfiles_handler(args,parser):
    set_logging(args)
    rs = read_file(args.subnargs[0])
    cs = read_file(args.subnargs[1])
    rsvec = compact_simple_file(rs)
    csvec = compact_simple_file(cs)
    rsvec.sort(key=TakeSimpleBase)
    csvec.sort(key=TakeSimpleBase)
    idx = 0
    jdx = 0
    hashvec = []
    while idx < len(rsvec) and jdx < len(csvec):
        if rsvec[idx].bname < csvec[jdx].bname:
            logging.info('idx[%d]%s < jdx[%d] %s'%(idx,rsvec[idx].bname,jdx,csvec[jdx].bname))
            idx += 1
        elif rsvec[idx].bname > csvec[jdx].bname:
            logging.info('idx[%d]%s > jdx[%d] %s'%(idx,rsvec[idx].bname,jdx,csvec[jdx].bname))
            jdx += 1
        else:
            logging.info('idx[%d]jdx[%d] %s [%s]'%(idx,jdx,csvec[jdx].bname,csvec[jdx].fname))
            hashvec.append(csvec[jdx])
            jdx += 1
            while jdx < len(csvec):
                if csvec[jdx].bname != rsvec[idx].bname:
                    break
                logging.info('idx[%d]jdx[%d] %s [%s]'%(idx,jdx,csvec[jdx].bname,csvec[jdx].fname))
                hashvec.append(csvec[jdx])
                jdx += 1
            idx += 1
    outs = ''
    for l in hashvec:
        outs += '%s\n'%(l.fname)
    write_file(outs,args.output)
    sys.exit(0)
    return

def format_tab_line(tab,s):
    rets = ''
    for i in range(tab):
        rets += '    '
    rets += s
    rets += '\n'
    return rets

def format_one_cp_line(srcfile,dstfile,tabs=0,retline=0):
    logging.info('retline [%d]'%(retline))
    rets = format_tab_line(tabs,'if [ -f "%s" ]'%(dstfile))
    retline += 1
    rets += format_tab_line(tabs,'then')
    retline += 1
    rets += format_tab_line(tabs+1,'if [ -f "%s" ]'%(srcfile))
    retline += 1
    rets += format_tab_line(tabs+1,'then')
    retline += 1
    rets += format_tab_line(tabs+2,'cp -f "%s" "%s"  || (echo "[line:%d]cp [%s] => [%s] error" >&2)'%(srcfile,dstfile,retline,srcfile,dstfile))
    retline += 1
    rets += format_tab_line(tabs+1,'else')
    retline += 1
    rets += format_tab_line(tabs+2,'echo "[line:%d]no [%s] srcfile" >&2'%(retline,srcfile))
    retline += 1
    rets += format_tab_line(tabs+1,'fi')
    retline += 1
    rets += format_tab_line(tabs,'else')
    retline += 1
    rets += format_tab_line(tabs+1,'echo "[line:%d]no [%s] dstfile" >&2'%(retline,dstfile))
    retline += 1
    rets += format_tab_line(tabs,'fi')
    retline += 1
    return rets,retline

def append_dir_files(s,ds):
    sarr = re.split('\n',s)
    rsvec = []
    for l in sarr:
        l = l.rstrip('\r')
        if len(l) == 0:
            continue
        curf = '%s/%s'%(ds,l)
        #logging.info('cf %s ds[%s]'%(curf,ds))
        curf = os.path.abspath(curf)
        #logging.info('abs cf %s'%(curf))
        rsvec.append(SimpleFile(curf))
    return rsvec


def formatcp_handler(args,parser):
    set_logging(args)
    srcdir = os.path.abspath('.')
    if args.srcdir is not None:
        srcdir = os.path.abspath(args.srcdir)
    dstdir = '/'
    if args.dstdir is not None:
        dstdir = os.path.abspath(args.dstdir)
    srcs = read_file(args.subnargs[0])
    dsts = read_file(args.subnargs[1])
    outs = format_tab_line(0,'#! /bin/sh')
    # we set 2 because count from 1 not 0
    retline = 2
    srcfiles = append_dir_files(srcs,srcdir)
    dstfiles = append_dir_files(dsts,dstdir)
    idx = 0
    jdx = 0

    while jdx < len(dstfiles) and idx < len(srcfiles):
        if dstfiles[jdx].bname == srcfiles[idx].bname:
            outs += '\n'
            retline += 1
            s, retline= format_one_cp_line(srcfiles[idx].fname,dstfiles[jdx].fname,0,retline)
            outs += s
            jdx += 1
            while jdx < len(dstfiles):
                if dstfiles[jdx].bname != srcfiles[idx].bname:
                    break
                outs += '\n'
                retline += 1
                s , retline= format_one_cp_line(srcfiles[idx].fname,dstfiles[jdx].fname,0,retline)
                outs += s                
                jdx += 1
            idx += 1
        elif dstfiles[jdx].bname < srcfiles[idx].bname:
            logging.error('skip jdx[%d] %s (%s)'%(jdx,dstfiles[jdx].bname,dstfiles[jdx].fname))
            jdx += 1
        else:
            logging.error('skip idx[%d] %s (%s)'%(idx,srcfiles[idx].bname,srcfiles[idx].fname))
            idx += 1

    write_file(outs,args.output)
    sys.exit(0)
    return

def add_exec_start(f):
    try:
        sarr = read_file_split(f)
    except:
        logging.error('%s'%(traceback.format_exc()))
        return False
    matchstart = -1
    matchend = -1
    startexpr = re.compile('^## EXECSTART',re.I)
    endexpr = re.compile('^## EXECEND',re.I)
    lineno = 0
    for l in sarr:
        lineno += 1
        if matchstart > 0 and  matchend < 0:
            if endexpr.match(l):
                matchend = lineno
        elif  matchstart < 0:
            if startexpr.match(l):
                matchstart = lineno

    if matchstart > 0 and matchend < 0:
        logging.error('%s not match '%(f))
        return False
    elif matchstart > 0 and matchend > 0:
        nsarr = []
        idx = 0
        while idx < len(sarr):
            if idx < matchstart or idx > matchend:
                nsarr.append(sarr[idx])
            idx += 1
        sarr = nsarr
    outs = ''
    for l in sarr:
        outs += '%s\n'%(l)

    outs += '## EXECSTART\n'
    outs += '[Service]\n'
    outs += 'ExecStart=/usr/bin/true\n'
    outs += '## EXECEND\n'
    try:
        write_file(outs,f)
    except:
        logging.error('%s'%(traceback.format_exc()))
        return False
    return True


def addexecstart_handler(args,parser):
    set_logging(args)
    retval = True
    for f in args.subnargs:
        retv = add_exec_start(f)
        if not retv:
            logging.error('%s failed'%(f))
            retval = False
    sys.exit(0)
    return


def main():
    commandline='''
    {
        "input|i" : null,
        "output|o" : null,
        "srcdir|s" : null,
        "dstdir|d" : null,
        "rpathrepl<rpathrepl_handler>##[rpath] to default buildfile from input parameter replace default /usr/lib/systemd##" : {
            "$" : "*"
        },
        "rpmfiles<rpmfiles_handler>##rpmfiles listfiles to match rpm files##" : {
            "$" : 2
        },
        "formatcp<formatcp_handler>##srcfilelist dstfilelist to copy##" : {
            "$" : 2
        },
        "addexecstart<addexecstart_handler>##files ... to add files##" : {
            "$" : "+"
        }
    }
    '''
    parser = extargsparse.ExtArgsParse()
    parser.load_command_line_string(commandline)
    load_log_commandline(parser)
    parser.parse_command_line(None,parser)
    raise Exception('can not reach here')
    return

if __name__ == '__main__':
    main()