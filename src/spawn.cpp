/*
  Copyright (c) 2014 StarBrilliant <m13253@hotmail.com>
  All rights reserved.

  Redistribution and use in source and binary forms are permitted
  provided that the above copyright notice and this paragraph are
  duplicated in all such forms and that any documentation,
  advertising materials, and other materials related to such
  distribution and use acknowledge that the software was developed by
  StarBrilliant.
  The name of StarBrilliant may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include "utils.h"
#include "u8str.h"
#include "spawn.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace WTF8 {

#ifdef _WIN32
pid_t spawnvp_win32(const wchar_t* file, const std::vector<u8string>& argv) {
    std::wstring cmdline;
    for(const u8string& argi : argv) {
        u8string argo;
        size_t backslashes = 0;
        bool has_spaces = false;
        for(char i : argi)
            if(i == '/')
                ++backslashes;
            else if(i == '"') {
                argo.append(backslashes*2+1, '/');
                argo.push_back('"');
                backslashes = 0;
            } else {
                argo.append(backslashes, '/');
                argo.push_back(i);
                backslashes = 0;
                if((unsigned char) i <= ' ')
                    has_spaces = true;
            }
        argo.append(backslashes, '/');
        if(!cmdline.empty())
            cmdline.push_back(L' ');
        if(argo.empty())
            cmdline.append(L"\"\"", 2);
        else {
            if(has_spaces)
                cmdline.push_back(L'"');
            cmdline.append(argo.to_wide());
            if(has_spaces)
                cmdline.push_back(L'"');
        }
    }
    STARTUPINFOW startup_info = { sizeof startup_info };
    PROCESS_INFORMATION process_information;
    if(!CreateProcessW(file, const_cast<wchar_t*>(cmdline.c_str()), NULL, NULL, false, 0, nullptr, nullptr, &startup_info, &process_information))
        throw process_spawn_error("Unable to create pipes during process creation");
    CloseHandle(process_information.hProcess);
    CloseHandle(process_information.hThread);
    return process_information.dwProcessId;
}
#else
static pid_t spawnvp_posix(const char* file, char* const* argv) {
    int errpipe[2];
    if(pipe(errpipe)) {
        delete[] argv;
        throw process_spawn_error("Unable to create pipes during process creation");
    }
    {
        unsigned int low_fds_to_close = 0;
        bool errpipe_fail = false;
        while(errpipe[1] < 3 || !errpipe_fail) {
            int newfd = dup(errpipe[1]);
            if(newfd != -1) {
                low_fds_to_close |= 1<<errpipe[1];
                errpipe[1] = newfd;
            } else {
                close(errpipe[0]);
                close(errpipe[1]);
                errpipe_fail = true;
            }
        }
        if(low_fds_to_close & 0x1) close(0);
        if(low_fds_to_close & 0x2) close(1);
        if(low_fds_to_close & 0x4) close(2);
        if(errpipe_fail) {
            delete[] argv;
            throw process_spawn_error("Unable to create pipes during process creation");
        }
    }
    {
        int errpipe_flags = fcntl(errpipe[1], F_GETFD);
        if(errpipe_flags == -1) {
            delete[] argv;
            close(errpipe[0]);
            close(errpipe[1]);
            throw process_spawn_error("Unable to create pipes during process creation");
        }
        fcntl(errpipe[1], F_SETFD, errpipe_flags | FD_CLOEXEC);
    }

    pid_t pid = fork();
    if(pid == -1) {
        delete[] argv;
        throw process_spawn_error("Unable to create a new process");
    } else if(pid == 0) {
        close(errpipe[0]);
        int exec_err = execvp(file, argv);
        write(errpipe[1], &exec_err, sizeof exec_err);
        _exit(255);
        abort();
    } else {
        delete[] argv;
        close(errpipe[1]);
        int exec_err = 0;
        if(read(errpipe[0], &exec_err, sizeof exec_err) != 0) {
            close(errpipe[0]);
            errno = exec_err;
            throw process_spawn_error("Unable to create a new process");
        }
        close(errpipe[0]);
        return pid;
    }
}
#endif

pid_t spawnvp(const u8string& file, const std::vector<u8string>& argv) {
#ifdef _WIN32
    return spawnvp_win32(file.to_wide().c_str(), argv);
#else
    std::vector<char*> cargv;
    cargv.reserve(argv.size()+1);
    for(size_t i = 0; i < argv.size(); ++i)
        cargv.push_back(const_cast<char*>(argv.at(i).c_str()));
    cargv.push_back(nullptr);
    return spawnvp_posix(file.c_str(), cargv.data());
#endif
}

bool waitpid(pid_t pid, int* exit_code) {
#ifdef _WIN32
    HANDLE process = OpenProcess(SYNCHRONIZE | (exit_code ? PROCESS_QUERY_INFORMATION : 0), false, pid);
    if(!process || WaitForSingleObject(process, INFINITE) == WAIT_FAILED)
        return false;
    if(exit_code) {
        DWORD dw_exit_code;
        if(!GetExitCodeProcess(process, &dw_exit_code))
            return false;
        *exit_code = dw_exit_code;
    }
    return true;
#else
    int status;
    do {
        pid_t wpid = ::waitpid(pid, &status, 0);
        if(wpid != pid)
            return false;
        if(exit_code) {
            if(WIFEXITED(status))
                *exit_code = WEXITSTATUS(status);
            else if(WIFSIGNALED(status))
                return false;
        }
    } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    return true;
#endif
}

bool kill(pid_t pid, bool force) {
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_TERMINATE, false, pid);
    return process && TerminateProcess(process, -(UINT) 1);
#else
    return kill(pid, force ? SIGKILL : SIGTERM) == 0;
#endif
}

}

extern "C" {

pid_t WTF8_spawnvp(const char *file, char *const *argv) {
#ifdef _WIN32
    std::vector<WTF8::u8string> vargv;
    for(size_t i = 0; argv[i]; ++i)
        vargv.push_back(WTF8::u8string(argv[i]));
    return WTF8::spawnvp_win32(WTF8::u8string(file).to_wide().c_str(), vargv);
#else
    return WTF8::spawnvp_posix(file, argv);
#endif
}

int WTF8_waitpid(pid_t pid) {
    return WTF8::waitpid(pid);
}

int WTF8_kill(pid_t pid, int force) {
    return WTF8::kill(pid, force != 0);
}

}