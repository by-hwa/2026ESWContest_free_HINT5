#include "process_runner.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

ProcessResult runProcess(const std::vector<std::string>& argv)
{
    if (argv.empty()) {
        throw std::invalid_argument("Cannot run an empty command");
    }

    int pipe_fd[2] {};
    if (::pipe(pipe_fd) != 0) {
        throw std::runtime_error("Failed to create process pipe");
    }

    const pid_t pid = ::fork();
    if (pid < 0) {
        ::close(pipe_fd[0]);
        ::close(pipe_fd[1]);
        throw std::runtime_error("Failed to start process");
    }
    if (pid == 0) {
        ::close(pipe_fd[0]);
        ::dup2(pipe_fd[1], STDOUT_FILENO);
        ::close(pipe_fd[1]);
        std::vector<char*> args;
        args.reserve(argv.size() + 1);
        for (const auto& argument : argv) {
            args.push_back(const_cast<char*>(argument.c_str()));
        }
        args.push_back(nullptr);
        ::execvp(args[0], args.data());
        _exit(127);
    }

    ::close(pipe_fd[1]);
    ProcessResult result;
    std::array<char, 4096> buffer {};
    ssize_t read_count = 0;
    while ((read_count = ::read(pipe_fd[0], buffer.data(), buffer.size())) > 0) {
        result.output.append(buffer.data(), static_cast<std::size_t>(read_count));
    }
    ::close(pipe_fd[0]);

    int status = 0;
    if (::waitpid(pid, &status, 0) < 0) {
        throw std::runtime_error("Failed to wait for process");
    }
    result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return result;
}

std::string trimOutput(std::string text)
{
    const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    const auto first = std::find_if_not(text.begin(), text.end(), is_space);
    if (first == text.end()) {
        return "";
    }
    const auto last = std::find_if_not(text.rbegin(), text.rend(), is_space).base();
    return std::string(first, last);
}
