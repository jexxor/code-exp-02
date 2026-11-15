#include "directory_picker.h"

#include "handles.h"
#include "wakeup.h"

#include <atomic>
#include <format>
#include <gio/gio.h>
#include <mutex>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

struct DirectoryPicker::Impl {
    WakeSignal events;
    GObjectHandle<GSubprocess> process;
    std::atomic<bool> finished{false};
    std::mutex mutex;
    std::optional<DirectorySelection> result;
    std::jthread worker;

    Impl(const std::string& directory, unsigned long parent)
    {
        GError* failure = nullptr;
#ifdef __APPLE__
        (void)parent;
        constexpr auto script =
            "on run argv\n"
            "set selected to choose folder with prompt \"Open project directory\" "
            "default location POSIX file (item 1 of argv)\n"
            "return POSIX path of selected\n"
            "end run";
        process.reset(g_subprocess_new(
            static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                          G_SUBPROCESS_FLAGS_STDERR_PIPE),
            &failure, "osascript", "-e", script, directory.c_str(), nullptr));
#else
        const auto filename = std::format("--filename={}/", directory);
        const auto attach = std::format("--attach={}", parent);
        process.reset(g_subprocess_new(
            static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                          G_SUBPROCESS_FLAGS_STDERR_SILENCE),
            &failure, "zenity", "--file-selection", "--directory", "--modal",
            "--title=Open project directory", filename.c_str(), attach.c_str(), nullptr));
#endif
        if (const GErrorHandle error{failure}; !process) {
#ifdef __APPLE__
            throw std::runtime_error(std::format("Could not start the directory picker. {}",
                                                 error->message));
#else
            throw std::runtime_error(std::format(
                "Could not start the directory picker. Install zenity. {}", error->message));
#endif
        }
        worker = std::jthread([this] { run(); });
    }

    DirectorySelection wait() const
    {
        gchar* text = nullptr;
        gchar* diagnostics = nullptr;
        GError* failure = nullptr;
        const bool communicated = g_subprocess_communicate_utf8(process.get(), nullptr, nullptr,
                                                                &text, &diagnostics, &failure);
        const GStringHandle output{text};
        const GStringHandle diagnostic{diagnostics};
        const GErrorHandle error{failure};
        if (!communicated)
            return {.directory = {}, .error = error->message};
        if (g_subprocess_get_successful(process.get())) {
            std::string directory = output ? output.get() : "";
            // Zenity appends one newline; whitespace within the path is significant.
            if (directory.ends_with('\n'))
                directory.pop_back();
            return {.directory = std::move(directory), .error = {}};
        }
#ifdef __APPLE__
        if (diagnostic && std::string_view(diagnostic.get()).contains("(-128)"))
            return {};
        return {.directory = {},
                .error = diagnostic ? diagnostic.get() : "The directory picker could not open."};
#else
        // Leaving with status 1 is the user cancelling.
        if (!g_subprocess_get_if_exited(process.get()) ||
            g_subprocess_get_exit_status(process.get()) != 1)
            return {.directory = {}, .error = "The directory picker could not open."};
        return {};
#endif
    }

    void run()
    {
        auto selection = wait();
        {
            const std::scoped_lock lock(mutex);
            result = std::move(selection);
        }
        finished = true;
        events.raise();
    }

    ~Impl()
    {
        if (!finished)
            g_subprocess_force_exit(process.get());
    }
};

DirectoryPicker::DirectoryPicker(const std::string& directory, unsigned long parent)
    : impl(std::make_unique<Impl>(directory, parent))
{
}
DirectoryPicker::~DirectoryPicker() = default;
int DirectoryPicker::event_fd() const
{
    return impl->events.fd();
}
std::optional<DirectorySelection> DirectoryPicker::take_result()
{
    impl->events.clear();
    const std::scoped_lock lock(impl->mutex);
    return std::exchange(impl->result, std::nullopt);
}
