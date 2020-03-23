import ConfigParser
import os.path as PATH;
import sys as sys;

def  reg_dispserver(iniFile,workDir,ip):
    assert PATH.isfile(iniFile)

    config  = ConfigParser.SafeConfigParser()
    config.read(iniFile)


    if config.has_section("rpcinterface:supervisor"):  
        config.set("rpcinterface:supervisor", "supervisor.rpcinterface_factory", "supervisor.rpcinterface:make_main_rpcinterface")
    else:
        config.add_section("rpcinterface:supervisor")
        config.set("rpcinterface:supervisor", "supervisor.rpcinterface_factory", "supervisor.rpcinterface:make_main_rpcinterface")

    if config.has_section("unix_http_server"):  
        config.set("unix_http_server", "file", "/tmp/supervisor.sock")
        config.set("unix_http_server", "chmod", "0777")        
    else:
        config.add_section("unix_http_server")
        config.set("unix_http_server", "file", "/tmp/supervisor.sock")
        config.set("unix_http_server", "chmod", "0777") 

    if config.has_section("supervisorctl"):  
        config.set("supervisorctl", "serverurl", "unix:///tmp/supervisor.sock")
    else:
        config.add_section("supervisorctl")
        config.set("supervisorctl", "serverurl", "unix:///tmp/supervisor.sock")
        
    if config.has_section("inet_http_server"):
        inet_http_server_port="%s:9001" % ip
        config.set("inet_http_server", "port", inet_http_server_port)
        config.set("inet_http_server", "username", "admin")
        config.set("inet_http_server", "password", "123456")
    else:
        config.add_section("inet_http_server")
        inet_http_server_port="%s:9001" % ip
        config.set("inet_http_server", "port", inet_http_server_port)
        config.set("inet_http_server", "username", "admin")
        config.set("inet_http_server", "password", "123456")

    if config.has_section("supervisord"):
        config.set("supervisord", "logfile", "/usr/local/logs/supervisord.log")
        config.set("supervisord", "logfile_maxbytes", "50MB")
        config.set("supervisord", "logfile_backups", "10")
        config.set("supervisord", "pidfile", "/tmp/supervisord.pid")
        config.set("supervisord", "nodaemon", "false")
        config.set("supervisord", "minfds", "1024")
        config.set("supervisord", "minprocs", "200")
    else:
        config.add_section("supervisord")
        config.set("supervisord", "logfile", "/usr/local/logs/supervisord.log")
        config.set("supervisord", "logfile_maxbytes", "50MB")
        config.set("supervisord", "logfile_backups", "10")
        config.set("supervisord", "pidfile", "/tmp/supervisord.pid")
        config.set("supervisord", "nodaemon", "false")
        config.set("supervisord", "minfds", "1024")
        config.set("supervisord", "minprocs", "200")


    command = '''/usr/local/DispatchServer/dispserver_d'''
    print command
    if config.has_section("program:dispserver"):
        config.set("program:dispserver", "directory", workDir)
        config.set("program:dispserver", "command", command)
        config.set("program:dispserver", "autostart", "true")
        config.set("program:dispserver", "autorstart", "true")
        config.set("program:dispserver", "stdout_logfile", "/usr/local/logs/dispserver_std.log")
        config.set("program:dispserver", "redirect_stderr", "true")
    else:
        config.add_section("program:dispserver")
        config.set("program:dispserver", "directory", workDir)
        config.set("program:dispserver", "command", command)
        config.set("program:dispserver", "autostart", "true")
        config.set("program:dispserver", "autorstart", "true")
        config.set("program:dispserver", "stdout_logfile", "/usr/local/logs/dispserver_std.log")
        config.set("program:dispserver", "redirect_stderr", "true")

    f = open(iniFile, 'wb')
    config.write(f)

def  unreg_dispserver(iniFile,workDir):
    assert PATH.isfile(iniFile)

    config  = ConfigParser.SafeConfigParser()
    config.read(iniFile)


    if config.has_section("program:dispserver"):
        config.remove_section("program:dispserver")

    f = open(iniFile, 'wb')
    config.write(f)


if __name__ == '__main__':
    if len(sys.argv) < 5:
        print "usage: mode_superbisord.py confpath workdir mode ip "
        sys.exit(1)

    iniPath = sys.argv[1]
    workDir = sys.argv[2]
    mode = sys.argv[3]
    ip = sys.argv[4]

    print iniPath
    print workDir
    print mode
    print ip

    if PATH.isfile(iniPath):
        if mode == 'reg':
            reg_dispserver(iniPath,workDir,ip)
        else:
            unreg_dispserver(iniPath, workDir)
    else:
        print "conf file not exists now.please create it first:" + iniPath
        sys.exit(1)

    sys.exit(0)
