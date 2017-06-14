#include <jube/Queue.h>

#include <cinttypes>
#include <csignal>
#include <cstdint>
#include <cstdio>

#include <array>
#include <atomic>
#include <random>

static std::mt19937 getCorrectlyInitializedEngine() {
  std::mt19937::result_type data[std::mt19937::state_size];
  std::random_device source;
  std::generate(std::begin(data), std::end(data), std::ref(source));
  std::seed_seq seeds(std::begin(data), std::end(data));
  return std::mt19937(seeds);
}

static std::atomic_bool finished(false);

static void finish(int /* sig */) {
  finished = true;
}

int main() {
  std::signal(SIGINT, finish);

  auto engine = getCorrectlyInitializedEngine();

  uint64_t currentWrite = 0;
  uint64_t currentRead = 0;
  uint64_t currentSize = 0;

  static constexpr std::size_t BufferSizeMax = 2048;
  std::array<uint64_t, BufferSizeMax> buffer;

  static constexpr std::size_t ChunkCountMax = 20;
  std::uniform_int_distribution<std::size_t> distChunk(1, ChunkCountMax);

  std::uniform_int_distribution<std::size_t> distSize(1, BufferSizeMax - 1);

  static constexpr std::size_t SizeThreshold = 1024;

  jube::Queue queue;

  while (!finished) {
    std::size_t chunkCount = distChunk(engine);

    for (std::size_t i = 0; i < chunkCount; ++i) {
      std::size_t size = distSize(engine);

      for (std::size_t j = 0; j < size; ++j) {
        buffer[j] = currentWrite++;
      }

      queue.push(buffer.data(), size * sizeof(uint64_t));
//       std::printf("Write %zu (current size: %zu)\n", size, queue.size() / sizeof(uint64_t));
      currentSize += size * sizeof(uint64_t);
    }

    assert(queue.size() == currentSize);

    while (queue.size() > SizeThreshold) {
      std::size_t size = distSize(engine);

      if (size * sizeof(uint64_t) > queue.size()) {
        size = queue.size() / sizeof(uint64_t);
      }

      queue.pop(buffer.data(), size * sizeof(uint64_t));
//       std::printf("Read %zu (current size: %zu)\n", size, queue.size() / sizeof(uint64_t));

      for (std::size_t j = 0; j < size; ++j) {
        if (buffer[j] != currentRead) {
          std::printf("Mismatch! Expected: '%" PRIu64 "' Actual: '%" PRIu64 "'\n", currentRead, buffer[j]);
        }

        currentRead++;
      }

      currentSize -= size * sizeof(uint64_t);
    }

    assert(queue.size() == currentSize);

    std::printf("Current: %" PRIu64 "/%" PRIu64 "\r", currentWrite, currentRead);
    std::fflush(stdout);
  }

  std::printf("\nClean finish!\n");
  return 0;
}
