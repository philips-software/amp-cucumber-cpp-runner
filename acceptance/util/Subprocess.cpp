#include "acceptance/util/Subprocess.hpp"
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <thread>
#include <windows.h>
#else
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace acceptance::util
{
    namespace
    {
        [[noreturn]] void ThrowRuntimeError(const std::string& message)
        {
            throw std::runtime_error{ message };
        }

#if defined(_WIN32)
        std::string GetLastErrorMessage()
        {
            const DWORD error = GetLastError();
            LPSTR buffer = nullptr;
            const auto size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&buffer), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                0,
                nullptr);

            if (size == 0 || buffer == nullptr)
                return "Win32 error code " + std::to_string(error);

            std::string message{ buffer, static_cast<std::size_t>(size) };
            LocalFree(buffer);
            return message;
        }

        std::wstring Utf8ToWide(const std::string& text)
        {
            if (text.empty())
                return {};

            const auto required = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
            if (required <= 0)
                ThrowRuntimeError("Failed to convert UTF-8 to UTF-16: " + GetLastErrorMessage());

            std::wstring out(static_cast<std::size_t>(required - 1), L'\0');
            const auto converted = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), required);
            if (converted <= 0)
                ThrowRuntimeError("Failed to convert UTF-8 to UTF-16: " + GetLastErrorMessage());

            return out;
        }

        std::wstring QuoteForCreateProcess(const std::wstring& argument)
        {
            if (argument.empty())
                return L"\"\"";

            if (argument.find_first_of(L" \t\"") == std::wstring::npos)
                return argument;

            std::wstring quoted;
            quoted.push_back(L'"');

            std::size_t slashCount = 0;
            for (const wchar_t ch : argument)
            {
                if (ch == L'\\')
                {
                    ++slashCount;
                    continue;
                }

                if (ch == L'"')
                {
                    quoted.append((slashCount * 2) + 1, L'\\');
                    quoted.push_back(L'"');
                    slashCount = 0;
                    continue;
                }

                if (slashCount > 0)
                {
                    quoted.append(slashCount, L'\\');
                    slashCount = 0;
                }

                quoted.push_back(ch);
            }

            if (slashCount > 0)
                quoted.append(slashCount * 2, L'\\');

            quoted.push_back(L'"');
            return quoted;
        }

        std::wstring BuildCommandLine(const std::filesystem::path& executable, const std::vector<std::string>& arguments)
        {
            std::wstring commandLine = QuoteForCreateProcess(executable.wstring());
            for (const auto& argument : arguments)
            {
                commandLine.push_back(L' ');
                commandLine += QuoteForCreateProcess(Utf8ToWide(argument));
            }
            return commandLine;
        }

        void ReadPipe(HANDLE handle, std::string& output)
        {
            std::array<char, 4096> buffer{};
            DWORD bytesRead = 0;
            while (ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead > 0)
                output.append(buffer.data(), bytesRead);
        }
#else
        struct Pipe
        {
            int readFd{ -1 };
            int writeFd{ -1 };

            ~Pipe()
            {
                if (readFd >= 0)
                    close(readFd);
                if (writeFd >= 0)
                    close(writeFd);
            }
        };

        Pipe CreatePipeOrThrow()
        {
            int fds[2] = { -1, -1 };
            if (pipe(fds) != 0)
                ThrowRuntimeError("Failed to create pipe: " + std::system_category().message(errno));

            return Pipe{ .readFd = fds[0], .writeFd = fds[1] };
        }

        void SetNonBlockingOrThrow(int fd)
        {
            const int flags = fcntl(fd, F_GETFL, 0);
            if (flags < 0)
                ThrowRuntimeError("Failed to get file status flags: " + std::system_category().message(errno));

            if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0)
                ThrowRuntimeError("Failed to set non-blocking mode: " + std::system_category().message(errno));
        }

        void CloseFd(int& fd)
        {
            if (fd >= 0)
            {
                close(fd);
                fd = -1;
            }
        }

        bool DrainFd(int fd, std::string& output)
        {
            std::array<char, 4096> buffer{};

            while (true)
            {
                const auto readSize = read(fd, buffer.data(), buffer.size());
                if (readSize > 0)
                {
                    output.append(buffer.data(), static_cast<std::size_t>(readSize));
                    continue;
                }

                if (readSize == 0)
                    return true;

                if (errno == EINTR)
                    continue;

                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return false;

                ThrowRuntimeError("Failed to read process output: " + std::system_category().message(errno));
            }
        }
