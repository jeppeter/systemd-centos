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
    buildfile = './build.ninja'
    replval = '/usr/lib/systemd'
    if len(args.subnargs) > 0:
        buildfile = args.subnargs[0]
    if len(args.subnargs) > 1:
        replval = args.subnargs[1]
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


def main():
    commandline='''
    {
        "input|i" : null,
        "output|o" : null,
        "rpathrepl<rpathrepl_handler>##[buildfile] [rpath] to default buildfile ./build.ninja replace default /usr/lib/systemd##" : {
            "$" : "*"
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