#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <mutex>
#include <stdexcept>
#ifdef __APPLE__
#include <fcntl.h>
#else
#include <sys/eventfd.h>
#endif
#include <unistd.h>
#include <utility>
#include <vector>

class FileDescriptor {
  public:
    FileDescriptor() = default;
    explicit FileDescriptor(int descriptor) : descriptor(descriptor) {}
    FileDescriptor(FileDescriptor&& other) noexcept : descriptor(other.release()) {}
    FileDescriptor& operator=(FileDescriptor&& other) noexcept
    {
        reset(other.release());
        return *this;
    }
    ~FileDescriptor() { reset(); }
    [[nodiscard]] int get() const { return descriptor; }
    explicit operator bool() const { return descriptor >= 0; }
    [[nodiscard]] int release() { return std::exchange(descriptor, -1); }
    void reset(int next = -1)
    {
        if (descriptor >= 0)
            ::close(descriptor);
        descriptor = next;
    }

  private:
    int descriptor = -1;
};

class WakeSignal {
  public:
    WakeSignal()
    {
#ifdef __APPLE__
        int ends[2];
        if (::pipe(ends) != 0)
            throw std::runtime_error("Could not create a wakeup pipe");
        event.reset(ends[0]);
        writer.reset(ends[1]);
        for (const int descriptor : ends) {
            if (::fcntl(descriptor, F_SETFL, O_NONBLOCK) < 0 ||
                ::fcntl(descriptor, F_SETFD, FD_CLOEXEC) < 0)
                throw std::runtime_error("Could not configure a wakeup pipe");
        }
#else
        event.reset(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC));
        if (!event)
            throw std::runtime_error("Could not create a wakeup descriptor");
#endif
    }
    [[nodiscard]] int fd() const { return event.get(); }
    void raise() const
    {
#ifdef __APPLE__
        const std::uint8_t value = 1;
        (void)!::write(writer.get(), &value, sizeof(value));
#else
        const std::uint64_t value = 1;
        (void)!::write(event.get(), &value, sizeof(value));
#endif
    }
    void clear() const
    {
#ifdef __APPLE__
        std::uint8_t values[256];
        while (::read(event.get(), values, sizeof(values)) > 0) {
        }
#else
        std::uint64_t value;
        while (::read(event.get(), &value, sizeof(value)) > 0) {
        }
#endif
    }

  private:
    FileDescriptor event;
#ifdef __APPLE__
    FileDescriptor writer;
#endif
};

template <class Item> class Mailbox {
  public:
    [[nodiscard]] int fd() const { return signal.fd(); }
    void post(Item item)
    {
        {
            const std::scoped_lock lock(mutex);
            items.push_back(std::move(item));
        }
        signal.raise();
    }
    void post(std::vector<Item> batch)
    {
        if (batch.empty())
            return;
        {
            const std::scoped_lock lock(mutex);
            std::ranges::move(batch, std::back_inserter(items));
        }
        signal.raise();
    }
    void wake() const { signal.raise(); }
    [[nodiscard]] std::vector<Item> take()
    {
        signal.clear();
        const std::scoped_lock lock(mutex);
        return std::exchange(items, {});
    }

  private:
    WakeSignal signal;
    std::mutex mutex;
    std::vector<Item> items;
};
