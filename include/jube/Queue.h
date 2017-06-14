/*
 * Copyright (c) 2017 Julien Bernard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef JUBE_QUEUE_H
#define JUBE_QUEUE_H

#include <cassert>

#include <algorithm>
#include <memory>

namespace jube {

  class Queue {
  public:
    static constexpr std::size_t InitialCapacity = sizeof(double) * 16;

    Queue()
    : m_capacity(InitialCapacity)
    , m_size(0)
    , m_head(0)
    , m_tail(0)
    {
      m_data = m_allocator.allocate(m_capacity);
    }

    ~Queue() {
      m_allocator.deallocate(m_data, m_capacity);
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    Queue(Queue&&) = default;
    Queue& operator=(Queue&&) = default;

    bool isEmpty() const {
      return m_size == 0;
    }

    std::size_t size() const {
      return m_size;
    }

    void push(const void *raw, std::size_t size) {
      while (m_size + size >= m_capacity) {
        grow();
      }

      assert(m_size + size < m_capacity);

      if (m_tail + size <= m_capacity) {
        std::copy_n(static_cast<const uint8_t*>(raw), size, m_data + m_tail);
        m_tail += size;

        if (m_tail == m_capacity) {
          m_tail = 0;
        }
      } else {
        std::size_t first = m_capacity - m_tail;
        std::size_t second = size - first;

        std::copy_n(static_cast<const uint8_t*>(raw), first, m_data + m_tail);
        std::copy_n(static_cast<const uint8_t*>(raw) + first, second, m_data);

        m_tail = second;
        assert(m_tail < m_capacity);
        assert(m_tail < m_head);
      }

      m_size += size;
      assert(invariant());
    }

    void pop(void *raw, std::size_t size) {
      assert(size <= m_size);

      if (m_head + size <= m_capacity) {
        std::copy_n(m_data + m_head, size, static_cast<uint8_t*>(raw));
        m_head += size;

        if (m_head == m_capacity) {
          m_head = 0;
        }
      } else {
        std::size_t first = m_capacity - m_head;
        std::size_t second = size - first;

        std::copy_n(m_data + m_head, first, static_cast<uint8_t*>(raw));
        std::copy_n(m_data, second, static_cast<uint8_t*>(raw) + first);

        m_head = second;
        assert(m_head < m_capacity);
        assert(m_head <= m_tail);
      }

      m_size -= size;
      assert(invariant());
    }

  private:
    void grow() {
      std::size_t capacity = m_capacity * 2;
      auto data = m_allocator.allocate(capacity);

      if (m_size > 0) {
        if (m_head < m_tail) {
          std::copy_n(m_data + m_head, m_size, data);
        } else {
          std::size_t first = m_capacity - m_head;
          std::size_t second = m_size - first;
          std::copy_n(m_data + m_head, first, data);
          std::copy_n(m_data, second, data + first);
        }
      }

      m_head = 0;
      m_tail = m_size;

      m_allocator.deallocate(m_data, m_capacity);
      m_data = data;
      m_capacity = capacity;
      assert(invariant());
    }

    bool invariant() const {
      return m_size == computedSize();
    }

    std::size_t computedSize() const {
      if (m_head <= m_tail) {
        return m_tail - m_head;
      }

      return m_capacity - m_head + m_tail;
    }

  private:
    std::allocator<uint8_t> m_allocator;
    std::size_t m_capacity;
    std::size_t m_size;
    std::size_t m_head;
    std::size_t m_tail;
    uint8_t *m_data;
  };

}

#endif // JUBE_QUEUE_H
