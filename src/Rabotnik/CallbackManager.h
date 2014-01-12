#pragma once

namespace Rabotnik
{
  /**
   * @brief Interface for callback managers.
   */
  struct CallbackManager {
    inline virtual ~CallbackManager() { }
    virtual void startCallback() = 0;
    virtual void stopCallback() = 0;
  };
}