#endif
    }

    ProcessResult RunProcess(
        const std::filesystem::path& executable,
        const std::vector<std::string>& arguments,
        const std::filesystem::path& workingDirectory)
    {
        if ((executable.is_absolute() || executable.has_parent_path()) && !std::filesystem::is_regular_file(executable))
            ThrowRuntimeError("Executable does not exist: " + executable.string());

#if defined(_WIN32)
        SECURITY_ATTRIBUTES securityAttributes{};
        securityAttributes.nLength = sizeof(securityAttributes);
        securityAttributes.lpSecurityDescriptor = nullptr;
        securityAttributes.bInheritHandle = TRUE;

        HANDLE standardOutputRead = nullptr;
        HANDLE standardOutputWrite = nullptr;
        HANDLE standardErrorRead = nullptr;
        HANDLE standardErrorWrite = nullptr;

        if (!CreatePipe(&standardOutputRead, &standardOutputWrite, &securityAttributes, 0))
            ThrowRuntimeError("Failed to create stdout pipe: " + GetLastErrorMessage());

        if (!CreatePipe(&standardErrorRead, &standardErrorWrite, &securityAttributes, 0))
        {
            CloseHandle(standardOutputRead);
            CloseHandle(standardOutputWrite);
            ThrowRuntimeError("Failed to create stderr pipe: " + GetLastErrorMessage());
        }

        if (!SetHandleInformation(standardOutputRead, HANDLE_FLAG_INHERIT, 0) || !SetHandleInformation(standardErrorRead, HANDLE_FLAG_INHERIT, 0))
        {
            CloseHandle(standardOutputRead);
            CloseHandle(standardOutputWrite);
            CloseHandle(standardErrorRead);
            CloseHandle(standardErrorWrite);
            ThrowRuntimeError("Failed to configure pipe inheritance: " + GetLastErrorMessage());
        }

        STARTUPINFOW startupInfo{};
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags = STARTF_USESTDHANDLES;
        startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startupInfo.hStdOutput = standardOutputWrite;
        startupInfo.hStdError = standardErrorWrite;

        PROCESS_INFORMATION processInformation{};

        auto commandLine = BuildCommandLine(executable, arguments);
        std::vector<wchar_t> commandLineBuffer(commandLine.begin(), commandLine.end());
        commandLineBuffer.push_back(L'\0');

        const auto executableWide = executable.wstring();
        const auto workingDirectoryWide = workingDirectory.empty() ? std::wstring{} : workingDirectory.wstring();

        const BOOL created = CreateProcessW(
            executableWide.c_str(),
            commandLineBuffer.data(),
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            workingDirectoryWide.empty() ? nullptr : workingDirectoryWide.c_str(),
            &startupInfo,
            &processInformation);

        CloseHandle(standardOutputWrite);
        CloseHandle(standardErrorWrite);

        if (!created)
        {
            CloseHandle(standardOutputRead);
            CloseHandle(standardErrorRead);
            ThrowRuntimeError("Failed to launch process: " + GetLastErrorMessage());
        }

        ProcessResult result{};
        std::thread outputReader{ ReadPipe, standardOutputRead, std::ref(result.standardOutput) };
        std::thread errorReader{ ReadPipe, standardErrorRead, std::ref(result.standardError) };

        WaitForSingleObject(processInformation.hProcess, INFINITE);

        DWORD exitCode = 0;
        if (!GetExitCodeProcess(processInformation.hProcess, &exitCode))
        {
            CloseHandle(standardOutputRead);
            CloseHandle(standardErrorRead);
            CloseHandle(processInformation.hThread);
            CloseHandle(processInformation.hProcess);
            outputReader.join();
            errorReader.join();
            ThrowRuntimeError("Failed to read process exit code: " + GetLastErrorMessage());
        }

        result.exitCode = static_cast<int>(exitCode);

        CloseHandle(standardOutputRead);
        CloseHandle(standardErrorRead);
        CloseHandle(processInformation.hThread);
        CloseHandle(processInformation.hProcess);

        outputReader.join();
        errorReader.join();

        return result;
#else
        Pipe standardOutput = CreatePipeOrThrow();
        Pipe standardError = CreatePipeOrThrow();

        SetNonBlockingOrThrow(standardOutput.readFd);
        SetNonBlockingOrThrow(standardError.readFd);

        const pid_t pid = fork();
        if (pid < 0)
            ThrowRuntimeError("Failed to fork process: " + std::system_category().message(errno));

        if (pid == 0)
        {
            if (!workingDirectory.empty() && chdir(workingDirectory.c_str()) != 0)
                _exit(127);

            if (dup2(standardOutput.writeFd, STDOUT_FILENO) < 0)
                _exit(127);
            if (dup2(standardError.writeFd, STDERR_FILENO) < 0)
                _exit(127);

            close(standardOutput.readFd);
            close(standardOutput.writeFd);
            close(standardError.readFd);
            close(standardError.writeFd);

            std::vector<std::string> executableAndArguments;
            executableAndArguments.reserve(arguments.size() + 1);
            executableAndArguments.emplace_back(executable.string());
            executableAndArguments.insert(executableAndArguments.end(), arguments.begin(), arguments.end());

            std::vector<char*> argv;
            argv.reserve(executableAndArguments.size() + 1);
            for (auto& arg : executableAndArguments)
                argv.push_back(const_cast<char*>(arg.c_str()));
            argv.push_back(nullptr);

            execvp(argv[0], argv.data());
            _exit(127);
        }

        CloseFd(standardOutput.writeFd);
        CloseFd(standardError.writeFd);

        ProcessResult result{};

        while (standardOutput.readFd >= 0 || standardError.readFd >= 0)
        {
            std::array<pollfd, 2> pollDescriptors{};
            pollDescriptors[0].fd = standardOutput.readFd;
            pollDescriptors[0].events = POLLIN;
            pollDescriptors[1].fd = standardError.readFd;
            pollDescriptors[1].events = POLLIN;

            const auto pollResult = poll(pollDescriptors.data(), pollDescriptors.size(), -1);
            if (pollResult < 0)
            {
                if (errno == EINTR)
                    continue;

                ThrowRuntimeError("Failed while waiting for process output: " + std::system_category().message(errno));
            }

            if (standardOutput.readFd >= 0 && (pollDescriptors[0].revents & (POLLIN | POLLHUP | POLLERR)) != 0)
            {
                if (DrainFd(standardOutput.readFd, result.standardOutput))
                    CloseFd(standardOutput.readFd);
            }

            if (standardError.readFd >= 0 && (pollDescriptors[1].revents & (POLLIN | POLLHUP | POLLERR)) != 0)
            {
                if (DrainFd(standardError.readFd, result.standardError))
                    CloseFd(standardError.readFd);
            }
        }

        int waitStatus = 0;
        while (waitpid(pid, &waitStatus, 0) < 0)
        {
            if (errno == EINTR)
                continue;

            ThrowRuntimeError("Failed to wait for process: " + std::system_category().message(errno));
        }

        if (WIFEXITED(waitStatus))
            result.exitCode = WEXITSTATUS(waitStatus);
        else if (WIFSIGNALED(waitStatus))
            result.exitCode = 128 + WTERMSIG(waitStatus);
        else
            result.exitCode = -1;

        return result;
#endif
    }
}
