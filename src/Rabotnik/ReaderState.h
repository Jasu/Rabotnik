#pragma once

namespace Rabotnik
{
  /**
   * @brief Describes the state of the thread.
   */
  enum ReaderState {
    /**
     * @brief Thread not running, no start pending.
     */
    READER_STATE_STOPPED,
    /**
     * @brief start() called, handler's initialization not finished.
     */
    READER_STATE_STARTING,
    /**
     * @brief Handler initialized, thread running.
     */
    READER_STATE_RUNNING,
    /**
     * @brief Thread is stopping, possibly waiting for callback or uninitialize
     * to finish.
     */
    READER_STATE_STOPPING,
  };
}

